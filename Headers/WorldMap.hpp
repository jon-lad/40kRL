#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

// sol::table is a template alias (basic_table_core<...>), not a plain class, so it
// cannot be forward-declared with `class table;`. sol's lightweight forward header
// provides the correct declaration without pulling in the full sol2 implementation.
#include <sol/forward.hpp>

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

// Maps a Region_Name (or the Universal_Tag "Universal") to a non-negative selection
// weight, mirroring the string-keyed `region` table in Scripts/Equipment.lua. Also
// aliased in Engine.hpp for use on EquipmentTemplate (identical redeclaration).
using RegionWeights = std::map<std::string, int>;

// The documented default region weighting applied to an untagged/malformed entry:
// { ImperialHuman = DEFAULT_REGION_WEIGHT }. Matches the terminal cumulative value
// convention used by the NPC `chance` columns (equipment-region-assignment Reqs 5.3, 6.3).
inline constexpr int DEFAULT_REGION_WEIGHT = 100;

// Region-utility free helpers (equipment-region-assignment feature). These are
// engine-isolated (no engine.* access) so they are unit/property testable without an
// initialized Engine, per the test-isolation steering rule. Implementations live in
// Source/WorldMap.cpp alongside regionForBiome / resolveDefaultRegion.

// Returns true when name is a valid Region_Name from the shared taxonomy (Ork, Eldar,
// DarkEldar, Necron, Tau, Tyranid, Kroot, Chaos, ImperialHuman, Servitor) or the
// Universal_Tag "Universal".
bool isValidRegionName(const std::string& name);

// Parses a sol2 `region` table into RegionWeights: ignores non-string keys, keeps only
// valid Region_Name/Universal keys with non-negative integer weights, and drops unknown
// keys and malformed weights per-pair without throwing. Does NOT apply the default.
RegionWeights parseRegionWeights(const sol::table& regionTable);

// Applies the documented default { ImperialHuman = DEFAULT_REGION_WEIGHT } when the
// supplied map is empty; leaves a non-empty map unchanged.
void applyRegionDefault(RegionWeights& weights);

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
