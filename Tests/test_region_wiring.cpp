// test_region_wiring.cpp
//
// Feature: region-based-npc-spawns — Task 7.1
// Unit / example tests for spawn wiring and region assignment at level creation
// (Design §5 "Map::addMonster — passing the region into Lua" and §8 "Level-creation
// call sites").
//
// These tests are written BEFORE the implementation (TDD workflow) and are
// EXPECTED TO FAIL to build/link and/or at runtime until:
//   - Task 5.3 adds Map::getRegionName() / Map::setRegionName() state,
//   - Task 7.2 makes Map::addMonster pass the level region into spawnEnemy, and
//   - Task 7.3 assigns the region (default on a context-free level) at creation.
//
// Engine isolation (per test-isolation steering): no global Engine is initialized
// here. resolveDefaultRegion() reads Scripts/Config.lua via the existing sol +
// get_or pattern and must not touch engine.gui / engine.map / engine.player.
//
// NOTE: A sibling task (7.4) appends a Lua-failure fallback edge-case TEST_CASE to
// this same file; the file is structured so additional TEST_CASEs append cleanly.
//
// Requirements covered:
//   - 1.1 (EXAMPLE):        exactly one Region_Name is assigned and retained.
//   - 1.5, 3.1, 3.2 (EXAMPLE): whole-level granularity — every spawn position on a
//                        level resolves to the same single Region_Name.
//   - 1.6, 2.1 (EXAMPLE):  a level created without biome context reports the
//                        Default_Region.
//   - 5.1:                 Map::addMonster passes the level Region_Name into Lua.

#include "lib/catch_amalgamated.hpp"

#include "main.hpp"
#include "WorldMap.hpp"

#include <cstdio>
#include <filesystem>
#include <string>

namespace {

// Scripts/Config.lua is loaded CWD-relative by resolveDefaultRegion(), mirroring
// Source/WorldMap.cpp and the other integration tests. Tests normally run from the
// repo root; probe a couple of fallbacks so a differing CWD (e.g. x64/Debug) still
// finds the file for the default-region presence check.
bool configLuaOnDisk()
{
    const char* candidates[] = {
        "Scripts/Config.lua",
        "../../Scripts/Config.lua",
        "../Scripts/Config.lua",
    };
    for (const char* p : candidates) {
        if (std::FILE* f = std::fopen(p, "r")) {
            std::fclose(f);
            return true;
        }
    }
    return false;
}

// A small fixed map keeps generation fast; the region field is independent of map
// dimensions and content.
constexpr int MAP_WIDTH  = 80;
constexpr int MAP_HEIGHT = 43;

} // namespace

// ─── Region assignment single / retained (Requirement 1.1, EXAMPLE) ────────────
// After a single region assignment, getRegionName() returns exactly one non-empty
// value and that value is unchanged on repeated reads (retained for the level's
// lifetime).
TEST_CASE("Map region is assigned once and retained unchanged on repeated reads",
          "[region-based-npc-spawns][region-wiring][example]")
{
    Map map(MAP_WIDTH, MAP_HEIGHT);
    map.init(false, LevelType::BSP);

    // Assign a single, explicit Region_Name.
    map.setRegionName("Ork");

    // Exactly one non-empty value is reported.
    const std::string first = map.getRegionName();
    REQUIRE_FALSE(first.empty());
    REQUIRE(first == "Ork");

    // Repeated reads are stable — the value is retained, not recomputed per read.
    const std::string second = map.getRegionName();
    const std::string third  = map.getRegionName();
    REQUIRE(second == first);
    REQUIRE(third == first);
}

// ─── Whole-level granularity (Requirements 1.5, 3.1, 3.2, EXAMPLE) ─────────────
// A single Region_Name applies uniformly to every spawn location on a level. The
// spawn selection uses the level's single regionName rather than any per-tile
// value, so the region resolved for spawning is identical across several distinct
// spawn positions. Map::addMonster must complete for each position using that one
// level region (Requirement 5.1).
TEST_CASE("Whole-level granularity: every spawn position resolves to the level's single region",
          "[region-based-npc-spawns][region-wiring][example]")
{
    Map map(MAP_WIDTH, MAP_HEIGHT);
    map.init(false, LevelType::BSP);

    map.setRegionName("Ork");

    // The region is a whole-level property: reading it does not depend on any (x, y)
    // spawn coordinate. Sampling "several spawn positions" cannot change the value.
    const std::string levelRegion = map.getRegionName();
    REQUIRE(levelRegion == "Ork");

    // Drive the spawn path at several distinct positions. Each call must use the one
    // level region (not a per-tile value); addMonster passes map.regionName into the
    // Lua spawnEnemy (Requirement 5.1). The invariant we assert is that the level's
    // single region is unchanged by spawning at any position.
    const std::pair<int, int> positions[] = {
        { 5, 5 }, { 10, 20 }, { 40, 8 }, { 60, 30 }, { 25, 25 }
    };
    for (const auto& [px, py] : positions) {
        map.addMonster(px, py);
        // Whole-level granularity: the region is the same for every spawn location.
        REQUIRE(map.getRegionName() == levelRegion);
    }
}

// ─── Default on a context-free level (Requirements 1.6, 2.1, EXAMPLE) ──────────
// A level generated without any world-map biome context resolves to the
// Default_Region. Task 7.3 assigns resolveDefaultRegion() at the context-free
// (initial BSP) creation site; with the shipped Config.lua that value is "Ork".
TEST_CASE("Context-free level reports the Default_Region",
          "[region-based-npc-spawns][region-wiring][example]")
{
    if (!configLuaOnDisk()) {
        SKIP("Scripts/Config.lua not found relative to the test CWD; run from the "
             "repo root to exercise the default-region check.");
    }

    // Model the initial, context-free BSP level: created with no biome and assigned
    // the default region at its creation site (Task 7.3 wires this into Engine::init;
    // here we exercise the same resolveDefaultRegion() contract on a bare Map).
    Map map(MAP_WIDTH, MAP_HEIGHT);
    map.init(false, LevelType::BSP);
    map.setRegionName(resolveDefaultRegion());

    const std::string region = map.getRegionName();

    // Every level must have a defined, non-empty region (Requirement 2.3 guarantees
    // resolveDefaultRegion() is never empty).
    REQUIRE_FALSE(region.empty());

    // The context-free level's region equals the Default_Region, which the shipped
    // Config.lua defines as "Ork" (Requirements 1.6, 2.1).
    REQUIRE(region == resolveDefaultRegion());
    REQUIRE(region == "Ork");
}

// ─── Lua-failure fallback (Requirement 7.4, EDGE_CASE) ─────────────────────────
// IF the Enemy_Script fails to load or execute, THEN the Spawn_System SHALL fall
// back to the existing hard-coded Ork/Nob spawn behaviour (Requirement 7.4). This
// fallback lives in Map::addMonster's `catch (const sol::error&)` branch and is
// unchanged by this feature — passing the region into spawnEnemy (Task 7.2) does not
// alter it. We force the Lua load path to fail by moving Scripts/Enemies.lua aside
// for the duration of the call so `lua.script_file("Scripts/Enemies.lua")` throws a
// sol::error, then assert an enemy from the hard-coded Ork set was still spawned.
//
// Engine isolation: the fallback branch only constructs an Actor and pushes it onto
// engine.actors (using Colors::orkSkin / Colors::nobArmour). It does not touch
// engine.gui / engine.map / engine.player, so the test is safe without a live Engine
// — we only need engine.actors, which is a plain std::list member on the global.
namespace {

namespace fs = std::filesystem;

// RAII guard: temporarily renames Scripts/Enemies.lua aside so the on-disk load in
// Map::addMonster fails, and restores it on scope exit (even if an assertion throws).
// Mirrors the rename-aside/restore pattern used by Tests/test_high_score_store.cpp.
class EnemiesScriptHidden {
public:
    EnemiesScriptHidden()
    {
        // Probe the same CWD-relative candidates the production/loader tests use, so
        // the guard finds the shipped script whether tests run from the repo root or
        // from x64/Debug.
        const char* candidates[] = {
            "Scripts/Enemies.lua",
            "../../Scripts/Enemies.lua",
            "../Scripts/Enemies.lua",
        };
        std::error_code ec;
        for (const char* p : candidates) {
            if (fs::exists(p, ec) && !ec) {
                scriptPath_ = p;
                break;
            }
        }
        if (!scriptPath_.empty()) {
            backupPath_ = scriptPath_ + ".bak-region-wiring-test";
            fs::rename(scriptPath_, backupPath_, ec);
            hidden_ = !ec;
        }
    }

    ~EnemiesScriptHidden()
    {
        if (hidden_) {
            std::error_code ec;
            fs::rename(backupPath_, scriptPath_, ec);
        }
    }

    // True only if the script was found and successfully moved aside; if false the
    // test cannot exercise the failure path and should skip.
    bool hidden() const { return hidden_; }

private:
    std::string scriptPath_;
    std::string backupPath_;
    bool        hidden_ = false;
};

} // namespace

TEST_CASE("Lua load failure falls back to the hard-coded Ork/Nob spawn",
          "[region-based-npc-spawns][region-wiring][edge-case]")
{
    // A monster spawn appends to engine.actors; start from a clean slate so the only
    // actor present after the call is the fallback spawn we are asserting about.
    engine.actors.clear();

    Map map(MAP_WIDTH, MAP_HEIGHT);
    map.init(false, LevelType::BSP);
    map.setRegionName("Ork");

    // Move Scripts/Enemies.lua aside for the duration of this scope. If the script
    // can't be located (unexpected CWD), skip rather than assert a false negative —
    // without a forced load failure there is nothing to exercise.
    EnemiesScriptHidden guard;
    if (!guard.hidden()) {
        SKIP("Scripts/Enemies.lua not found relative to the test CWD; run from the "
             "repo root so the Lua-failure fallback path can be exercised.");
    }

    // With the script missing, lua.script_file(...) inside Map::addMonster throws a
    // sol::error, driving the catch branch (Requirement 7.4). Exactly one enemy from
    // the hard-coded Ork set must be spawned regardless of the internal roll.
    const size_t before = engine.actors.size();
    map.addMonster(5, 5);
    const size_t after = engine.actors.size();

    // The fallback always spawns one enemy (Ork for roll < 80, Nob otherwise).
    REQUIRE(after == before + 1);

    // The spawned actor is from the hard-coded Ork set at the requested position.
    const Actor* spawned = engine.actors.back().get();
    REQUIRE(spawned != nullptr);
    REQUIRE(spawned->getX() == 5);
    REQUIRE(spawned->getY() == 5);
    const std::string name = spawned->name;
    REQUIRE((name == "Ork" || name == "Nob"));

    // Clean up the actor we spawned so no state leaks into sibling tests.
    engine.actors.clear();
}
