#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <algorithm>
#include <cstring>
#include <numeric>

// ─── Property-based tests for hit chance system ──────────────────────────────

// Feature: hit-chance-system, Property 1: Skill value clamping
// **Validates: Requirements 1.1**
TEST_CASE("PBT: skill value is clamped to [1, 99]", "[hit-chance][pbt]")
{
    rc::prop("for any int v in [-1000, 1000], Attacker skill == clamp(v, 1, 99)", []() {
        const int v = *rc::gen::inRange(-1000, 1000);
        Attacker a(5.0f, v);

        const int expected = std::max(1, std::min(99, v));
        RC_ASSERT(a.skillValue == expected);
    });
}

// Feature: hit-chance-system, Property 2: Hit roll range
// **Validates: Requirements 2.1**
TEST_CASE("PBT: defaultRoll via rollD100 always returns [1, 100]", "[hit-chance][pbt]")
{
    rc::prop("rollD100() returns values in [1, 100] across 100+ calls", []() {
        Attacker a(5.0f); // default-constructed uses defaultRoll internally
        for (int i = 0; i < 120; ++i) {
            const int roll = a.rollD100();
            RC_ASSERT(roll >= 1);
            RC_ASSERT(roll <= 100);
        }
    });
}

// Feature: hit-chance-system, Property 4: Threshold computation
// **Validates: Requirements 2.4, 5.1, 5.2**
TEST_CASE("PBT: computeThreshold equals clamp(skill + sum(modifiers), 1, 99)", "[hit-chance][pbt]")
{
    rc::prop("threshold = clamp(skill + sum(mods), 1, 99)", []() {
        const int skill = *rc::gen::inRange(1, 99);
        const auto mods = *rc::gen::container<int>(0, 10, rc::gen::inRange(-99, 99));

        Attacker a(5.0f, skill);
        for (int m : mods) {
            a.addModifier(m);
        }

        const int sum = std::accumulate(mods.begin(), mods.end(), 0);
        const int expected = std::max(1, std::min(99, skill + sum));
        RC_ASSERT(a.computeThreshold() == expected);
    });
}

// ─── Task 2.5: Property 3 — Hit determination correctness ────────────────────
// **Validates: Requirements 2.2, 2.3, 4.3, 7.1, 7.2, 7.3**
TEST_CASE("PBT: hit determination correctness (roll <= threshold hits, roll > threshold misses)", "[hit-chance][pbt]")
{
    rc::prop("attack hits iff roll <= computeThreshold()", []() {
        const int skill = *rc::gen::inRange(1, 99);
        const auto mods = *rc::gen::container<int>(0, 5, rc::gen::inRange(-50, 50));
        const int roll  = *rc::gen::inRange(1, 100);

        // Build attacker with injected roll
        Attacker attacker(5.0f, skill);
        for (int m : mods) {
            attacker.addModifier(m);
        }
        attacker.rollD100 = [roll]() { return roll; };

        const int threshold = attacker.computeThreshold();

        // Build owner and target actors
        Actor owner(0, 0, '@', "Owner", Colors::white);
        owner.attacker = std::shared_ptr<Attacker>(&attacker, [](Attacker*){});

        MonsterDestructible targetDest(1000.0f, 0.0f, "corpse", 0);
        Actor target(1, 1, 'g', "Target", Colors::white);
        target.destructible = std::shared_ptr<Destructible>(&targetDest, [](Destructible*){});

        const float hpBefore = targetDest.hp;
        attacker.attack(&owner, &target);
        const float hpAfter = targetDest.hp;

        if (roll <= threshold) {
            // Hit: HP should decrease (power=5, def=0 → damage=5)
            RC_ASSERT(hpAfter < hpBefore);
        } else {
            // Miss: HP unchanged
            RC_ASSERT(hpAfter == hpBefore);
        }
    });
}

// ─── Task 4.2: Property 6 — Save/load round trip ─────────────────────────────
// **Validates: Requirements 6.1, 6.2**
TEST_CASE("PBT: save/load round trip preserves power and skillValue", "[hit-chance][pbt]")
{
    rc::prop("save then load produces identical power and skillValue", []() {
        // Generate power as int [1, 500] / 10.0f to get [0.1, 50.0]
        const int powerInt = *rc::gen::inRange(1, 500);
        const float power = powerInt / 10.0f;
        const int skill = *rc::gen::inRange(1, 99);

        Attacker original(power, skill);

        // Save
        TCODZip zip;
        original.save(zip);

        // Save to buffer and reload
        // TCODZip doesn't have saveToBuffer, so save to a temp file
        zip.saveToFile("__test_attacker_roundtrip.sav");

        TCODZip zip2;
        zip2.loadFromFile("__test_attacker_roundtrip.sav");

        Attacker loaded(1.0f, 40);
        loaded.load(zip2);

        RC_ASSERT(loaded.power == original.power);
        RC_ASSERT(loaded.skillValue == original.skillValue);
    });
}

// ─── Task 4.3: Unit test — old save backward compatibility ───────────────────
TEST_CASE("Old save format (power only) loads with skillValue defaulting to 40", "[hit-chance][save]")
{
    const float oldPower = 7.5f;

    // Simulate old save format: just a float for power (no sentinel)
    TCODZip zip;
    // Old format wrote power as a float directly. In the new load(), the first
    // thing read is an int. If it's not the sentinel (-1), it's reinterpreted
    // as the old power float via memcpy. So we write the power float's bytes as an int.
    int powerAsInt;
    std::memcpy(&powerAsInt, &oldPower, sizeof(float));
    zip.putInt(powerAsInt);

    zip.saveToFile("__test_attacker_old_format.sav");

    TCODZip zip2;
    zip2.loadFromFile("__test_attacker_old_format.sav");

    Attacker loaded(1.0f, 50);
    loaded.load(zip2);

    REQUIRE(loaded.power == Catch::Approx(oldPower));
    REQUIRE(loaded.skillValue == 40);
}

// ─── Task 8.1: Unit test — miss skips damage (HP unchanged) ──────────────────
TEST_CASE("Miss skips damage: target HP unchanged when roll > threshold", "[hit-chance]")
{
    Attacker attacker(8.0f, 50);
    attacker.rollD100 = []() { return 100; }; // guaranteed miss (100 > 50)

    Actor owner(0, 0, '@', "Attacker", Colors::white);
    owner.attacker = std::shared_ptr<Attacker>(&attacker, [](Attacker*){});

    MonsterDestructible targetDest(20.0f, 0.0f, "corpse", 0);
    Actor target(1, 1, 'g', "Target", Colors::white);
    target.destructible = std::shared_ptr<Destructible>(&targetDest, [](Destructible*){});

    attacker.attack(&owner, &target);

    REQUIRE(targetDest.hp == Catch::Approx(20.0f));
}

// ─── Task 8.2: Unit test — hit applies damage normally ───────────────────────
TEST_CASE("Hit applies damage: target HP reduced by (power - defense)", "[hit-chance]")
{
    Attacker attacker(8.0f, 50);
    attacker.rollD100 = []() { return 1; }; // guaranteed hit (1 <= 50)

    Actor owner(0, 0, '@', "Attacker", Colors::white);
    owner.attacker = std::shared_ptr<Attacker>(&attacker, [](Attacker*){});

    MonsterDestructible targetDest(20.0f, 3.0f, "corpse", 0);
    Actor target(1, 1, 'g', "Target", Colors::white);
    target.destructible = std::shared_ptr<Destructible>(&targetDest, [](Destructible*){});

    attacker.attack(&owner, &target);

    // Damage formula in takeDamage: effective = power - defense = 8 - 3 = 5
    // HP: 20 - 5 = 15
    REQUIRE(targetDest.hp == Catch::Approx(15.0f));
}

// ─── Task 8.3: Unit test — dead target skips hit roll ────────────────────────
TEST_CASE("Dead target: attack skips hit roll entirely", "[hit-chance]")
{
    int rollCallCount = 0;
    Attacker attacker(8.0f, 50);
    attacker.rollD100 = [&rollCallCount]() { ++rollCallCount; return 1; };

    Actor owner(0, 0, '@', "Attacker", Colors::white);
    owner.attacker = std::shared_ptr<Attacker>(&attacker, [](Attacker*){});

    // Target is already dead (HP <= 0)
    MonsterDestructible targetDest(20.0f, 0.0f, "corpse", 0);
    targetDest.hp = 0.0f; // dead
    Actor target(1, 1, 'g', "Target", Colors::white);
    target.destructible = std::shared_ptr<Destructible>(&targetDest, [](Destructible*){});

    attacker.attack(&owner, &target);

    REQUIRE(rollCallCount == 0);
}

// ─── Task 8.4: Property 5 — miss doesn't crash for any actor name ────────────
// **Validates: Requirements 3.1, 3.2, 3.3, 3.4**
TEST_CASE("PBT: miss path does not crash for any actor names", "[hit-chance][pbt]")
{
    rc::prop("attack miss completes without crashing for random names", []() {
        auto ownerName = *rc::gen::string(1, 10);
        auto targetName = *rc::gen::string(1, 10);

        Attacker attacker(5.0f, 50);
        attacker.rollD100 = []() { return 100; }; // guaranteed miss

        Actor owner(0, 0, '@', ownerName, Colors::white);
        owner.attacker = std::shared_ptr<Attacker>(&attacker, [](Attacker*){});

        MonsterDestructible targetDest(100.0f, 0.0f, "corpse", 0);
        Actor target(1, 1, 'g', targetName, Colors::white);
        target.destructible = std::shared_ptr<Destructible>(&targetDest, [](Destructible*){});

        // Should not crash — miss path logs message and returns
        attacker.attack(&owner, &target);

        // Verify no damage was dealt (confirming miss path taken)
        RC_ASSERT(targetDest.hp == 100.0f);
    });
}
