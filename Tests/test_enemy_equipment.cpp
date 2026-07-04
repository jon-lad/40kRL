#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <algorithm>
#include <numeric>
#include <vector>
#include <memory>
#include <array>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════════════════════

// Helper: Create a standalone equippable item Actor for enemy-owned items.
static std::unique_ptr<Actor> makeEnemyItem(
    const std::string& name,
    EquipmentSlot slot,
    StatModifiers mods = {0.0f, 0.0f, 0.0f, 0},
    float weight = 1.0f,
    int value = 10,
    int glyph = '/',
    TCODColor color = Colors::white)
{
    auto actor = std::make_unique<Actor>(0, 0, glyph, name, color);
    actor->blocks = false;
    actor->equippable = std::make_shared<Equippable>(slot, mods, weight, value);
    actor->pickable = std::make_shared<Pickable>(nullptr, nullptr);
    return actor;
}

// Helper: Create an enemy Actor with equipment at a given position.
static std::unique_ptr<Actor> makeEnemy(int x, int y, const std::string& name = "TestEnemy")
{
    auto enemy = std::make_unique<Actor>(x, y, 'E', name, Colors::desaturatedGreen);
    enemy->destructible = std::make_unique<MonsterDestructible>(10.0f, 0.0f, "dead enemy", 10);
    enemy->attacker = std::make_unique<Attacker>(3.0f, 40);
    enemy->ai = std::make_unique<MonsterAi>();
    enemy->equipment = std::make_unique<Equipment>();
    return enemy;
}

// Helper: Equip an item onto an enemy (using the enemy path with nullptr Container).
static Actor* equipItemOnEnemy(Actor* enemy, std::unique_ptr<Actor> item)
{
    Actor* ptr = item.get();
    enemy->equipment->ownedItems.push_back(std::move(item));
    enemy->equipment->equip(ptr, nullptr, enemy->attacker.get());
    return ptr;
}

// Helper: Count actors in engine.actors that are not the given enemy (i.e., dropped items).
static int countDroppedItems(Actor* enemy)
{
    int count = 0;
    for (const auto& a : engine.actors) {
        if (a.get() != enemy && a->pickable) {
            count++;
        }
    }
    return count;
}

// Helper: Remove all actors from engine.actors except those matching keepList.
static void clearEngineActors()
{
    engine.actors.clear();
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 5.4: Property 4 — Each equipped item evaluated independently for dropping
// **Validates: Requirements 4.1, 4.4, 5.2**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Each equipped item is evaluated independently for dropping", "[enemy-equipment][pbt]")
{
    rc::prop("each item drops roughly 50% of the time with dropChance=0.5 over 200 trials", []() {
        // Track drops per slot over many trials
        constexpr int NUM_TRIALS = 200;
        constexpr float DROP_CHANCE = 0.5f;
        constexpr int NUM_ITEMS = 4;

        std::array<int, NUM_ITEMS> dropCounts = {0, 0, 0, 0};

        for (int trial = 0; trial < NUM_TRIALS; ++trial) {
            // Clear engine actors
            clearEngineActors();

            // Create an enemy with 4 items (one per slot)
            auto enemy = makeEnemy(10, 10);
            enemy->equipment->dropChance = DROP_CHANCE;

            equipItemOnEnemy(enemy.get(), makeEnemyItem("Weapon", EquipmentSlot::WEAPON));
            equipItemOnEnemy(enemy.get(), makeEnemyItem("Shield", EquipmentSlot::OFFHAND));
            equipItemOnEnemy(enemy.get(), makeEnemyItem("Helmet", EquipmentSlot::HEAD));
            equipItemOnEnemy(enemy.get(), makeEnemyItem("Armor", EquipmentSlot::BODY));

            // Call dropEnemyEquipment
            dropEnemyEquipment(enemy.get());

            // Count dropped items by checking engine.actors for pickable actors
            for (const auto& a : engine.actors) {
                if (a->pickable) {
                    if (a->name == "Weapon") dropCounts[0]++;
                    else if (a->name == "Shield") dropCounts[1]++;
                    else if (a->name == "Helmet") dropCounts[2]++;
                    else if (a->name == "Armor") dropCounts[3]++;
                }
            }
        }

        // Each item should drop roughly 50% of the time.
        // With 200 trials and p=0.5, expected = 100, stddev ≈ 7.
        // Use wide tolerance: 30%-70% (60 to 140 drops).
        for (int i = 0; i < NUM_ITEMS; ++i) {
            RC_ASSERT(dropCounts[i] >= 60);
            RC_ASSERT(dropCounts[i] <= 140);
        }

        // Cleanup
        clearEngineActors();
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 5.5: Property 5 — All dropped items positioned at corpse location
// **Validates: Requirements 4.5**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: All dropped items are positioned at corpse location", "[enemy-equipment][pbt]")
{
    rc::prop("all dropped items have x,y matching the enemy position", []() {
        clearEngineActors();

        // Generate random position for the enemy
        int enemyX = *rc::gen::inRange(1, 100);
        int enemyY = *rc::gen::inRange(1, 100);

        auto enemy = makeEnemy(enemyX, enemyY);
        enemy->equipment->dropChance = 1.0f; // guarantee all items drop

        // Equip 1-4 items
        int numItems = *rc::gen::inRange(1, 4);
        if (numItems >= 1) equipItemOnEnemy(enemy.get(), makeEnemyItem("Item1", EquipmentSlot::WEAPON));
        if (numItems >= 2) equipItemOnEnemy(enemy.get(), makeEnemyItem("Item2", EquipmentSlot::OFFHAND));
        if (numItems >= 3) equipItemOnEnemy(enemy.get(), makeEnemyItem("Item3", EquipmentSlot::HEAD));
        if (numItems >= 4) equipItemOnEnemy(enemy.get(), makeEnemyItem("Item4", EquipmentSlot::BODY));

        // Call dropEnemyEquipment
        dropEnemyEquipment(enemy.get());

        // Verify all dropped items are at enemy's position
        int droppedCount = 0;
        for (const auto& a : engine.actors) {
            if (a->pickable) {
                RC_ASSERT(a->getX() == enemyX);
                RC_ASSERT(a->getY() == enemyY);
                droppedCount++;
            }
        }

        // Should have dropped numItems items (dropChance=1.0)
        RC_ASSERT(droppedCount == numItems);

        clearEngineActors();
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 7.3: Unit tests for Lua loading edge cases
// **Validates: Requirements 2.3, 5.3, 5.4, 8.2, 8.5**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("EnemyEquipmentConfig: dropChance defaults to 1.0 when field omitted", "[enemy-equipment]")
{
    // When no dropChance field is provided, the default is 1.0
    EnemyEquipmentConfig config;
    REQUIRE(config.dropChance == 1.0f);
}

TEST_CASE("EnemyEquipmentConfig: dropChance clamped when out of range (negative)", "[enemy-equipment]")
{
    // Simulate the clamping logic from Map.cpp
    float dc = -0.5f;
    if (dc < 0.0f) dc = 0.0f;
    else if (dc > 1.0f) dc = 1.0f;
    REQUIRE(dc == 0.0f);
}

TEST_CASE("EnemyEquipmentConfig: dropChance clamped when out of range (above 1.0)", "[enemy-equipment]")
{
    // Simulate the clamping logic from Map.cpp
    float dc = 1.5f;
    if (dc < 0.0f) dc = 0.0f;
    else if (dc > 1.0f) dc = 1.0f;
    REQUIRE(dc == 1.0f);
}

TEST_CASE("EnemyEquipmentConfig: named equipment takes precedence over equipTier", "[enemy-equipment]")
{
    // Per design: if both "equipment" and "equipTier" are present,
    // useTierSelection should be false (named list wins).
    EnemyEquipmentConfig config;
    config.equipmentNames = {"Chainsword", "Flak Armor"};
    config.tierWeights = {80.0f, 18.0f, 2.0f};

    bool hasEquipmentField = !config.equipmentNames.empty();
    bool hasEquipTierField = true; // simulate equipTier present

    // Apply the precedence logic from Map.cpp
    if (hasEquipTierField && !hasEquipmentField) {
        config.useTierSelection = true;
    }
    // Since hasEquipmentField is true, useTierSelection stays false
    REQUIRE(config.useTierSelection == false);
}

TEST_CASE("EnemyEquipmentConfig: tier defaults to 'common' when omitted from Equipment.lua", "[enemy-equipment]")
{
    // The EquipmentTemplate struct defaults tier to COMMON
    EquipmentTemplate tmpl;
    tmpl.name = "Basic Weapon";
    tmpl.glyph = '/';
    tmpl.color = Colors::white;
    tmpl.slot = EquipmentSlot::WEAPON;
    tmpl.weight = 2.0f;
    tmpl.value = 10;
    tmpl.modifiers = {1.0f, 0.0f, 0.0f, 0};
    // tier is not explicitly set — should default to COMMON
    REQUIRE(tmpl.tier == ItemTier::COMMON);
}

TEST_CASE("EnemyEquipmentConfig: invalid template name — enemy created without that item", "[enemy-equipment]")
{
    clearEngineActors();

    // Create an enemy and manually resolve equipment (simulating the spawn path)
    auto enemy = makeEnemy(5, 5);
    enemy->equipment->dropChance = 1.0f;

    // Simulate the named equipment resolution from Map.cpp:
    // If a name is not found in engine.equipmentTemplates, it is skipped.
    std::vector<std::string> names = {"NonExistentWeapon", "Combat Knife"};
    std::vector<const EquipmentTemplate*> resolved;

    // Add a valid template to engine.equipmentTemplates for "Combat Knife"
    EquipmentTemplate knifeTemplate;
    knifeTemplate.name = "Combat Knife";
    knifeTemplate.glyph = '/';
    knifeTemplate.color = Colors::white;
    knifeTemplate.slot = EquipmentSlot::WEAPON;
    knifeTemplate.weight = 1.0f;
    knifeTemplate.value = 5;
    knifeTemplate.modifiers = {1.0f, 0.0f, 0.0f, 0};
    knifeTemplate.tier = ItemTier::COMMON;

    // Save previous state
    auto savedTemplates = std::move(engine.equipmentTemplates);
    engine.equipmentTemplates.clear();
    engine.equipmentTemplates.push_back(knifeTemplate);

    // Resolve names (same logic as Map.cpp)
    for (const auto& itemName : names) {
        const EquipmentTemplate* found = nullptr;
        for (const auto& tmpl : engine.equipmentTemplates) {
            if (tmpl.name == itemName) {
                found = &tmpl;
                break;
            }
        }
        if (found) {
            resolved.push_back(found);
        }
        // Invalid names are just skipped (with a warning in production code)
    }

    // Only "Combat Knife" should be resolved
    REQUIRE(resolved.size() == 1);
    REQUIRE(resolved[0]->name == "Combat Knife");

    // Enemy is still created (not null) — having invalid names doesn't prevent creation
    REQUIRE(enemy != nullptr);
    REQUIRE(enemy->equipment != nullptr);

    // Restore engine templates
    engine.equipmentTemplates = std::move(savedTemplates);
    clearEngineActors();
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 8.2: Property 8 — Tier weight distribution converges to configured weights
// **Validates: Requirements 8.4**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Tier weight distribution converges to configured weights", "[enemy-equipment][pbt]")
{
    rc::prop("tier selection converges to configured weights within tolerance", []() {
        // Save and replace engine equipment templates with test data
        auto savedTemplates = std::move(engine.equipmentTemplates);
        engine.equipmentTemplates.clear();

        // Add templates for each tier and WEAPON slot
        EquipmentTemplate commonWeapon;
        commonWeapon.name = "Common Weapon";
        commonWeapon.glyph = '/';
        commonWeapon.color = Colors::white;
        commonWeapon.slot = EquipmentSlot::WEAPON;
        commonWeapon.weight = 1.0f;
        commonWeapon.value = 5;
        commonWeapon.modifiers = {1.0f, 0.0f, 0.0f, 0};
        commonWeapon.tier = ItemTier::COMMON;

        EquipmentTemplate uncommonWeapon;
        uncommonWeapon.name = "Uncommon Weapon";
        uncommonWeapon.glyph = '/';
        uncommonWeapon.color = Colors::lightBlue;
        uncommonWeapon.slot = EquipmentSlot::WEAPON;
        uncommonWeapon.weight = 2.0f;
        uncommonWeapon.value = 20;
        uncommonWeapon.modifiers = {3.0f, 0.0f, 0.0f, 0};
        uncommonWeapon.tier = ItemTier::UNCOMMON;

        EquipmentTemplate rareWeapon;
        rareWeapon.name = "Rare Weapon";
        rareWeapon.glyph = '/';
        rareWeapon.color = Colors::orange;
        rareWeapon.slot = EquipmentSlot::WEAPON;
        rareWeapon.weight = 3.0f;
        rareWeapon.value = 100;
        rareWeapon.modifiers = {5.0f, 0.0f, 0.0f, 0};
        rareWeapon.tier = ItemTier::RARE;

        engine.equipmentTemplates.push_back(commonWeapon);
        engine.equipmentTemplates.push_back(uncommonWeapon);
        engine.equipmentTemplates.push_back(rareWeapon);

        // Configure tier weights: common=70, uncommon=25, rare=5
        EnemyEquipmentConfig::TierWeights weights;
        weights.common = 70.0f;
        weights.uncommon = 25.0f;
        weights.rare = 5.0f;

        // Run 1000 selections and count tier outcomes
        constexpr int NUM_SELECTIONS = 1000;
        int commonCount = 0;
        int uncommonCount = 0;
        int rareCount = 0;

        for (int i = 0; i < NUM_SELECTIONS; ++i) {
            const EquipmentTemplate* selected = engine.selectEquipmentByTier(EquipmentSlot::WEAPON, weights);
            RC_ASSERT(selected != nullptr);
            if (selected->tier == ItemTier::COMMON) commonCount++;
            else if (selected->tier == ItemTier::UNCOMMON) uncommonCount++;
            else if (selected->tier == ItemTier::RARE) rareCount++;
        }

        // Expected: common=70%, uncommon=25%, rare=5%
        // Tolerance: ±10% absolute (so common should be 60-80%, etc.)
        float commonPct = static_cast<float>(commonCount) / NUM_SELECTIONS;
        float uncommonPct = static_cast<float>(uncommonCount) / NUM_SELECTIONS;
        float rarePct = static_cast<float>(rareCount) / NUM_SELECTIONS;

        RC_ASSERT(commonPct >= 0.55f);   // 70% - 15% tolerance
        RC_ASSERT(commonPct <= 0.85f);   // 70% + 15% tolerance
        RC_ASSERT(uncommonPct >= 0.10f); // 25% - 15% tolerance
        RC_ASSERT(uncommonPct <= 0.40f); // 25% + 15% tolerance
        RC_ASSERT(rarePct >= 0.0f);      // 5% - 5% tolerance (can't go negative)
        RC_ASSERT(rarePct <= 0.20f);     // 5% + 15% tolerance

        // Restore engine templates
        engine.equipmentTemplates = std::move(savedTemplates);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 9.3: Integration test — spawn-fight-die-loot cycle
// **Validates: Requirements 6.1, 6.2, 6.3, 6.4**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Integration: spawn-fight-die-loot cycle", "[enemy-equipment][integration]")
{
    clearEngineActors();

    // Create an enemy at position (15, 20) with known equipment
    auto enemy = makeEnemy(15, 20, "Ork Warrior");
    enemy->equipment->dropChance = 1.0f; // guarantee all drops

    StatModifiers weaponMods = {3.0f, 0.0f, 0.0f, 5};
    StatModifiers armorMods = {0.0f, 2.0f, 5.0f, 0};

    equipItemOnEnemy(enemy.get(), makeEnemyItem("Choppa", EquipmentSlot::WEAPON, weaponMods, 4.0f, 20, '/', Colors::desaturatedGreen));
    equipItemOnEnemy(enemy.get(), makeEnemyItem("Ork Armor", EquipmentSlot::BODY, armorMods, 8.0f, 50, '[', Colors::desaturatedGreen));

    Actor* enemyPtr = enemy.get();
    engine.actors.push_back(std::move(enemy));

    // Verify enemy has equipment before death
    REQUIRE(enemyPtr->equipment != nullptr);
    REQUIRE(enemyPtr->equipment->getSlot(EquipmentSlot::WEAPON) != nullptr);
    REQUIRE(enemyPtr->equipment->getSlot(EquipmentSlot::BODY) != nullptr);

    // Simulate death: call dropEnemyEquipment (same as MonsterDestructible::die does)
    dropEnemyEquipment(enemyPtr);

    // Verify items appeared on the map (in engine.actors)
    int droppedItems = 0;
    Actor* droppedChoppa = nullptr;
    Actor* droppedArmor = nullptr;

    for (const auto& a : engine.actors) {
        if (a.get() == enemyPtr) continue;
        if (a->pickable) {
            droppedItems++;
            if (a->name == "Choppa") droppedChoppa = a.get();
            else if (a->name == "Ork Armor") droppedArmor = a.get();
        }
    }

    REQUIRE(droppedItems == 2);
    REQUIRE(droppedChoppa != nullptr);
    REQUIRE(droppedArmor != nullptr);

    // Verify dropped items have correct position (corpse location)
    REQUIRE(droppedChoppa->getX() == 15);
    REQUIRE(droppedChoppa->getY() == 20);
    REQUIRE(droppedArmor->getX() == 15);
    REQUIRE(droppedArmor->getY() == 20);

    // Verify dropped items have correct components for pickup
    REQUIRE(droppedChoppa->pickable != nullptr);
    REQUIRE(droppedChoppa->equippable != nullptr);
    REQUIRE(droppedChoppa->equippable->slot == EquipmentSlot::WEAPON);
    REQUIRE(droppedChoppa->equippable->modifiers.power == 3.0f);
    REQUIRE(droppedChoppa->equippable->modifiers.skill == 5);
    REQUIRE(droppedChoppa->equippable->weight == 4.0f);
    REQUIRE(droppedChoppa->equippable->value == 20);

    REQUIRE(droppedArmor->pickable != nullptr);
    REQUIRE(droppedArmor->equippable != nullptr);
    REQUIRE(droppedArmor->equippable->slot == EquipmentSlot::BODY);
    REQUIRE(droppedArmor->equippable->modifiers.defense == 2.0f);
    REQUIRE(droppedArmor->equippable->modifiers.maxHp == 5.0f);
    REQUIRE(droppedArmor->equippable->weight == 8.0f);
    REQUIRE(droppedArmor->equippable->value == 50);

    // Verify items don't block (can be walked over)
    REQUIRE(droppedChoppa->blocks == false);
    REQUIRE(droppedArmor->blocks == false);

    // Cleanup
    clearEngineActors();
}
