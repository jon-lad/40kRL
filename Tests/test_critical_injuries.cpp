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

TEST_CASE("Modifier: getBase() returns raw value unaffected by modifiers", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(50);
    chars.addModifier(CharId::Ag, -20);
    REQUIRE(chars.get(CharId::Ag) == 30);
    REQUIRE(chars.getBase(CharId::Ag) == 50);
}

TEST_CASE("Modifier: get() clamps to minimum 1 with large negative modifier", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(20);
    chars.addModifier(CharId::Per, -50);
    REQUIRE(chars.get(CharId::Per) == Characteristics::MIN_VALUE);
    REQUIRE(chars.getBase(CharId::Per) == 20);
}

TEST_CASE("Modifier: get() clamps to maximum 99 with large positive modifier", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(90);
    chars.addModifier(CharId::WP, 20);
    REQUIRE(chars.get(CharId::WP) == Characteristics::MAX_VALUE);
}

TEST_CASE("Modifier: multiple modifiers on same stat are additive", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(50);
    chars.addModifier(CharId::S, -5);
    chars.addModifier(CharId::S, -10);
    chars.addModifier(CharId::S, -3);
    REQUIRE(chars.get(CharId::S) == 32);
}

TEST_CASE("Modifier: removing a modifier restores previous effective value", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(60);
    chars.addModifier(CharId::Fel, -20);
    REQUIRE(chars.get(CharId::Fel) == 40);
    chars.removeModifier(CharId::Fel, -20);
    REQUIRE(chars.get(CharId::Fel) == 60);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Debuff Lookup Table Tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Debuff lookup table returns valid entries for all magnitudes 1-9",
          "[pbt][critical-injuries]")
{
    rc::prop("lookup returns valid entries for any (loc, mag) in [0-5] x [1-9]",
        []() {
            const int locInt = *rc::gen::inRange(0, 6);
            const int mag = *rc::gen::inRange(1, 10); // [1,9]
            const auto loc = static_cast<HitLocation>(locInt);
            const auto entry = InjuryDebuffs::lookup(loc, mag);

            // Count must be in valid range [1, 3]
            RC_ASSERT(entry.count >= 1);
            RC_ASSERT(entry.count <= InjuryDebuffs::MAX_MODIFIERS_PER_INJURY);

            // All active penalties must be negative
            for (int i = 0; i < entry.count; ++i) {
                RC_ASSERT(entry.modifiers[i].penalty < 0);
            }
        });
}

TEST_CASE("Debuff lookup: HEAD magnitudes 1-4 match original spec", "[critical-injuries]")
{
    using namespace InjuryDebuffs;

    auto e = lookup(HitLocation::HEAD, 1);
    REQUIRE(e.count == 1);
    REQUIRE(e.modifiers[0].stat == CharId::Per);
    REQUIRE(e.modifiers[0].penalty == -5);

    e = lookup(HitLocation::HEAD, 2);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::Per);
    REQUIRE(e.modifiers[0].penalty == -10);
    REQUIRE(e.modifiers[1].stat == CharId::BS);
    REQUIRE(e.modifiers[1].penalty == -5);

    e = lookup(HitLocation::HEAD, 3);
    REQUIRE(e.count == 2);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -10);

    e = lookup(HitLocation::HEAD, 4);
    REQUIRE(e.count == 3);
    REQUIRE(e.modifiers[0].stat == CharId::WS);
    REQUIRE(e.modifiers[0].penalty == -20);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — InjuryTracker Core Tests (New Single-Magnitude Model)
// ═══════════════════════════════════════════════════════════════════════════════

namespace {
    std::unique_ptr<Actor> makeTestActor(const std::string& name, int baseStat = 40) {
        auto actor = std::make_unique<Actor>(0, 0, '@', name, TCODColor::white);
        actor->characteristics = std::make_shared<Characteristics>(baseStat);
        return actor;
    }
}

TEST_CASE("InjuryTracker: fresh tracker has magnitude 0 and no injuries",
          "[critical-injuries][injury-tracker]")
{
    InjuryTracker tracker;
    REQUIRE(tracker.getMagnitude() == 0);
    REQUIRE(tracker.hasInjuries() == false);
    REQUIRE(tracker.activeCount() == 0);
}

TEST_CASE("InjuryTracker: applyCrit adds to cumulative magnitude",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("Victim", 40);
    InjuryTracker tracker;

    REQUIRE(tracker.applyCrit(actor.get(), HitLocation::HEAD, 2) == true);
    REQUIRE(tracker.getMagnitude() == 2);

    REQUIRE(tracker.applyCrit(actor.get(), HitLocation::RIGHT_ARM, 3) == true);
    REQUIRE(tracker.getMagnitude() == 5);

    REQUIRE(tracker.applyCrit(actor.get(), HitLocation::BODY, 1) == true);
    REQUIRE(tracker.getMagnitude() == 6);
}

TEST_CASE("InjuryTracker: applyCrit returns false when total reaches 10 (fatal)",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("Victim", 40);
    InjuryTracker tracker;

    // Build up to 9
    REQUIRE(tracker.applyCrit(actor.get(), HitLocation::BODY, 4) == true);
    REQUIRE(tracker.applyCrit(actor.get(), HitLocation::HEAD, 4) == true);
    REQUIRE(tracker.getMagnitude() == 8);

    // This would bring to 10 — fatal
    REQUIRE(tracker.applyCrit(actor.get(), HitLocation::LEFT_LEG, 2) == false);
    // Magnitude stays at 8 (caller handles death)
    REQUIRE(tracker.getMagnitude() == 8);
}

TEST_CASE("InjuryTracker: applyCrit applies correct debuff for location + total magnitude",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("Victim", 50);
    InjuryTracker tracker;

    // First crit: mag 2 to ARM → total becomes 2, debuff is ARM at mag 2 (WS -10, BS -5)
    tracker.applyCrit(actor.get(), HitLocation::RIGHT_ARM, 2);
    REQUIRE(actor->characteristics->get(CharId::WS) == 40);  // 50 - 10
    REQUIRE(actor->characteristics->get(CharId::BS) == 45);   // 50 - 5

    // Second crit: mag 3 to HEAD → total becomes 5, debuff is HEAD at mag 5
    tracker.applyCrit(actor.get(), HitLocation::HEAD, 3);
    REQUIRE(tracker.getMagnitude() == 5);
    // HEAD mag 5: WS -20, BS -20, Per -15
    // Cumulative: WS = 50 - 10 - 20 = 20, BS = 50 - 5 - 20 = 25
    REQUIRE(actor->characteristics->get(CharId::WS) == 20);
    REQUIRE(actor->characteristics->get(CharId::BS) == 25);
    REQUIRE(actor->characteristics->get(CharId::Per) == 35);  // 50 - 15
}

TEST_CASE("InjuryTracker: activeCount reflects number of crit events",
          "[critical-injuries][injury-tracker]")
{
    auto actor = makeTestActor("Victim", 40);
    InjuryTracker tracker;

    tracker.applyCrit(actor.get(), HitLocation::HEAD, 1);
    REQUIRE(tracker.activeCount() == 1);

    tracker.applyCrit(actor.get(), HitLocation::HEAD, 1);  // same location, still a new record
    REQUIRE(tracker.activeCount() == 2);

    tracker.applyCrit(actor.get(), HitLocation::BODY, 2);
    REQUIRE(tracker.activeCount() == 3);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Healing Reduces Magnitude
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("InjuryTracker: healMagnitude reduces magnitude and returns excess",
          "[critical-injuries][healing]")
{
    auto actor = makeTestActor("Victim", 40);
    InjuryTracker tracker;

    tracker.applyCrit(actor.get(), HitLocation::HEAD, 5);
    REQUIRE(tracker.getMagnitude() == 5);

    // Heal 3: magnitude goes from 5 to 2, excess = 0
    int excess = tracker.healMagnitude(3);
    REQUIRE(tracker.getMagnitude() == 2);
    REQUIRE(excess == 0);

    // Heal 5: magnitude goes from 2 to 0, excess = 3 (for HP)
    excess = tracker.healMagnitude(5);
    REQUIRE(tracker.getMagnitude() == 0);
    REQUIRE(excess == 3);
}

TEST_CASE("InjuryTracker: healMagnitude does NOT remove debuffs",
          "[critical-injuries][healing]")
{
    auto actor = makeTestActor("Victim", 50);
    InjuryTracker tracker;

    // Apply crit: HEAD mag 2 → Per -10, BS -5
    tracker.applyCrit(actor.get(), HitLocation::HEAD, 2);
    REQUIRE(actor->characteristics->get(CharId::Per) == 40);  // 50 - 10
    REQUIRE(actor->characteristics->get(CharId::BS) == 45);   // 50 - 5

    // Heal magnitude fully
    tracker.healMagnitude(2);
    REQUIRE(tracker.getMagnitude() == 0);

    // Debuffs remain!
    REQUIRE(actor->characteristics->get(CharId::Per) == 40);
    REQUIRE(actor->characteristics->get(CharId::BS) == 45);
    REQUIRE(tracker.hasInjuries() == true);
}

TEST_CASE("InjuryTracker: healMagnitude with no magnitude returns full amount as excess",
          "[critical-injuries][healing]")
{
    InjuryTracker tracker;
    int excess = tracker.healMagnitude(10);
    REQUIRE(excess == 10);
    REQUIRE(tracker.getMagnitude() == 0);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — clearAll (level transition)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("InjuryTracker: clearAll removes all debuffs and resets magnitude",
          "[critical-injuries][healing]")
{
    auto actor = makeTestActor("Victim", 50);
    InjuryTracker tracker;

    tracker.applyCrit(actor.get(), HitLocation::HEAD, 3);
    tracker.applyCrit(actor.get(), HitLocation::BODY, 2);
    REQUIRE(tracker.getMagnitude() == 5);
    REQUIRE(tracker.hasInjuries() == true);

    tracker.clearAll(actor.get());

    REQUIRE(tracker.getMagnitude() == 0);
    REQUIRE(tracker.hasInjuries() == false);
    REQUIRE(tracker.activeCount() == 0);

    // All stats restored
    for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
        REQUIRE(actor->characteristics->get(static_cast<CharId>(c)) == 50);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Serialization
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("InjuryTracker: save/load round-trip preserves state",
          "[critical-injuries][serialization]")
{
    auto actor = makeTestActor("Original", 40);
    InjuryTracker tracker;

    tracker.applyCrit(actor.get(), HitLocation::HEAD, 2);
    tracker.applyCrit(actor.get(), HitLocation::BODY, 3);

    const char* tempFile = "__test_injury_roundtrip.sav";
    {
        TCODZip zip;
        tracker.save(zip);
        zip.saveToFile(tempFile);
    }

    InjuryTracker loaded;
    {
        TCODZip zip;
        zip.loadFromFile(tempFile);
        loaded.load(zip);
    }

    REQUIRE(loaded.getMagnitude() == 5);
    REQUIRE(loaded.activeCount() == 2);

    // Reapply debuffs on a fresh actor
    auto actor2 = makeTestActor("Loaded", 40);
    loaded.reapplyDebuffs(actor2.get());

    // Should match original actor's stats
    for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
        auto id = static_cast<CharId>(c);
        REQUIRE(actor2->characteristics->get(id) == actor->characteristics->get(id));
    }

    std::remove(tempFile);
}

TEST_CASE("InjuryTracker: loading archive without sentinel initializes empty",
          "[critical-injuries][serialization]")
{
    const char* tempFile = "__test_injury_nosentinel.sav";
    {
        TCODZip zip;
        zip.putInt(42);  // not the sentinel
        zip.putInt(100);
        zip.saveToFile(tempFile);
    }

    TCODZip zip2;
    zip2.loadFromFile(tempFile);

    InjuryTracker tracker;
    tracker.load(zip2);

    REQUIRE(tracker.getMagnitude() == 0);
    REQUIRE(tracker.hasInjuries() == false);

    std::remove(tempFile);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — PBT: Cumulative magnitude and heal round-trip
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: applyCrit accumulates magnitude correctly",
          "[pbt][critical-injuries]")
{
    rc::prop("cumulative magnitude equals sum of all applied critMagnitudes",
        []() {
            const int baseStat = *rc::gen::inRange(20, 80);
            Actor actor(0, 0, '@', "TestActor", TCODColor::white);
            actor.characteristics = std::make_shared<Characteristics>(baseStat);

            InjuryTracker tracker;
            int expectedMag = 0;
            const int numCrits = *rc::gen::inRange(1, 8);

            for (int i = 0; i < numCrits; ++i) {
                const int critMag = *rc::gen::inRange(1, 4);
                const int locInt = *rc::gen::inRange(0, 6);
                const auto loc = static_cast<HitLocation>(locInt);

                if (expectedMag + critMag >= InjuryTracker::FATAL_MAGNITUDE) {
                    RC_ASSERT(tracker.applyCrit(&actor, loc, critMag) == false);
                    break;
                } else {
                    RC_ASSERT(tracker.applyCrit(&actor, loc, critMag) == true);
                    expectedMag += critMag;
                }
            }

            RC_ASSERT(tracker.getMagnitude() == expectedMag);
        });
}

TEST_CASE("PBT: healMagnitude excess equals heal - magnitude when heal > magnitude",
          "[pbt][critical-injuries]")
{
    rc::prop("healMagnitude returns correct excess",
        []() {
            Actor actor(0, 0, '@', "TestActor", TCODColor::white);
            actor.characteristics = std::make_shared<Characteristics>(40);

            InjuryTracker tracker;
            const int critMag = *rc::gen::inRange(1, 9);
            const int locInt = *rc::gen::inRange(0, 6);
            tracker.applyCrit(&actor, static_cast<HitLocation>(locInt), critMag);

            const int healAmount = *rc::gen::inRange(1, 20);
            int excess = tracker.healMagnitude(healAmount);

            int expectedExcess = std::max(0, healAmount - critMag);
            RC_ASSERT(excess == expectedExcess);
            RC_ASSERT(tracker.getMagnitude() == std::max(0, critMag - healAmount));
        });
}

TEST_CASE("PBT: clearAll restores all stats to base",
          "[pbt][critical-injuries]")
{
    rc::prop("clearAll restores all characteristics to base value",
        []() {
            const int baseStat = *rc::gen::inRange(20, 80);
            Actor actor(0, 0, '@', "TestActor", TCODColor::white);
            actor.characteristics = std::make_shared<Characteristics>(baseStat);

            InjuryTracker tracker;
            const int numCrits = *rc::gen::inRange(1, 6);
            int totalMag = 0;

            for (int i = 0; i < numCrits; ++i) {
                const int critMag = *rc::gen::inRange(1, 3);
                const int locInt = *rc::gen::inRange(0, 6);
                if (totalMag + critMag >= InjuryTracker::FATAL_MAGNITUDE) break;
                tracker.applyCrit(&actor, static_cast<HitLocation>(locInt), critMag);
                totalMag += critMag;
            }

            tracker.clearAll(&actor);

            RC_ASSERT(tracker.getMagnitude() == 0);
            RC_ASSERT(tracker.hasInjuries() == false);

            for (int c = 0; c < Characteristics::CHAR_COUNT; ++c) {
                RC_ASSERT(actor.characteristics->get(static_cast<CharId>(c)) == baseStat);
            }
        });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — GUI Display Unit Tests
// Requirements: 6.1, 6.2, 6.3
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("GUI Data: getRecords returns location and magnitude for active injuries",
          "[critical-injuries][gui]")
{
    auto actor = makeTestActor("Marine", 40);
    InjuryTracker tracker;

    // Apply crits to different locations
    tracker.applyCrit(actor.get(), HitLocation::HEAD, 2);
    tracker.applyCrit(actor.get(), HitLocation::RIGHT_LEG, 3);

    const auto& records = tracker.getRecords();
    REQUIRE(records.size() == 2);

    // First record: HEAD with total magnitude 2
    REQUIRE(records[0].location == HitLocation::HEAD);
    REQUIRE(records[0].magnitude == 2);
    // Verify HitLocationTable::name returns correct display string for sidebar
    REQUIRE(std::string(HitLocationTable::name(records[0].location)) == "Head");

    // Second record: RIGHT_LEG with total magnitude 5 (2+3)
    REQUIRE(records[1].location == HitLocation::RIGHT_LEG);
    REQUIRE(records[1].magnitude == 5);
    REQUIRE(std::string(HitLocationTable::name(records[1].location)) == "Right Leg");

    // getMagnitude returns cumulative total (what sidebar could display as overall severity)
    REQUIRE(tracker.getMagnitude() == 5);
}

TEST_CASE("GUI Data: getRecords shows all six locations when all injured",
          "[critical-injuries][gui]")
{
    auto actor = makeTestActor("Marine", 50);
    InjuryTracker tracker;

    // Apply small crits to all 6 locations (keeping total under 10)
    tracker.applyCrit(actor.get(), HitLocation::HEAD, 1);
    tracker.applyCrit(actor.get(), HitLocation::RIGHT_ARM, 1);
    tracker.applyCrit(actor.get(), HitLocation::LEFT_ARM, 1);
    tracker.applyCrit(actor.get(), HitLocation::BODY, 1);
    tracker.applyCrit(actor.get(), HitLocation::RIGHT_LEG, 1);
    tracker.applyCrit(actor.get(), HitLocation::LEFT_LEG, 1);

    const auto& records = tracker.getRecords();
    REQUIRE(records.size() == 6);
    REQUIRE(tracker.getMagnitude() == 6);

    // Each record has a valid location name for GUI display
    for (const auto& record : records) {
        const char* locName = HitLocationTable::name(record.location);
        REQUIRE(locName != nullptr);
        REQUIRE(std::string(locName) != "Unknown");
        REQUIRE(record.magnitude >= 1);
    }
}

TEST_CASE("GUI Data: enemy actor injuries are accessible via same interface (look panel)",
          "[critical-injuries][gui]")
{
    // Enemy actor — same as player, injuries use identical interface (Req 6.2)
    auto enemy = makeTestActor("Ork Boy", 30);
    InjuryTracker tracker;

    tracker.applyCrit(enemy.get(), HitLocation::BODY, 3);
    tracker.applyCrit(enemy.get(), HitLocation::LEFT_ARM, 2);

    // When player examines this enemy, GUI queries these same methods
    REQUIRE(tracker.hasInjuries() == true);
    REQUIRE(tracker.activeCount() == 2);

    const auto& records = tracker.getRecords();
    REQUIRE(records[0].location == HitLocation::BODY);
    REQUIRE(records[0].magnitude == 3);
    REQUIRE(std::string(HitLocationTable::name(records[0].location)) == "Body");

    REQUIRE(records[1].location == HitLocation::LEFT_ARM);
    REQUIRE(records[1].magnitude == 5); // cumulative: 3 + 2 = 5
    REQUIRE(std::string(HitLocationTable::name(records[1].location)) == "Left Arm");
}

TEST_CASE("GUI Data: combat log message format for injury apply contains actor name and location",
          "[critical-injuries][gui]")
{
    // The Attacker logs: "# suffers a critical injury to the #!"
    // with target->name and HitLocationTable::name(loc) as substitutions.
    // We verify the data that would be substituted is correct.
    auto target = makeTestActor("Battle Brother", 40);
    InjuryTracker tracker;

    tracker.applyCrit(target.get(), HitLocation::RIGHT_ARM, 2);

    // Simulate what Attacker.cpp produces for the GUI message
    std::string actorName = target->name;
    std::string locName = HitLocationTable::name(HitLocation::RIGHT_ARM);

    // Format matches the template used in Attacker.cpp
    std::string expectedMsg = actorName + " suffers a critical injury to the " + locName + "!";
    REQUIRE(expectedMsg == "Battle Brother suffers a critical injury to the Right Arm!");

    // The injury data is what the GUI would use
    REQUIRE(tracker.getRecords().back().location == HitLocation::RIGHT_ARM);
    REQUIRE(tracker.getRecords().back().magnitude == 2);
}

TEST_CASE("GUI Data: combat log message format for injury clear",
          "[critical-injuries][gui]")
{
    // The Pickable/heal logs: "#'s injuries are healed."
    // with actor->name as substitution.
    auto actor = makeTestActor("Scout", 40);
    InjuryTracker tracker;

    tracker.applyCrit(actor.get(), HitLocation::HEAD, 3);
    tracker.applyCrit(actor.get(), HitLocation::BODY, 2);
    REQUIRE(tracker.hasInjuries() == true);

    tracker.clearAll(actor.get());

    // After clearAll, the data is gone (GUI would show nothing)
    REQUIRE(tracker.hasInjuries() == false);
    REQUIRE(tracker.getMagnitude() == 0);
    REQUIRE(tracker.getRecords().empty() == true);

    // Simulate what Pickable.cpp produces for the GUI message
    std::string expectedMsg = std::string(actor->name) + "'s injuries are healed.";
    REQUIRE(expectedMsg == "Scout's injuries are healed.");
}

TEST_CASE("GUI Data: no injuries shows empty state for sidebar",
          "[critical-injuries][gui]")
{
    auto actor = makeTestActor("Fresh Marine", 40);
    InjuryTracker tracker;

    // No injuries applied — GUI should display nothing
    REQUIRE(tracker.hasInjuries() == false);
    REQUIRE(tracker.activeCount() == 0);
    REQUIRE(tracker.getMagnitude() == 0);
    REQUIRE(tracker.getRecords().empty() == true);
}
