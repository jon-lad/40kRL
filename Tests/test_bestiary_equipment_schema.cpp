#include "lib/catch_amalgamated.hpp"

#include "BestiaryTestHarness.hpp"

#include <optional>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — example tests for the new equipment schema (Task 12.2)
//
// These tests load Scripts/Equipment.lua through the engine-isolated
// BestiaryTestHarness (a private sol::state, no global Engine) and:
//
//   1. Spot-check a NEW ranged weapon  (Lasgun,          `ranged` populated)
//   2. Spot-check a NEW melee weapon   (Eldar Chainsword, `melee` populated)
//   3. Spot-check a NEW body armour    (Light Flak Coat, `armourLocations` populated)
//   4. Assert every PRE-EXISTING entry remains byte-identical to the shipped script.
//
// The three new-entry spot-checks are the values the design cites:
//   - Lasgun          — design.md §5 example (Hired Gun profile):
//                        1d10+3 E; Pen 0; Basic 30m; S/3/–; Clip 60; Reload Full
//   - Eldar Chainsword — Reference/RT-Bestiary.md (Eldar Guardian, IV.4 Troop):
//                        1d10+5 R [2R+3StB]; Pen 2; Melee; Balanced, Razor Sharp, Tearing
//   - Light Flak Coat  — design.md §5 example (Hired Gun profile):
//                        Arms 2, Body 2, Legs 2
//
// TDD-RED: written before Task 12.3 adds the new weapon/armour entries to
// Equipment.lua. Until 12.3 lands, the three new-entry spot-checks are EXPECTED
// TO FAIL (the entries do not yet exist). The "pre-existing entries unchanged"
// checks assert the current shipped entries are byte-identical and should PASS now.
//
// Validates: Requirements 9.2, 9.3, 9.4, 9.5, 9.7
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Fetch a nested sub-table (ranged / melee / armourLocations) off an equipment
// entry, or nullopt when the key is absent / not a table.
std::optional<sol::table> subTable(const bestiary::EquipmentEntry& e, const char* key) {
    sol::object obj = e.table[key];
    if (obj.get_type() != sol::type::table) return std::nullopt;
    return obj.as<sol::table>();
}

// Read an int off a sub-table with a sentinel fallback.
int tblInt(const sol::table& t, const char* key, int fallback) {
    sol::optional<int> v = t[key];
    return v ? *v : fallback;
}

// Read a string off a sub-table with a sentinel fallback.
std::string tblStr(const sol::table& t, const char* key, const std::string& fallback) {
    sol::optional<std::string> v = t[key];
    return v ? *v : fallback;
}

} // namespace

// ── New ranged weapon: Lasgun (`ranged` populated) ─────────────────────────────
//
// Validates: Requirements 9.2 (schema shape), 9.3 (ranged table populated).
TEST_CASE("New ranged weapon Lasgun has a populated ranged table",
          "[bestiary-npcs][equipment][Feature: bestiary-npcs, Task 12.2]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this test.");
    }

    auto entry = harness.equipmentByName("Lasgun");
    REQUIRE(entry.has_value());

    // Requirement 9.2: base schema fields.
    CHECK_FALSE(entry->name.empty());
    CHECK(entry->name == "Lasgun");
    CHECK(entry->fieldOr<std::string>("glyph", "").size() == 1);
    CHECK_FALSE(entry->fieldOr<std::string>("color", "").empty());
    CHECK(entry->fieldOr<std::string>("slot", "") == "weapon");
    CHECK(entry->fieldOr<double>("weight", -1.0) >= 0.0);

    // Requirement 9.3: the ranged sub-table is populated with the cited profile.
    auto ranged = subTable(*entry, "ranged");
    REQUIRE(ranged.has_value());
    CHECK(tblStr(*ranged, "damageDice", "") == "1d10+3");
    CHECK(tblInt(*ranged, "penetration", -1) == 0);
    CHECK(tblInt(*ranged, "range", -1) == 30);
    CHECK(tblInt(*ranged, "rateOfFire", -1) == 3);
    CHECK(tblInt(*ranged, "clipSize", -1) == 60);
    CHECK(tblInt(*ranged, "reloadTime", -1) == 1);
    // range must be > 0 for the loader to accept the entry.
    CHECK(tblInt(*ranged, "range", 0) > 0);
}

// ── New melee weapon: Eldar Chainsword (`melee` populated) ──────────────────────
//
// Validates: Requirements 9.2 (schema shape), 9.4 (melee table populated).
TEST_CASE("New melee weapon Eldar Chainsword has a populated melee table",
          "[bestiary-npcs][equipment][Feature: bestiary-npcs, Task 12.2]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this test.");
    }

    auto entry = harness.equipmentByName("Eldar Chainsword");
    REQUIRE(entry.has_value());

    // Requirement 9.2: base schema fields.
    CHECK_FALSE(entry->name.empty());
    CHECK(entry->name == "Eldar Chainsword");
    CHECK(entry->fieldOr<std::string>("glyph", "").size() == 1);
    CHECK_FALSE(entry->fieldOr<std::string>("color", "").empty());
    CHECK(entry->fieldOr<std::string>("slot", "") == "weapon");
    CHECK(entry->fieldOr<double>("weight", -1.0) >= 0.0);

    // Requirement 9.4: the melee sub-table is populated with the cited profile.
    auto melee = subTable(*entry, "melee");
    REQUIRE(melee.has_value());
    CHECK(tblStr(*melee, "damageDice", "") == "1d10+5");
    CHECK(tblInt(*melee, "penetration", -1) == 2);

    // qualities must be present as a table (may be empty), per the schema.
    sol::object qobj = (*melee)["qualities"];
    CHECK(qobj.get_type() == sol::type::table);
}

// ── New body armour: Light Flak Coat (`armourLocations` populated) ──────────────
//
// Validates: Requirements 9.2 (schema shape), 9.5 (armourLocations populated).
TEST_CASE("New body armour Light Flak Coat has a populated armourLocations table",
          "[bestiary-npcs][equipment][Feature: bestiary-npcs, Task 12.2]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this test.");
    }

    auto entry = harness.equipmentByName("Light Flak Coat");
    REQUIRE(entry.has_value());

    // Requirement 9.2: base schema fields.
    CHECK_FALSE(entry->name.empty());
    CHECK(entry->name == "Light Flak Coat");
    CHECK(entry->fieldOr<std::string>("glyph", "").size() == 1);
    CHECK_FALSE(entry->fieldOr<std::string>("color", "").empty());
    CHECK(entry->fieldOr<std::string>("slot", "") == "body");
    CHECK(entry->fieldOr<double>("weight", -1.0) >= 0.0);

    // Requirement 9.5: the armourLocations sub-table is populated for all six
    // locations, with the cited AP values and 0 for any uncited location (head).
    auto loc = subTable(*entry, "armourLocations");
    REQUIRE(loc.has_value());
    CHECK(tblInt(*loc, "head", -1) == 0);
    CHECK(tblInt(*loc, "body", -1) == 2);
    CHECK(tblInt(*loc, "leftArm", -1) == 2);
    CHECK(tblInt(*loc, "rightArm", -1) == 2);
    CHECK(tblInt(*loc, "leftLeg", -1) == 2);
    CHECK(tblInt(*loc, "rightLeg", -1) == 2);
}

// ── Pre-existing entries remain byte-identical ─────────────────────────────────
//
// The shipped Equipment.lua defines 15 entries (7 player-oriented + 8 Ork). Task
// 12.3 must retain every one of them with all field values unchanged. This test
// pins the full field set of each shipped entry so any accidental edit is caught.
//
// Validates: Requirement 9.7 (existing entries retained unchanged).
namespace {

// Assert a weapon's scalar fields match the shipped values.
void checkScalars(const bestiary::EquipmentEntry& e,
                  const std::string& glyph, const std::string& color,
                  const std::string& slot, double weight, int value,
                  double power, double defense, double maxHp, int skill,
                  const std::string& tier) {
    CHECK(e.fieldOr<std::string>("glyph", "") == glyph);
    CHECK(e.fieldOr<std::string>("color", "") == color);
    CHECK(e.fieldOr<std::string>("slot", "") == slot);
    CHECK(e.fieldOr<double>("weight", -999.0) == Catch::Approx(weight));
    CHECK(e.fieldOr<int>("value", -999) == value);
    CHECK(e.fieldOr<double>("power", -999.0) == Catch::Approx(power));
    CHECK(e.fieldOr<double>("defense", -999.0) == Catch::Approx(defense));
    CHECK(e.fieldOr<double>("maxHp", -999.0) == Catch::Approx(maxHp));
    CHECK(e.fieldOr<int>("skill", -999) == skill);
    CHECK(e.fieldOr<std::string>("tier", "") == tier);
}

void checkMelee(const bestiary::EquipmentEntry& e, const std::string& dice, int pen) {
    auto m = subTable(e, "melee");
    REQUIRE(m.has_value());
    CHECK(tblStr(*m, "damageDice", "") == dice);
    CHECK(tblInt(*m, "penetration", -999) == pen);
}

void checkRanged(const bestiary::EquipmentEntry& e, const std::string& dice, int pen,
                 int range, int rof, int clip, int reload) {
    auto r = subTable(e, "ranged");
    REQUIRE(r.has_value());
    CHECK(tblStr(*r, "damageDice", "") == dice);
    CHECK(tblInt(*r, "penetration", -999) == pen);
    CHECK(tblInt(*r, "range", -999) == range);
    CHECK(tblInt(*r, "rateOfFire", -999) == rof);
    CHECK(tblInt(*r, "clipSize", -999) == clip);
    CHECK(tblInt(*r, "reloadTime", -999) == reload);
}

void checkArmour(const bestiary::EquipmentEntry& e, int head, int body,
                 int lArm, int rArm, int lLeg, int rLeg) {
    auto a = subTable(e, "armourLocations");
    REQUIRE(a.has_value());
    CHECK(tblInt(*a, "head", -999) == head);
    CHECK(tblInt(*a, "body", -999) == body);
    CHECK(tblInt(*a, "leftArm", -999) == lArm);
    CHECK(tblInt(*a, "rightArm", -999) == rArm);
    CHECK(tblInt(*a, "leftLeg", -999) == lLeg);
    CHECK(tblInt(*a, "rightLeg", -999) == rLeg);
}

} // namespace

TEST_CASE("Pre-existing equipment entries remain byte-identical",
          "[bestiary-npcs][equipment][Feature: bestiary-npcs, Task 12.2]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this test.");
    }

    auto require = [&](const std::string& name) {
        auto e = harness.equipmentByName(name);
        REQUIRE(e.has_value());
        return *e;
    };

    // ── Player-oriented entries ──────────────────────────────────────────────
    {
        auto e = require("Combat Knife");
        checkScalars(e, "-", "white", "weapon", 1.0, 15, 1.0, 0.0, 0.0, 5, "common");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Melee");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "Primitive");
        CHECK(e.fieldOr<std::string>("damageType", "") == "R");
        checkMelee(e, "1d5", 0);
    }
    {
        auto e = require("Chainsword");
        checkScalars(e, "/", "lightBlue", "weapon", 3.5, 50, 3.0, 0.0, 0.0, 0, "uncommon");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Melee");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "Primitive");
        CHECK(e.fieldOr<std::string>("damageType", "") == "R");
        checkMelee(e, "1d10", 2);
    }
    {
        auto e = require("Power Sword");
        checkScalars(e, "|", "lightGrey", "weapon", 5.0, 80, 4.0, 0.0, 0.0, 10, "rare");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Melee");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "Primitive");
        CHECK(e.fieldOr<std::string>("damageType", "") == "E");
        checkMelee(e, "1d10", 5);
    }
    {
        auto e = require("Laspistol");
        checkScalars(e, ")", "lightRed", "weapon", 1.5, 20, 1.0, 0.0, 0.0, 0, "common");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Pistol");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "Las");
        CHECK(e.fieldOr<std::string>("damageType", "") == "E");
        checkMelee(e, "1d5", 0);
        checkRanged(e, "1d10", 0, 30, 1, 30, 1);
    }
    {
        auto e = require("Autogun");
        checkScalars(e, "}", "lightGrey", "weapon", 4.5, 35, 1.0, 0.0, 0.0, 0, "uncommon");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Basic");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "SP");
        CHECK(e.fieldOr<std::string>("damageType", "") == "I");
        checkMelee(e, "1d5", 0);
        checkRanged(e, "1d10", 0, 40, 3, 24, 1);
    }
    {
        auto e = require("Flak Armor");
        checkScalars(e, "[", "lighterOrange", "body", 8.0, 30, 0.0, 2.0, 0.0, -5, "uncommon");
        checkArmour(e, 0, 3, 3, 3, 3, 3);
    }
    {
        auto e = require("Carapace Helm");
        checkScalars(e, "^", "lightGrey", "head", 4.0, 40, 0.0, 1.0, 5.0, -2, "uncommon");
        checkArmour(e, 4, 0, 0, 0, 0, 0);
    }

    // ── Ork entries ──────────────────────────────────────────────────────────
    {
        auto e = require("Choppa");
        checkScalars(e, "/", "desaturatedGreen", "weapon", 4.0, 10, 2.0, 0.0, 0.0, -5, "common");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Melee");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "Primitive");
        CHECK(e.fieldOr<std::string>("damageType", "") == "R");
        checkMelee(e, "1d10", 0);
    }
    {
        auto e = require("Slugga");
        checkScalars(e, ")", "desaturatedGreen", "weapon", 2.5, 12, 1.5, 0.0, 0.0, 0, "common");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Pistol");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "SP");
        CHECK(e.fieldOr<std::string>("damageType", "") == "I");
        checkMelee(e, "1d10", 0);
        checkRanged(e, "1d10", 0, 15, 1, 6, 1);
    }
    {
        auto e = require("Scrap Shield");
        checkScalars(e, "(", "lightYellow", "offhand", 5.0, 8, 0.0, 1.0, 0.0, -3, "common");
    }
    {
        auto e = require("Shoota");
        checkScalars(e, "}", "desaturatedGreen", "weapon", 5.5, 35, 2.5, 0.0, 0.0, 5, "uncommon");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Basic");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "SP");
        CHECK(e.fieldOr<std::string>("damageType", "") == "I");
        checkMelee(e, "1d5", 0);
        checkRanged(e, "1d10", 0, 30, 3, 18, 1);
    }
    {
        auto e = require("Big Choppa");
        checkScalars(e, "/", "lightGreen", "weapon", 7.0, 40, 3.5, 0.0, 0.0, -8, "uncommon");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Melee");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "Primitive");
        CHECK(e.fieldOr<std::string>("damageType", "") == "I");
        checkMelee(e, "2d5", 2);
    }
    {
        auto e = require("Ork Armor");
        checkScalars(e, "[", "desaturatedGreen", "body", 10.0, 25, 0.0, 1.5, 3.0, -8, "uncommon");
        checkArmour(e, 0, 2, 2, 2, 1, 1);
    }
    {
        auto e = require("Power Klaw");
        checkScalars(e, "{", "lightYellow", "weapon", 9.0, 90, 5.0, 0.5, 0.0, -10, "rare");
        CHECK(e.fieldOr<std::string>("sizeClass", "") == "Melee");
        CHECK(e.fieldOr<std::string>("weaponGroup", "") == "Exotic");
        CHECK(e.fieldOr<std::string>("damageType", "") == "I");
        checkMelee(e, "2d10", 7);
    }
}
