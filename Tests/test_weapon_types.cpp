#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "WeaponTypes.h"

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
