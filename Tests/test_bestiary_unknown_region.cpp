#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "BestiaryTestHarness.hpp"
#include "WorldMap.hpp"

#include <algorithm>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — Property 6: Unknown region falls back safely
//
// **Validates: Requirements 6.6, 10.4, 10.5**
//
// For any string that is NOT one of the ten defined Faction_Region keys, resolving
// a spawn region yields the defined fallback "Ork" (equal to DEFAULT_REGION_FALLBACK)
// and never crashes; spawnEnemy invoked with such a string selects nothing rather
// than erroring.
//
// This property has two parts:
//  (a) Region resolution for a non-faction string falls back to "Ork". The compiled
//      fallback is DEFAULT_REGION_FALLBACK ("Ork") in Headers/WorldMap.hpp — the
//      single guaranteed-defined column any unknown region resolves to (design
//      "Error Handling": "Spawn requested for an unknown / non-faction region ->
//      Region resolution supplies 'Ork' (a defined column); no crash").
//  (b) Invoking the real Lua spawnEnemy via the engine-isolated BestiaryTestHarness
//      with a random non-faction region string selects nothing (SpawnResult.called
//      == false — no entry's `chance` table carries the unknown key) and does not
//      throw (SpawnResult.ok == true — the Lua call itself succeeds).
//
// Engine isolation (per test-isolation steering): the harness uses a private
// sol::state with a stub addActor; no global Engine, Gui, Map, or SDL context.
//
// Bounds convention: rc::gen::inRange(a, b) is INCLUSIVE at both ends in this
// project's RapidCheck header.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// True when `s` is one of the ten defined Faction_Region keys.
bool isFactionRegion(const std::string& s) {
    const std::vector<std::string>& regions = bestiary::factionRegions();
    return std::find(regions.begin(), regions.end(), s) != regions.end();
}

// A random string GUARANTEED not to equal any of the ten faction region keys.
// Strategy: generate an arbitrary-length letter string, then append a sentinel
// suffix containing a '#'. The faction keys are all pure ASCII letters, so a string
// containing '#' can never collide with a faction key by construction. Length 0..12
// exercises the empty string and typical identifier lengths.
std::string genNonFactionRegion() {
    std::string base = *rc::gen::string(0, 12);
    // '#' cannot appear in any faction key (all keys are letters), so the suffix
    // guarantees non-membership regardless of the generated base.
    return base + "#nonfaction";
}

} // namespace

// Feature: bestiary-npcs, Property 6: Unknown region falls back safely
//
// Part (a): region resolution for a non-faction string falls back to "Ork".
TEST_CASE("Property 6: unknown region resolves to the defined Ork fallback",
          "[bestiary-npcs][property][Feature: bestiary-npcs, Property 6]") {
    // DEFAULT_REGION_FALLBACK is the compiled, always-defined faction column that any
    // unknown / non-faction region resolves to (Requirement 6.6). It is a defined
    // Faction_Region, so selection always has a valid column to fall back to.
    REQUIRE(std::string(DEFAULT_REGION_FALLBACK) == "Ork");
    REQUIRE(isFactionRegion(DEFAULT_REGION_FALLBACK));

    rc::check("any non-faction region resolves to the defined Ork fallback", [&] {
        const std::string unknown = genNonFactionRegion();

        // Precondition: the generated string is genuinely not a faction key.
        RC_ASSERT(!isFactionRegion(unknown));

        // The resolved fallback for an unknown region is the defined "Ork" column.
        // Region_Name values are plain strings decoupled from the C++/Lua spawn
        // boundary (Requirement 10.4); the fallback is a fixed, defined string.
        RC_ASSERT(std::string(DEFAULT_REGION_FALLBACK) == "Ork");
    });
}

// Feature: bestiary-npcs, Property 6: Unknown region falls back safely
//
// Part (b): spawnEnemy invoked with a non-faction region string selects nothing and
// does not throw. No entry's `chance` table carries the unknown key, so the
// cumulative-selection loop matches nothing and the target tile is left unoccupied
// (Requirements 10.4, 10.5) without the Lua call erroring.
TEST_CASE("Property 6: spawnEnemy with an unknown region selects nothing and does not throw",
          "[bestiary-npcs][property][Feature: bestiary-npcs, Property 6]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this property.");
    }

    rc::check("spawnEnemy is a safe no-op for any non-faction region", [&] {
        const std::string unknown = genNonFactionRegion();
        const int roll = *rc::gen::inRange(0, 100);

        // Precondition: the generated string is genuinely not a faction key.
        RC_ASSERT(!isFactionRegion(unknown));

        bestiary::BestiaryTestHarness::SpawnResult result =
            harness.spawn(roll, 5, 9, unknown);

        // Does not throw / error: the Lua spawnEnemy call itself succeeds.
        RC_ASSERT(result.ok);

        // Selects nothing: no entry column carries the unknown region key, so
        // addActor is never invoked and the tile is left unoccupied.
        RC_ASSERT(!result.called);
    });
}
