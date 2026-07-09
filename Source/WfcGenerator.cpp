#include "WfcGenerator.h"

#include <algorithm>
#include <queue>
#include <random>
#include <numeric>

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
// WfcGenerator::checkConnectivity (stub — real implementation in task 3.1)
// ---------------------------------------------------------------------------

bool WfcGenerator::checkConnectivity(const std::vector<int>& grid, int width, int height,
                                     const WfcTileset& tileset, int minWalkableCells) {
	// Stub: always returns true. Task 3.1 will replace with flood-fill implementation.
	(void)grid;
	(void)width;
	(void)height;
	(void)tileset;
	(void)minWalkableCells;
	return true;
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
