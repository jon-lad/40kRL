#include "WorldMap.h"
#include "main.h"
#include <sol/sol.hpp>
#include <algorithm>
#include <cmath>

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

void placeHiveCities(WorldMapState& state, sol::state& lua) {
	// ── 1. Load city placement parameters from Lua config (with compiled defaults) ──

	int cityCount = 3;
	int separation = 15;
	std::vector<std::string> nameTable;

	try {
		sol::table cfg = lua["config"];
		if (cfg.valid()) {
			cityCount  = cfg.get_or("worldMapCityCount", cityCount);
			separation = cfg.get_or("worldMapCitySeparation", separation);

			sol::optional<sol::table> namesOpt = cfg["worldMapCityNames"];
			if (namesOpt) {
				sol::table names = namesOpt.value();
				for (size_t i = 1; i <= names.size(); ++i) {
					sol::optional<std::string> name = names[i];
					if (name) {
						nameTable.push_back(name.value());
					}
				}
			}
		}
	} catch (const sol::error&) {
		// Config load failed — use compiled defaults silently.
	}

	// Clamp city count to valid range [1, 20]
	if (cityCount < 1) cityCount = 1;
	if (cityCount > 20) cityCount = 20;

	// Clamp separation to valid range [1, 80]
	if (separation < 1) separation = 1;
	if (separation > 80) separation = 80;

	// Provide default name table if none loaded
	if (nameTable.empty()) {
		nameTable = {"Hive Primus", "Hive Secundus", "Hive Tertius", "Hive Quartus", "Hive Quintus"};
	}

	// ── 2. Create deterministic RNG from worldSeed ──

	TCODRandom rng(state.worldSeed, TCOD_RNG_CMWC);

	// ── 3. Place cities using rejection sampling ──

	int failedCities = 0;

	for (int i = 0; i < cityCount; ++i) {
		bool placed = false;

		for (int attempt = 0; attempt < 100; ++attempt) {
			int x = rng.getInt(0, WORLD_MAP_WIDTH - 1);
			int y = rng.getInt(0, WORLD_MAP_HEIGHT - 1);

			// Check biome is WASTELAND or ASH_DESERT
			BiomeType biome = state.biomes[x + y * WORLD_MAP_WIDTH];
			if (biome != BiomeType::WASTELAND && biome != BiomeType::ASH_DESERT) {
				continue;
			}

			// Check Euclidean distance to all previously placed cities >= separation
			bool tooClose = false;
			for (const auto& city : state.cities) {
				float dx = static_cast<float>(x - city.x);
				float dy = static_cast<float>(y - city.y);
				float dist = std::sqrt(dx * dx + dy * dy);
				if (dist < static_cast<float>(separation)) {
					tooClose = true;
					break;
				}
			}
			if (tooClose) {
				continue;
			}

			// ── 4. Assign name from Lua table ──

			std::string cityName;
			int nameIndex = static_cast<int>(state.cities.size());
			if (nameIndex < static_cast<int>(nameTable.size())) {
				cityName = nameTable[nameIndex];
			} else {
				// Table exhausted: reuse names with numeric suffix
				int baseIndex = nameIndex % static_cast<int>(nameTable.size());
				int suffix = (nameIndex / static_cast<int>(nameTable.size())) + 1;
				cityName = nameTable[baseIndex] + " " + std::to_string(suffix);
			}

			// Place the city
			HiveCity city;
			city.x = x;
			city.y = y;
			city.name = cityName;
			state.cities.push_back(city);

			// Mark biome as HIVE_CITY
			state.biomes[x + y * WORLD_MAP_WIDTH] = BiomeType::HIVE_CITY;

			placed = true;
			break;
		}

		if (!placed) {
			++failedCities;
		}
	}

	// ── 5. Log warning if any cities could not be placed ──

	if (failedCities > 0) {
		engine.gui->message(Colors::damage,
			"Warning: could not place # hive cities (separation/biome constraints).",
			failedCities);
	}
}
