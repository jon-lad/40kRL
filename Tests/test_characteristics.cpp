#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: rogue-trader-stats — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 1: Clamping preserves range invariant ──────────────────────────
// **Validates: Requirements 1.2, 1.3**
//
// For any integer in [-1000, 1000], set() then get() returns max(1, min(99, input)).

TEST_CASE("PBT: Property 1 — clamping preserves range invariant", "[pbt][property][Feature: rogue-trader-stats]")
{
    rc::prop("set() then get() returns max(1, min(99, input)) for any integer in [-1000, 1000]", []() {
        const int input = *rc::gen::inRange(-1000, 1000);
        // Pick a random characteristic to test
        const int charIndex = *rc::gen::inRange(0, static_cast<int>(CharId::COUNT) - 1);
        const CharId id = static_cast<CharId>(charIndex);

        Characteristics chars;
        chars.set(id, input);

        const int expected = std::max(Characteristics::MIN_VALUE,
                                      std::min(Characteristics::MAX_VALUE, input));
        RC_ASSERT(chars.get(id) == expected);

        // Additional invariant: value is always in [1, 99]
        RC_ASSERT(chars.get(id) >= Characteristics::MIN_VALUE);
        RC_ASSERT(chars.get(id) <= Characteristics::MAX_VALUE);
    });
}

// ─── Property 2: Bonus equals integer division by 10 ─────────────────────────
// **Validates: Requirements 1.4**
//
// For any value in [1, 99], bonus() returns value / 10.

TEST_CASE("PBT: Property 2 — bonus equals integer division by 10", "[pbt][property][Feature: rogue-trader-stats]")
{
    rc::prop("bonus() returns value / 10 for any value in [1, 99]", []() {
        const int value = *rc::gen::inRange(Characteristics::MIN_VALUE, Characteristics::MAX_VALUE);
        // Pick a random characteristic to test
        const int charIndex = *rc::gen::inRange(0, static_cast<int>(CharId::COUNT) - 1);
        const CharId id = static_cast<CharId>(charIndex);

        Characteristics chars;
        chars.set(id, value);

        const int expectedBonus = value / 10;
        RC_ASSERT(chars.bonus(id) == expectedBonus);
    });
}

// ─── Property 3: Serialization round-trip ────────────────────────────────────
// **Validates: Requirements 4.1, 4.2, 4.3**
//
// Generate 9 random values in [1, 99], save to TCODZip, load into fresh instance,
// verify all 9 values match.

TEST_CASE("PBT: Property 3 — serialization round-trip", "[pbt][property][Feature: rogue-trader-stats]")
{
    rc::prop("save then load preserves all 9 characteristic values", []() {
        // Generate 9 random values in the valid range
        Characteristics original;
        for (int i = 0; i < Characteristics::CHAR_COUNT; ++i) {
            const int value = *rc::gen::inRange(Characteristics::MIN_VALUE, Characteristics::MAX_VALUE);
            original.set(static_cast<CharId>(i), value);
        }

        // Save to a TCODZip archive
        TCODZip zip;
        original.save(zip);

        // Save the archive to a temporary buffer, then load it back
        zip.saveToFile("_test_characteristics_roundtrip.sav");

        TCODZip zip2;
        zip2.loadFromFile("_test_characteristics_roundtrip.sav");

        // Load into a fresh instance
        Characteristics loaded;
        loaded.load(zip2);

        // Verify all 9 values match
        for (int i = 0; i < Characteristics::CHAR_COUNT; ++i) {
            const CharId id = static_cast<CharId>(i);
            RC_ASSERT(loaded.get(id) == original.get(id));
        }
    });
}
