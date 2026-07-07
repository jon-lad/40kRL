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
