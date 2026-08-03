#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"
#include "InjuryDebuffs.h"

#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Unit Tests for Characteristics Modifier Overlay
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 1.2, 4.2**

// ─── addModifier / removeModifier adjusts effective value from get() ─────────

TEST_CASE("Modifier: addModifier adjusts effective value from get()", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(40);

    chars.addModifier(CharId::WS, -10);

    REQUIRE(chars.get(CharId::WS) == 30);
}

TEST_CASE("Modifier: removeModifier reverses addModifier effect", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(40);

    chars.addModifier(CharId::BS, -15);
    REQUIRE(chars.get(CharId::BS) == 25);

    chars.removeModifier(CharId::BS, -15);
    REQUIRE(chars.get(CharId::BS) == 40);
}

// ─── getBase() returns raw value unaffected by modifiers ─────────────────────

TEST_CASE("Modifier: getBase() returns raw value unaffected by modifiers", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(50);

    chars.addModifier(CharId::Ag, -20);

    // get() should reflect the modifier
    REQUIRE(chars.get(CharId::Ag) == 30);
    // getBase() should return the original base value
    REQUIRE(chars.getBase(CharId::Ag) == 50);
}

TEST_CASE("Modifier: getBase() unchanged after multiple modifiers", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(45);

    chars.addModifier(CharId::T, -10);
    chars.addModifier(CharId::T, -5);

    REQUIRE(chars.getBase(CharId::T) == 45);
    REQUIRE(chars.get(CharId::T) == 30);
}

// ─── get() clamps result to [1, 99] even with large negative modifiers ──────

TEST_CASE("Modifier: get() clamps to minimum 1 with large negative modifier", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(20);

    // Apply a modifier that would bring the value below 1
    chars.addModifier(CharId::Per, -50);

    REQUIRE(chars.get(CharId::Per) == Characteristics::MIN_VALUE);
    // Base should still be 20
    REQUIRE(chars.getBase(CharId::Per) == 20);
}

TEST_CASE("Modifier: get() clamps to maximum 99 with large positive modifier", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(90);

    // Apply a positive modifier that would exceed 99
    chars.addModifier(CharId::WP, 20);

    REQUIRE(chars.get(CharId::WP) == Characteristics::MAX_VALUE);
    REQUIRE(chars.getBase(CharId::WP) == 90);
}

// ─── Multiple modifiers on same stat are additive ────────────────────────────

TEST_CASE("Modifier: multiple modifiers on same stat are additive", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(50);

    chars.addModifier(CharId::S, -5);
    chars.addModifier(CharId::S, -10);
    chars.addModifier(CharId::S, -3);

    // Total modifier: -18, effective = 50 - 18 = 32
    REQUIRE(chars.get(CharId::S) == 32);
    REQUIRE(chars.getBase(CharId::S) == 50);
}

TEST_CASE("Modifier: additive modifiers across different stats are independent", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(40);

    chars.addModifier(CharId::WS, -10);
    chars.addModifier(CharId::BS, -5);

    REQUIRE(chars.get(CharId::WS) == 30);
    REQUIRE(chars.get(CharId::BS) == 35);
    // Other stats unaffected
    REQUIRE(chars.get(CharId::S) == 40);
}

// ─── Removing a modifier restores previous effective value ───────────────────

TEST_CASE("Modifier: removing a modifier restores previous effective value", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(60);

    chars.addModifier(CharId::Fel, -20);
    REQUIRE(chars.get(CharId::Fel) == 40);

    chars.removeModifier(CharId::Fel, -20);
    REQUIRE(chars.get(CharId::Fel) == 60);
}

TEST_CASE("Modifier: removing one of multiple modifiers partially restores value", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(50);

    chars.addModifier(CharId::Int, -10);
    chars.addModifier(CharId::Int, -5);
    REQUIRE(chars.get(CharId::Int) == 35);

    // Remove just the -10 modifier
    chars.removeModifier(CharId::Int, -10);
    // Should still have -5 applied
    REQUIRE(chars.get(CharId::Int) == 45);
}

TEST_CASE("Modifier: removing all modifiers fully restores base value", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(55);

    chars.addModifier(CharId::Ag, -15);
    chars.addModifier(CharId::Ag, -10);
    REQUIRE(chars.get(CharId::Ag) == 30);

    chars.removeModifier(CharId::Ag, -15);
    chars.removeModifier(CharId::Ag, -10);
    REQUIRE(chars.get(CharId::Ag) == 55);
    REQUIRE(chars.getBase(CharId::Ag) == 55);
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Property 1: Debuff lookup table matches specification
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 2.1–2.16**

TEST_CASE("PBT: Property 1 — Debuff lookup table matches specification",
          "[pbt][critical-injuries]")
{
    // Feature: critical-injuries, Property 1: Debuff lookup table matches specification
    rc::prop("lookup returns exactly the spec-defined penalties for any valid (loc, mag)",
        []() {
            const int locInt = *rc::gen::inRange(0, 6);
            const int mag = *rc::gen::inRange(1, 5); // [1,4]

            RC_PRE(locInt >= 0 && locInt < 6);
            RC_PRE(mag >= 1 && mag <= 4);

            const auto loc = static_cast<HitLocation>(locInt);
            const auto entry = InjuryDebuffs::lookup(loc, mag);

            // Verify against the constexpr TABLE (the spec-encoded source of truth)
            const auto& tableEntry = InjuryDebuffs::TABLE[locInt][mag - 1];

            RC_ASSERT(entry.count == tableEntry.count);

            for (int i = 0; i < tableEntry.count; ++i) {
                RC_ASSERT(entry.modifiers[i].stat == tableEntry.modifiers[i].stat);
                RC_ASSERT(entry.modifiers[i].penalty == tableEntry.modifiers[i].penalty);
            }

            // Additional invariants from the spec:
            // All penalties must be negative (debuffs)
            for (int i = 0; i < entry.count; ++i) {
                RC_ASSERT(entry.modifiers[i].penalty < 0);
            }
            // Count must be in valid range [1, 3]
            RC_ASSERT(entry.count >= 1);
            RC_ASSERT(entry.count <= InjuryDebuffs::MAX_MODIFIERS_PER_INJURY);
        });
}

// Exhaustive deterministic verification that the TABLE matches the spec requirements 2.1-2.16.
TEST_CASE("Debuff lookup table correctness — all 24 entries match spec", "[critical-injuries]")
{
    using namespace InjuryDebuffs;

    // HEAD mag 1: Per -5 (Req 2.1)
    auto e = lookup(HitLocation::HEAD, 1);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::Per);
    REQUIRE(e.modifiers[0].penalty == -5);

    // HEAD mag 2: Per -10, BS -5 (Req 2.2)
    e = lookup(HitLocation::HEAD, 2);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::Per);
    REQUIRE(e.modifiers[0].penalty == -10);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -5);

    // HEAD mag 3: WS -10, BS -10 (Req 2.3)
    e = lookup(HitLocation::HEAD, 3);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -10);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -10);

    // HEAD mag 4: WS -20, BS -20, Int -10 (Req 2.4)
    e = lookup(HitLocation::HEAD, 4);
    REQUIRE(e.count == 3);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -20);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -20);
    REQUIRE(e.modifiers[2].stat == CharId::Int);
    REQUIRE(e.modifiers[2].penalty == -10);

    // RIGHT_ARM mag 1: WS -5 (Req 2.5)
    e = lookup(HitLocation::RIGHT_ARM, 1);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -5);

    // RIGHT_ARM mag 2: WS -10, BS -5 (Req 2.6)
    e = lookup(HitLocation::RIGHT_ARM, 2);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -10);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -5);

    // RIGHT_ARM mag 3: WS -10, BS -10 (Req 2.7)
    e = lookup(HitLocation::RIGHT_ARM, 3);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -10);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -10);

    // RIGHT_ARM mag 4: WS -20, BS -20 (Req 2.8)
    e = lookup(HitLocation::RIGHT_ARM, 4);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -20);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -20);

    // LEFT_ARM mag 1: WS -5 (Req 2.5)
    e = lookup(HitLocation::LEFT_ARM, 1);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -5);

    // LEFT_ARM mag 2: WS -10, BS -5 (Req 2.6)
    e = lookup(HitLocation::LEFT_ARM, 2);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -10);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -5);

    // LEFT_ARM mag 3: WS -10, BS -10 (Req 2.7)
    e = lookup(HitLocation::LEFT_ARM, 3);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -10);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -10);

    // LEFT_ARM mag 4: WS -20, BS -20 (Req 2.8)
    e = lookup(HitLocation::LEFT_ARM, 4);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -20);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -20);

    // BODY mag 1: T -5 (Req 2.9)
    e = lookup(HitLocation::BODY, 1);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::T);
    REQUIRE(e.modifiers[0].penalty == -5);

    // BODY mag 2: T -10 (Req 2.10)
    e = lookup(HitLocation::BODY, 2);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::T);
    REQUIRE(e.modifiers[0].penalty == -10);

    // BODY mag 3: T -10, S -5 (Req 2.11)
    e = lookup(HitLocation::BODY, 3);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::T);
    REQUIRE(e.modifiers[0].penalty == -10);
    REQUIRE(e.modifiers[1].stat == CharId::S);
    REQUIRE(e.modifiers[1].penalty == -5);

    // BODY mag 4: T -20, S -10 (Req 2.12)
    e = lookup(HitLocation::BODY, 4);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::T);
    REQUIRE(e.modifiers[0].penalty == -20);
    REQUIRE(e.modifiers[1].stat == CharId::S);
    REQUIRE(e.modifiers[1].penalty == -10);

    // RIGHT_LEG mag 1: Ag -5 (Req 2.13)
    e = lookup(HitLocation::RIGHT_LEG, 1);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::Ag);
    REQUIRE(e.modifiers[0].penalty == -5);

    // RIGHT_LEG mag 2: Ag -10 (Req 2.14)
    e = lookup(HitLocation::RIGHT_LEG, 2);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::Ag);
    REQUIRE(e.modifiers[0].penalty == -10);

    // RIGHT_LEG mag 3: Ag -15, WS -5 (Req 2.15)
    e = lookup(HitLocation::RIGHT_LEG, 3);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::Ag);
    REQUIRE(e.modifiers[0].penalty == -15);
    REQUIRE(e.modifiers[1].stat == CharId::WS);
    REQUIRE(e.modifiers[1].penalty == -5);

    // RIGHT_LEG mag 4: Ag -20, WS -10 (Req 2.16)
    e = lookup(HitLocation::RIGHT_LEG, 4);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::Ag);
    REQUIRE(e.modifiers[0].penalty == -20);
    REQUIRE(e.modifiers[1].stat == CharId::WS);
    REQUIRE(e.modifiers[1].penalty == -10);

    // LEFT_LEG mag 1: Ag -5 (Req 2.13)
    e = lookup(HitLocation::LEFT_LEG, 1);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::Ag);
    REQUIRE(e.modifiers[0].penalty == -5);

    // LEFT_LEG mag 2: Ag -10 (Req 2.14)
    e = lookup(HitLocation::LEFT_LEG, 2);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::Ag);
    REQUIRE(e.modifiers[0].penalty == -10);

    // LEFT_LEG mag 3: Ag -15, WS -5 (Req 2.15)
    e = lookup(HitLocation::LEFT_LEG, 3);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::Ag);
    REQUIRE(e.modifiers[0].penalty == -15);
    REQUIRE(e.modifiers[1].stat == CharId::WS);
    REQUIRE(e.modifiers[1].penalty == -5);

    // LEFT_LEG mag 4: Ag -20, WS -10 (Req 2.16)
    e = lookup(HitLocation::LEFT_LEG, 4);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::Ag);
    REQUIRE(e.modifiers[0].penalty == -20);
    REQUIRE(e.modifiers[1].stat == CharId::WS);
    REQUIRE(e.modifiers[1].penalty == -10);
}
