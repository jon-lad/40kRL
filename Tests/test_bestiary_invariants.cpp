#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "BestiaryTestHarness.hpp"

#include <sol/sol.hpp>

#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — roster-wide invariant property tests
//
// Loads Scripts/Enemies.lua / Scripts/Equipment.lua through the sol2-based
// BestiaryTestHarness (no global Engine — test-isolation steering) and asserts the
// design's roster-wide Correctness Properties over every Enemy_Entry the harness
// surfaces via spawnEnemy across the ten faction columns.
//
// TDD-RED: written BEFORE the faction columns are populated (Tasks 2.3, 3–9). Until
// the roster exists these properties may vacuously pass (empty roster) or FAIL once
// entries are added with out-of-range / non-integer characteristics. The file is
// expected to COMPILE.
//
// RapidCheck rc::gen::inRange uses INCLUSIVE bounds [a, b] in this project's custom
// header (see test-isolation.md); all bounds below are inclusive.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// The nine Rogue Trader characteristics stored as integer fields on an Enemy_Entry
// (design's Enemy_Entry schema / Requirement 5.1).
const std::vector<std::string>& characteristicKeys() {
    static const std::vector<std::string> keys = {
        "ws", "bs", "s", "t", "ag", "int", "per", "wp", "fel",
    };
    return keys;
}

} // namespace

// ─── Property 4: Characteristics are integers in range ───────────────────────────
// **Validates: Requirements 2.1, 2.2, 5.1**
//
// For every Enemy_Entry, each of the nine characteristics (ws, bs, s, t, ag, int,
// per, wp, fel) is an integer in the inclusive range [0, 100]. The base value is
// copied from the Reference_Profile (Req 2.1); an em-dash characteristic becomes 0
// (Req 2.2); every entry carries the nine characteristic fields (Req 5.1).
TEST_CASE("PBT: Property 4 — characteristics are integers in [0, 100]",
          "[pbt][property][Feature: bestiary-npcs][Property 4]")
{
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua could not be loaded");
    }

    const std::vector<bestiary::EnemyEntry>& roster = harness.enemies();

    // Nothing to check on an empty roster; the property is quantified over entries.
    // (Roster completeness is covered separately by Task 10.5.)
    if (roster.empty()) {
        SKIP("Bestiary roster is empty — no entries to validate yet (TDD-RED)");
    }

    const std::vector<std::string>& chars = characteristicKeys();

    // rc::check runs >= 100 iterations by default.
    rc::check("every entry's nine characteristics are integers in [0, 100]", [&]() {
        // Pick a random entry (inclusive index bounds) and a random characteristic.
        const int entryIdx = *rc::gen::inRange(0, static_cast<int>(roster.size()) - 1);
        const int charIdx = *rc::gen::inRange(0, static_cast<int>(chars.size()) - 1);

        const bestiary::EnemyEntry& entry = roster[static_cast<std::size_t>(entryIdx)];
        const std::string& key = chars[static_cast<std::size_t>(charIdx)];

        // The characteristic field must be present (Req 5.1) and hold a number.
        sol::object value = entry.table[key];
        RC_ASSERT(value.valid());
        RC_ASSERT(value.get_type() == sol::type::number);

        // It must be an INTEGER (no fractional part), not merely a number. Lua 5.4
        // distinguishes integers from floats; a base characteristic is an integer.
        const double raw = value.as<double>();
        const long long asInt = value.as<long long>();
        RC_ASSERT(static_cast<double>(asInt) == raw);

        // Integer in the inclusive range [0, 100] (Req 2.1, 2.2).
        RC_ASSERT(asInt >= 0);
        RC_ASSERT(asInt <= 100);
    });
}
