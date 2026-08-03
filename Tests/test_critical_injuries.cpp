#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"
#include "InjuryDebuffs.h"
#include "InjuryTracker.h"

#include <algorithm>
#include <cstdio>

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


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Property 4: Heal clears all injuries (round-trip)
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 4.1, 4.2**

TEST_CASE("PBT: Property 4 — Heal clears all injuries (round-trip)",
          "[pbt][critical-injuries]")
{
    // Feature: critical-injuries, Property 4: Heal clears all injuries (round-trip)
    rc::prop("clearAll restores all magnitudes to zero and stats to base for any injury state",
        []() {
            // 1. Create an Actor with known base Characteristics
            const int baseValue = *rc::gen::inRange(20, 80); // reasonable base stat range
            Actor owner(0, 0, '@', "TestSubject", TCODColor::white);
            owner.characteristics = std::make_shared<Characteristics>(baseValue);

            // 2. Generate random injuries: 0-6 locations, magnitudes 1-4
            //    Each location can have 0 or 1 injury (per design: one injury per location max).
            //    We generate a random magnitude for each of the 6 locations; 0 means no injury.
            std::array<int, 6> injuryMagnitudes;
            for (int i = 0; i < 6; ++i) {
                injuryMagnitudes[i] = *rc::gen::inRange(0, 5); // 0 = none, 1-4 = active
            }

            // 3. Create InjuryTracker and apply all generated injuries
            InjuryTracker tracker;
            for (int i = 0; i < 6; ++i) {
                if (injuryMagnitudes[i] > 0) {
                    const auto loc = static_cast<HitLocation>(i);
                    tracker.applyInjury(&owner, loc, injuryMagnitudes[i]);
                }
            }

            // 4. Call clearAll to remove all injuries and reverse debuffs
            tracker.clearAll(&owner);

            // 5. Verify all magnitudes are zero
            for (int i = 0; i < 6; ++i) {
                const auto loc = static_cast<HitLocation>(i);
                RC_ASSERT(tracker.getMagnitude(loc) == 0);
            }

            // 6. Verify each Characteristic's get() value equals its base value
            //    (all modifiers fully reversed)
            for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
                const auto charId = static_cast<CharId>(c);
                RC_ASSERT(owner.characteristics->get(charId) == baseValue);
                RC_ASSERT(owner.characteristics->getModifier(charId) == 0);
            }
        });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Property 3: Escalation increments magnitude
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 3.1, 3.4**

TEST_CASE("PBT: Property 3 — Escalation increments magnitude",
          "[pbt][critical-injuries]")
{
    rc::prop("applying injury to already-injured location escalates magnitude by 1 with no duplicates",
        []() {
            // Generate a random HitLocation (0-5)
            const int locInt = *rc::gen::inRange(0, 5);
            const auto loc = static_cast<HitLocation>(locInt);

            // Generate an initial magnitude in [1,3] (must leave room to escalate)
            const int initialMag = *rc::gen::inRange(1, 3);

            // Create an Actor with Characteristics
            Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
            actor.characteristics = std::make_shared<Characteristics>(40);

            // Create an InjuryTracker and apply the initial injury
            InjuryTracker tracker;
            bool applied = tracker.applyInjury(&actor, loc, initialMag);
            RC_ASSERT(applied);
            RC_ASSERT(tracker.getMagnitude(loc) == initialMag);

            // Apply another injury at the SAME location (should escalate)
            bool escalated = tracker.applyInjury(&actor, loc, initialMag);
            RC_ASSERT(escalated);

            // Verify magnitude incremented by exactly 1
            RC_ASSERT(tracker.getMagnitude(loc) == initialMag + 1);

            // Verify no duplicate injury records — only one active injury location
            RC_ASSERT(tracker.activeCount() == 1);
        });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Unit Tests for Escalation Edge Cases
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 3.2, 1.3, 1.4**

// ─── Helper: create a minimal Actor with Characteristics for injury tests ────

namespace {
    // Creates a test actor with a Characteristics component initialized to a given base.
    std::unique_ptr<Actor> makeTestActor(const std::string& name, int baseStat = 40) {
        auto actor = std::make_unique<Actor>(0, 0, '@', name, TCODColor::white);
        actor->characteristics = std::make_shared<Characteristics>(baseStat);
        return actor;
    }
}

// ─── Escalation from magnitude 4 returns false (triggers fatal) ──────────────
// Validates: Requirement 3.2

TEST_CASE("InjuryTracker: escalation from magnitude 4 returns false",
          "[critical-injuries][escalation]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    // Apply initial injuries to reach magnitude 4 via escalation
    // First injury at mag 1
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::HEAD, 1) == true);
    REQUIRE(tracker.getMagnitude(HitLocation::HEAD) == 1);

    // Escalate to 2
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::HEAD, 1) == true);
    REQUIRE(tracker.getMagnitude(HitLocation::HEAD) == 2);

    // Escalate to 3
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::HEAD, 1) == true);
    REQUIRE(tracker.getMagnitude(HitLocation::HEAD) == 3);

    // Escalate to 4
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::HEAD, 1) == true);
    REQUIRE(tracker.getMagnitude(HitLocation::HEAD) == 4);

    // Attempting to escalate from 4 should return false (fatal trigger)
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::HEAD, 1) == false);
    // Magnitude should remain at 4 (not 5)
    REQUIRE(tracker.getMagnitude(HitLocation::HEAD) == 4);
}

TEST_CASE("InjuryTracker: direct mag 4 then escalation returns false",
          "[critical-injuries][escalation]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    // Apply directly at magnitude 4
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::BODY, 4) == true);
    REQUIRE(tracker.getMagnitude(HitLocation::BODY) == 4);

    // Escalation attempt should return false
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::BODY, 2) == false);
    // Magnitude stays at 4
    REQUIRE(tracker.getMagnitude(HitLocation::BODY) == 4);
}

// ─── Magnitude clamping: values outside [1,4] are clamped ────────────────────
// Validates: Requirement 1.4 (design error-handling section)

TEST_CASE("InjuryTracker: magnitude 0 is clamped to 1",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    // magnitude 0 should be clamped to 1
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::LEFT_ARM, 0) == true);
    REQUIRE(tracker.getMagnitude(HitLocation::LEFT_ARM) == 1);
}

TEST_CASE("InjuryTracker: negative magnitude is clamped to 1",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    // Negative magnitude should be clamped to 1
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::RIGHT_LEG, -5) == true);
    REQUIRE(tracker.getMagnitude(HitLocation::RIGHT_LEG) == 1);
}

TEST_CASE("InjuryTracker: magnitude 5 is clamped to 4",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    // magnitude 5 should be clamped to 4
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::LEFT_LEG, 5) == true);
    REQUIRE(tracker.getMagnitude(HitLocation::LEFT_LEG) == 4);
}

TEST_CASE("InjuryTracker: magnitude 10 is clamped to 4",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    // Large magnitude should be clamped to 4
    REQUIRE(tracker.applyInjury(actor.get(), HitLocation::BODY, 10) == true);
    REQUIRE(tracker.getMagnitude(HitLocation::BODY) == 4);
}

// ─── hasInjuries() and activeCount() reflect state correctly ─────────────────
// Validates: Requirement 1.3

TEST_CASE("InjuryTracker: hasInjuries() is false on fresh tracker",
          "[critical-injuries][injury-tracker]")
{
    InjuryTracker tracker;
    REQUIRE(tracker.hasInjuries() == false);
    REQUIRE(tracker.activeCount() == 0);
}

TEST_CASE("InjuryTracker: hasInjuries() true after single injury",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    tracker.applyInjury(actor.get(), HitLocation::HEAD, 2);

    REQUIRE(tracker.hasInjuries() == true);
    REQUIRE(tracker.activeCount() == 1);
}

TEST_CASE("InjuryTracker: activeCount() reflects multiple locations",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    tracker.applyInjury(actor.get(), HitLocation::HEAD, 1);
    tracker.applyInjury(actor.get(), HitLocation::BODY, 2);
    tracker.applyInjury(actor.get(), HitLocation::LEFT_LEG, 3);

    REQUIRE(tracker.hasInjuries() == true);
    REQUIRE(tracker.activeCount() == 3);
}

TEST_CASE("InjuryTracker: escalation does not increase activeCount",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    tracker.applyInjury(actor.get(), HitLocation::RIGHT_ARM, 1);
    REQUIRE(tracker.activeCount() == 1);

    // Escalate same location — should stay at 1 active
    tracker.applyInjury(actor.get(), HitLocation::RIGHT_ARM, 2);
    REQUIRE(tracker.activeCount() == 1);
    REQUIRE(tracker.getMagnitude(HitLocation::RIGHT_ARM) == 2);
}

TEST_CASE("InjuryTracker: clearAll resets hasInjuries and activeCount",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("TestVictim", 40);
    InjuryTracker tracker;

    tracker.applyInjury(actor.get(), HitLocation::HEAD, 1);
    tracker.applyInjury(actor.get(), HitLocation::BODY, 3);
    tracker.applyInjury(actor.get(), HitLocation::LEFT_LEG, 2);
    REQUIRE(tracker.activeCount() == 3);

    tracker.clearAll(actor.get());

    REQUIRE(tracker.hasInjuries() == false);
    REQUIRE(tracker.activeCount() == 0);
}

// ─── Player and enemy actors receive identical debuffs for same injury ───────
// Validates: Requirement 1.3

TEST_CASE("InjuryTracker: player and enemy get identical debuffs for same injury",
          "[critical-injuries][injury-tracker]")
{
    // Create two actors with identical base stats
    auto player = makeTestActor("Player", 40);
    auto enemy  = makeTestActor("Ork Boy", 40);

    InjuryTracker playerTracker;
    InjuryTracker enemyTracker;

    // Apply the same injury to both
    playerTracker.applyInjury(player.get(), HitLocation::HEAD, 3);
    enemyTracker.applyInjury(enemy.get(), HitLocation::HEAD, 3);

    // HEAD mag 3: WS -10, BS -10
    // Both should have identical effective stats
    REQUIRE(player->characteristics->get(CharId::WS) == enemy->characteristics->get(CharId::WS));
    REQUIRE(player->characteristics->get(CharId::BS) == enemy->characteristics->get(CharId::BS));

    // Verify the actual penalty values (base 40 - 10 = 30)
    REQUIRE(player->characteristics->get(CharId::WS) == 30);
    REQUIRE(player->characteristics->get(CharId::BS) == 30);
    REQUIRE(enemy->characteristics->get(CharId::WS) == 30);
    REQUIRE(enemy->characteristics->get(CharId::BS) == 30);
}

TEST_CASE("InjuryTracker: player and enemy get identical debuffs for body injury",
          "[critical-injuries][injury-tracker]")
{
    auto player = makeTestActor("Player", 50);
    auto enemy  = makeTestActor("Chaos Marine", 50);

    InjuryTracker playerTracker;
    InjuryTracker enemyTracker;

    // Apply BODY magnitude 4: T -20, S -10
    playerTracker.applyInjury(player.get(), HitLocation::BODY, 4);
    enemyTracker.applyInjury(enemy.get(), HitLocation::BODY, 4);

    // T should be 50 - 20 = 30
    REQUIRE(player->characteristics->get(CharId::T) == 30);
    REQUIRE(enemy->characteristics->get(CharId::T) == 30);

    // S should be 50 - 10 = 40
    REQUIRE(player->characteristics->get(CharId::S) == 40);
    REQUIRE(enemy->characteristics->get(CharId::S) == 40);

    // Verify exact equality between player and enemy
    REQUIRE(player->characteristics->get(CharId::T) == enemy->characteristics->get(CharId::T));
    REQUIRE(player->characteristics->get(CharId::S) == enemy->characteristics->get(CharId::S));
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Property 2: Injury application and debuff state invariant
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 1.1, 1.2, 3.3**

TEST_CASE("PBT: Property 2 — Injury application and debuff state invariant",
          "[pbt][critical-injuries]")
{
    // Feature: critical-injuries, Property 2: Injury application and debuff state invariant
    rc::prop("effective stats equal base + sum of active debuffs (clamped [1,99]) after injury sequence",
        []() {
            // Generate a random base stat value in [10, 90] to keep room for penalties
            const int baseStat = *rc::gen::inRange(10, 91);

            // Create an Actor with Characteristics set to the base value
            Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
            actor.characteristics = std::make_shared<Characteristics>(baseStat);

            // Create an InjuryTracker
            InjuryTracker tracker;

            // Generate a random number of injury operations (1 to 12)
            const int numOps = *rc::gen::inRange(1, 13);

            // Track whether at least one applyInjury succeeded
            bool anyApplied = false;

            for (int i = 0; i < numOps; ++i) {
                // Pick a random location and magnitude
                const int locInt = *rc::gen::inRange(0, 6);
                const int mag = *rc::gen::inRange(1, 5); // [1,4]
                const auto loc = static_cast<HitLocation>(locInt);

                bool result = tracker.applyInjury(&actor, loc, mag);
                if (result) anyApplied = true;
            }

            // The invariant requires that applyInjury actually records injuries.
            // If we applied at least one injury to a fresh tracker, it must have injuries.
            // (first apply to any location should always succeed)
            RC_ASSERT(anyApplied);
            RC_ASSERT(tracker.hasInjuries());

            // Now verify the invariant:
            // For each Characteristic, get() must equal base + sum of all active debuff modifiers, clamped [1,99]

            // Compute expected modifiers by summing the debuff table entries for each active injury
            std::array<int, Characteristics::CHAR_COUNT> expectedModifiers{};

            for (int locIdx = 0; locIdx < InjuryTracker::MAX_LOCATIONS; ++locIdx) {
                int activeMag = tracker.getMagnitude(static_cast<HitLocation>(locIdx));
                if (activeMag > 0) {
                    auto entry = InjuryDebuffs::lookup(static_cast<HitLocation>(locIdx), activeMag);
                    for (int m = 0; m < entry.count; ++m) {
                        int statIdx = static_cast<int>(entry.modifiers[m].stat);
                        expectedModifiers[statIdx] += entry.modifiers[m].penalty;
                    }
                }
            }

            // Check each Characteristic
            for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
                int expectedValue = baseStat + expectedModifiers[c];
                // Clamp to [1, 99]
                if (expectedValue < Characteristics::MIN_VALUE)
                    expectedValue = Characteristics::MIN_VALUE;
                if (expectedValue > Characteristics::MAX_VALUE)
                    expectedValue = Characteristics::MAX_VALUE;

                int actualValue = actor.characteristics->get(static_cast<CharId>(c));

                RC_ASSERT(actualValue == expectedValue);
            }
        });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Property 5: Serialization round-trip
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 5.1, 5.2, 5.3**

TEST_CASE("PBT: Property 5 — Serialization round-trip",
          "[pbt][critical-injuries]")
{
    // Feature: critical-injuries, Property 5: Serialization round-trip
    rc::prop("save then load + reapplyDebuffs preserves magnitudes and effective stats",
        []() {
            // 1. Generate a base stat value for the Actor's Characteristics
            const int baseValue = *rc::gen::inRange(20, 80);

            // 2. Generate random magnitudes (0-4) for each of 6 locations
            std::array<int, 6> magnitudes;
            for (int i = 0; i < 6; ++i) {
                magnitudes[i] = *rc::gen::inRange(0, 5); // 0 = no injury, 1-4 = active
            }

            // 3. Create original Actor + InjuryTracker, apply all non-zero injuries
            Actor originalActor(0, 0, '@', "Original", TCODColor::white);
            originalActor.characteristics = std::make_shared<Characteristics>(baseValue);

            InjuryTracker originalTracker;
            for (int i = 0; i < 6; ++i) {
                if (magnitudes[i] > 0) {
                    originalTracker.applyInjury(&originalActor, static_cast<HitLocation>(i), magnitudes[i]);
                }
            }

            // 4. Save to a TCODZip archive via temp file
            const char* tempFile = "__test_injury_serialization_roundtrip.sav";
            {
                TCODZip zip;
                originalTracker.save(zip);
                zip.saveToFile(tempFile);
            }

            // 5. Load into a new InjuryTracker from the archive
            InjuryTracker loadedTracker;
            {
                TCODZip zip;
                zip.loadFromFile(tempFile);
                loadedTracker.load(zip);
            }

            // 6. Create a fresh Actor with same base stats, reapply debuffs
            Actor loadedActor(0, 0, '@', "Loaded", TCODColor::white);
            loadedActor.characteristics = std::make_shared<Characteristics>(baseValue);
            loadedTracker.reapplyDebuffs(&loadedActor);

            // 7. Verify getMagnitude matches for all locations
            for (int i = 0; i < 6; ++i) {
                const auto loc = static_cast<HitLocation>(i);
                RC_ASSERT(loadedTracker.getMagnitude(loc) == originalTracker.getMagnitude(loc));
            }

            // 8. Verify Characteristics get() values match between original and loaded
            for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
                const auto charId = static_cast<CharId>(c);
                RC_ASSERT(loadedActor.characteristics->get(charId) ==
                          originalActor.characteristics->get(charId));
            }

            // Cleanup temp file
            std::remove(tempFile);
        });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Unit Test: Backward-compatible load
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 5.4**

TEST_CASE("InjuryTracker: loading archive without injury sentinel initializes empty tracker",
          "[critical-injuries][serialization]")
{
    // Write some arbitrary data that is NOT the injury sentinel (0x494E4A52).
    // This simulates a save archive from a version without critical injury support.
    const char* tempFile = "__test_injury_backward_compat.sav";
    {
        TCODZip zip;
        // Write some non-sentinel integers (e.g. old Actor data that might appear
        // at this position in the archive before the injury system existed)
        zip.putInt(42);       // Some arbitrary int (not 0x494E4A52)
        zip.putInt(100);      // Another value
        zip.putInt(7);        // Another value
        zip.saveToFile(tempFile);
    }

    // Load the archive and attempt to load an InjuryTracker from it
    TCODZip zip2;
    zip2.loadFromFile(tempFile);

    InjuryTracker tracker;
    tracker.load(zip2);

    // Verify no crash occurred and tracker is in empty state
    REQUIRE(tracker.hasInjuries() == false);

    // Verify all magnitudes are zero
    REQUIRE(tracker.getMagnitude(HitLocation::HEAD) == 0);
    REQUIRE(tracker.getMagnitude(HitLocation::RIGHT_ARM) == 0);
    REQUIRE(tracker.getMagnitude(HitLocation::LEFT_ARM) == 0);
    REQUIRE(tracker.getMagnitude(HitLocation::BODY) == 0);
    REQUIRE(tracker.getMagnitude(HitLocation::RIGHT_LEG) == 0);
    REQUIRE(tracker.getMagnitude(HitLocation::LEFT_LEG) == 0);

    // Verify normal operation continues: apply an injury and confirm it works
    auto actor = makeTestActor("PostLoadActor", 40);
    bool applied = tracker.applyInjury(actor.get(), HitLocation::BODY, 2);
    REQUIRE(applied == true);
    REQUIRE(tracker.getMagnitude(HitLocation::BODY) == 2);
    REQUIRE(tracker.hasInjuries() == true);

    // Verify the debuff was applied correctly (BODY mag 2: T -10)
    REQUIRE(actor->characteristics->get(CharId::T) == 30);

    // Clean up temp file
    std::remove(tempFile);
}

TEST_CASE("InjuryTracker: loading with valid sentinel reads injury data correctly",
          "[critical-injuries][serialization]")
{
    // This is a positive control: write a proper sentinel + magnitudes, verify they load.
    // This test will pass once task 5.3 implements save/load, but documents expected behaviour.
    const char* tempFile = "__test_injury_valid_sentinel.sav";
    {
        TCODZip zip;
        zip.putInt(0x494E4A52);  // "INJR" sentinel
        zip.putInt(2);           // HEAD magnitude 2
        zip.putInt(0);           // RIGHT_ARM: no injury
        zip.putInt(0);           // LEFT_ARM: no injury
        zip.putInt(3);           // BODY magnitude 3
        zip.putInt(0);           // RIGHT_LEG: no injury
        zip.putInt(1);           // LEFT_LEG magnitude 1
        zip.saveToFile(tempFile);
    }

    TCODZip zip2;
    zip2.loadFromFile(tempFile);

    InjuryTracker tracker;
    tracker.load(zip2);

    // Verify magnitudes loaded correctly
    REQUIRE(tracker.getMagnitude(HitLocation::HEAD) == 2);
    REQUIRE(tracker.getMagnitude(HitLocation::RIGHT_ARM) == 0);
    REQUIRE(tracker.getMagnitude(HitLocation::LEFT_ARM) == 0);
    REQUIRE(tracker.getMagnitude(HitLocation::BODY) == 3);
    REQUIRE(tracker.getMagnitude(HitLocation::RIGHT_LEG) == 0);
    REQUIRE(tracker.getMagnitude(HitLocation::LEFT_LEG) == 1);

    REQUIRE(tracker.hasInjuries() == true);
    REQUIRE(tracker.activeCount() == 3);

    // Clean up temp file
    std::remove(tempFile);
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Unit Tests: Healing Clears Injuries
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 4.1, 4.3, 4.4**

// ─── heal() + clearAll removes all injuries and restores stats ───────────────
// Validates: Requirement 4.1, 4.4
// NOTE: This test exercises the expected post-heal behaviour. Task 7.2 will wire
// heal() to call injuryTracker->clearAll(). Until then, the caller simulates it.

TEST_CASE("Healing: clearAll after heal removes all injuries and restores stats",
          "[critical-injuries][healing]")
{
    // Create an Actor with known base stats and a Destructible for healing
    auto actor = makeTestActor("WoundedMarine", 45);
    actor->destructible = std::make_shared<PlayerDestructible>(20.0f, 3.0f, "dead marine", 0);

    // Damage the actor so heal() has something to restore
    actor->destructible->hp = 5.0f;

    // Apply multiple injuries across different locations
    auto tracker = std::make_unique<InjuryTracker>();
    tracker->applyInjury(actor.get(), HitLocation::HEAD, 2);       // Per -10, BS -5
    tracker->applyInjury(actor.get(), HitLocation::BODY, 3);       // T -10, S -5
    tracker->applyInjury(actor.get(), HitLocation::RIGHT_LEG, 1);  // Ag -5

    // Verify injuries are active and stats are debuffed
    REQUIRE(tracker->hasInjuries() == true);
    REQUIRE(tracker->activeCount() == 3);
    REQUIRE(actor->characteristics->get(CharId::Per) == 35);  // 45 - 10
    REQUIRE(actor->characteristics->get(CharId::BS) == 40);   // 45 - 5
    REQUIRE(actor->characteristics->get(CharId::T) == 35);    // 45 - 10
    REQUIRE(actor->characteristics->get(CharId::S) == 40);    // 45 - 5
    REQUIRE(actor->characteristics->get(CharId::Ag) == 40);   // 45 - 5

    // Simulate the healing event: heal HP then clear injuries
    // (Task 7.2 will make heal() call clearAll automatically)
    actor->destructible->heal(20.0f);
    tracker->clearAll(actor.get());

    // Verify HP is restored
    REQUIRE(actor->destructible->hp == 20.0f);

    // Verify all injuries are cleared
    REQUIRE(tracker->hasInjuries() == false);
    REQUIRE(tracker->activeCount() == 0);
    REQUIRE(tracker->getMagnitude(HitLocation::HEAD) == 0);
    REQUIRE(tracker->getMagnitude(HitLocation::BODY) == 0);
    REQUIRE(tracker->getMagnitude(HitLocation::RIGHT_LEG) == 0);

    // Verify all stats are fully restored to base values
    for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
        REQUIRE(actor->characteristics->get(static_cast<CharId>(c)) == 45);
        REQUIRE(actor->characteristics->getModifier(static_cast<CharId>(c)) == 0);
    }
}

TEST_CASE("Healing: clearAll on actor with max-severity injuries restores all stats",
          "[critical-injuries][healing]")
{
    // Worst-case: all 6 locations injured at magnitude 4
    auto actor = makeTestActor("CriticalPatient", 50);
    actor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "dead patient", 0);
    actor->destructible->hp = 1.0f;

    auto tracker = std::make_unique<InjuryTracker>();
    tracker->applyInjury(actor.get(), HitLocation::HEAD, 4);       // WS -20, BS -20, Int -10
    tracker->applyInjury(actor.get(), HitLocation::RIGHT_ARM, 4);  // WS -20, BS -20
    tracker->applyInjury(actor.get(), HitLocation::LEFT_ARM, 4);   // WS -20, BS -20
    tracker->applyInjury(actor.get(), HitLocation::BODY, 4);       // T -20, S -10
    tracker->applyInjury(actor.get(), HitLocation::RIGHT_LEG, 4);  // Ag -20, WS -10
    tracker->applyInjury(actor.get(), HitLocation::LEFT_LEG, 4);   // Ag -20, WS -10

    // Verify injuries are active
    REQUIRE(tracker->activeCount() == 6);

    // Verify heavy debuffs are applied (WS gets -20-20-20-10-10 = -80, clamped to min 1)
    REQUIRE(actor->characteristics->get(CharId::WS) == Characteristics::MIN_VALUE); // 50-80 clamped

    // Simulate full heal + clear
    actor->destructible->heal(30.0f);
    tracker->clearAll(actor.get());

    // Verify complete recovery
    REQUIRE(tracker->hasInjuries() == false);
    REQUIRE(tracker->activeCount() == 0);
    REQUIRE(actor->destructible->hp == 30.0f);

    // All stats must return to base value of 50
    for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
        REQUIRE(actor->characteristics->get(static_cast<CharId>(c)) == 50);
        REQUIRE(actor->characteristics->getModifier(static_cast<CharId>(c)) == 0);
    }
}

TEST_CASE("Healing: clearAll on actor with no injuries is a safe no-op",
          "[critical-injuries][healing]")
{
    auto actor = makeTestActor("HealthyMarine", 40);
    actor->destructible = std::make_shared<PlayerDestructible>(20.0f, 3.0f, "dead marine", 0);

    InjuryTracker tracker;

    // Verify no injuries and base stats
    REQUIRE(tracker.hasInjuries() == false);

    // Calling clearAll on an actor with no injuries should be safe
    tracker.clearAll(actor.get());

    // Stats should remain unchanged at base
    for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
        REQUIRE(actor->characteristics->get(static_cast<CharId>(c)) == 40);
        REQUIRE(actor->characteristics->getModifier(static_cast<CharId>(c)) == 0);
    }
}

// ─── Level transition clears player injuries ─────────────────────────────────
// Validates: Requirement 4.3
// Engine::nextLevel() will call player->injuryTracker->clearAll(player).
// We test the clearAll path that the level transition will invoke.

TEST_CASE("Level transition: clearing player injuries restores stats",
          "[critical-injuries][healing]")
{
    // Simulate the player having injuries before a level transition
    auto player = makeTestActor("Player", 40);
    player->destructible = std::make_shared<PlayerDestructible>(25.0f, 3.0f, "dead player", 0);
    player->destructible->hp = 10.0f;  // partially damaged

    // Assign an InjuryTracker to the player (as it would be in-game)
    player->injuryTracker = std::make_unique<InjuryTracker>();

    // Apply injuries as if accumulated during the level
    player->injuryTracker->applyInjury(player.get(), HitLocation::HEAD, 1);       // Per -5
    player->injuryTracker->applyInjury(player.get(), HitLocation::LEFT_ARM, 2);   // WS -10, BS -5
    player->injuryTracker->applyInjury(player.get(), HitLocation::RIGHT_LEG, 3);  // Ag -15, WS -5

    // Verify injuries are in place
    REQUIRE(player->injuryTracker->hasInjuries() == true);
    REQUIRE(player->injuryTracker->activeCount() == 3);
    REQUIRE(player->characteristics->get(CharId::Per) == 35);  // 40 - 5
    REQUIRE(player->characteristics->get(CharId::WS) == 25);   // 40 - 10 - 5
    REQUIRE(player->characteristics->get(CharId::BS) == 35);   // 40 - 5
    REQUIRE(player->characteristics->get(CharId::Ag) == 25);   // 40 - 15

    // Simulate the level transition clearing (Engine::nextLevel will do this)
    player->injuryTracker->clearAll(player.get());

    // Verify all injuries cleared
    REQUIRE(player->injuryTracker->hasInjuries() == false);
    REQUIRE(player->injuryTracker->activeCount() == 0);
    REQUIRE(player->injuryTracker->getMagnitude(HitLocation::HEAD) == 0);
    REQUIRE(player->injuryTracker->getMagnitude(HitLocation::LEFT_ARM) == 0);
    REQUIRE(player->injuryTracker->getMagnitude(HitLocation::RIGHT_LEG) == 0);

    // Verify all stats restored to base
    for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
        REQUIRE(player->characteristics->get(static_cast<CharId>(c)) == 40);
        REQUIRE(player->characteristics->getModifier(static_cast<CharId>(c)) == 0);
    }
}

TEST_CASE("Level transition: player with escalated injuries gets full clear",
          "[critical-injuries][healing]")
{
    // Test that escalated injuries (magnitude > initial) are properly cleared
    auto player = makeTestActor("Player", 50);
    player->destructible = std::make_shared<PlayerDestructible>(30.0f, 3.0f, "dead player", 0);
    player->injuryTracker = std::make_unique<InjuryTracker>();

    // Apply initial injury then escalate it
    player->injuryTracker->applyInjury(player.get(), HitLocation::BODY, 1);  // T -5
    REQUIRE(player->characteristics->get(CharId::T) == 45);

    // Escalate: BODY goes from 1 → 2 (T -10)
    player->injuryTracker->applyInjury(player.get(), HitLocation::BODY, 1);
    REQUIRE(player->injuryTracker->getMagnitude(HitLocation::BODY) == 2);
    REQUIRE(player->characteristics->get(CharId::T) == 40);  // 50 - 10

    // Escalate again: BODY goes from 2 → 3 (T -10, S -5)
    player->injuryTracker->applyInjury(player.get(), HitLocation::BODY, 1);
    REQUIRE(player->injuryTracker->getMagnitude(HitLocation::BODY) == 3);
    REQUIRE(player->characteristics->get(CharId::T) == 40);  // 50 - 10
    REQUIRE(player->characteristics->get(CharId::S) == 45);  // 50 - 5

    // Simulate level transition
    player->injuryTracker->clearAll(player.get());

    // Verify full recovery even from escalated injuries
    REQUIRE(player->injuryTracker->hasInjuries() == false);
    REQUIRE(player->injuryTracker->getMagnitude(HitLocation::BODY) == 0);
    REQUIRE(player->characteristics->get(CharId::T) == 50);
    REQUIRE(player->characteristics->get(CharId::S) == 50);

    // All modifiers should be zero
    for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
        REQUIRE(player->characteristics->getModifier(static_cast<CharId>(c)) == 0);
    }
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Unit Tests for Attacker Integration
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 1.1, 1.4, 3.2**
// Tags: [critical-injuries][attacker-integration]
//
// These tests exercise the Attacker::resolveCharacterAttack flow with injectable
// RNG to force critical hits, verifying that the injury system is invoked correctly.
// They are TDD tests — expected to FAIL until task 6.2 integrates InjuryTracker
// into the critical hit block.

namespace {
    // Creates a character Actor with configurable stats for attacker integration tests.
    // The actor has Attacker, Destructible, and Characteristics components.
    std::unique_ptr<Actor> makeAttackerTestActor(
        const std::string& name, int ws, int s, int t, int ag, float hp)
    {
        auto actor = std::make_unique<Actor>(0, 0, '@', name, Colors::white);

        auto chars = std::make_shared<Characteristics>(30);
        chars->set(CharId::WS, ws);
        chars->set(CharId::S, s);
        chars->set(CharId::T, t);
        chars->set(CharId::Ag, ag);
        actor->characteristics = chars;

        auto dest = std::make_shared<MonsterDestructible>(hp, 0.0f, "corpse", 0);
        actor->destructible = dest;

        auto atk = std::make_shared<Attacker>(0.0f, ws);
        actor->attacker = atk;

        return actor;
    }
}

// ─── Non-fatal crit at magnitude 1 creates injury on target (HEAD) ───────────
// Validates: Requirement 1.1

TEST_CASE("Attacker integration: non-fatal crit magnitude 1 creates injury on target",
          "[critical-injuries][attacker-integration]")
{
    // Owner: WS=50, S=30 (SB=3)
    auto owner = makeAttackerTestActor("Attacker", 50, 30, 30, 30, 20.0f);
    // Target: T=10 (TB=1), Ag=1 (dodge always fails), HP=4
    // finalDamage = (dieRoll=3 + SB=3) - armour(0) - TB(1) = 5
    // critMagnitude = 5 - 4 = 1
    auto target = makeAttackerTestActor("Target", 30, 30, 10, 1, 4.0f);

    // Inject RNG: roll=1 guarantees hit (1 <= WS 50) and HEAD location
    // dodge roll=99 guarantees fail (99 > Ag 1)
    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 1;   // hit roll: 1 <= WS 50, HEAD location
            case 2: return 99;  // dodge roll: 99 > Ag 1, fail
            default: return 99;
        }
    };
    // Die always returns 3 (unarmed 1d5)
    owner->attacker->rollDie = [](int /*sides*/) { return 3; };

    // Execute attack
    owner->attacker->attack(owner.get(), target.get());

    // After attack, target should have an injuryTracker with magnitude 1 at HEAD
    REQUIRE(target->injuryTracker != nullptr);
    REQUIRE(target->injuryTracker->getMagnitude(HitLocation::HEAD) == 1);
}

// ─── Non-fatal crit at magnitude 2 creates injury on target (HEAD) ───────────
// Validates: Requirement 1.1

TEST_CASE("Attacker integration: non-fatal crit magnitude 2 creates injury on target",
          "[critical-injuries][attacker-integration]")
{
    // Owner: WS=50, S=30 (SB=3)
    auto owner = makeAttackerTestActor("Attacker", 50, 30, 30, 30, 20.0f);
    // Target: T=10 (TB=1), Ag=1, HP=3
    // finalDamage = (3 + 3) - 0 - 1 = 5
    // critMagnitude = 5 - 3 = 2
    auto target = makeAttackerTestActor("Target", 30, 30, 10, 1, 3.0f);

    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 1;   // hit: HEAD
            case 2: return 99;  // dodge fail
            default: return 99;
        }
    };
    owner->attacker->rollDie = [](int /*sides*/) { return 3; };

    owner->attacker->attack(owner.get(), target.get());

    REQUIRE(target->injuryTracker != nullptr);
    REQUIRE(target->injuryTracker->getMagnitude(HitLocation::HEAD) == 2);
}

// ─── Non-fatal crit at magnitude 3 creates injury (HEAD) ─────────────────────
// Validates: Requirement 1.1

TEST_CASE("Attacker integration: non-fatal crit magnitude 3 creates injury on target",
          "[critical-injuries][attacker-integration]")
{
    // Owner: WS=50, S=30 (SB=3)
    auto owner = makeAttackerTestActor("Attacker", 50, 30, 30, 30, 20.0f);
    // Target: T=10 (TB=1), Ag=1, HP=2
    // finalDamage = (3 + 3) - 0 - 1 = 5
    // critMagnitude = 5 - 2 = 3
    auto target = makeAttackerTestActor("Target", 30, 30, 10, 1, 2.0f);

    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 1;   // hit: HEAD
            case 2: return 99;  // dodge fail
            default: return 99;
        }
    };
    owner->attacker->rollDie = [](int /*sides*/) { return 3; };

    owner->attacker->attack(owner.get(), target.get());

    REQUIRE(target->injuryTracker != nullptr);
    REQUIRE(target->injuryTracker->getMagnitude(HitLocation::HEAD) == 3);
}

// ─── Non-fatal crit at magnitude 4 creates injury (HEAD) ─────────────────────
// Validates: Requirement 1.1

TEST_CASE("Attacker integration: non-fatal crit magnitude 4 creates injury on target",
          "[critical-injuries][attacker-integration]")
{
    // Owner: WS=50, S=30 (SB=3)
    auto owner = makeAttackerTestActor("Attacker", 50, 30, 30, 30, 20.0f);
    // Target: T=10 (TB=1), Ag=1, HP=1
    // finalDamage = (3 + 3) - 0 - 1 = 5
    // critMagnitude = 5 - 1 = 4
    auto target = makeAttackerTestActor("Target", 30, 30, 10, 1, 1.0f);

    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 1;   // hit: HEAD
            case 2: return 99;  // dodge fail
            default: return 99;
        }
    };
    owner->attacker->rollDie = [](int /*sides*/) { return 3; };

    owner->attacker->attack(owner.get(), target.get());

    REQUIRE(target->injuryTracker != nullptr);
    REQUIRE(target->injuryTracker->getMagnitude(HitLocation::HEAD) == 4);
}

// ─── Escalation to magnitude 5 triggers fatal effect and kills target ────────
// Validates: Requirement 3.2

TEST_CASE("Attacker integration: escalation to magnitude 5 kills target",
          "[critical-injuries][attacker-integration]")
{
    // Owner: WS=50, S=30 (SB=3)
    auto owner = makeAttackerTestActor("Attacker", 50, 30, 30, 30, 20.0f);
    // Target: T=10 (TB=1), Ag=1, HP=4
    // finalDamage = (3 + 3) - 0 - 1 = 5
    // critMagnitude = 5 - 4 = 1 (non-fatal on its own)
    auto target = makeAttackerTestActor("Target", 30, 30, 10, 1, 4.0f);

    // Pre-seed the target with an existing injury at HEAD magnitude 4.
    // When the attack lands a crit at HEAD, escalation from 4 should trigger fatal.
    target->injuryTracker = std::make_unique<InjuryTracker>();
    target->injuryTracker->applyInjury(target.get(), HitLocation::HEAD, 4);
    REQUIRE(target->injuryTracker->getMagnitude(HitLocation::HEAD) == 4);

    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 1;   // hit: HEAD
            case 2: return 99;  // dodge fail
            default: return 99;
        }
    };
    owner->attacker->rollDie = [](int /*sides*/) { return 3; };

    // Target should be alive before the attack
    REQUIRE_FALSE(target->destructible->isDead());

    owner->attacker->attack(owner.get(), target.get());

    // Escalation from mag 4 → 5 should kill the target
    REQUIRE(target->destructible->isDead());
}

// ─── HP is set to 1 for non-fatal crits below magnitude 3 ───────────────────
// Validates: Requirement 1.4

TEST_CASE("Attacker integration: HP set to 1 for magnitude 1 crit (existing behaviour preserved)",
          "[critical-injuries][attacker-integration]")
{
    // Owner: WS=50, S=30 (SB=3)
    auto owner = makeAttackerTestActor("Attacker", 50, 30, 30, 30, 20.0f);
    // Target: T=10 (TB=1), Ag=1, HP=4
    // finalDamage = (3 + 3) - 0 - 1 = 5
    // critMagnitude = 5 - 4 = 1 (below 3, so target survives at HP=1)
    auto target = makeAttackerTestActor("Target", 30, 30, 10, 1, 4.0f);

    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 1;   // hit: HEAD
            case 2: return 99;  // dodge fail
            default: return 99;
        }
    };
    owner->attacker->rollDie = [](int /*sides*/) { return 3; };

    owner->attacker->attack(owner.get(), target.get());

    // Target survives at HP=1 (existing behaviour for crits below magnitude 3)
    REQUIRE_FALSE(target->destructible->isDead());
    REQUIRE(target->destructible->hp == Catch::Approx(1.0f));
}

TEST_CASE("Attacker integration: HP set to 1 for magnitude 2 crit (existing behaviour preserved)",
          "[critical-injuries][attacker-integration]")
{
    // Owner: WS=50, S=30 (SB=3)
    auto owner = makeAttackerTestActor("Attacker", 50, 30, 30, 30, 20.0f);
    // Target: T=10 (TB=1), Ag=1, HP=3
    // finalDamage = (3 + 3) - 0 - 1 = 5
    // critMagnitude = 5 - 3 = 2 (below 3, so target survives at HP=1)
    auto target = makeAttackerTestActor("Target", 30, 30, 10, 1, 3.0f);

    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 1;   // hit: HEAD
            case 2: return 99;  // dodge fail
            default: return 99;
        }
    };
    owner->attacker->rollDie = [](int /*sides*/) { return 3; };

    owner->attacker->attack(owner.get(), target.get());

    // Target survives at HP=1 (existing behaviour for crits below magnitude 3)
    REQUIRE_FALSE(target->destructible->isDead());
    REQUIRE(target->destructible->hp == Catch::Approx(1.0f));
}
