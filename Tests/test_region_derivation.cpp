#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"

#include "WorldMap.hpp"

#include <string>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: region-based-npc-spawns — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 1: Biome-to-region mapping is total and deterministic ──────────
// Feature: region-based-npc-spawns, Property 1: Biome-to-region mapping is total and deterministic
// **Validates: Requirements 1.2, 1.3, 1.4**
//
// For any BiomeType value, regionForBiome returns exactly one non-empty
// Region_Name, and calling it again with the same biome returns a string-equal
// result. Because the same derivation function serves every level source
// (outdoor, BSP-from-world-tile), this single property covers derivation at all
// call sites.

TEST_CASE("PBT: Property 1 — biome-to-region mapping is total and deterministic",
          "[pbt][property][region-based-npc-spawns]")
{
    rc::prop("regionForBiome returns a non-empty, deterministic Region_Name for every BiomeType", []() {
        // Generate across all five BiomeType values. rc::gen::inRange uses
        // INCLUSIVE bounds [a, b] in this project, so the upper bound is the
        // last enum value (HIVE_CITY) — every biome is reachable.
        const int biomeValue = *rc::gen::inRange(0, static_cast<int>(BiomeType::HIVE_CITY));
        const BiomeType biome = static_cast<BiomeType>(biomeValue);

        // Totality: the mapping yields a non-empty Region_Name for every input.
        const std::string region = regionForBiome(biome);
        RC_ASSERT(!region.empty());

        // Determinism: repeated calls with the same biome are string-equal.
        const std::string region2 = regionForBiome(biome);
        RC_ASSERT(region == region2);
    });
}
