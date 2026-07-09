#include "WfcGenerator.h"

#include <algorithm>
#include <queue>
#include <random>
#include <numeric>
#include <sol/sol.hpp>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Direction enum for internal use
enum class Dir { North, South, East, West };

// Get the neighbour index in a given direction, or -1 if out of bounds.
static int neighbourIndex(int idx, Dir dir, int width, int height) {
	int x = idx % width;
	int y = idx / width;
	switch (dir) {
	case Dir::North: y -= 1; break;
	case Dir::South: y += 1; break;
	case Dir::East:  x += 1; break;
	case Dir::West:  x -= 1; break;
	}
	if (x < 0 || x >= width || y < 0 || y >= height) return -1;
	return x + y * width;
}

// Get the adjacency list for a tile in a given direction.
static const std::vector<std::string>& getAdj(const WfcTile& tile, Dir dir) {
	switch (dir) {
	case Dir::North: return tile.adjNorth;
	case Dir::South: return tile.adjSouth;
	case Dir::East:  return tile.adjEast;
	case Dir::West:  return tile.adjWest;
	}
	return tile.adjNorth; // unreachable
}

// Check if a tile ID is present in an adjacency list.
static bool adjContains(const std::vector<std::string>& adjList, const std::string& id) {
	return std::find(adjList.begin(), adjList.end(), id) != adjList.end();
}

// ---------------------------------------------------------------------------
// WfcGenerator::selectLowestEntropy
// ---------------------------------------------------------------------------

int WfcGenerator::selectLowestEntropy(const std::vector<Cell>& cells, std::mt19937& rng) {
	int minSize = std::numeric_limits<int>::max();

	// First pass: find the minimum domain size > 1
	for (const auto& cell : cells) {
		int sz = static_cast<int>(cell.domain.size());
		if (sz > 1 && sz < minSize) {
			minSize = sz;
		}
	}

	// If no cell has domain > 1, everything is collapsed (or contradiction)
	if (minSize == std::numeric_limits<int>::max()) {
		return -1;
	}

	// Second pass: collect all cells with that minimum size
	std::vector<int> candidates;
	for (int i = 0; i < static_cast<int>(cells.size()); ++i) {
		if (static_cast<int>(cells[i].domain.size()) == minSize) {
			candidates.push_back(i);
		}
	}

	// Break ties with RNG
	if (candidates.size() == 1) {
		return candidates[0];
	}
	std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size()) - 1);
	return candidates[dist(rng)];
}

// ---------------------------------------------------------------------------
// WfcGenerator::collapseCell
// ---------------------------------------------------------------------------

int WfcGenerator::collapseCell(Cell& cell, const WfcTileset& tileset, std::mt19937& rng) {
	// Compute total weight of tiles in the cell's domain
	float totalWeight = 0.0f;
	for (int tileIdx : cell.domain) {
		totalWeight += tileset.tiles[tileIdx].weight;
	}

	// Generate random float in [0, totalWeight)
	std::uniform_real_distribution<float> dist(0.0f, totalWeight);
	float threshold = dist(rng);

	// Walk domain accumulating weights until threshold exceeded
	float accumulated = 0.0f;
	int selected = cell.domain.back(); // fallback to last (shouldn't happen)
	for (int tileIdx : cell.domain) {
		accumulated += tileset.tiles[tileIdx].weight;
		if (accumulated > threshold) {
			selected = tileIdx;
			break;
		}
	}

	// Collapse: set domain to only the selected tile
	cell.domain.clear();
	cell.domain.push_back(selected);
	return selected;
}

// ---------------------------------------------------------------------------
// WfcGenerator::propagate
// ---------------------------------------------------------------------------

bool WfcGenerator::propagate(std::vector<Cell>& cells, int startIdx,
                             int width, int height, const WfcTileset& tileset) {
	std::queue<int> queue;
	queue.push(startIdx);

	// Track which cells are already in the queue to avoid duplicates
	std::vector<bool> inQueue(cells.size(), false);
	inQueue[startIdx] = true;

	static const Dir directions[] = { Dir::North, Dir::South, Dir::East, Dir::West };

	while (!queue.empty()) {
		int current = queue.front();
		queue.pop();
		inQueue[current] = false;

		// For each direction, check the neighbour
		for (Dir dir : directions) {
			int nIdx = neighbourIndex(current, dir, width, height);
			if (nIdx < 0) continue; // out of bounds

			Cell& nbr = cells[nIdx];
			if (nbr.domain.size() <= 1) {
				// Already collapsed or contradiction — skip
				if (nbr.domain.empty()) return false;
				continue;
			}

			// Filter neighbour's domain: keep only tiles allowed by at least one
			// tile in current cell's domain in the corresponding direction
			std::vector<int> newDomain;
			newDomain.reserve(nbr.domain.size());

			for (int nTile : nbr.domain) {
				const std::string& nTileId = tileset.tiles[nTile].id;
				bool allowed = false;
				for (int cTile : cells[current].domain) {
					const auto& adjList = getAdj(tileset.tiles[cTile], dir);
					if (adjContains(adjList, nTileId)) {
						allowed = true;
						break;
					}
				}
				if (allowed) {
					newDomain.push_back(nTile);
				}
			}

			// If domain didn't change, no need to propagate further from this neighbour
			if (newDomain.size() == nbr.domain.size()) {
				continue;
			}

			// If domain became empty, contradiction
			if (newDomain.empty()) {
				return false;
			}

			// Update the neighbour's domain
			nbr.domain = std::move(newDomain);

			// Add neighbour to queue if not already there
			if (!inQueue[nIdx]) {
				queue.push(nIdx);
				inQueue[nIdx] = true;
			}
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// WfcGenerator::checkConnectivity
// ---------------------------------------------------------------------------

bool WfcGenerator::checkConnectivity(const std::vector<int>& grid, int width, int height,
                                     const WfcTileset& tileset, int minWalkableCells) {
	int totalCells = width * height;

	// Find total walkable cells and the first walkable cell (left-to-right, top-to-bottom)
	int totalWalkable = 0;
	int startIdx = -1;
	for (int i = 0; i < totalCells; ++i) {
		if (tileset.tiles[grid[i]].walkable) {
			totalWalkable++;
			if (startIdx == -1) startIdx = i;
		}
	}

	// Check minimum walkable threshold
	if (totalWalkable < minWalkableCells) return false;

	// If no walkable cells at all, fail
	if (startIdx == -1) return false;

	// BFS from startIdx using four-directional adjacency
	std::vector<bool> visited(totalCells, false);
	std::queue<int> bfsQueue;
	bfsQueue.push(startIdx);
	visited[startIdx] = true;
	int reachedCount = 0;

	while (!bfsQueue.empty()) {
		int idx = bfsQueue.front();
		bfsQueue.pop();
		reachedCount++;

		int x = idx % width;
		int y = idx / width;

		// Check 4 cardinal neighbours (N, S, E, W)
		const int dx[] = {0, 0, 1, -1};
		const int dy[] = {-1, 1, 0, 0};
		for (int d = 0; d < 4; d++) {
			int nx = x + dx[d];
			int ny = y + dy[d];
			if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
			int nIdx = nx + ny * width;
			if (!visited[nIdx] && tileset.tiles[grid[nIdx]].walkable) {
				visited[nIdx] = true;
				bfsQueue.push(nIdx);
			}
		}
	}

	// All walkable cells must be reachable from the start (single connected component)
	return (reachedCount == totalWalkable);
}

// ---------------------------------------------------------------------------
// WfcGenerator::generate
// ---------------------------------------------------------------------------

WfcResult WfcGenerator::generate(long rngSeed, const WfcTileset& tileset,
                                 const WfcConfig& config) {
	int totalCells = config.gridWidth * config.gridHeight;
	int tileCount = static_cast<int>(tileset.tiles.size());
	int minWalkable = static_cast<int>(config.minWalkablePercent * totalCells);

	for (int attempt = 0; attempt <= config.maxRestarts; ++attempt) {
		// Seed RNG deterministically per attempt
		std::mt19937 rng(static_cast<unsigned long>(rngSeed + attempt));

		// Initialise all cells with full domain
		std::vector<Cell> cells(totalCells);
		std::vector<int> fullDomain(tileCount);
		std::iota(fullDomain.begin(), fullDomain.end(), 0);
		for (auto& cell : cells) {
			cell.domain = fullDomain;
		}

		bool contradiction = false;

		// Main WFC loop: select -> collapse -> propagate
		while (true) {
			int idx = selectLowestEntropy(cells, rng);
			if (idx == -1) {
				// All cells collapsed — grid is complete
				break;
			}

			collapseCell(cells[idx], tileset, rng);

			if (!propagate(cells, idx, config.gridWidth, config.gridHeight, tileset)) {
				contradiction = true;
				break;
			}
		}

		if (contradiction) {
			continue; // Try next attempt
		}

		// Build the resolved grid
		std::vector<int> grid(totalCells);
		for (int i = 0; i < totalCells; ++i) {
			grid[i] = cells[i].domain[0];
		}

		// Run connectivity check (stub returns true for now)
		if (!checkConnectivity(grid, config.gridWidth, config.gridHeight, tileset, minWalkable)) {
			continue; // Failed connectivity, try next attempt
		}

		// Success
		WfcResult result;
		result.success = true;
		result.grid = std::move(grid);
		result.attemptsUsed = attempt + 1;
		result.seedUsed = rngSeed + attempt;
		return result;
	}

	// All attempts exhausted — return failure
	WfcResult result;
	result.success = false;
	result.attemptsUsed = config.maxRestarts + 1;
	result.seedUsed = rngSeed;
	return result;
}


// ---------------------------------------------------------------------------
// loadWfcTileset — Parse and validate WFC tileset from Lua
// ---------------------------------------------------------------------------

WfcTileset loadWfcTileset(const std::string& filepath, LogCallback logger) {
	WfcTileset tileset;

	// Open Lua state and load the script file
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::string);

	try {
		lua.script_file(filepath);
	} catch (const sol::error& e) {
		if (logger) {
			logger(std::string("WfcTiles.lua: failed to load — ") + e.what());
		}
		return tileset; // empty tileset
	}

	sol::table tilesTable = lua["wfc_tiles"];
	if (!tilesTable.valid()) {
		if (logger) {
			logger("WfcTiles.lua: missing 'wfc_tiles' table.");
		}
		return tileset; // empty tileset
	}

	// First pass: parse all tile entries, skipping invalid ones
	std::vector<WfcTile> parsedTiles;

	for (size_t i = 1; i <= tilesTable.size(); i++) {
		sol::object obj = tilesTable[i];
		if (obj.get_type() != sol::type::table) {
			if (logger) {
				logger("WfcTiles.lua: entry #" + std::to_string(i) + " is not a table, skipping.");
			}
			continue;
		}
		sol::table entry = obj.as<sol::table>();

		// Required fields
		std::string emptyStr;
		std::string id = entry.get_or("id", emptyStr);
		if (id.empty()) {
			if (logger) {
				logger("WfcTiles.lua: entry #" + std::to_string(i) + " missing 'id', skipping.");
			}
			continue;
		}

		sol::optional<int> glyphOpt = entry["glyph"];
		if (!glyphOpt.has_value()) {
			if (logger) {
				logger("WfcTiles.lua: tile '" + id + "' missing 'glyph', skipping.");
			}
			continue;
		}

		std::string colorName = entry.get_or("color", emptyStr);
		if (colorName.empty()) {
			if (logger) {
				logger("WfcTiles.lua: tile '" + id + "' missing 'color', skipping.");
			}
			continue;
		}

		sol::optional<bool> walkableOpt = entry["walkable"];
		if (!walkableOpt.has_value()) {
			if (logger) {
				logger("WfcTiles.lua: tile '" + id + "' missing 'walkable', skipping.");
			}
			continue;
		}

		sol::optional<bool> transparentOpt = entry["transparent"];
		if (!transparentOpt.has_value()) {
			if (logger) {
				logger("WfcTiles.lua: tile '" + id + "' missing 'transparent', skipping.");
			}
			continue;
		}

		// Adjacency table
		sol::optional<sol::table> adjOpt = entry["adjacency"];
		if (!adjOpt.has_value()) {
			if (logger) {
				logger("WfcTiles.lua: tile '" + id + "' missing 'adjacency', skipping.");
			}
			continue;
		}
		sol::table adj = adjOpt.value();

		// Validate four direction lists exist
		sol::optional<sol::table> northOpt = adj["north"];
		sol::optional<sol::table> southOpt = adj["south"];
		sol::optional<sol::table> eastOpt  = adj["east"];
		sol::optional<sol::table> westOpt  = adj["west"];

		if (!northOpt.has_value() || !southOpt.has_value() ||
		    !eastOpt.has_value() || !westOpt.has_value()) {
			if (logger) {
				logger("WfcTiles.lua: tile '" + id + "' adjacency missing direction list(s), skipping.");
			}
			continue;
		}

		// Helper to read a direction list from a Lua table
		auto readDirList = [](sol::table tbl) -> std::vector<std::string> {
			std::vector<std::string> result;
			for (size_t j = 1; j <= tbl.size(); j++) {
				sol::optional<std::string> s = tbl[j];
				if (s.has_value()) {
					result.push_back(s.value());
				}
			}
			return result;
		};

		WfcTile tile;
		tile.id = id;
		tile.glyph = glyphOpt.value();
		tile.colorName = colorName;
		tile.walkable = walkableOpt.value();
		tile.transparent = transparentOpt.value();
		tile.weight = entry.get_or("weight", 1.0f);
		tile.adjNorth = readDirList(northOpt.value());
		tile.adjSouth = readDirList(southOpt.value());
		tile.adjEast = readDirList(eastOpt.value());
		tile.adjWest = readDirList(westOpt.value());

		parsedTiles.push_back(std::move(tile));
	}

	// Build a set of valid tile IDs for adjacency reference validation
	std::unordered_map<std::string, int> validIds;
	for (int i = 0; i < static_cast<int>(parsedTiles.size()); i++) {
		validIds[parsedTiles[i].id] = i;
	}

	// Second pass: validate adjacency references — remove entries referencing nonexistent IDs
	for (auto& tile : parsedTiles) {
		auto filterAdj = [&](std::vector<std::string>& adjList, const std::string& tileId, const std::string& dirName) {
			std::vector<std::string> filtered;
			filtered.reserve(adjList.size());
			for (const auto& ref : adjList) {
				if (validIds.find(ref) != validIds.end()) {
					filtered.push_back(ref);
				} else {
					if (logger) {
						logger("WfcTiles.lua: tile '" + tileId + "' " + dirName +
						       " references nonexistent tile '" + ref + "', removing.");
					}
				}
			}
			adjList = std::move(filtered);
		};

		filterAdj(tile.adjNorth, tile.id, "north");
		filterAdj(tile.adjSouth, tile.id, "south");
		filterAdj(tile.adjEast, tile.id, "east");
		filterAdj(tile.adjWest, tile.id, "west");
	}

	// Third pass: validate and auto-correct symmetry
	// If tile A allows B east, then B must allow A west (and vice versa for all directions)
	auto ensureContains = [](std::vector<std::string>& list, const std::string& id) -> bool {
		if (std::find(list.begin(), list.end(), id) == list.end()) {
			list.push_back(id);
			return true; // was added (was missing)
		}
		return false; // already present
	};

	for (auto& tileA : parsedTiles) {
		// If A allows B north, B must allow A south
		for (const auto& bId : tileA.adjNorth) {
			auto it = validIds.find(bId);
			if (it != validIds.end()) {
				WfcTile& tileB = parsedTiles[it->second];
				if (ensureContains(tileB.adjSouth, tileA.id)) {
					if (logger) {
						logger("WfcTiles.lua: tile '" + tileA.id + "' allows '" + bId +
						       "' north but '" + bId + "' was missing '" + tileA.id +
						       "' south — auto-corrected.");
					}
				}
			}
		}
		// If A allows B south, B must allow A north
		for (const auto& bId : tileA.adjSouth) {
			auto it = validIds.find(bId);
			if (it != validIds.end()) {
				WfcTile& tileB = parsedTiles[it->second];
				if (ensureContains(tileB.adjNorth, tileA.id)) {
					if (logger) {
						logger("WfcTiles.lua: tile '" + tileA.id + "' allows '" + bId +
						       "' south but '" + bId + "' was missing '" + tileA.id +
						       "' north — auto-corrected.");
					}
				}
			}
		}
		// If A allows B east, B must allow A west
		for (const auto& bId : tileA.adjEast) {
			auto it = validIds.find(bId);
			if (it != validIds.end()) {
				WfcTile& tileB = parsedTiles[it->second];
				if (ensureContains(tileB.adjWest, tileA.id)) {
					if (logger) {
						logger("WfcTiles.lua: tile '" + tileA.id + "' allows '" + bId +
						       "' east but '" + bId + "' was missing '" + tileA.id +
						       "' west — auto-corrected.");
					}
				}
			}
		}
		// If A allows B west, B must allow A east
		for (const auto& bId : tileA.adjWest) {
			auto it = validIds.find(bId);
			if (it != validIds.end()) {
				WfcTile& tileB = parsedTiles[it->second];
				if (ensureContains(tileB.adjEast, tileA.id)) {
					if (logger) {
						logger("WfcTiles.lua: tile '" + tileA.id + "' allows '" + bId +
						       "' west but '" + bId + "' was missing '" + tileA.id +
						       "' east — auto-corrected.");
					}
				}
			}
		}
	}

	// Enforce minimum 5 valid tiles
	if (parsedTiles.size() < 5) {
		if (logger) {
			logger("WfcTiles.lua: only " + std::to_string(parsedTiles.size()) +
			       " valid tiles loaded (minimum 5 required). Returning empty tileset.");
		}
		return WfcTileset{}; // empty tileset
	}

	// Build final tileset with idToIndex map
	tileset.tiles = std::move(parsedTiles);
	for (int i = 0; i < static_cast<int>(tileset.tiles.size()); i++) {
		tileset.idToIndex[tileset.tiles[i].id] = i;
	}

	return tileset;
}
