#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <string>
#include <string_view>
#include <vector>
#include <optional>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: weapon-types — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 1: Parser round-trip for classification enums ──────────────────
// Feature: weapon-types, Property 1: Parser round-trip for classification enums
// **Validates: Requirements 1.1, 2.1, 3.1**
//
// For any valid string, parse then display returns the original string.
// For invalid strings, parse returns nullopt.

namespace {

// Valid string sets for each enum
const std::vector<std::string> validSizeStrings = {
    "Melee", "Pistol", "Basic", "Heavy", "Thrown"
};

const std::vector<std::string> validGroupStrings = {
    "Las", "SP", "Bolt", "Melta", "Plasma", "Flame", "Primitive", "Launcher", "Exotic"
};

const std::vector<std::string> validDamageStrings = {
    "E", "X", "I", "R"
};

// Invalid strings that should NOT parse (including case-sensitivity violations)
const std::vector<std::string> invalidSizeStrings = {
    "melee", "MELEE", "pistol", "PISTOL", "basic", "BASIC",
    "heavy", "HEAVY", "thrown", "THROWN", "", "Unknown", "Ranged", "las"
};

const std::vector<std::string> invalidGroupStrings = {
    "las", "LAS", "sp", "bolt", "melta", "plasma", "flame",
    "primitive", "launcher", "exotic", "", "Unknown", "Laser", "BOLT"
};

const std::vector<std::string> invalidDamageStrings = {
    "e", "x", "i", "r", "Energy", "Explosive", "Impact", "Rending",
    "", "Unknown", "D", "A"
};

} // anonymous namespace

TEST_CASE("PBT: Property 1 — SizeClassification parse/display round-trip",
          "[pbt][property][weapon-types]")
{
    rc::prop("valid SizeClassification strings round-trip through parse then display", []() {
        // Pick a random valid string
        auto validStr = *rc::gen::elementOf(validSizeStrings);

        // Parse should succeed
        auto parsed = parseSizeClassification(validStr);
        RC_ASSERT(parsed.has_value());

        // Display should return the original string
        auto displayed = sizeClassificationName(parsed.value());
        RC_ASSERT(displayed == validStr);
    });
}

TEST_CASE("PBT: Property 1 — WeaponGroup parse/display round-trip",
          "[pbt][property][weapon-types]")
{
    rc::prop("valid WeaponGroup strings round-trip through parse then display", []() {
        // Pick a random valid string
        auto validStr = *rc::gen::elementOf(validGroupStrings);

        // Parse should succeed
        auto parsed = parseWeaponGroup(validStr);
        RC_ASSERT(parsed.has_value());

        // Display should return the original string
        auto displayed = weaponGroupName(parsed.value());
        RC_ASSERT(displayed == validStr);
    });
}

TEST_CASE("PBT: Property 1 — DamageType parse/display round-trip",
          "[pbt][property][weapon-types]")
{
    rc::prop("valid DamageType strings round-trip through parse then display", []() {
        // Pick a random valid string
        auto validStr = *rc::gen::elementOf(validDamageStrings);

        // Parse should succeed
        auto parsed = parseDamageType(validStr);
        RC_ASSERT(parsed.has_value());

        // Display should return the original string
        auto displayed = damageTypeName(parsed.value());
        RC_ASSERT(displayed == validStr);
    });
}

TEST_CASE("PBT: Property 1 — invalid strings return nullopt for SizeClassification",
          "[pbt][property][weapon-types]")
{
    rc::prop("invalid SizeClassification strings return nullopt", []() {
        // Pick a random invalid string
        auto invalidStr = *rc::gen::elementOf(invalidSizeStrings);

        auto parsed = parseSizeClassification(invalidStr);
        RC_ASSERT(!parsed.has_value());
    });
}

TEST_CASE("PBT: Property 1 — invalid strings return nullopt for WeaponGroup",
          "[pbt][property][weapon-types]")
{
    rc::prop("invalid WeaponGroup strings return nullopt", []() {
        // Pick a random invalid string
        auto invalidStr = *rc::gen::elementOf(invalidGroupStrings);

        auto parsed = parseWeaponGroup(invalidStr);
        RC_ASSERT(!parsed.has_value());
    });
}

TEST_CASE("PBT: Property 1 — invalid strings return nullopt for DamageType",
          "[pbt][property][weapon-types]")
{
    rc::prop("invalid DamageType strings return nullopt", []() {
        // Pick a random invalid string
        auto invalidStr = *rc::gen::elementOf(invalidDamageStrings);

        auto parsed = parseDamageType(invalidStr);
        RC_ASSERT(!parsed.has_value());
    });
}

// Random garbage strings should also return nullopt
TEST_CASE("PBT: Property 1 — random garbage strings return nullopt for all parsers",
          "[pbt][property][weapon-types]")
{
    rc::prop("random strings that aren't in valid sets return nullopt", []() {
        // Generate a random string of length 1-10
        auto randomStr = *rc::gen::string(1, 10);

        // Check if it happens to be a valid string (skip if so)
        bool isValidSize = std::find(validSizeStrings.begin(), validSizeStrings.end(), randomStr) != validSizeStrings.end();
        bool isValidGroup = std::find(validGroupStrings.begin(), validGroupStrings.end(), randomStr) != validGroupStrings.end();
        bool isValidDamage = std::find(validDamageStrings.begin(), validDamageStrings.end(), randomStr) != validDamageStrings.end();

        if (!isValidSize) {
            RC_ASSERT(!parseSizeClassification(randomStr).has_value());
        }
        if (!isValidGroup) {
            RC_ASSERT(!parseWeaponGroup(randomStr).has_value());
        }
        if (!isValidDamage) {
            RC_ASSERT(!parseDamageType(randomStr).has_value());
        }
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: weapon-types — Unit Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Unit tests: exhaustive check of all valid SizeClassification values ─────
// **Validates: Requirements 1.1**

TEST_CASE("All 5 SizeClassification values parse correctly", "[weapon-types]")
{
    REQUIRE(parseSizeClassification("Melee") == SizeClassification::MELEE);
    REQUIRE(parseSizeClassification("Pistol") == SizeClassification::PISTOL);
    REQUIRE(parseSizeClassification("Basic") == SizeClassification::BASIC);
    REQUIRE(parseSizeClassification("Heavy") == SizeClassification::HEAVY);
    REQUIRE(parseSizeClassification("Thrown") == SizeClassification::THROWN);
}

// ─── Unit tests: exhaustive check of all valid WeaponGroup values ────────────
// **Validates: Requirements 2.1**

TEST_CASE("All 9 WeaponGroup values parse correctly", "[weapon-types]")
{
    REQUIRE(parseWeaponGroup("Las") == WeaponGroup::LAS);
    REQUIRE(parseWeaponGroup("SP") == WeaponGroup::SP);
    REQUIRE(parseWeaponGroup("Bolt") == WeaponGroup::BOLT);
    REQUIRE(parseWeaponGroup("Melta") == WeaponGroup::MELTA);
    REQUIRE(parseWeaponGroup("Plasma") == WeaponGroup::PLASMA);
    REQUIRE(parseWeaponGroup("Flame") == WeaponGroup::FLAME);
    REQUIRE(parseWeaponGroup("Primitive") == WeaponGroup::PRIMITIVE);
    REQUIRE(parseWeaponGroup("Launcher") == WeaponGroup::LAUNCHER);
    REQUIRE(parseWeaponGroup("Exotic") == WeaponGroup::EXOTIC);
}

// ─── Unit tests: exhaustive check of all valid DamageType values ─────────────
// **Validates: Requirements 3.1**

TEST_CASE("All 4 DamageType values parse correctly", "[weapon-types]")
{
    REQUIRE(parseDamageType("E") == DamageType::E);
    REQUIRE(parseDamageType("X") == DamageType::X);
    REQUIRE(parseDamageType("I") == DamageType::I);
    REQUIRE(parseDamageType("R") == DamageType::R);
}

// ─── Unit tests: display names return correct strings ────────────────────────
// **Validates: Requirements 1.1, 2.1, 3.1**

TEST_CASE("sizeClassificationName returns correct display strings", "[weapon-types]")
{
    REQUIRE(sizeClassificationName(SizeClassification::MELEE) == "Melee");
    REQUIRE(sizeClassificationName(SizeClassification::PISTOL) == "Pistol");
    REQUIRE(sizeClassificationName(SizeClassification::BASIC) == "Basic");
    REQUIRE(sizeClassificationName(SizeClassification::HEAVY) == "Heavy");
    REQUIRE(sizeClassificationName(SizeClassification::THROWN) == "Thrown");
}

TEST_CASE("weaponGroupName returns correct display strings", "[weapon-types]")
{
    REQUIRE(weaponGroupName(WeaponGroup::LAS) == "Las");
    REQUIRE(weaponGroupName(WeaponGroup::SP) == "SP");
    REQUIRE(weaponGroupName(WeaponGroup::BOLT) == "Bolt");
    REQUIRE(weaponGroupName(WeaponGroup::MELTA) == "Melta");
    REQUIRE(weaponGroupName(WeaponGroup::PLASMA) == "Plasma");
    REQUIRE(weaponGroupName(WeaponGroup::FLAME) == "Flame");
    REQUIRE(weaponGroupName(WeaponGroup::PRIMITIVE) == "Primitive");
    REQUIRE(weaponGroupName(WeaponGroup::LAUNCHER) == "Launcher");
    REQUIRE(weaponGroupName(WeaponGroup::EXOTIC) == "Exotic");
}

TEST_CASE("damageTypeName returns correct display strings", "[weapon-types]")
{
    REQUIRE(damageTypeName(DamageType::E) == "E");
    REQUIRE(damageTypeName(DamageType::X) == "X");
    REQUIRE(damageTypeName(DamageType::I) == "I");
    REQUIRE(damageTypeName(DamageType::R) == "R");
}

// ─── Unit tests: case-sensitivity — wrong case returns nullopt ───────────────
// **Validates: Requirements 1.1, 2.1, 3.1**

TEST_CASE("parseSizeClassification is case-sensitive", "[weapon-types]")
{
    REQUIRE(!parseSizeClassification("melee").has_value());
    REQUIRE(!parseSizeClassification("MELEE").has_value());
    REQUIRE(!parseSizeClassification("pistol").has_value());
    REQUIRE(!parseSizeClassification("PISTOL").has_value());
    REQUIRE(!parseSizeClassification("basic").has_value());
    REQUIRE(!parseSizeClassification("BASIC").has_value());
    REQUIRE(!parseSizeClassification("heavy").has_value());
    REQUIRE(!parseSizeClassification("HEAVY").has_value());
    REQUIRE(!parseSizeClassification("thrown").has_value());
    REQUIRE(!parseSizeClassification("THROWN").has_value());
}

TEST_CASE("parseWeaponGroup is case-sensitive", "[weapon-types]")
{
    REQUIRE(!parseWeaponGroup("las").has_value());
    REQUIRE(!parseWeaponGroup("LAS").has_value());
    REQUIRE(!parseWeaponGroup("sp").has_value());
    REQUIRE(!parseWeaponGroup("bolt").has_value());
    REQUIRE(!parseWeaponGroup("BOLT").has_value());
    REQUIRE(!parseWeaponGroup("melta").has_value());
    REQUIRE(!parseWeaponGroup("plasma").has_value());
    REQUIRE(!parseWeaponGroup("flame").has_value());
    REQUIRE(!parseWeaponGroup("primitive").has_value());
    REQUIRE(!parseWeaponGroup("launcher").has_value());
    REQUIRE(!parseWeaponGroup("exotic").has_value());
}

TEST_CASE("parseDamageType is case-sensitive", "[weapon-types]")
{
    REQUIRE(!parseDamageType("e").has_value());
    REQUIRE(!parseDamageType("x").has_value());
    REQUIRE(!parseDamageType("i").has_value());
    REQUIRE(!parseDamageType("r").has_value());
    REQUIRE(!parseDamageType("Energy").has_value());
    REQUIRE(!parseDamageType("Explosive").has_value());
    REQUIRE(!parseDamageType("Impact").has_value());
    REQUIRE(!parseDamageType("Rending").has_value());
}

// ─── Unit tests: empty and whitespace strings return nullopt ─────────────────

TEST_CASE("Empty strings return nullopt for all parsers", "[weapon-types]")
{
    REQUIRE(!parseSizeClassification("").has_value());
    REQUIRE(!parseWeaponGroup("").has_value());
    REQUIRE(!parseDamageType("").has_value());
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: weapon-types — Task 3.1: Classification Storage on Equippable
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 3: Equip preserves classification metadata ─────────────────────
// Feature: weapon-types, Property 3: Equip preserves classification metadata
// **Validates: Requirements 5.1, 5.2, 5.3, 5.4**
//
// For any Equippable with classification fields set, reading those fields back
// produces the identical values that were set.

TEST_CASE("PBT: Property 3 — classification fields are preserved on Equippable",
          "[pbt][property][weapon-types]")
{
    rc::prop("setting classification fields retains them unchanged", []() {
        // Generate random enum values within valid ranges
        auto scIdx = *rc::gen::inRange(0, 5);
        auto wgIdx = *rc::gen::inRange(0, 9);
        auto dtIdx = *rc::gen::inRange(0, 4);

        auto sc = static_cast<SizeClassification>(scIdx);
        auto wg = static_cast<WeaponGroup>(wgIdx);
        auto dt = static_cast<DamageType>(dtIdx);

        // Create an Equippable (weapon slot, no stat modifiers)
        Equippable equip(EquipmentSlot::WEAPON, StatModifiers{}, 1.0f, 10);

        // Set classification fields
        equip.sizeClass = sc;
        equip.weaponGroup = wg;
        equip.damageType = dt;

        // Verify fields are retained unchanged
        RC_ASSERT(equip.sizeClass.has_value());
        RC_ASSERT(equip.sizeClass.value() == sc);
        RC_ASSERT(equip.weaponGroup.has_value());
        RC_ASSERT(equip.weaponGroup.value() == wg);
        RC_ASSERT(equip.damageType.has_value());
        RC_ASSERT(equip.damageType.value() == dt);
    });
}

// ─── Unit test: non-weapon items have nullopt classification fields ──────────
// **Validates: Requirements 5.2, 5.3**

TEST_CASE("Non-weapon Equippable has nullopt classification fields", "[weapon-types]")
{
    // Create an armour-slot item without setting classification
    Equippable armour(EquipmentSlot::BODY, StatModifiers{}, 5.0f, 50);

    REQUIRE_FALSE(armour.sizeClass.has_value());
    REQUIRE_FALSE(armour.weaponGroup.has_value());
    REQUIRE_FALSE(armour.damageType.has_value());
}

TEST_CASE("Offhand Equippable has nullopt classification fields", "[weapon-types]")
{
    // Create an offhand item (e.g., shield) without classification
    Equippable shield(EquipmentSlot::OFFHAND, StatModifiers{}, 3.0f, 25);

    REQUIRE_FALSE(shield.sizeClass.has_value());
    REQUIRE_FALSE(shield.weaponGroup.has_value());
    REQUIRE_FALSE(shield.damageType.has_value());
}

// ─── Unit test: setting all specific enum values on a weapon Equippable ──────
// **Validates: Requirements 5.1, 5.2, 5.3, 5.4**

TEST_CASE("Equippable stores each SizeClassification correctly", "[weapon-types]")
{
    Equippable equip(EquipmentSlot::WEAPON, StatModifiers{}, 1.0f, 10);

    equip.sizeClass = SizeClassification::MELEE;
    REQUIRE(equip.sizeClass.value() == SizeClassification::MELEE);

    equip.sizeClass = SizeClassification::PISTOL;
    REQUIRE(equip.sizeClass.value() == SizeClassification::PISTOL);

    equip.sizeClass = SizeClassification::BASIC;
    REQUIRE(equip.sizeClass.value() == SizeClassification::BASIC);

    equip.sizeClass = SizeClassification::HEAVY;
    REQUIRE(equip.sizeClass.value() == SizeClassification::HEAVY);

    equip.sizeClass = SizeClassification::THROWN;
    REQUIRE(equip.sizeClass.value() == SizeClassification::THROWN);
}

TEST_CASE("Equippable stores each WeaponGroup correctly", "[weapon-types]")
{
    Equippable equip(EquipmentSlot::WEAPON, StatModifiers{}, 1.0f, 10);

    equip.weaponGroup = WeaponGroup::LAS;
    REQUIRE(equip.weaponGroup.value() == WeaponGroup::LAS);

    equip.weaponGroup = WeaponGroup::SP;
    REQUIRE(equip.weaponGroup.value() == WeaponGroup::SP);

    equip.weaponGroup = WeaponGroup::BOLT;
    REQUIRE(equip.weaponGroup.value() == WeaponGroup::BOLT);

    equip.weaponGroup = WeaponGroup::MELTA;
    REQUIRE(equip.weaponGroup.value() == WeaponGroup::MELTA);

    equip.weaponGroup = WeaponGroup::PLASMA;
    REQUIRE(equip.weaponGroup.value() == WeaponGroup::PLASMA);

    equip.weaponGroup = WeaponGroup::FLAME;
    REQUIRE(equip.weaponGroup.value() == WeaponGroup::FLAME);

    equip.weaponGroup = WeaponGroup::PRIMITIVE;
    REQUIRE(equip.weaponGroup.value() == WeaponGroup::PRIMITIVE);

    equip.weaponGroup = WeaponGroup::LAUNCHER;
    REQUIRE(equip.weaponGroup.value() == WeaponGroup::LAUNCHER);

    equip.weaponGroup = WeaponGroup::EXOTIC;
    REQUIRE(equip.weaponGroup.value() == WeaponGroup::EXOTIC);
}

TEST_CASE("Equippable stores each DamageType correctly", "[weapon-types]")
{
    Equippable equip(EquipmentSlot::WEAPON, StatModifiers{}, 1.0f, 10);

    equip.damageType = DamageType::E;
    REQUIRE(equip.damageType.value() == DamageType::E);

    equip.damageType = DamageType::X;
    REQUIRE(equip.damageType.value() == DamageType::X);

    equip.damageType = DamageType::I;
    REQUIRE(equip.damageType.value() == DamageType::I);

    equip.damageType = DamageType::R;
    REQUIRE(equip.damageType.value() == DamageType::R);
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: weapon-types — Equipment Loader Validation Tests (Task 4.1)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 2: Range acceptance predicate ──────────────────────────────────
// Feature: weapon-types, Property 2: Range acceptance predicate
// **Validates: Requirements 4.1, 4.2, 4.3**
//
// For any integer n, isValidRange(n) returns true iff n > 0.

TEST_CASE("PBT: Property 2 — Range acceptance predicate",
          "[pbt][property][weapon-types]")
{
    rc::prop("isValidRange accepts n iff n > 0", []() {
        // Generate random integers in a wide range including negatives, zero, and positives
        int n = *rc::gen::inRange(-1000, 1001);

        bool result = isValidRange(n);
        bool expected = (n > 0);
        RC_ASSERT(result == expected);
    });
}

TEST_CASE("PBT: Property 2 — Range acceptance boundary cases",
          "[pbt][property][weapon-types]")
{
    rc::prop("isValidRange rejects zero and all negative integers", []() {
        // Generate non-positive integers specifically
        int n = *rc::gen::inRange(-10000, 1); // range is [-10000, 0]
        RC_ASSERT(!isValidRange(n));
    });
}

// ─── Property 8: Weapon entry validation completeness ────────────────────────
// Feature: weapon-types, Property 8: Weapon entry validation completeness
// **Validates: Requirements 1.2, 1.3, 1.4, 2.2, 2.3, 2.4, 2.5, 3.2, 3.3, 3.4, 3.5, 8.2, 8.3, 8.4**
//
// For any Equipment.lua entry that contains a melee or ranged table, the entry
// is accepted by the loader iff all three classification fields (sizeClass,
// weaponGroup, damageType) are present and valid.

TEST_CASE("PBT: Property 8 — Weapon entries accepted iff all three classification fields present and valid",
          "[pbt][property][weapon-types]")
{
    rc::prop("weapon entry accepted iff sizeClass, weaponGroup, and damageType all present and valid", []() {
        // Generate whether each field is present (true) or missing (false)
        // Use inRange(0,2) as a bool generator to avoid MSVC template-in-macro issues
        bool hasSizeClass = (*rc::gen::inRange(0, 2)) == 1;
        bool hasWeaponGroup = (*rc::gen::inRange(0, 2)) == 1;
        bool hasDamageType = (*rc::gen::inRange(0, 2)) == 1;

        // When present, decide if value is valid or invalid
        bool sizeClassValid = (*rc::gen::inRange(0, 2)) == 1;
        bool weaponGroupValid = (*rc::gen::inRange(0, 2)) == 1;
        bool damageTypeValid = (*rc::gen::inRange(0, 2)) == 1;

        // Build field strings
        std::string sizeStr = "";
        if (hasSizeClass) {
            if (sizeClassValid) {
                sizeStr = *rc::gen::elementOf(validSizeStrings);
            } else {
                sizeStr = "InvalidSize";
            }
        }

        std::string groupStr = "";
        if (hasWeaponGroup) {
            if (weaponGroupValid) {
                groupStr = *rc::gen::elementOf(validGroupStrings);
            } else {
                groupStr = "InvalidGroup";
            }
        }

        std::string dmgStr = "";
        if (hasDamageType) {
            if (damageTypeValid) {
                dmgStr = *rc::gen::elementOf(validDamageStrings);
            } else {
                dmgStr = "InvalidDmg";
            }
        }

        // This is a weapon entry (has melee or ranged)
        auto result = validateWeaponClassification("TestWeapon", true, sizeStr, groupStr, dmgStr);

        // Expected: accepted only if all three are present AND valid
        bool allPresentAndValid = hasSizeClass && sizeClassValid
                               && hasWeaponGroup && weaponGroupValid
                               && hasDamageType && damageTypeValid;

        RC_ASSERT(result.accepted == allPresentAndValid);

        // If rejected, warning message should be non-empty
        if (!result.accepted) {
            RC_ASSERT(!result.warningMessage.empty());
        }
    });
}

TEST_CASE("PBT: Property 8 — Non-weapon entries always accepted regardless of classification fields",
          "[pbt][property][weapon-types]")
{
    rc::prop("non-weapon entries are always accepted regardless of field presence/validity", []() {
        // Generate random presence/absence of fields
        bool hasSizeClass = (*rc::gen::inRange(0, 2)) == 1;
        bool hasWeaponGroup = (*rc::gen::inRange(0, 2)) == 1;
        bool hasDamageType = (*rc::gen::inRange(0, 2)) == 1;

        std::string sizeStr = hasSizeClass ? "Melee" : "";
        std::string groupStr = hasWeaponGroup ? "Las" : "";
        std::string dmgStr = hasDamageType ? "E" : "";

        // This is NOT a weapon entry (armour, shield, etc.)
        auto result = validateWeaponClassification("FlakArmor", false, sizeStr, groupStr, dmgStr);

        // Non-weapon entries are always accepted
        RC_ASSERT(result.accepted == true);
        RC_ASSERT(result.warningMessage.empty());
    });
}

// ─── Unit tests: Range acceptance predicate ──────────────────────────────────
// **Validates: Requirements 4.1, 4.2, 4.3**

TEST_CASE("isValidRange accepts positive integers", "[weapon-types]")
{
    REQUIRE(isValidRange(1));
    REQUIRE(isValidRange(15));
    REQUIRE(isValidRange(30));
    REQUIRE(isValidRange(100));
    REQUIRE(isValidRange(999));
}

TEST_CASE("isValidRange rejects zero and negative integers", "[weapon-types]")
{
    REQUIRE(!isValidRange(0));
    REQUIRE(!isValidRange(-1));
    REQUIRE(!isValidRange(-10));
    REQUIRE(!isValidRange(-999));
}

// ─── Unit tests: Non-weapon entries accepted without classification fields ───
// **Validates: Requirements 2.5, 3.5**

TEST_CASE("Non-weapon entries are accepted without any classification fields", "[weapon-types]")
{
    // Armour entry with no classification fields
    auto result = validateWeaponClassification("Flak Armor", false, "", "", "");
    REQUIRE(result.accepted);
    REQUIRE(result.warningMessage.empty());
}

TEST_CASE("Non-weapon entries are accepted with partial classification fields", "[weapon-types]")
{
    // Shield with only sizeClass set (partial — still accepted for non-weapons)
    auto result = validateWeaponClassification("Scrap Shield", false, "Melee", "", "");
    REQUIRE(result.accepted);
    REQUIRE(result.warningMessage.empty());
}

// ─── Unit tests: Missing required fields on weapons produce warnings and skip ─
// **Validates: Requirements 1.4, 2.4, 3.4, 8.2, 8.4**

TEST_CASE("Weapon entry missing sizeClass is rejected with warning", "[weapon-types]")
{
    auto result = validateWeaponClassification("TestSword", true, "", "Primitive", "R");
    REQUIRE(!result.accepted);
    REQUIRE(result.warningMessage.find("missing required sizeClass") != std::string::npos);
    REQUIRE(result.warningMessage.find("TestSword") != std::string::npos);
}

TEST_CASE("Weapon entry missing weaponGroup is rejected with warning", "[weapon-types]")
{
    auto result = validateWeaponClassification("TestSword", true, "Melee", "", "R");
    REQUIRE(!result.accepted);
    REQUIRE(result.warningMessage.find("missing required weaponGroup") != std::string::npos);
    REQUIRE(result.warningMessage.find("TestSword") != std::string::npos);
}

TEST_CASE("Weapon entry missing damageType is rejected with warning", "[weapon-types]")
{
    auto result = validateWeaponClassification("TestSword", true, "Melee", "Primitive", "");
    REQUIRE(!result.accepted);
    REQUIRE(result.warningMessage.find("missing required damageType") != std::string::npos);
    REQUIRE(result.warningMessage.find("TestSword") != std::string::npos);
}

TEST_CASE("Weapon entry missing all three classification fields is rejected", "[weapon-types]")
{
    auto result = validateWeaponClassification("BrokenWeapon", true, "", "", "");
    REQUIRE(!result.accepted);
    REQUIRE(!result.warningMessage.empty());
    REQUIRE(result.warningMessage.find("BrokenWeapon") != std::string::npos);
}

// ─── Unit tests: Invalid enum strings produce warnings and skip ──────────────
// **Validates: Requirements 1.3, 2.3, 3.3, 8.3, 8.4**

TEST_CASE("Weapon entry with invalid sizeClass string is rejected with warning", "[weapon-types]")
{
    auto result = validateWeaponClassification("BadSword", true, "Huge", "Primitive", "R");
    REQUIRE(!result.accepted);
    REQUIRE(result.warningMessage.find("invalid sizeClass") != std::string::npos);
    REQUIRE(result.warningMessage.find("Huge") != std::string::npos);
    REQUIRE(result.warningMessage.find("BadSword") != std::string::npos);
}

TEST_CASE("Weapon entry with invalid weaponGroup string is rejected with warning", "[weapon-types]")
{
    auto result = validateWeaponClassification("BadGun", true, "Pistol", "Laser", "E");
    REQUIRE(!result.accepted);
    REQUIRE(result.warningMessage.find("invalid weaponGroup") != std::string::npos);
    REQUIRE(result.warningMessage.find("Laser") != std::string::npos);
    REQUIRE(result.warningMessage.find("BadGun") != std::string::npos);
}

TEST_CASE("Weapon entry with invalid damageType string is rejected with warning", "[weapon-types]")
{
    auto result = validateWeaponClassification("BadAxe", true, "Melee", "Primitive", "Z");
    REQUIRE(!result.accepted);
    REQUIRE(result.warningMessage.find("invalid damageType") != std::string::npos);
    REQUIRE(result.warningMessage.find("Z") != std::string::npos);
    REQUIRE(result.warningMessage.find("BadAxe") != std::string::npos);
}

TEST_CASE("Weapon entry with wrong-case sizeClass is rejected", "[weapon-types]")
{
    // "melee" instead of "Melee" — case sensitivity enforcement
    auto result = validateWeaponClassification("CaseSword", true, "melee", "Primitive", "R");
    REQUIRE(!result.accepted);
    REQUIRE(result.warningMessage.find("invalid sizeClass") != std::string::npos);
}

// ─── Unit tests: Valid weapon entries are accepted ───────────────────────────
// **Validates: Requirements 1.2, 2.2, 3.2, 8.2**

TEST_CASE("Weapon entry with all valid classification fields is accepted", "[weapon-types]")
{
    auto result = validateWeaponClassification("Laspistol", true, "Pistol", "Las", "E");
    REQUIRE(result.accepted);
    REQUIRE(result.warningMessage.empty());
}

TEST_CASE("Weapon entry with each valid combination is accepted", "[weapon-types]")
{
    // Melee weapon
    auto r1 = validateWeaponClassification("Combat Knife", true, "Melee", "Primitive", "R");
    REQUIRE(r1.accepted);

    // Basic ranged weapon
    auto r2 = validateWeaponClassification("Autogun", true, "Basic", "SP", "I");
    REQUIRE(r2.accepted);

    // Heavy weapon
    auto r3 = validateWeaponClassification("Heavy Bolter", true, "Heavy", "Bolt", "X");
    REQUIRE(r3.accepted);

    // Thrown weapon
    auto r4 = validateWeaponClassification("Krak Grenade", true, "Thrown", "Launcher", "X");
    REQUIRE(r4.accepted);
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: weapon-types — Proficiency Check Tests (Task 6.1)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 4: Proficiency modifier is determined by talent membership ─────
// Feature: weapon-types, Property 4: Proficiency modifier is determined by talent membership
// **Validates: Requirements 6.1, 6.2**
//
// For any WeaponGroup g, proficiencyModifier returns −20 iff the actor's talent set
// lacks "Weapon Training (<GroupName>)", and 0 if it contains the matching talent.

TEST_CASE("PBT: Property 4 — proficiencyModifier returns -20 iff talent missing",
          "[pbt][property][weapon-types]")
{
    rc::prop("proficiencyModifier is -20 when talent absent, 0 when present", []() {
        // Pick a random WeaponGroup
        auto wgIdx = *rc::gen::inRange(0, 9);
        auto group = static_cast<WeaponGroup>(wgIdx);
        std::string groupName(weaponGroupName(group));

        // Decide whether the actor has the matching talent
        bool hasTalent = (*rc::gen::inRange(0, 2)) == 1;

        // Create an actor with a CareerProgression component
        Actor actor(0, 0, '@', "TestChar", TCODColor::white);
        actor.career = std::make_shared<CareerProgression>();

        if (hasTalent) {
            actor.career->talents.insert("Weapon Training (" + groupName + ")");
        }

        // Also add some random unrelated talents to make the test more robust
        auto numExtra = *rc::gen::inRange(0, 4);
        for (int i = 0; i < numExtra; ++i) {
            actor.career->talents.insert("Unrelated Talent " + std::to_string(i));
        }

        int modifier = proficiencyModifier(&actor, group);

        if (hasTalent) {
            RC_ASSERT(modifier == 0);
        } else {
            RC_ASSERT(modifier == -20);
        }
    });
}

TEST_CASE("PBT: Property 4 — hasProficiency matches talent membership",
          "[pbt][property][weapon-types]")
{
    rc::prop("hasProficiency returns true iff matching talent present", []() {
        // Pick a random WeaponGroup
        auto wgIdx = *rc::gen::inRange(0, 9);
        auto group = static_cast<WeaponGroup>(wgIdx);
        std::string groupName(weaponGroupName(group));

        // Decide whether the actor has the matching talent
        bool hasTalent = (*rc::gen::inRange(0, 2)) == 1;

        Actor actor(0, 0, '@', "TestChar", TCODColor::white);
        actor.career = std::make_shared<CareerProgression>();

        if (hasTalent) {
            actor.career->talents.insert("Weapon Training (" + groupName + ")");
        }

        bool result = hasProficiency(&actor, group);
        RC_ASSERT(result == hasTalent);
    });
}

// ─── Property 5: Proficiency penalty stacks additively ───────────────────────
// Feature: weapon-types, Property 5: Proficiency penalty stacks additively
// **Validates: Requirements 6.3**
//
// Adding −20 to existing modifiers produces correct clamped threshold:
// clamp(skillValue + sum(existingModifiers) - 20, 1, 99)

TEST_CASE("PBT: Property 5 — proficiency penalty stacks additively with modifiers",
          "[pbt][property][weapon-types]")
{
    rc::prop("adding -20 to modifiers produces clamp(skill + mods - 20, 1, 99)", []() {
        // Generate a skill value in reasonable range
        int skillValue = *rc::gen::inRange(1, 100);

        // Generate 0-5 existing modifiers in range [-30, +30]
        auto numMods = *rc::gen::inRange(0, 6);
        std::vector<int> existingMods;
        for (int i = 0; i < numMods; ++i) {
            int mod = *rc::gen::inRange(-30, 31);
            existingMods.push_back(mod);
        }

        // Create an Attacker with the given skill and modifiers
        Attacker attacker(0.0f, skillValue);
        for (int m : existingMods) {
            attacker.addModifier(m);
        }

        // Compute threshold before adding penalty (verify basic consistency)
        int thresholdBefore = attacker.computeThreshold();

        // Add the proficiency penalty
        attacker.addModifier(-20);

        // Compute threshold after adding penalty
        int thresholdAfter = attacker.computeThreshold();

        // The relationship: thresholdAfter == clamp(thresholdBefore's raw - 20, 1, 99)
        // Or equivalently: the raw value decreases by exactly 20
        int modSum = 0;
        for (int m : existingMods) modSum += m;
        int rawBefore = attacker.skillValue + modSum;
        int rawAfter = rawBefore - 20;
        int expectedAfter = std::max(1, std::min(99, rawAfter));
        RC_ASSERT(thresholdAfter == expectedAfter);
    });
}

// ─── Unit tests: Proficiency check with matching talent ──────────────────────
// **Validates: Requirements 6.2**

TEST_CASE("Actor with matching weapon training talent has proficiency", "[weapon-types]")
{
    Actor actor(0, 0, '@', "Guardsman", TCODColor::white);
    actor.career = std::make_shared<CareerProgression>();
    actor.career->talents.insert("Weapon Training (Las)");

    REQUIRE(hasProficiency(&actor, WeaponGroup::LAS));
    REQUIRE(proficiencyModifier(&actor, WeaponGroup::LAS) == 0);
}

TEST_CASE("Actor with multiple weapon training talents has correct proficiency", "[weapon-types]")
{
    Actor actor(0, 0, '@', "Veteran", TCODColor::white);
    actor.career = std::make_shared<CareerProgression>();
    actor.career->talents.insert("Weapon Training (Las)");
    actor.career->talents.insert("Weapon Training (SP)");
    actor.career->talents.insert("Weapon Training (Bolt)");

    REQUIRE(hasProficiency(&actor, WeaponGroup::LAS));
    REQUIRE(hasProficiency(&actor, WeaponGroup::SP));
    REQUIRE(hasProficiency(&actor, WeaponGroup::BOLT));
    REQUIRE(proficiencyModifier(&actor, WeaponGroup::LAS) == 0);
    REQUIRE(proficiencyModifier(&actor, WeaponGroup::SP) == 0);
    REQUIRE(proficiencyModifier(&actor, WeaponGroup::BOLT) == 0);
}

// ─── Unit tests: Proficiency check without matching talent ───────────────────
// **Validates: Requirements 6.1**

TEST_CASE("Actor lacking weapon training talent has no proficiency", "[weapon-types]")
{
    Actor actor(0, 0, '@', "Untrained", TCODColor::white);
    actor.career = std::make_shared<CareerProgression>();
    // No talents added

    REQUIRE_FALSE(hasProficiency(&actor, WeaponGroup::LAS));
    REQUIRE(proficiencyModifier(&actor, WeaponGroup::LAS) == -20);
}

TEST_CASE("Actor with wrong weapon training talent still lacks proficiency for other groups", "[weapon-types]")
{
    Actor actor(0, 0, '@', "Specialist", TCODColor::white);
    actor.career = std::make_shared<CareerProgression>();
    actor.career->talents.insert("Weapon Training (Las)");

    // Has Las training but not SP, Bolt, etc.
    REQUIRE_FALSE(hasProficiency(&actor, WeaponGroup::SP));
    REQUIRE_FALSE(hasProficiency(&actor, WeaponGroup::BOLT));
    REQUIRE_FALSE(hasProficiency(&actor, WeaponGroup::EXOTIC));
    REQUIRE(proficiencyModifier(&actor, WeaponGroup::SP) == -20);
    REQUIRE(proficiencyModifier(&actor, WeaponGroup::BOLT) == -20);
    REQUIRE(proficiencyModifier(&actor, WeaponGroup::EXOTIC) == -20);
}

// ─── Unit tests: Proficiency penalty stacking ────────────────────────────────
// **Validates: Requirements 6.3**

TEST_CASE("Proficiency penalty of -20 stacks with existing modifiers", "[weapon-types]")
{
    Attacker attacker(0.0f, 50);
    attacker.addModifier(10);  // +10 bonus
    attacker.addModifier(-20); // -20 proficiency penalty

    // Expected: clamp(50 + 10 - 20, 1, 99) = 40
    REQUIRE(attacker.computeThreshold() == 40);
}

TEST_CASE("Proficiency penalty clamped to minimum 1", "[weapon-types]")
{
    Attacker attacker(0.0f, 10);
    attacker.addModifier(-20); // -20 proficiency penalty

    // Expected: clamp(10 - 20, 1, 99) = 1 (clamped to minimum)
    REQUIRE(attacker.computeThreshold() == 1);
}

TEST_CASE("Proficiency penalty clamped correctly at high skill", "[weapon-types]")
{
    Attacker attacker(0.0f, 95);
    attacker.addModifier(20);  // +20 bonus
    attacker.addModifier(-20); // -20 proficiency penalty

    // Expected: clamp(95 + 20 - 20, 1, 99) = 95
    REQUIRE(attacker.computeThreshold() == 95);
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: weapon-types — Combat-Mode Gate Tests (Task 7.1)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 6: Size classification combat-mode gate ────────────────────────
// Feature: weapon-types, Property 6: Size classification combat-mode gate
// **Validates: Requirements 7.1, 7.2, 7.3, 7.4, 7.5**
//
// For any (SizeClassification, distance, range) tuple:
//   PISTOL: ranged allowed at distances 1..range
//   BASIC:  ranged allowed at distances 2..range (blocked at d=1)
//   HEAVY:  ranged allowed at distances 2..range (blocked at d=1)
//   MELEE:  ranged NEVER allowed
//   THROWN: ranged allowed at distances 1..range

TEST_CASE("PBT: Property 6 — Size classification combat-mode gate",
          "[pbt][property][weapon-types]")
{
    rc::prop("isRangedAttackAllowed matches size-class rules for random inputs", []() {
        // Generate a random SizeClassification
        auto scIdx = *rc::gen::inRange(0, 5);
        auto sizeClass = static_cast<SizeClassification>(scIdx);

        // Generate a random range (1..50) and distance (1..60, can exceed range)
        int range = *rc::gen::inRange(1, 51);
        int distance = *rc::gen::inRange(1, 61);

        bool allowed = isRangedAttackAllowed(sizeClass, distance, range);

        switch (sizeClass) {
            case SizeClassification::PISTOL:
                // Pistol: allowed at any distance 1..range
                RC_ASSERT(allowed == (distance >= 1 && distance <= range));
                break;
            case SizeClassification::BASIC:
                // Basic: allowed at distances 2..range (blocked at d=1)
                RC_ASSERT(allowed == (distance >= 2 && distance <= range));
                break;
            case SizeClassification::HEAVY:
                // Heavy: allowed at distances 2..range (blocked at d=1)
                RC_ASSERT(allowed == (distance >= 2 && distance <= range));
                break;
            case SizeClassification::MELEE:
                // Melee: ranged NEVER allowed
                RC_ASSERT(allowed == false);
                break;
            case SizeClassification::THROWN:
                // Thrown: allowed at any distance 1..range
                RC_ASSERT(allowed == (distance >= 1 && distance <= range));
                break;
        }
    });
}

// ─── Unit tests: Combat-mode gate specific scenarios ─────────────────────────
// **Validates: Requirements 7.1, 7.2, 7.3, 7.4, 7.5**

TEST_CASE("Pistol at adjacent target (d=1) is allowed", "[weapon-types]")
{
    REQUIRE(isRangedAttackAllowed(SizeClassification::PISTOL, 1, 30));
}

TEST_CASE("Pistol at max range is allowed", "[weapon-types]")
{
    REQUIRE(isRangedAttackAllowed(SizeClassification::PISTOL, 30, 30));
}

TEST_CASE("Pistol beyond range is blocked", "[weapon-types]")
{
    REQUIRE_FALSE(isRangedAttackAllowed(SizeClassification::PISTOL, 31, 30));
}

TEST_CASE("Basic weapon at adjacent target (d=1) is blocked", "[weapon-types]")
{
    REQUIRE_FALSE(isRangedAttackAllowed(SizeClassification::BASIC, 1, 30));
}

TEST_CASE("Basic weapon at distance 2 is allowed", "[weapon-types]")
{
    REQUIRE(isRangedAttackAllowed(SizeClassification::BASIC, 2, 30));
}

TEST_CASE("Basic weapon at max range is allowed", "[weapon-types]")
{
    REQUIRE(isRangedAttackAllowed(SizeClassification::BASIC, 30, 30));
}

TEST_CASE("Basic weapon beyond range is blocked", "[weapon-types]")
{
    REQUIRE_FALSE(isRangedAttackAllowed(SizeClassification::BASIC, 31, 30));
}

TEST_CASE("Heavy weapon at adjacent target (d=1) is blocked", "[weapon-types]")
{
    REQUIRE_FALSE(isRangedAttackAllowed(SizeClassification::HEAVY, 1, 45));
}

TEST_CASE("Heavy weapon at distance 2 is allowed", "[weapon-types]")
{
    REQUIRE(isRangedAttackAllowed(SizeClassification::HEAVY, 2, 45));
}

TEST_CASE("Melee weapon ranged attack is never allowed", "[weapon-types]")
{
    // Melee cannot attack at any distance via ranged
    REQUIRE_FALSE(isRangedAttackAllowed(SizeClassification::MELEE, 1, 1));
    REQUIRE_FALSE(isRangedAttackAllowed(SizeClassification::MELEE, 1, 30));
    REQUIRE_FALSE(isRangedAttackAllowed(SizeClassification::MELEE, 5, 30));
}

TEST_CASE("Thrown weapon at adjacent target (d=1) is allowed", "[weapon-types]")
{
    REQUIRE(isRangedAttackAllowed(SizeClassification::THROWN, 1, 10));
}

TEST_CASE("Thrown weapon at max range is allowed", "[weapon-types]")
{
    REQUIRE(isRangedAttackAllowed(SizeClassification::THROWN, 10, 10));
}

TEST_CASE("Thrown weapon beyond range is blocked", "[weapon-types]")
{
    REQUIRE_FALSE(isRangedAttackAllowed(SizeClassification::THROWN, 11, 10));
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: weapon-types — Serialization Tests (Task 9.1)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 7: Serialization round-trip for classification data ─────────────
// Feature: weapon-types, Property 7: Serialization round-trip for classification data
// **Validates: Requirements 9.1, 9.2**
//
// For any valid combination of (SizeClassification, WeaponGroup, DamageType)
// stored on an Equippable, saving to a TCODZip buffer and loading from that
// buffer produces an Equippable with identical classification values.

TEST_CASE("PBT: Property 7 — Serialization round-trip preserves classification data",
          "[pbt][property][weapon-types]")
{
    const char* tempFile = "__test_classification_roundtrip.sav";

    rc::prop("save/load cycle preserves classification values exactly", [&]() {
        // Generate random classification values
        auto scIdx = *rc::gen::inRange(0, 5);
        auto wgIdx = *rc::gen::inRange(0, 9);
        auto dtIdx = *rc::gen::inRange(0, 4);

        auto sc = static_cast<SizeClassification>(scIdx);
        auto wg = static_cast<WeaponGroup>(wgIdx);
        auto dt = static_cast<DamageType>(dtIdx);

        // Create an Equippable with classification fields set
        Equippable original(EquipmentSlot::WEAPON, StatModifiers{}, 1.0f, 10);
        original.sizeClass = sc;
        original.weaponGroup = wg;
        original.damageType = dt;

        // Optionally add melee or ranged stats (should not affect classification serialization)
        bool addMelee = (*rc::gen::inRange(0, 2)) == 1;
        bool addRanged = (*rc::gen::inRange(0, 2)) == 1;

        if (addMelee) {
            MeleeStats ms;
            ms.damageDice = {1, 10};
            ms.penetration = *rc::gen::inRange(0, 5);
            original.meleeStats = ms;
        }

        if (addRanged) {
            RangedStats rs;
            rs.damageDice = {1, 10};
            rs.penetration = *rc::gen::inRange(0, 5);
            rs.range = *rc::gen::inRange(1, 100);
            rs.rateOfFire = *rc::gen::inRange(1, 4);
            rs.clipSize = *rc::gen::inRange(1, 30);
            rs.reloadTime = 1;
            original.rangedStats = rs;
            original.currentAmmo = *rc::gen::inRange(0, rs.clipSize + 1);
        }

        // Save to file
        TCODZip zip;
        original.save(zip);
        zip.saveToFile(tempFile);

        // Load from file
        TCODZip zip2;
        zip2.loadFromFile(tempFile);
        Equippable loaded(EquipmentSlot::WEAPON, StatModifiers{}, 0.0f, 0);
        loaded.load(zip2);

        // Verify classification fields are preserved exactly
        RC_ASSERT(loaded.sizeClass.has_value());
        RC_ASSERT(loaded.weaponGroup.has_value());
        RC_ASSERT(loaded.damageType.has_value());
        RC_ASSERT(loaded.sizeClass.value() == sc);
        RC_ASSERT(loaded.weaponGroup.value() == wg);
        RC_ASSERT(loaded.damageType.value() == dt);
    });

    // Cleanup temp file
    std::remove(tempFile);
}

// ─── Unit test: Old save compatibility (V2 loads with nullopt classification) ─
// **Validates: Requirements 9.2**
//
// A V2-format save (current format, no classification data) loads with
// classification fields as std::nullopt — graceful degradation for old saves.

TEST_CASE("V2 save data loads with nullopt classification fields (old save compatibility)",
          "[weapon-types]")
{
    const char* tempFile = "__test_v2_compat_classification.sav";

    // Create an Equippable with classification set, but save using current V2 format.
    // The current save() writes V2 (sentinel -10) which does NOT include classification.
    // So after save/load, classification should be nullopt.
    Equippable original(EquipmentSlot::WEAPON, StatModifiers{0.5f, 1.0f, 5.0f, 40}, 2.5f, 100);
    original.sizeClass = SizeClassification::PISTOL;
    original.weaponGroup = WeaponGroup::LAS;
    original.damageType = DamageType::E;

    // Add ranged stats to make it a more realistic weapon
    RangedStats rs;
    rs.damageDice = {1, 10};
    rs.penetration = 0;
    rs.range = 30;
    rs.rateOfFire = 1;
    rs.clipSize = 30;
    rs.reloadTime = 1;
    original.rangedStats = rs;
    original.currentAmmo = 30;

    // Save using current (V2) format
    TCODZip zip;
    original.save(zip);
    zip.saveToFile(tempFile);

    // Load back — V2 format has no classification data
    TCODZip zip2;
    zip2.loadFromFile(tempFile);
    Equippable loaded(EquipmentSlot::WEAPON, StatModifiers{}, 0.0f, 0);
    loaded.load(zip2);

    // Classification fields should be nullopt (old save has no classification data)
    REQUIRE_FALSE(loaded.sizeClass.has_value());
    REQUIRE_FALSE(loaded.weaponGroup.has_value());
    REQUIRE_FALSE(loaded.damageType.has_value());

    // But other fields should still load correctly
    REQUIRE(static_cast<int>(loaded.slot) == static_cast<int>(EquipmentSlot::WEAPON));
    REQUIRE(loaded.modifiers.power == Catch::Approx(0.5f));
    REQUIRE(loaded.modifiers.defense == Catch::Approx(1.0f));
    REQUIRE(loaded.modifiers.maxHp == Catch::Approx(5.0f));
    REQUIRE(loaded.modifiers.skill == 40);
    REQUIRE(loaded.weight == Catch::Approx(2.5f));
    REQUIRE(loaded.value == 100);
    REQUIRE(loaded.rangedStats.has_value());
    REQUIRE(loaded.rangedStats->range == 30);
    REQUIRE(loaded.currentAmmo == 30);

    // Cleanup
    std::remove(tempFile);
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: weapon-types — checkCombatMode Tests (Task 7.1 - message variant)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 6 (extended): checkCombatMode returns message when blocked ─────
// Feature: weapon-types, Property 6: Size classification combat-mode gate
// **Validates: Requirements 7.1, 7.2, 7.3, 7.4, 7.5**
//
// checkCombatMode returns allowed=true with empty message when permitted,
// and allowed=false with non-empty message when blocked.

TEST_CASE("PBT: Property 6 (extended) — checkCombatMode message consistency",
          "[pbt][property][weapon-types]")
{
    rc::prop("checkCombatMode message is empty iff allowed, non-empty iff blocked", []() {
        // Generate a random SizeClassification
        auto scIdx = *rc::gen::inRange(0, 5);
        auto sizeClass = static_cast<SizeClassification>(scIdx);

        // Generate a random range (1..50) and distance (1..60, can exceed range)
        int range = *rc::gen::inRange(1, 51);
        int distance = *rc::gen::inRange(1, 61);

        auto result = checkCombatMode(sizeClass, distance, range);

        // allowed field must match isRangedAttackAllowed (except MELEE at d=1 is a melee-only case)
        // For MELEE: checkCombatMode should block ranged (allowed=false with message)
        // For others: same logic as isRangedAttackAllowed
        bool expectedAllowed = false;
        switch (sizeClass) {
            case SizeClassification::PISTOL:
                expectedAllowed = (distance >= 1 && distance <= range);
                break;
            case SizeClassification::BASIC:
                expectedAllowed = (distance >= 2 && distance <= range);
                break;
            case SizeClassification::HEAVY:
                expectedAllowed = (distance >= 2 && distance <= range);
                break;
            case SizeClassification::MELEE:
                // Melee at d=1: allowed (melee attack). Melee at d>1: blocked.
                expectedAllowed = (distance == 1);
                break;
            case SizeClassification::THROWN:
                expectedAllowed = (distance >= 1 && distance <= range);
                break;
        }

        RC_ASSERT(result.allowed == expectedAllowed);

        // Message consistency: non-empty iff blocked
        if (result.allowed) {
            RC_ASSERT(result.message.empty());
        } else {
            RC_ASSERT(!result.message.empty());
        }
    });
}

// ─── Unit tests: checkCombatMode message content ─────────────────────────────
// **Validates: Requirements 7.1, 7.2, 7.3, 7.4, 7.5**

TEST_CASE("checkCombatMode: Pistol at adjacent target is allowed with empty message", "[weapon-types]")
{
    auto result = checkCombatMode(SizeClassification::PISTOL, 1, 30);
    REQUIRE(result.allowed);
    REQUIRE(result.message.empty());
}

TEST_CASE("checkCombatMode: Basic weapon at adjacent target produces block message", "[weapon-types]")
{
    auto result = checkCombatMode(SizeClassification::BASIC, 1, 30);
    REQUIRE_FALSE(result.allowed);
    REQUIRE_FALSE(result.message.empty());
    // Message should indicate the weapon cannot be used at this range
    REQUIRE(result.message.find("cannot") != std::string::npos);
}

TEST_CASE("checkCombatMode: Heavy weapon at adjacent target produces block message", "[weapon-types]")
{
    auto result = checkCombatMode(SizeClassification::HEAVY, 1, 45);
    REQUIRE_FALSE(result.allowed);
    REQUIRE_FALSE(result.message.empty());
    REQUIRE(result.message.find("cannot") != std::string::npos);
}

TEST_CASE("checkCombatMode: Basic weapon at distance 2 is allowed", "[weapon-types]")
{
    auto result = checkCombatMode(SizeClassification::BASIC, 2, 30);
    REQUIRE(result.allowed);
    REQUIRE(result.message.empty());
}

TEST_CASE("checkCombatMode: Melee weapon at adjacent target is allowed", "[weapon-types]")
{
    auto result = checkCombatMode(SizeClassification::MELEE, 1, 1);
    REQUIRE(result.allowed);
    REQUIRE(result.message.empty());
}

TEST_CASE("checkCombatMode: Melee weapon at distance > 1 produces block message", "[weapon-types]")
{
    auto result = checkCombatMode(SizeClassification::MELEE, 5, 30);
    REQUIRE_FALSE(result.allowed);
    REQUIRE_FALSE(result.message.empty());
}

TEST_CASE("checkCombatMode: Thrown weapon at any distance within range is allowed", "[weapon-types]")
{
    auto result = checkCombatMode(SizeClassification::THROWN, 1, 10);
    REQUIRE(result.allowed);
    REQUIRE(result.message.empty());

    auto result2 = checkCombatMode(SizeClassification::THROWN, 10, 10);
    REQUIRE(result2.allowed);
    REQUIRE(result2.message.empty());
}

TEST_CASE("checkCombatMode: Thrown weapon beyond range is blocked", "[weapon-types]")
{
    auto result = checkCombatMode(SizeClassification::THROWN, 11, 10);
    REQUIRE_FALSE(result.allowed);
    REQUIRE_FALSE(result.message.empty());
}

TEST_CASE("checkCombatMode: Pistol beyond range is blocked with message", "[weapon-types]")
{
    auto result = checkCombatMode(SizeClassification::PISTOL, 31, 30);
    REQUIRE_FALSE(result.allowed);
    REQUIRE_FALSE(result.message.empty());
}
