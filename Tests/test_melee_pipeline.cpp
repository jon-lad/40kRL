#include "lib/catch_amalgamated.hpp"
#include "main.h"

#include <memory>

// ─── Full pipeline integration tests for Rogue Trader melee combat ───────────
// Validates: Requirements 1.1, 4.1, 5.5, 10.1, 10.5

// Helper: creates an Actor with Attacker, Destructible, and Characteristics.
// WS, S, Ag, T can be configured. Returns a shared_ptr-managed Actor.
static std::unique_ptr<Actor> makeCharacter(
    const std::string& name, int ws, int s, int ag, int t, float hp)
{
    auto actor = std::make_unique<Actor>(0, 0, '@', name, Colors::white);

    auto chars = std::make_shared<Characteristics>(30);
    chars->set(CharId::WS, ws);
    chars->set(CharId::S, s);
    chars->set(CharId::Ag, ag);
    chars->set(CharId::T, t);
    actor->characteristics = chars;

    auto dest = std::make_shared<MonsterDestructible>(hp, 0.0f, "corpse", 0);
    actor->destructible = dest;

    auto atk = std::make_shared<Attacker>(0.0f, ws);
    actor->attacker = atk;

    return actor;
}

// Helper: creates a bare destructible Actor (no Characteristics — like a crate).
static std::unique_ptr<Actor> makeDestructible(const std::string& name, float hp)
{
    auto actor = std::make_unique<Actor>(0, 0, '#', name, Colors::white);
    auto dest = std::make_shared<MonsterDestructible>(hp, 0.0f, "remains", 0);
    actor->destructible = dest;
    return actor;
}

// Helper: creates a melee weapon Actor with specified MeleeStats.
static std::unique_ptr<Actor> makeMeleeWeapon(
    const std::string& name, DiceSpec dice, int pen)
{
    auto weapon = std::make_unique<Actor>(0, 0, '/', name, Colors::white);
    auto equip = std::make_shared<Equippable>(
        EquipmentSlot::WEAPON,
        StatModifiers{0.0f, 0.0f, 0.0f, 0},
        1.0f, 10);
    equip->meleeStats = MeleeStats{dice, pen, {}};
    weapon->equippable = equip;
    return weapon;
}

// ─── Test: Hit, dodge fails, parry fails, damage dealt ───────────────────────
TEST_CASE("Pipeline: hit + dodge fail + parry fail = damage dealt", "[melee-pipeline]")
{
    auto owner = makeCharacter("Attacker", 50, 40, 30, 30, 20.0f);
    auto target = makeCharacter("Target", 40, 30, 30, 30, 20.0f);

    // Equip target with a melee weapon (so parry is attempted)
    auto weapon = makeMeleeWeapon("Sword", DiceSpec{1, 10}, 0);
    target->equipment = std::make_unique<Equipment>();
    target->equipment->equip(weapon.get(), nullptr, nullptr);

    // Roll sequence: hit(10), dodge(99), parry(99)
    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 10;  // hit roll: 10 <= WS 50, success
            case 2: return 99;  // dodge roll: 99 > Ag 30, fail
            case 3: return 99;  // parry roll: 99 > WS 40, fail
            default: return 99;
        }
    };
    // Weapon die always returns 5 (unarmed: 1d5)
    owner->attacker->rollDie = [](int sides) { return 5; };

    const float hpBefore = target->destructible->hp;
    owner->attacker->attack(owner.get(), target.get());

    // rawDamage = 5 (die) + 4 (SB from S=40) = 9
    // effectiveArmour = 0 (no armour equipped)
    // TB = 3 (T=30)
    // finalDamage = max(0, 9 - 0 - 3) = 6
    REQUIRE(target->destructible->hp == Catch::Approx(hpBefore - 6.0f));
}

// ─── Test: Miss — HP unchanged ───────────────────────────────────────────────
TEST_CASE("Pipeline: miss leaves target HP unchanged", "[melee-pipeline]")
{
    auto owner = makeCharacter("Attacker", 50, 40, 30, 30, 20.0f);
    auto target = makeCharacter("Target", 40, 30, 30, 30, 20.0f);

    // Roll: 80 > WS 50 → miss
    owner->attacker->rollD100 = []() { return 80; };
    owner->attacker->rollDie = [](int sides) { return 5; };

    const float hpBefore = target->destructible->hp;
    owner->attacker->attack(owner.get(), target.get());

    REQUIRE(target->destructible->hp == Catch::Approx(hpBefore));
}

// ─── Test: Dodge success — HP unchanged ──────────────────────────────────────
TEST_CASE("Pipeline: dodge success negates hit, HP unchanged", "[melee-pipeline]")
{
    auto owner = makeCharacter("Attacker", 50, 40, 30, 30, 20.0f);
    auto target = makeCharacter("Target", 40, 30, 60, 30, 20.0f); // Ag=60

    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 10;  // hit roll: success
            case 2: return 20;  // dodge roll: 20 <= Ag 60, success
            default: return 99;
        }
    };
    owner->attacker->rollDie = [](int sides) { return 5; };

    const float hpBefore = target->destructible->hp;
    owner->attacker->attack(owner.get(), target.get());

    REQUIRE(target->destructible->hp == Catch::Approx(hpBefore));
}

// ─── Test: Parry success with melee weapon — HP unchanged ────────────────────
TEST_CASE("Pipeline: parry success negates hit, HP unchanged", "[melee-pipeline]")
{
    auto owner = makeCharacter("Attacker", 50, 40, 30, 30, 20.0f);
    auto target = makeCharacter("Target", 60, 30, 30, 30, 20.0f); // WS=60 for parry

    // Equip target with a melee weapon so parry is attempted
    auto weapon = makeMeleeWeapon("Sword", DiceSpec{1, 10}, 0);
    target->equipment = std::make_unique<Equipment>();
    target->equipment->equip(weapon.get(), nullptr, nullptr);

    int rollIdx = 0;
    owner->attacker->rollD100 = [&rollIdx]() {
        ++rollIdx;
        switch (rollIdx) {
            case 1: return 10;  // hit roll: success (10 <= 50)
            case 2: return 99;  // dodge roll: fail (99 > Ag 30)
            case 3: return 20;  // parry roll: success (20 <= WS 60)
            default: return 99;
        }
    };
    owner->attacker->rollDie = [](int sides) { return 5; };

    const float hpBefore = target->destructible->hp;
    owner->attacker->attack(owner.get(), target.get());

    REQUIRE(target->destructible->hp == Catch::Approx(hpBefore));
}

// ─── Test: No parry when unarmed (parry skipped) ─────────────────────────────
TEST_CASE("Pipeline: parry skipped when target has no melee weapon", "[melee-pipeline]")
{
    auto owner = makeCharacter("Attacker", 50, 40, 30, 30, 20.0f);
    auto target = makeCharacter("Target", 99, 30, 30, 30, 20.0f); // WS=99 but no weapon

    // No equipment on target → parry should be skipped
    // Only 2 d100 rolls consumed: hit + dodge (no parry)
    int rollCount = 0;
    owner->attacker->rollD100 = [&rollCount]() {
        ++rollCount;
        switch (rollCount) {
            case 1: return 10;  // hit roll: success
            case 2: return 99;  // dodge roll: fail (99 > Ag 30)
            default: return 1;  // should NOT be reached (parry skipped)
        }
    };
    owner->attacker->rollDie = [](int sides) { return 5; };

    owner->attacker->attack(owner.get(), target.get());

    // Only 2 d100 rolls should have been consumed (hit + dodge, no parry)
    REQUIRE(rollCount == 2);
}

// ─── Test: Destructible auto-hit (no Characteristics on target) ──────────────
TEST_CASE("Pipeline: destructible auto-hit bypasses hit roll", "[melee-pipeline]")
{
    auto owner = makeCharacter("Attacker", 50, 40, 30, 30, 20.0f);
    auto target = makeDestructible("Crate", 50.0f);

    // No d100 roll should be consumed for auto-hit path
    int d100Count = 0;
    owner->attacker->rollD100 = [&d100Count]() { ++d100Count; return 50; };
    owner->attacker->rollDie = [](int sides) { return 5; };

    const float hpBefore = target->destructible->hp;
    owner->attacker->attack(owner.get(), target.get());

    // No d100 rolls consumed (auto-hit)
    REQUIRE(d100Count == 0);

    // Damage = dieRoll(5) + SB(4 from S=40) = 9
    // takeDamage applies directly (no armour, no TB for destructibles)
    REQUIRE(target->destructible->hp == Catch::Approx(hpBefore - 9.0f));
}

// ─── Test: Default SB of 3 when attacker has no Characteristics ──────────────
TEST_CASE("Pipeline: default SB of 3 for attacker without Characteristics", "[melee-pipeline]")
{
    // Create attacker without Characteristics
    auto owner = std::make_unique<Actor>(0, 0, '^', "Trap", Colors::white);
    auto atk = std::make_shared<Attacker>(0.0f, 40);
    owner->attacker = atk;
    // No characteristics on owner!

    auto target = makeDestructible("Barrel", 50.0f);

    owner->attacker->rollD100 = []() { return 50; }; // shouldn't be called
    owner->attacker->rollDie = [](int sides) { return 4; }; // die returns 4

    const float hpBefore = target->destructible->hp;
    owner->attacker->attack(owner.get(), target.get());

    // Damage = dieRoll(4) + defaultSB(3) = 7
    REQUIRE(target->destructible->hp == Catch::Approx(hpBefore - 7.0f));
}

// ─── Test: Dead target early return ──────────────────────────────────────────
TEST_CASE("Pipeline: dead target causes early return, no crash, no HP change", "[melee-pipeline]")
{
    auto owner = makeCharacter("Attacker", 50, 40, 30, 30, 20.0f);
    auto target = makeCharacter("Target", 40, 30, 30, 30, 20.0f);

    // Kill target
    target->destructible->hp = 0.0f;

    int rollCount = 0;
    owner->attacker->rollD100 = [&rollCount]() { ++rollCount; return 10; };
    owner->attacker->rollDie = [](int sides) { return 5; };

    owner->attacker->attack(owner.get(), target.get());

    // No rolls consumed — early return
    REQUIRE(rollCount == 0);
    // HP unchanged (still 0)
    REQUIRE(target->destructible->hp == Catch::Approx(0.0f));
}
