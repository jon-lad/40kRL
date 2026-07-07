#include "lib/catch_amalgamated.hpp"
#include "main.h"

#include <memory>

// ═══════════════════════════════════════════════════════════════════════════════
// Unit Tests for Shoot ('s') and Reload ('r') Input Handling
// ═══════════════════════════════════════════════════════════════════════════════
//
// These tests verify the player input validation logic in PlayerAi::handleActionKey
// for the 's' (shoot) and 'r' (reload) key cases.
//
// Validates: Requirements 1.1, 1.2, 1.3, 2.1, 2.2, 2.3, 2.4

// ─── Helper: Create a minimal actor with equipment component ─────────────────

static std::unique_ptr<Actor> makePlayerWithEquipment() {
    auto actor = std::make_unique<Actor>(5, 5, '@', "TestPlayer", Colors::white);
    actor->equipment = std::make_unique<Equipment>();
    actor->container = std::make_unique<Container>(26);
    actor->ai = std::make_shared<PlayerAi>();
    actor->attacker = std::make_shared<Attacker>(3.0f);
    actor->destructible = std::make_shared<PlayerDestructible>(30.0f, 1.0f, "your cadaver", 0);
    return actor;
}

static std::unique_ptr<Actor> makeRangedWeapon(int clipSize, int currentAmmo) {
    auto weapon = std::make_unique<Actor>(0, 0, ')', "Laspistol", Colors::lightRed);
    StatModifiers mods{0.0f, 0.0f, 0.0f, 0};
    weapon->equippable = std::make_shared<Equippable>(EquipmentSlot::WEAPON, mods, 1.5f, 20);

    RangedStats rs;
    rs.damageDice = DiceSpec{1, 10};
    rs.penetration = 0;
    rs.range = 30;
    rs.rateOfFire = 1;
    rs.clipSize = clipSize;
    rs.reloadTime = 1;
    weapon->equippable->rangedStats = rs;
    weapon->equippable->currentAmmo = currentAmmo;

    return weapon;
}

static std::unique_ptr<Actor> makeMeleeOnlyWeapon() {
    auto weapon = std::make_unique<Actor>(0, 0, '/', "Chainsword", Colors::lightBlue);
    StatModifiers mods{3.0f, 0.0f, 0.0f, 0};
    weapon->equippable = std::make_shared<Equippable>(EquipmentSlot::WEAPON, mods, 3.5f, 50);

    MeleeStats ms;
    ms.damageDice = DiceSpec{1, 10};
    ms.penetration = 2;
    ms.qualities = {"Tearing"};
    weapon->equippable->meleeStats = ms;
    // No rangedStats — melee only

    return weapon;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 's' Key (Shoot) Tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Shoot input: 's' with no weapon equipped shows 'no ranged weapon' message", "[ranged][input]") {
    // Setup: player has equipment component but no weapon in slot
    auto player = makePlayerWithEquipment();
    Actor* owner = player.get();

    // Ensure the engine is in a consistent state
    engine.gameStatus = Engine::IDLE;
    engine.targetingCtx = std::nullopt;

    // Verify precondition: no weapon in slot
    REQUIRE(owner->equipment->getSlot(EquipmentSlot::WEAPON) == nullptr);

    // Act: call handleActionKey with 's'
    auto* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
    REQUIRE(playerAi != nullptr);
    playerAi->handleActionKey(owner, 's');

    // Assert: state remains unchanged (no targeting mode entered)
    CHECK(engine.gameStatus == Engine::IDLE);
    CHECK_FALSE(engine.targetingCtx.has_value());
}

TEST_CASE("Shoot input: 's' with melee-only weapon shows 'no ranged weapon' message", "[ranged][input]") {
    // Setup: player has a melee weapon (no rangedStats)
    auto player = makePlayerWithEquipment();
    Actor* owner = player.get();

    auto weapon = makeMeleeOnlyWeapon();
    Actor* weaponPtr = weapon.get();
    owner->equipment->ownedItems.push_back(std::move(weapon));
    owner->equipment->equip(weaponPtr, nullptr, owner->attacker.get());

    engine.gameStatus = Engine::IDLE;
    engine.targetingCtx = std::nullopt;

    // Verify precondition: weapon is equipped but has no rangedStats
    Actor* equipped = owner->equipment->getSlot(EquipmentSlot::WEAPON);
    REQUIRE(equipped != nullptr);
    REQUIRE(equipped->equippable != nullptr);
    REQUIRE_FALSE(equipped->equippable->rangedStats.has_value());

    // Act
    auto* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
    playerAi->handleActionKey(owner, 's');

    // Assert: no targeting entered
    CHECK(engine.gameStatus == Engine::IDLE);
    CHECK_FALSE(engine.targetingCtx.has_value());
}

TEST_CASE("Shoot input: 's' with zero ammo shows 'weapon is empty' message", "[ranged][input]") {
    // Setup: player has ranged weapon but zero ammo
    auto player = makePlayerWithEquipment();
    Actor* owner = player.get();

    auto weapon = makeRangedWeapon(30, 0);  // clipSize=30, currentAmmo=0
    Actor* weaponPtr = weapon.get();
    owner->equipment->ownedItems.push_back(std::move(weapon));
    owner->equipment->equip(weaponPtr, nullptr, owner->attacker.get());

    engine.gameStatus = Engine::IDLE;
    engine.targetingCtx = std::nullopt;

    // Verify precondition: weapon equipped, has rangedStats, zero ammo
    Actor* equipped = owner->equipment->getSlot(EquipmentSlot::WEAPON);
    REQUIRE(equipped != nullptr);
    REQUIRE(equipped->equippable->rangedStats.has_value());
    REQUIRE(equipped->equippable->currentAmmo == 0);

    // Act
    auto* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
    playerAi->handleActionKey(owner, 's');

    // Assert: no targeting entered (weapon empty blocks it)
    CHECK(engine.gameStatus == Engine::IDLE);
    CHECK_FALSE(engine.targetingCtx.has_value());
}

TEST_CASE("Shoot input: 's' with valid ranged weapon and ammo enters TARGETING state", "[ranged][input]") {
    // Setup: player has ranged weapon with ammo
    auto player = makePlayerWithEquipment();
    Actor* owner = player.get();

    auto weapon = makeRangedWeapon(30, 15);  // clipSize=30, currentAmmo=15
    Actor* weaponPtr = weapon.get();
    owner->equipment->ownedItems.push_back(std::move(weapon));
    owner->equipment->equip(weaponPtr, nullptr, owner->attacker.get());

    engine.gameStatus = Engine::IDLE;
    engine.targetingCtx = std::nullopt;

    // Verify preconditions
    Actor* equipped = owner->equipment->getSlot(EquipmentSlot::WEAPON);
    REQUIRE(equipped != nullptr);
    REQUIRE(equipped->equippable->rangedStats.has_value());
    REQUIRE(equipped->equippable->currentAmmo > 0);

    // Act
    auto* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
    playerAi->handleActionKey(owner, 's');

    // Assert: game transitions to TARGETING state
    CHECK(engine.gameStatus == Engine::TARGETING);
    REQUIRE(engine.targetingCtx.has_value());

    // Verify targeting context is correctly populated
    CHECK(engine.targetingCtx->item == equipped);
    CHECK(engine.targetingCtx->owner == owner);
    CHECK(engine.targetingCtx->maxRange == static_cast<float>(equipped->equippable->rangedStats->range));
    CHECK(engine.targetingCtx->type == TargetSelector::SelectorType::SELECTED_MONSTER);
    CHECK(engine.targetingCtx->isRangedAttack == true);

    // Cleanup
    engine.gameStatus = Engine::IDLE;
    engine.targetingCtx = std::nullopt;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 'r' Key (Reload) Tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Reload input: 'r' with no weapon equipped shows 'no ranged weapon to reload' message", "[ranged][input]") {
    // Setup: player has no weapon
    auto player = makePlayerWithEquipment();
    Actor* owner = player.get();

    engine.gameStatus = Engine::IDLE;

    // Verify precondition
    REQUIRE(owner->equipment->getSlot(EquipmentSlot::WEAPON) == nullptr);

    // Act
    auto* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
    playerAi->handleActionKey(owner, 'r');

    // Assert: state remains IDLE (reload blocked)
    CHECK(engine.gameStatus == Engine::IDLE);
}

TEST_CASE("Reload input: 'r' with melee-only weapon shows 'no ranged weapon to reload' message", "[ranged][input]") {
    // Setup: player has melee weapon only
    auto player = makePlayerWithEquipment();
    Actor* owner = player.get();

    auto weapon = makeMeleeOnlyWeapon();
    Actor* weaponPtr = weapon.get();
    owner->equipment->ownedItems.push_back(std::move(weapon));
    owner->equipment->equip(weaponPtr, nullptr, owner->attacker.get());

    engine.gameStatus = Engine::IDLE;

    // Verify precondition
    Actor* equipped = owner->equipment->getSlot(EquipmentSlot::WEAPON);
    REQUIRE(equipped != nullptr);
    REQUIRE_FALSE(equipped->equippable->rangedStats.has_value());

    // Act
    auto* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
    playerAi->handleActionKey(owner, 'r');

    // Assert: state remains IDLE
    CHECK(engine.gameStatus == Engine::IDLE);
}

TEST_CASE("Reload input: 'r' with full ammo shows 'already fully loaded' message", "[ranged][input]") {
    // Setup: player has ranged weapon with full ammo
    auto player = makePlayerWithEquipment();
    Actor* owner = player.get();

    auto weapon = makeRangedWeapon(30, 30);  // clipSize=30, currentAmmo=30 (full)
    Actor* weaponPtr = weapon.get();
    owner->equipment->ownedItems.push_back(std::move(weapon));
    owner->equipment->equip(weaponPtr, nullptr, owner->attacker.get());

    engine.gameStatus = Engine::IDLE;

    // Verify precondition
    Actor* equipped = owner->equipment->getSlot(EquipmentSlot::WEAPON);
    REQUIRE(equipped != nullptr);
    REQUIRE(equipped->equippable->rangedStats.has_value());
    REQUIRE(equipped->equippable->currentAmmo >= equipped->equippable->rangedStats->clipSize);

    // Act
    auto* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
    playerAi->handleActionKey(owner, 'r');

    // Assert: state remains IDLE (reload blocked because already full)
    CHECK(engine.gameStatus == Engine::IDLE);
    // Ammo unchanged
    CHECK(equipped->equippable->currentAmmo == 30);
}

TEST_CASE("Reload input: 'r' with partial ammo reloads to full and advances turn", "[ranged][input]") {
    // Setup: player has ranged weapon with partial ammo
    auto player = makePlayerWithEquipment();
    Actor* owner = player.get();

    auto weapon = makeRangedWeapon(30, 10);  // clipSize=30, currentAmmo=10
    Actor* weaponPtr = weapon.get();
    owner->equipment->ownedItems.push_back(std::move(weapon));
    owner->equipment->equip(weaponPtr, nullptr, owner->attacker.get());

    engine.gameStatus = Engine::IDLE;

    // Verify precondition
    Actor* equipped = owner->equipment->getSlot(EquipmentSlot::WEAPON);
    REQUIRE(equipped != nullptr);
    REQUIRE(equipped->equippable->rangedStats.has_value());
    REQUIRE(equipped->equippable->currentAmmo < equipped->equippable->rangedStats->clipSize);

    // Act
    auto* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
    playerAi->handleActionKey(owner, 'r');

    // Assert: ammo is now full
    CHECK(equipped->equippable->currentAmmo == equipped->equippable->rangedStats->clipSize);
    CHECK(equipped->equippable->currentAmmo == 30);

    // Assert: turn advances (gameStatus == NEW_TURN)
    CHECK(engine.gameStatus == Engine::NEW_TURN);

    // Cleanup
    engine.gameStatus = Engine::IDLE;
}

TEST_CASE("Reload input: 'r' with zero ammo reloads to full and advances turn", "[ranged][input]") {
    // Setup: player has ranged weapon with zero ammo (completely empty)
    auto player = makePlayerWithEquipment();
    Actor* owner = player.get();

    auto weapon = makeRangedWeapon(6, 0);  // clipSize=6, currentAmmo=0
    Actor* weaponPtr = weapon.get();
    owner->equipment->ownedItems.push_back(std::move(weapon));
    owner->equipment->equip(weaponPtr, nullptr, owner->attacker.get());

    engine.gameStatus = Engine::IDLE;

    // Verify precondition
    Actor* equipped = owner->equipment->getSlot(EquipmentSlot::WEAPON);
    REQUIRE(equipped != nullptr);
    REQUIRE(equipped->equippable->currentAmmo == 0);

    // Act
    auto* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
    playerAi->handleActionKey(owner, 'r');

    // Assert: ammo restored to clipSize
    CHECK(equipped->equippable->currentAmmo == 6);

    // Assert: turn advances
    CHECK(engine.gameStatus == Engine::NEW_TURN);

    // Cleanup
    engine.gameStatus = Engine::IDLE;
}
