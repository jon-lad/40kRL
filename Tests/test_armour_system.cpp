#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <array>
#include <memory>
#include <numeric>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Helpers
// ═══════════════════════════════════════════════════════════════════════════════

// Helper: Create a standalone equippable item Actor with an ArmourProfile.
static std::unique_ptr<Actor> makeArmourActor(
    const std::string& name,
    EquipmentSlot slot,
    const ArmourProfile& profile,
    float weight = 1.0f)
{
    auto actor = std::make_unique<Actor>(0, 0, '[', name, Colors::white);
    StatModifiers mods{ 0.0f, 0.0f, 0.0f, 0 };
    actor->equippable = std::make_shared<Equippable>(slot, mods, weight, 10);
    actor->equippable->armourProfile = profile;
    return actor;
}

// Helper: Create an equippable item Actor WITHOUT an ArmourProfile.
static std::unique_ptr<Actor> makeNonArmourActor(
    const std::string& name,
    EquipmentSlot slot,
    float weight = 1.0f)
{
    auto actor = std::make_unique<Actor>(0, 0, '/', name, Colors::white);
    StatModifiers mods{ 2.0f, 0.0f, 0.0f, 0 };
    actor->equippable = std::make_shared<Equippable>(slot, mods, weight, 10);
    // No armourProfile set — remains std::nullopt
    return actor;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Property-Based Test — Property 12: Armour sum by location
// ═══════════════════════════════════════════════════════════════════════════════

// **Validates: Requirements 9.4**
TEST_CASE("Property 12: Armour sum by location", "[pbt][armour]")
{
    rc::prop("getArmourAtLocation(loc) equals sum of each equipped item's armour at that location", []() {
        Equipment equipment;
        Container inventory(0); // unlimited capacity
        Attacker attacker(5.0f, 50);

        // Generate random armour values for up to 4 items (one per slot)
        const int numItems = *rc::gen::inRange(1, 5); // 1 to 4 items

        // Track expected armour per location
        std::array<int, static_cast<int>(HitLocation::COUNT)> expectedArmour = {};

        for (int i = 0; i < numItems; ++i) {
            EquipmentSlot slot = static_cast<EquipmentSlot>(i);

            // Generate random armour profile values [0, 10] per location
            ArmourProfile profile;
            for (int loc = 0; loc < static_cast<int>(HitLocation::COUNT); ++loc) {
                profile.values[loc] = *rc::gen::inRange(0, 11); // [0, 10]
                expectedArmour[loc] += profile.values[loc];
            }

            auto item = makeArmourActor("Armour" + std::to_string(i), slot, profile);
            Actor* ptr = item.get();
            inventory.add(std::move(item));
            equipment.equip(ptr, &inventory, &attacker);
        }

        // Verify getArmourAtLocation matches expected sum for each location
        for (int loc = 0; loc < static_cast<int>(HitLocation::COUNT); ++loc) {
            HitLocation hitLoc = static_cast<HitLocation>(loc);
            RC_ASSERT(equipment.getArmourAtLocation(hitLoc) == expectedArmour[loc]);
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Example-Based Tests — Fallback and edge-case scenarios
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Items without armourProfile do not contribute to armour sum", "[armour]")
{
    Equipment equipment;
    Container inventory(0);
    Attacker attacker(5.0f, 50);

    // Equip a weapon with no armourProfile
    auto weapon = makeNonArmourActor("Chainsword", EquipmentSlot::WEAPON);
    Actor* weaponPtr = weapon.get();
    inventory.add(std::move(weapon));
    equipment.equip(weaponPtr, &inventory, &attacker);

    // All locations should return 0
    for (int loc = 0; loc < static_cast<int>(HitLocation::COUNT); ++loc) {
        HitLocation hitLoc = static_cast<HitLocation>(loc);
        REQUIRE(equipment.getArmourAtLocation(hitLoc) == 0);
    }
}

TEST_CASE("Mixed equipped items: only those with armourProfile contribute", "[armour]")
{
    Equipment equipment;
    Container inventory(0);
    Attacker attacker(5.0f, 50);

    // Equip a weapon without armourProfile in WEAPON slot
    auto weapon = makeNonArmourActor("Chainsword", EquipmentSlot::WEAPON);
    Actor* weaponPtr = weapon.get();
    inventory.add(std::move(weapon));
    equipment.equip(weaponPtr, &inventory, &attacker);

    // Equip body armour with specific armourProfile in BODY slot
    ArmourProfile bodyProfile;
    bodyProfile.values[static_cast<int>(HitLocation::HEAD)] = 0;
    bodyProfile.values[static_cast<int>(HitLocation::RIGHT_ARM)] = 3;
    bodyProfile.values[static_cast<int>(HitLocation::LEFT_ARM)] = 3;
    bodyProfile.values[static_cast<int>(HitLocation::BODY)] = 4;
    bodyProfile.values[static_cast<int>(HitLocation::RIGHT_LEG)] = 2;
    bodyProfile.values[static_cast<int>(HitLocation::LEFT_LEG)] = 2;

    auto bodyArmour = makeArmourActor("Flak Armor", EquipmentSlot::BODY, bodyProfile);
    Actor* bodyPtr = bodyArmour.get();
    inventory.add(std::move(bodyArmour));
    equipment.equip(bodyPtr, &inventory, &attacker);

    // Verify: only body armour contributes
    REQUIRE(equipment.getArmourAtLocation(HitLocation::HEAD) == 0);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::RIGHT_ARM) == 3);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::LEFT_ARM) == 3);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::BODY) == 4);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::RIGHT_LEG) == 2);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::LEFT_LEG) == 2);
}

TEST_CASE("Multiple armour items stack correctly per location", "[armour]")
{
    Equipment equipment;
    Container inventory(0);
    Attacker attacker(5.0f, 50);

    // Body armour: covers body and limbs
    ArmourProfile bodyProfile;
    bodyProfile.values[static_cast<int>(HitLocation::HEAD)] = 0;
    bodyProfile.values[static_cast<int>(HitLocation::RIGHT_ARM)] = 3;
    bodyProfile.values[static_cast<int>(HitLocation::LEFT_ARM)] = 3;
    bodyProfile.values[static_cast<int>(HitLocation::BODY)] = 5;
    bodyProfile.values[static_cast<int>(HitLocation::RIGHT_LEG)] = 3;
    bodyProfile.values[static_cast<int>(HitLocation::LEFT_LEG)] = 3;

    auto bodyArmour = makeArmourActor("Flak Armor", EquipmentSlot::BODY, bodyProfile);
    Actor* bodyPtr = bodyArmour.get();
    inventory.add(std::move(bodyArmour));
    equipment.equip(bodyPtr, &inventory, &attacker);

    // Helmet: covers head only
    ArmourProfile headProfile;
    headProfile.values[static_cast<int>(HitLocation::HEAD)] = 4;
    headProfile.values[static_cast<int>(HitLocation::RIGHT_ARM)] = 0;
    headProfile.values[static_cast<int>(HitLocation::LEFT_ARM)] = 0;
    headProfile.values[static_cast<int>(HitLocation::BODY)] = 0;
    headProfile.values[static_cast<int>(HitLocation::RIGHT_LEG)] = 0;
    headProfile.values[static_cast<int>(HitLocation::LEFT_LEG)] = 0;

    auto helmet = makeArmourActor("Carapace Helm", EquipmentSlot::HEAD, headProfile);
    Actor* headPtr = helmet.get();
    inventory.add(std::move(helmet));
    equipment.equip(headPtr, &inventory, &attacker);

    // Verify stacking: head = 4 (helmet only), body = 5 (body armour only)
    REQUIRE(equipment.getArmourAtLocation(HitLocation::HEAD) == 4);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::RIGHT_ARM) == 3);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::LEFT_ARM) == 3);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::BODY) == 5);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::RIGHT_LEG) == 3);
    REQUIRE(equipment.getArmourAtLocation(HitLocation::LEFT_LEG) == 3);
}

TEST_CASE("Empty equipment returns zero armour for all locations", "[armour]")
{
    Equipment equipment;

    for (int loc = 0; loc < static_cast<int>(HitLocation::COUNT); ++loc) {
        HitLocation hitLoc = static_cast<HitLocation>(loc);
        REQUIRE(equipment.getArmourAtLocation(hitLoc) == 0);
    }
}
