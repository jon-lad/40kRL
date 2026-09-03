#pragma once

#include <cstdint>
#include <string>
#include <vector>

// World map dimensions: each tile represents a 10km x 10km region.
static constexpr int WORLD_MAP_WIDTH  = 160;
static constexpr int WORLD_MAP_HEIGHT = 86;

// Biome classification for world map tiles, determined by Perlin noise thresholds.
enum class BiomeType : uint8_t {
	TOXIC_SWAMP  = 0,  // lowest noise values (< swampThreshold)
	DEAD_FOREST  = 1,  // [swampThreshold, forestThreshold)
	ASH_DESERT   = 2,  // [forestThreshold, desertThreshold)
	WASTELAND    = 3,  // >= desertThreshold
	HIVE_CITY    = 4   // overridden by city placement
};

// A marked fast-travel destination on the world map.
struct HiveCity {
	int x;              // world map tile coordinate
	int y;              // world map tile coordinate
	std::string name;   // up to 64 characters
};

// Holds all state for the world map overlay, active only during WORLD_MAP status.
struct WorldMapState {
	uint32_t worldSeed = 0;
	int playerX = 0;
	int playerY = 0;
	int cursorX = 0;
	int cursorY = 0;
	std::vector<BiomeType> biomes;   // flat array [x + y * WORLD_MAP_WIDTH], 160x86
	std::vector<HiveCity> cities;
	bool generated = false;          // true after first terrain + city generation
};

// Classifies a noise value into a biome type based on threshold ordering.
// Thresholds must satisfy: swampThreshold < forestThreshold < desertThreshold.
// The function is total: every float input maps to exactly one BiomeType.
BiomeType classifyBiome(float noiseValue, float swampThreshold, float forestThreshold, float desertThreshold);

// Compiled fallback Region_Name used when config.defaultRegion is absent/empty.
// Guarantees resolveDefaultRegion() never returns an empty string (Requirement 2.3).
inline constexpr const char* DEFAULT_REGION_FALLBACK = "Ork";

// Total mapping: every BiomeType maps to exactly one non-empty Region_Name (a race
// name in this iteration). This is the single point of BiomeType -> Region_Name
// derivation (Requirement 9.3). All biomes currently map to "Ork" so spawning is
// preserved everywhere (Requirement 7); later iterations expand this table.
std::string regionForBiome(BiomeType biome);

// Returns the configured Default_Region (config.defaultRegion in Scripts/Config.lua),
// or the compiled fallback DEFAULT_REGION_FALLBACK when the value is absent/empty.
// Never returns an empty string (Requirements 2.2, 2.3).
std::string resolveDefaultRegion();

// Forward declarations for sol types (avoids pulling sol2 into the header).
namespace sol { class state; }

// Generates terrain biomes for the world map using Perlin noise.
// Reads noise parameters (scale, octaves, lacunarity, thresholds) from the Lua
// config table, with compiled defaults as fallback. Clamps invalid config values
// to valid bounds and logs warnings via Gui. Fills state.biomes (160×86 flat array).
void generateWorldMapTerrain(WorldMapState& state, sol::state& lua);

// Places hive cities on the world map using a rejection-sampling algorithm.
// Reads city count, separation distance, and name table from the Lua config table.
// Cities are placed only on WASTELAND or ASH_DESERT tiles, with a minimum Euclidean
// separation between all pairs. Uses worldSeed for deterministic RNG. On placement
// failure (100 attempts exhausted for a city), logs a warning via Gui.
void placeHiveCities(WorldMapState& state, sol::state& lua);
