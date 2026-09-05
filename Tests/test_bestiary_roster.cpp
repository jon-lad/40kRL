#include "lib/catch_amalgamated.hpp"

#include "BestiaryTestHarness.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — roster completeness example test (Task 10.5)
//
// Asserts the Bestiary defines exactly the 69 named Troop entries cited in
// Reference/RT-Bestiary.md (56 Chapter IV faction Troops + 13 Chapter V core
// profiles), and spot-checks each faction section's count. Loads Scripts/Enemies.lua
// through the sol2-based BestiaryTestHarness (no global Engine — test-isolation
// steering), which surfaces every declared Enemy_Entry by driving the real
// spawnEnemy across all ten faction columns and the full roll range.
//
// Requirement mapping: 1.1 (exactly 69 Troop entries), 1.2–1.9 (per-section IV.1,
// IV.3–IV.9 rosters), 1.10 (Chapter V §5.1–§5.4 roster).
//
// TDD-RED: written BEFORE the faction columns are populated (Tasks 2.3, 3–9). Until
// the roster lands these assertions are EXPECTED TO FAIL (missing names / wrong
// counts). The file is expected to COMPILE against the existing harness.
//
// Note on Colonist variants: the six Colonist_Variants (Adept, Bloodskinner,
// Entertainer, Hired Gun, Scum, Voidfarer) are built ADDITIONALLY as modifications
// of the Colonist base template (see requirements Introduction) and are NOT part of
// the 69 named Troop_Profiles. This test verifies the 69 named Troops; it does not
// require the variants to be absent (they are validated by the Colonist test).
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// ── The 69 named Troop_Profiles, grouped by the faction section that cites them ──
// (Requirements 1.2–1.10). These names must appear byte-for-byte as Enemy_Entry
// `name` values.

// IV.1 Renegades, Heretics & Mutants (6) — Requirement 1.2
const std::vector<std::string> kChaosRenegades = {
    "Chaos Cultist", "Mutant Fighter", "Dark Disciple",
    "Mutant Wretch", "Renegade Soldier", "Renegade Veteran",
};

// IV.3 Daemons of Chaos (8) — Requirement 1.3
const std::vector<std::string> kChaosDaemons = {
    "Nurgling", "Brimstone Horror", "Blue Horror", "Screamer of Tzeentch",
    "Ebon Gheist", "Chaos Fury", "Nether Spawn", "Gibbering Malingerer",
};

// IV.4 Craftworld Eldar (2) — Requirement 1.4
const std::vector<std::string> kEldarCraftworld = {
    "Eldar Guardian", "Eldar Ranger",
};

// IV.5 Harlequins & Dark Eldar (5) — Requirement 1.5
const std::vector<std::string> kDarkEldar = {
    "Wych", "Razorwing Flock", "Ariadne Helspider", "Kabalite", "Hellion",
};

// IV.6 Necrons (3) — Requirement 1.6
const std::vector<std::string> kNecron = {
    "Necron Warrior", "Canoptek Scarab", "Canoptek Scarab Swarm",
};

// IV.7 Orks (13) — Requirement 1.7
const std::vector<std::string> kOrk = {
    "Snotling", "Snotling Mob", "Gretchin", "Ork Boy", "Burna Boy",
    "Tankbusta", "Loota", "Storm Boy", "Kommando", "'Ard Boy",
    "Skar Boy", "Runtherd", "Attack Squig",
};

// IV.8 Tau, Kroot & Vespid (7) — Requirement 1.8
const std::vector<std::string> kTau = {
    "Fire Warrior", "Pathfinder", "Water Caste Diplomat", "Earth Caste Engineer",
    "Kroot Carnivore", "Kroot Hound", "Vespid Stingwing",
};

// IV.9 Tyranids (12) — Requirement 1.9
const std::vector<std::string> kTyranid = {
    "Late Generation Hybrid", "Early Generation Hybrid", "Ripper", "Ripper Swarm",
    "Hormagaunt", "Termagant", "Mucolid Spore", "Mieotic Spore",
    "Biovore", "Pyrovore", "Gargoyle", "Mycetic Spore",
};

// Chapter V core-rulebook Troops (13) — Requirement 1.10
// §5.1 Imperial Humans (5), §5.2 Servitors (3), §5.3 Xenos (3), §5.4 Warp (1).
// Colonist is the §5.1 base template; its six variants are built additionally and
// are not part of this named-Troop count.
const std::vector<std::string> kChapterV = {
    // §5.1 Imperial Humans
    "Colonist", "Mutant Outcast", "Oathsworn Bodyguard", "Renegade", "Warp Witch",
    // §5.2 Servitors
    "Battle Servitor (Charron-Pattern)", "Grapplehawk (Falax-Pattern)",
    "Servitor Drone", "Servo Skull",
    // §5.3 Xenos
    "Eldar Corsair", "Ork Freebooter", "Kroot Mercenary",
    // §5.4 Warp Predator
    "Warp Predator (Ebon Geist)",
};

// The full 69-name roster assembled in section order.
std::vector<std::string> allTroopNames() {
    std::vector<std::string> names;
    auto append = [&names](const std::vector<std::string>& g) {
        names.insert(names.end(), g.begin(), g.end());
    };
    append(kChaosRenegades);
    append(kChaosDaemons);
    append(kEldarCraftworld);
    append(kDarkEldar);
    append(kNecron);
    append(kOrk);
    append(kTau);
    append(kTyranid);
    append(kChapterV);
    return names;
}

// The six Colonist_Variants built additionally on the Colonist base (Requirement 4).
// Excluded from the 69-Troop count.
const std::vector<std::string> kColonistVariants = {
    "Adept", "Bloodskinner", "Entertainer", "Hired Gun", "Scum", "Voidfarer",
};

// Does an entry with this exact name exist in the roster?
bool rosterHas(const bestiary::BestiaryTestHarness& harness, const std::string& name) {
    return harness.enemyByName(name).has_value();
}

} // namespace

// ─── Sanity: the expected roster list is itself 69 unique names ──────────────────
TEST_CASE("Roster: the expected named-Troop list has exactly 69 unique names",
          "[bestiary-npcs][roster]")
{
    const std::vector<std::string> names = allTroopNames();
    REQUIRE(names.size() == 69);

    const std::set<std::string> unique(names.begin(), names.end());
    REQUIRE(unique.size() == 69); // no accidental duplicates in the fixture
}

// ─── Roster completeness: every one of the 69 named Troops is present ────────────
// **Requirements 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 1.10**
TEST_CASE("Roster: the Bestiary defines every one of the 69 named Troop entries",
          "[bestiary-npcs][roster]")
{
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua could not be loaded");
    }

    std::vector<std::string> missing;
    for (const std::string& name : allTroopNames()) {
        if (!rosterHas(harness, name)) {
            missing.push_back(name);
        }
    }

    INFO("Missing named Troop entries: " << missing.size());
    for (const std::string& name : missing) {
        UNSCOPED_INFO("  missing: " << name);
    }
    REQUIRE(missing.empty());
}

// ─── Roster count: exactly 69 named Troops, plus the six Colonist variants ───────
// **Requirement 1.1** — "exactly one Enemy_Entry for each of the 69 Troop_Profiles".
TEST_CASE("Roster: the Bestiary defines exactly the 69 named Troops (variants extra)",
          "[bestiary-npcs][roster]")
{
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua could not be loaded");
    }

    const std::vector<bestiary::EnemyEntry>& roster = harness.enemies();

    const std::vector<std::string> troops = allTroopNames();
    const std::set<std::string> troopSet(troops.begin(), troops.end());
    const std::set<std::string> variantSet(kColonistVariants.begin(),
                                            kColonistVariants.end());

    // Every roster entry is either one of the 69 named Troops or one of the six
    // Colonist variants — nothing unexpected leaks into the Bestiary.
    std::vector<std::string> unexpected;
    for (const bestiary::EnemyEntry& e : roster) {
        if (troopSet.count(e.name) == 0 && variantSet.count(e.name) == 0) {
            unexpected.push_back(e.name);
        }
    }
    INFO("Unexpected (non-Troop, non-variant) entries: " << unexpected.size());
    for (const std::string& name : unexpected) {
        UNSCOPED_INFO("  unexpected: " << name);
    }
    REQUIRE(unexpected.empty());

    // Count the roster entries that are named Troops: must equal exactly 69.
    std::size_t troopCount = 0;
    for (const bestiary::EnemyEntry& e : roster) {
        if (troopSet.count(e.name) != 0) {
            ++troopCount;
        }
    }
    REQUIRE(troopCount == 69);
}

// ─── Per-faction section counts (spot-checks) ───────────────────────────────────
// Verify each faction section contributes exactly its cited number of named Troops,
// keyed off the Faction_Region column each section maps to (Requirement 6.2).
namespace {

// Count how many of `names` are present in the roster.
std::size_t presentCount(const bestiary::BestiaryTestHarness& harness,
                         const std::vector<std::string>& names) {
    std::size_t n = 0;
    for (const std::string& name : names) {
        if (rosterHas(harness, name)) ++n;
    }
    return n;
}

} // namespace

TEST_CASE("Roster: each faction section defines its cited number of named Troops",
          "[bestiary-npcs][roster]")
{
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua could not be loaded");
    }

    // Chapter IV faction sections (Requirements 1.2–1.9).
    CHECK(presentCount(harness, kChaosRenegades) == 6);  // IV.1
    CHECK(presentCount(harness, kChaosDaemons) == 8);    // IV.3
    CHECK(presentCount(harness, kEldarCraftworld) == 2); // IV.4
    CHECK(presentCount(harness, kDarkEldar) == 5);       // IV.5
    CHECK(presentCount(harness, kNecron) == 3);          // IV.6
    CHECK(presentCount(harness, kOrk) == 13);            // IV.7
    CHECK(presentCount(harness, kTau) == 7);             // IV.8
    CHECK(presentCount(harness, kTyranid) == 12);        // IV.9

    // Chapter V core-rulebook Troops (Requirement 1.10).
    CHECK(presentCount(harness, kChapterV) == 13);       // §5.1–§5.4
}

// ─── Per-column counts (spot-checks against Faction_Region columns) ──────────────
// Requirement 6.2 maps sections onto columns. Spot-check that the column sizes match
// the section rosters plus their §5.3 additions (Eldar Corsair→Eldar, Ork Freebooter
// →Ork, Kroot Mercenary→Tau) and the Colonist variants (→ImperialHuman).
TEST_CASE("Roster: Faction_Region columns hold their expected number of entries",
          "[bestiary-npcs][roster]")
{
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua could not be loaded");
    }

    // Chaos: IV.1 (6) + IV.3 (8) = 14.
    CHECK(harness.columnFor("Chaos").size() == 14);
    // Eldar: IV.4 (2) + Eldar Corsair (§5.3) = 3.
    CHECK(harness.columnFor("Eldar").size() == 3);
    // DarkEldar: IV.5 (5).
    CHECK(harness.columnFor("DarkEldar").size() == 5);
    // Necron: IV.6 (3).
    CHECK(harness.columnFor("Necron").size() == 3);
    // Ork: IV.7 (13) + Ork Freebooter (§5.3) = 14.
    CHECK(harness.columnFor("Ork").size() == 14);
    // Tau: IV.8 (7) + Kroot Mercenary (§5.3) = 8.
    CHECK(harness.columnFor("Tau").size() == 8);
    // Tyranid: IV.9 (12).
    CHECK(harness.columnFor("Tyranid").size() == 12);
    // ImperialHuman: §5.1 named Troops (Colonist + Mutant Outcast + Oathsworn
    // Bodyguard + Renegade + Warp Witch = 5) + six Colonist variants = 11.
    CHECK(harness.columnFor("ImperialHuman").size() == 11);
    // Servitor: §5.2 (4).
    CHECK(harness.columnFor("Servitor").size() == 4);
    // Warp: §5.4 (1).
    CHECK(harness.columnFor("Warp").size() == 1);
}
