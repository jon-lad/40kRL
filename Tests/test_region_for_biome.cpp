#include "lib/catch_amalgamated.hpp"

#include "WorldMap.hpp"

#include <string>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — Example Tests for regionForBiome mapping
// ═══════════════════════════════════════════════════════════════════════════════
//
// **Validates: Requirements 6.3, 6.5, 6.6**
//
// These example tests pin the finite biome -> faction-region mapping that task 13.3
// implements. They are written BEFORE that implementation (TDD workflow) and are
// EXPECTED TO FAIL until Source/WorldMap.cpp regionForBiome is expanded from the
// placeholder all-"Ork" switch to the five biome->faction-region mappings.
//
// Engine isolation (per test-isolation steering): regionForBiome is a pure function
// that takes a BiomeType and returns a Region_Name string. It does not touch the
// global engine, so it is safe to call directly in the test binary.

TEST_CASE("regionForBiome maps each BiomeType to its faction region",
          "[bestiary-npcs][region-for-biome]")
{
    SECTION("HIVE_CITY maps to ImperialHuman") {
        REQUIRE(regionForBiome(BiomeType::HIVE_CITY) == "ImperialHuman");
    }

    SECTION("ASH_DESERT maps to Ork") {
        REQUIRE(regionForBiome(BiomeType::ASH_DESERT) == "Ork");
    }

    SECTION("DEAD_FOREST maps to Eldar") {
        REQUIRE(regionForBiome(BiomeType::DEAD_FOREST) == "Eldar");
    }

    SECTION("WASTELAND maps to Servitor") {
        REQUIRE(regionForBiome(BiomeType::WASTELAND) == "Servitor");
    }

    SECTION("TOXIC_SWAMP maps to Chaos") {
        REQUIRE(regionForBiome(BiomeType::TOXIC_SWAMP) == "Chaos");
    }
}

TEST_CASE("regionForBiome falls back to Ork for an out-of-range cast",
          "[bestiary-npcs][region-for-biome]")
{
    // An out-of-range value cast into BiomeType hits the trailing
    // `return DEFAULT_REGION_FALLBACK` ("Ork"), guaranteeing a defined,
    // non-empty faction region (Requirement 6.6).
    const BiomeType outOfRange = static_cast<BiomeType>(99);
    REQUIRE(regionForBiome(outOfRange) == "Ork");
    REQUIRE(regionForBiome(outOfRange) == DEFAULT_REGION_FALLBACK);
}
