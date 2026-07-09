#pragma once

#include <functional>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

// A single logical tile type within a WFC tileset. Each tile has a unique identifier,
// display properties, walkability/transparency, and directional adjacency rules
// specifying which other tile types may appear in each cardinal direction.
struct WfcTile {
	std::string id;           // unique identifier (e.g. "corridor")
	int glyph;               // display character
	std::string colorName;   // colour name for Colors::colorFromName()
	bool walkable;
	bool transparent;
	float weight = 1.0f;     // selection bias during collapse (higher = more likely)
	// Adjacency: allowed neighbour tile IDs per direction
	std::vector<std::string> adjNorth;
	std::vector<std::string> adjSouth;
	std::vector<std::string> adjEast;
	std::vector<std::string> adjWest;
};

// A validated collection of WFC tiles with a fast ID-to-index lookup.
struct WfcTileset {
	std::vector<WfcTile> tiles;
	std::unordered_map<std::string, int> idToIndex; // fast ID -> index lookup

	// Returns tile index for a given ID, or -1 if not found.
	int indexOf(const std::string& id) const {
		auto it = idToIndex.find(id);
		return (it != idToIndex.end()) ? it->second : -1;
	}

	// True if the tileset has enough tiles for valid WFC generation (minimum 5).
	bool isValid() const { return tiles.size() >= 5; }
};

// Result of a WFC generation attempt.
struct WfcResult {
	bool success;            // true if a valid connected grid was produced
	std::vector<int> grid;   // flat array [x + y * width], values are tile indices
	int attemptsUsed;        // how many restarts were needed
	long seedUsed;           // the actual seed that produced this grid
};

// Configuration parameters for WFC generation, loaded from Config.lua.
struct WfcConfig {
	int maxRestarts = 10;
	float minWalkablePercent = 0.20f;
	int minStairDistance = 20;
	int gridWidth = 160;
	int gridHeight = 86;
	std::optional<long> seedOverride; // from Config.lua wfcSeed
	int minMonsters = 8;
	int maxMonsters = 16;
	int minItems = 3;
	int maxItems = 7;
};

// Standalone WFC level generator. No dependency on Map, Engine, or libtcod.
// Accepts a pre-validated tileset and grid dimensions, runs the WFC constraint-solving
// algorithm, and outputs a fully-resolved 2D grid of tile assignments.
class WfcGenerator {
public:
	// Internal state for a single generation attempt (public for property-based testing)
	struct Cell {
		std::vector<int> domain; // indices into tileset.tiles
	};

	// Run WFC algorithm. Returns resolved grid or failure indicator.
	// rngSeed: deterministic seed for generation
	// tileset: pre-validated tileset with adjacency rules
	// config: generation parameters (grid size, restart limit, walkable threshold)
	static WfcResult generate(long rngSeed, const WfcTileset& tileset,
	                          const WfcConfig& config);

	// Connectivity check: returns true if all walkable cells form one connected component
	// (four-directional) and walkable count >= minWalkableCells. Exposed for testing.
	static bool checkConnectivity(const std::vector<int>& grid, int width, int height,
	                              const WfcTileset& tileset, int minWalkableCells);

	// Select lowest-entropy cell (fewest domain options). Ties broken by RNG.
	// Exposed for property-based testing.
	static int selectLowestEntropy(const std::vector<Cell>& cells, std::mt19937& rng);

	// Collapse a cell: weighted random selection from its domain.
	// Exposed for property-based testing.
	static int collapseCell(Cell& cell, const WfcTileset& tileset, std::mt19937& rng);

	// Propagate constraints from a collapsed cell. Returns false on contradiction.
	// Exposed for property-based testing.
	static bool propagate(std::vector<Cell>& cells, int startIdx,
	                      int width, int height, const WfcTileset& tileset);
};

// Callback type for logging warnings/errors during tileset and config loading.
using LogCallback = std::function<void(const std::string&)>;

// Load and validate WFC tileset from a Lua file. Returns empty tileset on failure.
// Logs warnings/errors via the provided callback.
WfcTileset loadWfcTileset(const std::string& filepath, LogCallback logger);

// Load WFC config parameters from a Lua config file.
// Missing values use compiled defaults; out-of-range values are clamped with warnings.
WfcConfig loadWfcConfig(const std::string& filepath, LogCallback logger);
