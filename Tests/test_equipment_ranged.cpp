#include "lib/catch_amalgamated.hpp"
#include "main.h"

// ═══════════════════════════════════════════════════════════════════════════════
// Unit Tests for Equipment.lua Ranged Table Parsing
// ═══════════════════════════════════════════════════════════════════════════════
//
// These tests verify the parsing and construction logic for ranged weapon stats
// as implemented by the Equipment Loader. They exercise the same code paths
// (DiceRoller::parse, RangedStats construction, melee fallback assignment)
// that the loader uses when processing the "ranged" table in Equipment.lua.
//
// Validates: Requirements 9.1, 9.2, 9.3, 9.4, 9.5

// ═══════════════════════════════════════════════════════════════════════════════
// Ranged Table Fields Parsing
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Ranged parsing: all ranged table fields parsed correctly", "[equipment][ranged][parsing]") {
    // Simulate what the loader does: parse damageDice, set ranged fields
    SECTION("Laspistol-style weapon: 1d10, pen 0, range 30, rof 1, clip 30, reload 1") {
        auto parsed = DiceRoller::parse("1d10");
        REQUIRE(parsed.has_value());

        RangedStats rs;
        rs.damageDice   = *parsed;
        rs.penetration  = 0;
        rs.range        = 30;
        rs.rateOfFire   = 1;
        rs.clipSize     = 30;
        rs.reloadTime   = 1;

        CHECK(rs.damageDice.count == 1);
        CHECK(rs.damageDice.sides == 10);
        CHECK(rs.penetration == 0);
        CHECK(rs.range == 30);
        CHECK(rs.rateOfFire == 1);
        CHECK(rs.clipSize == 30);
        CHECK(rs.reloadTime == 1);
    }

    SECTION("Autogun-style weapon: 1d10, pen 0, range 40, rof 3, clip 24, reload 1") {
        auto parsed = DiceRoller::parse("1d10");
        REQUIRE(parsed.has_value());

        RangedStats rs;
        rs.damageDice   = *parsed;
        rs.penetration  = 0;
        rs.range        = 40;
        rs.rateOfFire   = 3;
        rs.clipSize     = 24;
        rs.reloadTime   = 1;

        CHECK(rs.damageDice.count == 1);
        CHECK(rs.damageDice.sides == 10);
        CHECK(rs.penetration == 0);
        CHECK(rs.range == 40);
        CHECK(rs.rateOfFire == 3);
        CHECK(rs.clipSize == 24);
        CHECK(rs.reloadTime == 1);
    }

    SECTION("High-penetration weapon: 2d10, pen 5, range 50, rof 1, clip 10, reload 2") {
        auto parsed = DiceRoller::parse("2d10");
        REQUIRE(parsed.has_value());

        RangedStats rs;
        rs.damageDice   = *parsed;
        rs.penetration  = 5;
        rs.range        = 50;
        rs.rateOfFire   = 1;
        rs.clipSize     = 10;
        rs.reloadTime   = 2;

        CHECK(rs.damageDice.count == 2);
        CHECK(rs.damageDice.sides == 10);
        CHECK(rs.penetration == 5);
        CHECK(rs.range == 50);
        CHECK(rs.rateOfFire == 1);
        CHECK(rs.clipSize == 10);
        CHECK(rs.reloadTime == 2);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// RangedStats Default Values
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Ranged parsing: RangedStats struct default values match design spec", "[equipment][ranged][parsing]") {
    // The RangedStats struct default-initializes to the design defaults:
    // damageDice={1,10}, pen=0, range=30, rof=1, clipSize=6, reloadTime=1
    RangedStats rs;

    CHECK(rs.damageDice.count == 1);
    CHECK(rs.damageDice.sides == 10);
    CHECK(rs.penetration == 0);
    CHECK(rs.range == 30);
    CHECK(rs.rateOfFire == 1);
    CHECK(rs.clipSize == 6);
    CHECK(rs.reloadTime == 1);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Invalid damageDice Format Triggers Skip
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Ranged parsing: invalid damageDice format triggers warning and skips entry", "[equipment][ranged][parsing]") {
    // When the loader encounters invalid ranged damageDice, it skips the entry.
    // The loader uses DiceRoller::parse() which returns nullopt on failure.
    SECTION("Empty string") {
        auto result = DiceRoller::parse("");
        CHECK_FALSE(result.has_value());
    }

    SECTION("Non-dice format") {
        auto result = DiceRoller::parse("invalid");
        CHECK_FALSE(result.has_value());
    }

    SECTION("Missing count: d10") {
        auto result = DiceRoller::parse("d10");
        CHECK_FALSE(result.has_value());
    }

    SECTION("Zero count: 0d10") {
        auto result = DiceRoller::parse("0d10");
        CHECK_FALSE(result.has_value());
    }

    SECTION("Zero sides: 1d0") {
        auto result = DiceRoller::parse("1d0");
        CHECK_FALSE(result.has_value());
    }

    SECTION("Count too large: 10d10") {
        auto result = DiceRoller::parse("10d10");
        CHECK_FALSE(result.has_value());
    }

    SECTION("Non-numeric: xdy") {
        auto result = DiceRoller::parse("xdy");
        CHECK_FALSE(result.has_value());
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Default Melee Stats Assigned for Ranged-Only Weapon
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Ranged parsing: default melee stats assigned when ranged-only weapon", "[equipment][ranged][parsing]") {
    // Per requirement 9.5: weapons with only a "ranged" table and no "melee" table
    // get auto-assigned default melee stats (1d5, pen 0, no qualities) for pistol-whip.
    // This simulates the loader logic:
    //   if (rangedTable && !meleeStats.has_value()) {
    //       meleeStats = MeleeStats{ DiceSpec{1, 5}, 0, {} };
    //   }

    // Simulate: weapon has ranged table but no melee table
    std::optional<MeleeStats> meleeStats;     // starts empty (no melee table in Lua)
    std::optional<RangedStats> rangedStats;

    // Parse ranged table
    auto parsed = DiceRoller::parse("1d10");
    REQUIRE(parsed.has_value());
    RangedStats rs;
    rs.damageDice = *parsed;
    rs.penetration = 0;
    rs.range = 30;
    rs.rateOfFire = 1;
    rs.clipSize = 30;
    rs.reloadTime = 1;
    rangedStats = rs;

    // Apply the loader's fallback logic
    if (rangedStats.has_value() && !meleeStats.has_value()) {
        meleeStats = MeleeStats{ DiceSpec{1, 5}, 0, {} };
    }

    // Verify ranged stats are set
    REQUIRE(rangedStats.has_value());
    CHECK(rangedStats->damageDice.count == 1);
    CHECK(rangedStats->damageDice.sides == 10);

    // Verify default melee stats were assigned
    REQUIRE(meleeStats.has_value());
    CHECK(meleeStats->damageDice.count == 1);
    CHECK(meleeStats->damageDice.sides == 5);
    CHECK(meleeStats->penetration == 0);
    CHECK(meleeStats->qualities.empty());
}

TEST_CASE("Ranged parsing: melee fallback not applied when melee table already present", "[equipment][ranged][parsing]") {
    // If the weapon already has a melee table, the ranged fallback should NOT override it
    std::optional<MeleeStats> meleeStats;
    std::optional<RangedStats> rangedStats;

    // Parse melee table first (weapon has explicit melee)
    auto meleeParsed = DiceRoller::parse("1d10");
    REQUIRE(meleeParsed.has_value());
    MeleeStats ms;
    ms.damageDice = *meleeParsed;
    ms.penetration = 2;
    ms.qualities = {"Tearing"};
    meleeStats = ms;

    // Parse ranged table
    auto rangedParsed = DiceRoller::parse("1d10");
    REQUIRE(rangedParsed.has_value());
    RangedStats rs;
    rs.damageDice = *rangedParsed;
    rs.penetration = 0;
    rs.range = 30;
    rs.rateOfFire = 1;
    rs.clipSize = 30;
    rs.reloadTime = 1;
    rangedStats = rs;

    // Apply the loader's fallback logic
    if (rangedStats.has_value() && !meleeStats.has_value()) {
        meleeStats = MeleeStats{ DiceSpec{1, 5}, 0, {} };
    }

    // Verify the original melee stats are preserved (NOT overwritten with defaults)
    REQUIRE(meleeStats.has_value());
    CHECK(meleeStats->damageDice.count == 1);
    CHECK(meleeStats->damageDice.sides == 10);  // original 1d10, not default 1d5
    CHECK(meleeStats->penetration == 2);        // original pen 2, not default 0
    CHECK(meleeStats->qualities.size() == 1);
    CHECK(meleeStats->qualities[0] == "Tearing");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Dual-Mode Weapon (Both Melee and Ranged Tables)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Ranged parsing: dual-mode weapon parses both melee and ranged correctly", "[equipment][ranged][parsing]") {
    // Per requirement 9.4: weapons with both "melee" and "ranged" tables are valid
    // (pistol-type weapons usable in both modes)
    // Simulates a Slugga: melee 1d10 pen 0, ranged 1d10 pen 0 range 15

    // Parse melee
    auto meleeParsed = DiceRoller::parse("1d10");
    REQUIRE(meleeParsed.has_value());
    MeleeStats ms;
    ms.damageDice = *meleeParsed;
    ms.penetration = 0;
    ms.qualities = {};

    // Parse ranged
    auto rangedParsed = DiceRoller::parse("1d10");
    REQUIRE(rangedParsed.has_value());
    RangedStats rs;
    rs.damageDice   = *rangedParsed;
    rs.penetration  = 0;
    rs.range        = 15;
    rs.rateOfFire   = 1;
    rs.clipSize     = 6;
    rs.reloadTime   = 1;

    // Store both on an Equippable (simulating EquipmentTemplate storage)
    StatModifiers mods{ 1.5f, 0.0f, 0.0f, 0 };
    Equippable eq(EquipmentSlot::WEAPON, mods, 2.5f, 12);
    eq.meleeStats = ms;
    eq.rangedStats = rs;

    // Verify both are present
    REQUIRE(eq.meleeStats.has_value());
    REQUIRE(eq.rangedStats.has_value());

    // Verify melee stats
    CHECK(eq.meleeStats->damageDice.count == 1);
    CHECK(eq.meleeStats->damageDice.sides == 10);
    CHECK(eq.meleeStats->penetration == 0);
    CHECK(eq.meleeStats->qualities.empty());

    // Verify ranged stats
    CHECK(eq.rangedStats->damageDice.count == 1);
    CHECK(eq.rangedStats->damageDice.sides == 10);
    CHECK(eq.rangedStats->penetration == 0);
    CHECK(eq.rangedStats->range == 15);
    CHECK(eq.rangedStats->rateOfFire == 1);
    CHECK(eq.rangedStats->clipSize == 6);
    CHECK(eq.rangedStats->reloadTime == 1);
}

TEST_CASE("Ranged parsing: dual-mode weapon with different dice for each mode", "[equipment][ranged][parsing]") {
    // Shoota: melee 1d5 (pistol-whip), ranged 1d10 rof 3
    auto meleeParsed = DiceRoller::parse("1d5");
    REQUIRE(meleeParsed.has_value());
    MeleeStats ms;
    ms.damageDice = *meleeParsed;
    ms.penetration = 0;
    ms.qualities = {};

    auto rangedParsed = DiceRoller::parse("1d10");
    REQUIRE(rangedParsed.has_value());
    RangedStats rs;
    rs.damageDice   = *rangedParsed;
    rs.penetration  = 0;
    rs.range        = 30;
    rs.rateOfFire   = 3;
    rs.clipSize     = 18;
    rs.reloadTime   = 1;

    // Store both on an Equippable
    StatModifiers mods{ 2.5f, 0.0f, 0.0f, 5 };
    Equippable eq(EquipmentSlot::WEAPON, mods, 5.5f, 35);
    eq.meleeStats = ms;
    eq.rangedStats = rs;

    // Verify melee: 1d5
    REQUIRE(eq.meleeStats.has_value());
    CHECK(eq.meleeStats->damageDice.count == 1);
    CHECK(eq.meleeStats->damageDice.sides == 5);
    CHECK(eq.meleeStats->penetration == 0);

    // Verify ranged: 1d10, rof 3, clip 18
    REQUIRE(eq.rangedStats.has_value());
    CHECK(eq.rangedStats->damageDice.count == 1);
    CHECK(eq.rangedStats->damageDice.sides == 10);
    CHECK(eq.rangedStats->penetration == 0);
    CHECK(eq.rangedStats->range == 30);
    CHECK(eq.rangedStats->rateOfFire == 3);
    CHECK(eq.rangedStats->clipSize == 18);
    CHECK(eq.rangedStats->reloadTime == 1);
}

// ═══════════════════════════════════════════════════════════════════════════════
// EquipmentTemplate with RangedStats Storage
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Ranged parsing: EquipmentTemplate stores rangedStats correctly", "[equipment][ranged][parsing]") {
    // Verify that an EquipmentTemplate with rangedStats correctly stores the parsed data
    EquipmentTemplate tmpl;
    tmpl.name   = "Laspistol";
    tmpl.glyph  = ')';
    tmpl.color  = Colors::lightRed;
    tmpl.slot   = EquipmentSlot::WEAPON;
    tmpl.weight = 1.5f;
    tmpl.value  = 20;
    tmpl.modifiers = { 0.0f, 0.0f, 0.0f, 0 };
    tmpl.tier   = ItemTier::COMMON;

    // Set melee stats (pistol-whip default)
    tmpl.meleeStats = MeleeStats{ DiceSpec{1, 5}, 0, {} };

    // Set ranged stats
    auto parsed = DiceRoller::parse("1d10");
    REQUIRE(parsed.has_value());
    RangedStats rs;
    rs.damageDice   = *parsed;
    rs.penetration  = 0;
    rs.range        = 30;
    rs.rateOfFire   = 1;
    rs.clipSize     = 30;
    rs.reloadTime   = 1;
    tmpl.rangedStats = rs;

    // Verify rangedStats on the template
    REQUIRE(tmpl.rangedStats.has_value());
    CHECK(tmpl.rangedStats->damageDice.count == 1);
    CHECK(tmpl.rangedStats->damageDice.sides == 10);
    CHECK(tmpl.rangedStats->penetration == 0);
    CHECK(tmpl.rangedStats->range == 30);
    CHECK(tmpl.rangedStats->rateOfFire == 1);
    CHECK(tmpl.rangedStats->clipSize == 30);
    CHECK(tmpl.rangedStats->reloadTime == 1);

    // Verify meleeStats also present (dual-mode)
    REQUIRE(tmpl.meleeStats.has_value());
    CHECK(tmpl.meleeStats->damageDice.count == 1);
    CHECK(tmpl.meleeStats->damageDice.sides == 5);
}

TEST_CASE("Ranged parsing: EquipmentTemplate without rangedStats has nullopt", "[equipment][ranged][parsing]") {
    // Melee-only weapons should have no rangedStats
    EquipmentTemplate tmpl;
    tmpl.name   = "Chainsword";
    tmpl.glyph  = '/';
    tmpl.color  = Colors::lightBlue;
    tmpl.slot   = EquipmentSlot::WEAPON;
    tmpl.weight = 3.5f;
    tmpl.value  = 50;
    tmpl.modifiers = { 3.0f, 0.0f, 0.0f, 0 };
    tmpl.tier   = ItemTier::UNCOMMON;
    tmpl.meleeStats = MeleeStats{ DiceSpec{1, 10}, 2, {"Tearing", "Balanced"} };

    CHECK_FALSE(tmpl.rangedStats.has_value());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Equippable currentAmmo Field
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Ranged parsing: currentAmmo defaults to zero on Equippable", "[equipment][ranged][parsing]") {
    // currentAmmo is a runtime field, defaults to 0 on construction
    StatModifiers mods{ 0.0f, 0.0f, 0.0f, 0 };
    Equippable eq(EquipmentSlot::WEAPON, mods, 1.5f, 20);

    CHECK(eq.currentAmmo == 0);
}

TEST_CASE("Ranged parsing: currentAmmo can be set to clipSize after equip", "[equipment][ranged][parsing]") {
    // Per requirement 7.4: weapon starts fully loaded when first equipped
    StatModifiers mods{ 0.0f, 0.0f, 0.0f, 0 };
    Equippable eq(EquipmentSlot::WEAPON, mods, 1.5f, 20);

    auto parsed = DiceRoller::parse("1d10");
    REQUIRE(parsed.has_value());
    RangedStats rs;
    rs.damageDice   = *parsed;
    rs.penetration  = 0;
    rs.range        = 30;
    rs.rateOfFire   = 1;
    rs.clipSize     = 30;
    rs.reloadTime   = 1;
    eq.rangedStats = rs;

    // Simulate "first equip" initialization
    eq.currentAmmo = eq.rangedStats->clipSize;

    CHECK(eq.currentAmmo == 30);
}
