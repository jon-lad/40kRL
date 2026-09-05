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

// Read the live, declaration-ordered `enemies` table out of the loaded script and
// build the reference model INDEPENDENTLY of spawnEnemy's own selection logic.
//
// `enemies` is a local upvalue captured by the global spawnEnemy closure, so we
// reach it via the standard debug library (debug.getupvalue) rather than a global.
// This surfaces every Enemy_Entry in declaration order; for each we record the
// entry's `name` and its `chance[region]` cumulative threshold (absent key => the
// entry is simply omitted from this region's column, i.e. never selectable here).
//
// The resulting ModelEntry list is the specification the C++ referenceSelect model
// operates on — derived purely from the CURRENT Enemies.lua data for whatever region
// is chosen, with no hard-coded per-faction assumptions.
std::vector<ModelEntry> deriveColumnModel(sol::state& lua, const std::string& region) {
    std::vector<ModelEntry> model;

    // Fetch the `enemies` upvalue from the spawnEnemy closure.
    sol::protected_function getupvalue = lua["debug"]["getupvalue"];
    sol::object spawnEnemyObj = lua["spawnEnemy"];

    sol::table enemies;
    bool found = false;
    for (int i = 1; !found; ++i) {
        sol::protected_function_result res = getupvalue(spawnEnemyObj, i);
        if (!res.valid()) break;
        sol::object nameObj = res.get<sol::object>(0);
        if (nameObj == sol::lua_nil) break;          // no more upvalues
        std::string upName = nameObj.as<std::string>();
        if (upName == "enemies") {
            enemies = res.get<sol::table>(1);
            found = true;
        }
    }
    if (!found) return model;                         // couldn't locate; empty model

    // Walk entries in declaration order (ipairs order == array part order).
    for (std::size_t i = 1; i <= enemies.size(); ++i) {
        sol::optional<sol::table> entryOpt = enemies[i];
        if (!entryOpt) continue;
        sol::table entry = *entryOpt;
        sol::optional<std::string> nameOpt = entry["name"];
        std::string name = nameOpt.value_or(std::string());

        sol::optional<sol::table> chanceOpt = entry["chance"];
        if (!chanceOpt) continue;
        sol::optional<int> thresholdOpt = (*chanceOpt)[region];
        if (!thresholdOpt.has_value()) continue;      // entry not in this region column

        model.push_back({ name, { { region, *thresholdOpt } } });
    }
    return model;
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
        // Mix of populated Faction_Region columns AND genuinely-absent columns.
        // Case-sensitive: "ork" (lowercase) is NOT a Region_Name, and "" is empty;
        // both must yield no selection, exercising the missing-key / no-column branch.
        const std::string region = *rc::gen::elementOf(std::vector<std::string>{
            "Ork", "Tyranid", "Eldar", "Necron", "Chaos", "", "ork" });

        // Build a fresh sol::state per iteration; register a spy addActor that
        // records the selected entry's name (mirrors Map::addActor). The debug
        // library is opened so we can read the live `enemies` upvalue for the model.
        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string,
                           sol::lib::debug);

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

        // Reference expectation is DATA-DRIVEN: build the column for whatever region
        // was chosen straight from the loaded script, then apply the cumulative rule.
        // Populated columns (Ork/Tyranid/Eldar/Necron/Chaos) yield real selections;
        // absent columns ("" / "ork") produce an empty model => no selection.
        const std::vector<ModelEntry> model = deriveColumnModel(lua, region);
        const std::optional<std::string> expected = referenceSelect(model, region, roll);

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
        // Mix of populated Faction_Region columns AND genuinely-absent columns to
        // cover both the exactly-once and zero-call branches. "" and "ork" are NOT
        // Region_Names, so they must produce zero addActor calls.
        const std::string region = *rc::gen::elementOf(std::vector<std::string>{
            "Ork", "Tyranid", "Eldar", "Necron", "Chaos", "", "ork" });

        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string,
                           sol::lib::debug);

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

        // Data-driven reference: derive the chosen region's column from the loaded
        // script and apply the cumulative-selection rule. Absent columns yield no
        // selection => zero addActor calls.
        const std::vector<ModelEntry> model = deriveColumnModel(lua, region);
        const std::optional<std::string> expected = referenceSelect(model, region, roll);

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
