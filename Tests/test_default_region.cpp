// test_default_region.cpp
//
// Feature: region-based-npc-spawns — Task 2.1
// Unit tests for resolveDefaultRegion() (Design §2 "Default region resolution").
//
// These tests are written BEFORE the implementation (TDD workflow) and are
// EXPECTED TO FAIL to build/link until Task 2.2 declares/defines
// resolveDefaultRegion() and DEFAULT_REGION_FALLBACK in Headers/WorldMap.hpp /
// Source/WorldMap.cpp, and Task 2.3 adds `defaultRegion = "Ork"` to Config.lua.
//
// Engine isolation (per test-isolation steering): resolveDefaultRegion() reads
// Scripts/Config.lua via the existing sol + get_or pattern and must not touch the
// global engine.gui / engine.map / engine.player. No Engine is initialized here.
//
// Requirements covered:
//   - 2.2 (INTEGRATION): the configured defaultRegion value is read from config.
//   - 2.3 (EDGE_CASE):  a compiled fallback guarantees a non-empty Region_Name
//                        when the config value is absent.

#include "lib/catch_amalgamated.hpp"

#include "WorldMap.hpp"

#include <cstdio>
#include <string>

namespace {

// The production resolver reads the on-disk Scripts/Config.lua (CWD-relative),
// mirroring how Source/WorldMap.cpp and Tests/test_weapon_types.cpp load scripts.
// Tests normally run from the repo root; probe a couple of fallbacks so a differing
// CWD (e.g. x64/Debug) still finds the file for the presence check below.
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

} // namespace

// ─── Config wiring (Requirement 2.2, INTEGRATION) ──────────────────────────────
// With Scripts/Config.lua present and carrying a known `defaultRegion` (set to
// "Ork" by Task 2.3), resolveDefaultRegion() reads that configured value.
TEST_CASE("resolveDefaultRegion reads the configured defaultRegion from Config.lua",
          "[region-based-npc-spawns][default-region][integration]")
{
    if (!configLuaOnDisk()) {
        SKIP("Scripts/Config.lua not found relative to the test CWD; run from the "
             "repo root to exercise the config-wiring check.");
    }

    const std::string region = resolveDefaultRegion();

    // The resolver must surface the configured value, which Task 2.3 defines as "Ork".
    REQUIRE(region == "Ork");
}

// ─── Compiled fallback (Requirement 2.3, EDGE_CASE) ────────────────────────────
// The compiled fallback constant guarantees a non-empty Region_Name so that every
// Level has a defined region even when `defaultRegion` is absent from config.
TEST_CASE("resolveDefaultRegion falls back to a non-empty compiled default",
          "[region-based-npc-spawns][default-region][edge-case]")
{
    // The compiled fallback is exposed as a constant and must be non-empty "Ork".
    REQUIRE(std::string(DEFAULT_REGION_FALLBACK) == "Ork");
    REQUIRE_FALSE(std::string(DEFAULT_REGION_FALLBACK).empty());

    // resolveDefaultRegion() must never return an empty string: when the config
    // value is absent/empty it returns the compiled fallback. Whatever the current
    // on-disk config contains, the result is a non-empty Region_Name.
    const std::string region = resolveDefaultRegion();
    REQUIRE_FALSE(region.empty());
}
