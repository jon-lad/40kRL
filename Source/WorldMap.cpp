#include "WorldMap.h"

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
