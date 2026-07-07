#include "WorldMap.h"
#include "main.h"
#include <sol/sol.hpp>
#include <algorithm>

BiomeType classifyBiome(float noiseValue, float swampThreshold, float forestThreshold, float desertThreshold) {
	if (noiseValue < swampThreshold) {
		return BiomeType::TOXIC_SWAMP;
	}
	if (noiseValue < forestThreshold) {
		return BiomeType::DEAD_FOREST;
	}
	if (noiseValue < desertThreshold) {
		return BiomeType::ASH_DESERT;
	}
	return BiomeType::WASTELAND;
}

void generateWorldMapTerrain(WorldMapState& state, sol::state& lua) {
	// ── 1. Load noise/threshold parameters from Lua config (with compiled defaults) ──

	float noiseScale       = 0.03f;
	int   octaves          = 4;
	float lacunarity       = 2.0f;
	float swampThreshold   = -0.4f;
	float forestThreshold  = -0.1f;
	float desertThreshold  = 0.2f;

	try {
		sol::table cfg = lua["config"];
		if (cfg.valid()) {
			noiseScale      = cfg.get_or("worldMapNoiseScale",      noiseScale);
			octaves         = cfg.get_or("worldMapOctaves",         octaves);
			lacunarity      = cfg.get_or("worldMapLacunarity",      lacunarity);
			swampThreshold  = cfg.get_or("worldMapSwampThreshold",  swampThreshold);
			forestThreshold = cfg.get_or("worldMapForestThreshold", forestThreshold);
			desertThreshold = cfg.get_or("worldMapDesertThreshold", desertThreshold);
		}
	} catch (const sol::error&) {
		// Config load failed — use compiled defaults silently.
	}

	// ── 2. Clamp out-of-range values and log warnings via Gui ──

	bool clamped = false;

	if (noiseScale <= 0.0f || noiseScale >= 1.0f) {
		noiseScale = std::max(0.001f, std::min(0.999f, noiseScale));
		clamped = true;
	}
	if (octaves < 1 || octaves > 8) {
		octaves = std::max(1, std::min(8, octaves));
		clamped = true;
	}
	if (lacunarity < 1.0f || lacunarity > 4.0f) {
		lacunarity = std::max(1.0f, std::min(4.0f, lacunarity));
		clamped = true;
	}
	if (swampThreshold < -1.0f || swampThreshold > 1.0f) {
		swampThreshold = std::max(-1.0f, std::min(1.0f, swampThreshold));
		clamped = true;
	}
	if (forestThreshold < -1.0f || forestThreshold > 1.0f) {
		forestThreshold = std::max(-1.0f, std::min(1.0f, forestThreshold));
		clamped = true;
	}
	if (desertThreshold < -1.0f || desertThreshold > 1.0f) {
		desertThreshold = std::max(-1.0f, std::min(1.0f, desertThreshold));
		clamped = true;
	}

	if (clamped) {
		engine.gui->message(Colors::damage,
			"Warning: world map config values clamped to valid ranges.");
	}

	// ── 3. Create TCODNoise with 2 dimensions, seeded from worldSeed ──

	TCODRandom rng(state.worldSeed, TCOD_RNG_CMWC);
	TCODNoise noise(2, lacunarity, 1.0f / lacunarity, &rng);

	// ── 4. Sample noise at each (x, y), classify biome, fill state.biomes ──

	state.biomes.resize(WORLD_MAP_WIDTH * WORLD_MAP_HEIGHT);

	for (int y = 0; y < WORLD_MAP_HEIGHT; ++y) {
		for (int x = 0; x < WORLD_MAP_WIDTH; ++x) {
			float coords[2] = { x * noiseScale, y * noiseScale };
			float value = noise.get(coords, TCOD_NOISE_PERLIN);

			state.biomes[x + y * WORLD_MAP_WIDTH] = classifyBiome(
				value, swampThreshold, forestThreshold, desertThreshold);
		}
	}
}
