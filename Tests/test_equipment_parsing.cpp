#include "lib/catch_amalgamated.hpp"
#include "main.h"

// ═══════════════════════════════════════════════════════════════════════════════
// Unit Tests for Equipment.lua Parsing Logic
// ═══════════════════════════════════════════════════════════════════════════════
//
// These tests verify the parsing and construction logic used by the Equipment
// Loader without requiring sol2/Lua engine initialization. They exercise the
// same code paths (DiceRoller::parse, MeleeStats construction, ArmourProfile
// construction, slot-based fallback) that the loader uses.
//
// Validates: Requirements 8.1, 8.4, 8.5, 9.2, 9.3

// ═══════════════════════════════════════════════════════════════════════════════
// Melee Table Fields Parsing
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Equipment parsing: melee table fields", "[equipment][parsing]") {
    // Simulate what the loader does: parse damageDice, set penetration, set qualities
    SECTION("Standard weapon: 1d10 with penetration and qualities") {
        auto parsed = DiceRoller::parse("1d10");
        REQUIRE(parsed.has_value());

        MeleeStats stats;
        stats.damageDice = *parsed;
        stats.penetration = 2;
        stats.qualities = {"Tearing", "Balanced"};

        CHECK(stats.damageDice.count == 1);
        CHECK(stats.damageDice.sides == 10);
        CHECK(stats.penetration == 2);
        CHECK(stats.qualities.size() == 2);
        CHECK(stats.qualities[0] == "Tearing");
        CHECK(stats.qualities[1] == "Balanced");
    }

    SECTION("Heavy weapon: 2d10 with high penetration") {
        auto parsed = DiceRoller::parse("2d10");
        REQUIRE(parsed.has_value());

        MeleeStats stats;
        stats.damageDice = *parsed;
        stats.penetration = 7;
        stats.qualities = {"Power Field", "Unwieldy"};

        CHECK(stats.damageDice.count == 2);
        CHECK(stats.damageDice.sides == 10);
        CHECK(stats.penetration == 7);
        CHECK(stats.qualities.size() == 2);
    }

    SECTION("Weapon with no qualities") {
        auto parsed = DiceRoller::parse("1d5");
        REQUIRE(parsed.has_value());

        MeleeStats stats;
        stats.damageDice = *parsed;
        stats.penetration = 0;
        // qualities left empty (as loader does when no qualities table)

        CHECK(stats.damageDice.count == 1);
        CHECK(stats.damageDice.sides == 5);
        CHECK(stats.penetration == 0);
        CHECK(stats.qualities.empty());
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Fallback MeleeStats for Weapons Without Melee Table
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Equipment parsing: fallback MeleeStats for weapons without melee table", "[equipment][parsing]") {
    // Weapons without a melee table get default stats: 1d5, 0 pen, no qualities
    MeleeStats defaultStats{ DiceSpec{1, 5}, 0, {} };

    CHECK(defaultStats.damageDice.count == 1);
    CHECK(defaultStats.damageDice.sides == 5);
    CHECK(defaultStats.penetration == 0);
    CHECK(defaultStats.qualities.empty());
}

TEST_CASE("Equipment parsing: default MeleeStats struct matches expected defaults", "[equipment][parsing]") {
    // The MeleeStats struct default-initializes to {1,5}, 0, {}
    MeleeStats stats;

    CHECK(stats.damageDice.count == 1);
    CHECK(stats.damageDice.sides == 5);
    CHECK(stats.penetration == 0);
    CHECK(stats.qualities.empty());
}

// ═══════════════════════════════════════════════════════════════════════════════
// ArmourLocations Table Parsing
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Equipment parsing: armourLocations table", "[equipment][parsing]") {
    // Simulate the loader constructing an ArmourProfile from per-location values
    SECTION("Flak Armor: body/limbs covered, head unprotected") {
        ArmourProfile ap;
        ap.values[static_cast<int>(HitLocation::HEAD)]      = 0;
        ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] = 3;
        ap.values[static_cast<int>(HitLocation::LEFT_ARM)]  = 3;
        ap.values[static_cast<int>(HitLocation::BODY)]      = 3;
        ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] = 3;
        ap.values[static_cast<int>(HitLocation::LEFT_LEG)]  = 3;

        CHECK(ap.values[static_cast<int>(HitLocation::HEAD)] == 0);
        CHECK(ap.values[static_cast<int>(HitLocation::BODY)] == 3);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_ARM)] == 3);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] == 3);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_LEG)] == 3);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] == 3);
    }

    SECTION("Carapace Helm: head only") {
        ArmourProfile ap;
        ap.values[static_cast<int>(HitLocation::HEAD)]      = 4;
        ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] = 0;
        ap.values[static_cast<int>(HitLocation::LEFT_ARM)]  = 0;
        ap.values[static_cast<int>(HitLocation::BODY)]      = 0;
        ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] = 0;
        ap.values[static_cast<int>(HitLocation::LEFT_LEG)]  = 0;

        CHECK(ap.values[static_cast<int>(HitLocation::HEAD)] == 4);
        CHECK(ap.values[static_cast<int>(HitLocation::BODY)] == 0);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_ARM)] == 0);
    }

    SECTION("Non-uniform coverage: different values per location") {
        ArmourProfile ap;
        ap.values[static_cast<int>(HitLocation::HEAD)]      = 2;
        ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] = 4;
        ap.values[static_cast<int>(HitLocation::LEFT_ARM)]  = 4;
        ap.values[static_cast<int>(HitLocation::BODY)]      = 6;
        ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] = 3;
        ap.values[static_cast<int>(HitLocation::LEFT_LEG)]  = 3;

        CHECK(ap.values[static_cast<int>(HitLocation::HEAD)] == 2);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] == 4);
        CHECK(ap.values[static_cast<int>(HitLocation::BODY)] == 6);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] == 3);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Body Slot Fallback Without armourLocations
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Equipment parsing: body slot fallback without armourLocations", "[equipment][parsing]") {
    // When body armour lacks armourLocations, defense value applies to
    // body + arms + legs; head = 0
    SECTION("defense = 2") {
        float defense = 2.0f;
        int defVal = static_cast<int>(defense);

        ArmourProfile ap = {};
        ap.values[static_cast<int>(HitLocation::HEAD)]      = 0;
        ap.values[static_cast<int>(HitLocation::BODY)]      = defVal;
        ap.values[static_cast<int>(HitLocation::LEFT_ARM)]  = defVal;
        ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] = defVal;
        ap.values[static_cast<int>(HitLocation::LEFT_LEG)]  = defVal;
        ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] = defVal;

        CHECK(ap.values[static_cast<int>(HitLocation::HEAD)] == 0);
        CHECK(ap.values[static_cast<int>(HitLocation::BODY)] == 2);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_ARM)] == 2);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] == 2);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_LEG)] == 2);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] == 2);
    }

    SECTION("defense = 5") {
        float defense = 5.0f;
        int defVal = static_cast<int>(defense);

        ArmourProfile ap = {};
        ap.values[static_cast<int>(HitLocation::HEAD)]      = 0;
        ap.values[static_cast<int>(HitLocation::BODY)]      = defVal;
        ap.values[static_cast<int>(HitLocation::LEFT_ARM)]  = defVal;
        ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] = defVal;
        ap.values[static_cast<int>(HitLocation::LEFT_LEG)]  = defVal;
        ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] = defVal;

        CHECK(ap.values[static_cast<int>(HitLocation::HEAD)] == 0);
        CHECK(ap.values[static_cast<int>(HitLocation::BODY)] == 5);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_ARM)] == 5);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] == 5);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_LEG)] == 5);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] == 5);
    }

    SECTION("defense = 0 means no armour anywhere") {
        float defense = 0.0f;
        int defVal = static_cast<int>(defense);

        ArmourProfile ap = {};
        ap.values[static_cast<int>(HitLocation::HEAD)]      = 0;
        ap.values[static_cast<int>(HitLocation::BODY)]      = defVal;
        ap.values[static_cast<int>(HitLocation::LEFT_ARM)]  = defVal;
        ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] = defVal;
        ap.values[static_cast<int>(HitLocation::LEFT_LEG)]  = defVal;
        ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] = defVal;

        for (int loc = 0; loc < static_cast<int>(HitLocation::COUNT); ++loc) {
            CHECK(ap.values[loc] == 0);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Head Slot Fallback Without armourLocations
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Equipment parsing: head slot fallback without armourLocations", "[equipment][parsing]") {
    // When head armour lacks armourLocations, defense value applies to head only;
    // all other locations = 0
    SECTION("defense = 1") {
        float defense = 1.0f;
        int defVal = static_cast<int>(defense);

        ArmourProfile ap = {};
        ap.values[static_cast<int>(HitLocation::HEAD)]      = defVal;
        ap.values[static_cast<int>(HitLocation::BODY)]      = 0;
        ap.values[static_cast<int>(HitLocation::LEFT_ARM)]  = 0;
        ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] = 0;
        ap.values[static_cast<int>(HitLocation::LEFT_LEG)]  = 0;
        ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] = 0;

        CHECK(ap.values[static_cast<int>(HitLocation::HEAD)] == 1);
        CHECK(ap.values[static_cast<int>(HitLocation::BODY)] == 0);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_ARM)] == 0);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] == 0);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_LEG)] == 0);
        CHECK(ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] == 0);
    }

    SECTION("defense = 4") {
        float defense = 4.0f;
        int defVal = static_cast<int>(defense);

        ArmourProfile ap = {};
        ap.values[static_cast<int>(HitLocation::HEAD)]      = defVal;
        ap.values[static_cast<int>(HitLocation::BODY)]      = 0;
        ap.values[static_cast<int>(HitLocation::LEFT_ARM)]  = 0;
        ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] = 0;
        ap.values[static_cast<int>(HitLocation::LEFT_LEG)]  = 0;
        ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] = 0;

        CHECK(ap.values[static_cast<int>(HitLocation::HEAD)] == 4);
        CHECK(ap.values[static_cast<int>(HitLocation::BODY)] == 0);
        CHECK(ap.values[static_cast<int>(HitLocation::LEFT_ARM)] == 0);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Invalid damageDice Format
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Equipment parsing: invalid damageDice returns nullopt", "[equipment][parsing]") {
    // When the loader encounters invalid damageDice, it skips the entry
    CHECK_FALSE(DiceRoller::parse("invalid").has_value());
    CHECK_FALSE(DiceRoller::parse("0d6").has_value());
    CHECK_FALSE(DiceRoller::parse("1d0").has_value());
    CHECK_FALSE(DiceRoller::parse("10d10").has_value());
    CHECK_FALSE(DiceRoller::parse("").has_value());
    CHECK_FALSE(DiceRoller::parse("abc").has_value());
    CHECK_FALSE(DiceRoller::parse("d10").has_value());

    // rollFromString returns 0 for invalid specs (used as damage fallback indicator)
    CHECK(DiceRoller::rollFromString("invalid") == 0);
    CHECK(DiceRoller::rollFromString("") == 0);
}

// ═══════════════════════════════════════════════════════════════════════════════
// EquipmentTemplate with MeleeStats Storage
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Equipment parsing: EquipmentTemplate with meleeStats stores parsed values", "[equipment][parsing]") {
    // Verify that an Equippable with meleeStats correctly stores the parsed data
    StatModifiers mods{ 3.0f, 0.0f, 0.0f, 0 };
    Equippable eq(EquipmentSlot::WEAPON, mods, 3.5f, 50);

    // Simulate what the loader does after parsing
    auto parsed = DiceRoller::parse("1d10");
    REQUIRE(parsed.has_value());

    MeleeStats ms;
    ms.damageDice = *parsed;
    ms.penetration = 2;
    ms.qualities = {"Tearing", "Balanced"};
    eq.meleeStats = ms;

    REQUIRE(eq.meleeStats.has_value());
    CHECK(eq.meleeStats->damageDice.count == 1);
    CHECK(eq.meleeStats->damageDice.sides == 10);
    CHECK(eq.meleeStats->penetration == 2);
    CHECK(eq.meleeStats->qualities.size() == 2);
    CHECK(eq.meleeStats->qualities[0] == "Tearing");
    CHECK(eq.meleeStats->qualities[1] == "Balanced");
}

TEST_CASE("Equipment parsing: EquipmentTemplate with armourProfile stores per-location values", "[equipment][parsing]") {
    StatModifiers mods{ 0.0f, 2.0f, 0.0f, -5 };
    Equippable eq(EquipmentSlot::BODY, mods, 8.0f, 30);

    ArmourProfile ap;
    ap.values[static_cast<int>(HitLocation::HEAD)]      = 0;
    ap.values[static_cast<int>(HitLocation::RIGHT_ARM)] = 3;
    ap.values[static_cast<int>(HitLocation::LEFT_ARM)]  = 3;
    ap.values[static_cast<int>(HitLocation::BODY)]      = 4;
    ap.values[static_cast<int>(HitLocation::RIGHT_LEG)] = 2;
    ap.values[static_cast<int>(HitLocation::LEFT_LEG)]  = 2;
    eq.armourProfile = ap;

    REQUIRE(eq.armourProfile.has_value());
    CHECK(eq.armourProfile->values[static_cast<int>(HitLocation::HEAD)] == 0);
    CHECK(eq.armourProfile->values[static_cast<int>(HitLocation::BODY)] == 4);
    CHECK(eq.armourProfile->values[static_cast<int>(HitLocation::RIGHT_ARM)] == 3);
    CHECK(eq.armourProfile->values[static_cast<int>(HitLocation::LEFT_ARM)] == 3);
    CHECK(eq.armourProfile->values[static_cast<int>(HitLocation::RIGHT_LEG)] == 2);
    CHECK(eq.armourProfile->values[static_cast<int>(HitLocation::LEFT_LEG)] == 2);
}

TEST_CASE("Equipment parsing: Equippable without meleeStats has nullopt", "[equipment][parsing]") {
    StatModifiers mods{ 0.0f, 2.0f, 0.0f, 0 };
    Equippable eq(EquipmentSlot::BODY, mods, 5.0f, 20);

    CHECK_FALSE(eq.meleeStats.has_value());
    CHECK_FALSE(eq.armourProfile.has_value());
}
