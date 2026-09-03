#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.hpp"

#include <sol/sol.hpp>

#include <cstdio>
#include <optional>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: region-based-npc-spawns
//
// Property tests for region-scoped cumulative selection (Property 2) and
// spawn-call fidelity / empty-region no-spawn (Property 3). Both properties are
// exercised against the real Lua spawnEnemy defined in Scripts/Enemies.lua, driven
// through an in-memory sol::state with a spy addActor callback (mirroring the
// production Map::addActor path). No global Engine is initialized — these tests are
// fully headless (per the test-isolation steering).
//
// TDD-RED: these tests are EXPECTED TO FAIL until task 4.3 converts Enemies.lua's
// flat `chance` integer into a per-region keyed table and gives spawnEnemy a
// `region` parameter (spawnEnemy(roll, x, y, region)). Against the current shipped
// script (spawnEnemy(roll, x, y) with a flat chance), the region-scoped assertions
// below will not hold.
//
// Bounds convention: rc::gen::inRange(a, b) is INCLUSIVE at both ends in this
// project. The roll domain per the design is the integer range [0, 100].
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// A minimal mirror of an Enemy_Entry for the reference model: a name plus the
// per-region cumulative threshold table (a missing key means "not present").
struct ModelEntry {
    std::string name;
    // region -> cumulative threshold; absence => treated as chance 0 (never selected)
    std::vector<std::pair<std::string, int>> chanceByRegion;

    std::optional<int> thresholdFor(const std::string& region) const {
        for (const auto& kv : chanceByRegion) {
            if (kv.first == region) return kv.second;
        }
        return std::nullopt;
    }
};

// Reference cumulative-selection model (the specification of correct behaviour):
// iterate entries in declaration order; the first entry whose region-scoped
// threshold is strictly greater than the roll is selected. A missing key is chance
// 0 and can never be selected (0 > roll is false for roll in [0,100]). If no entry
// qualifies, there is no selection.
std::optional<std::string> referenceSelect(const std::vector<ModelEntry>& enemies,
                                            const std::string& region,
                                            int roll) {
    for (const auto& e : enemies) {
        std::optional<int> threshold = e.thresholdFor(region);
        if (threshold.has_value() && roll < threshold.value()) {
            return e.name;
        }
    }
    return std::nullopt;
}

// Resolve Scripts/Enemies.lua relative to the test CWD. Production (Source/Map.cpp)
// and other integration tests load "Scripts/<file>.lua" from the repo root; probe a
// couple of fallbacks so a differing CWD still resolves.
std::string findEnemiesScript() {
    const std::vector<std::string> candidatePaths = {
        "Scripts/Enemies.lua",
        "../../Scripts/Enemies.lua", // if CWD == x64/Debug
        "../Scripts/Enemies.lua"
    };
    for (const auto& p : candidatePaths) {
        if (std::FILE* f = std::fopen(p.c_str(), "r")) {
            std::fclose(f);
            return p;
        }
    }
    return std::string();
}

// The reference model of the shipped Ork column (Gretchin 50, Ork 75, Shoota Boy
// 90, Nob 100), in declaration order. Requirement 7.2 / 7.3.
std::vector<ModelEntry> orkColumnModel() {
    return {
        { "Gretchin",   { { "Ork", 50 } } },
        { "Ork",        { { "Ork", 75 } } },
        { "Shoota Boy", { { "Ork", 90 } } },
        { "Nob",        { { "Ork", 100 } } },
    };
}

} // namespace

// Feature: region-based-npc-spawns, Property 2: Region-scoped cumulative selection is correct
// For any region column (a set of Enemy_Entry cumulative thresholds keyed by the
// requested Region_Name, where a missing key is treated as chance 0) and any integer
// roll in [0, 100], spawnEnemy selects the first Enemy_Entry in declaration order
// whose region-scoped threshold is strictly greater than the roll, matching a
// reference cumulative-selection model. Entries without a key for the requested
// region are never selected, and a column with no qualifying entry (all-zero /
// absent column) yields no selection.
// Validates: Requirements 4.2, 4.3, 4.4, 4.5, 7.3
TEST_CASE("Region-scoped cumulative selection matches the reference model",
          "[region-based-npc-spawns]") {
    const std::string enemiesPath = findEnemiesScript();
    if (enemiesPath.empty()) {
        SKIP("Scripts/Enemies.lua not found relative to the test CWD; run from the "
             "repo root to exercise this property.");
    }

    // The shipped script exposes the real "Ork" region column. We generate a roll in
    // [0, 100] and a region key drawn from a set that includes "Ork" (a populated
    // column) and several absent columns (which must yield no selection). This
    // exercises: correct first-in-order selection, missing-key = chance 0, and
    // empty/absent column = no selection — all against the production spawnEnemy.
    rc::check("selection equals reference model across rolls and regions", [&] {
        const int roll = *rc::gen::inRange(0, 100);
        // Case-sensitive: "ork" is intentionally NOT the "Ork" column.
        const std::string region = *rc::gen::elementOf(std::vector<std::string>{
            "Ork", "Tyranid", "Eldar", "", "ork" });

        // Build a fresh sol::state per iteration; register a spy addActor that
        // records the selected entry's name (mirrors Map::addActor).
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string);

        std::vector<std::string> spawnedNames;
        lua.set_function("addActor", [&](int /*x*/, int /*y*/, sol::table entry) {
            sol::optional<std::string> n = entry["name"];
            spawnedNames.push_back(n.value_or(std::string()));
        });

        lua.script_file(enemiesPath);

        sol::protected_function spawnEnemy = lua["spawnEnemy"];
        RC_ASSERT(spawnEnemy.valid());

        sol::protected_function_result r = spawnEnemy(roll, 0, 0, region);
        RC_ASSERT(r.valid());

        // Reference expectation: only the "Ork" column is populated in the shipped
        // script; every other region key is an absent column -> no selection.
        std::optional<std::string> expected;
        if (region == "Ork") {
            expected = referenceSelect(orkColumnModel(), "Ork", roll);
        } else {
            expected = std::nullopt; // absent column
        }

        if (expected.has_value()) {
            RC_ASSERT(spawnedNames.size() == 1u);
            RC_ASSERT(spawnedNames[0] == expected.value());
        } else {
            RC_ASSERT(spawnedNames.empty());
        }
    });
}

// Feature: region-based-npc-spawns, Property 3: Spawn-call fidelity and empty-region no-spawn
// For any region column and any roll in [0, 100], spawnEnemy calls the injected
// addActor exactly once with the entry chosen by the selection model when a
// selection exists, and calls addActor zero times when no entry matches — leaving
// the spawn location unoccupied.
// Validates: Requirements 5.3, 6.1, 6.2
TEST_CASE("Spawn-call fidelity: addActor invoked once on selection, zero on empty region",
          "[region-based-npc-spawns]") {
    const std::string enemiesPath = findEnemiesScript();
    if (enemiesPath.empty()) {
        SKIP("Scripts/Enemies.lua not found relative to the test CWD; run from the "
             "repo root to exercise this property.");
    }

    rc::check("addActor call count and payload track the selection model", [&] {
        const int roll = *rc::gen::inRange(0, 100);
        // Bias toward populated ("Ork") and empty (absent) columns to cover both the
        // exactly-once and zero-call branches.
        const std::string region = *rc::gen::elementOf(std::vector<std::string>{
            "Ork", "Ork", "Necron", "", "Chaos" });

        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string);

        int addActorCalls = 0;
        std::string lastSpawnedName;
        int lastX = -1, lastY = -1;
        lua.set_function("addActor", [&](int x, int y, sol::table entry) {
            ++addActorCalls;
            sol::optional<std::string> n = entry["name"];
            lastSpawnedName = n.value_or(std::string());
            lastX = x;
            lastY = y;
        });

        lua.script_file(enemiesPath);

        sol::protected_function spawnEnemy = lua["spawnEnemy"];
        RC_ASSERT(spawnEnemy.valid());

        const int spawnX = *rc::gen::inRange(0, 63);
        const int spawnY = *rc::gen::inRange(0, 63);
        sol::protected_function_result r = spawnEnemy(roll, spawnX, spawnY, region);
        RC_ASSERT(r.valid());

        std::optional<std::string> expected;
        if (region == "Ork") {
            expected = referenceSelect(orkColumnModel(), "Ork", roll);
        } else {
            expected = std::nullopt; // absent column -> empty-region no-spawn
        }

        if (expected.has_value()) {
            RC_ASSERT(addActorCalls == 1);
            RC_ASSERT(lastSpawnedName == expected.value());
            RC_ASSERT(lastX == spawnX);
            RC_ASSERT(lastY == spawnY);
        } else {
            RC_ASSERT(addActorCalls == 0);
        }
    });
}
