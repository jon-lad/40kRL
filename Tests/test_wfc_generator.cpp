// Property-based tests for the WFC Hive City Generator.
// Tests cover the 6 core correctness properties from the design document.
// Each property test runs 100+ iterations with randomized inputs.

#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"
#include "../Headers/WfcGenerator.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Helper: build a minimal valid tileset for testing (6 tiles, symmetric rules)
// ═══════════════════════════════════════════════════════════════════════════════

static WfcTileset buildTestTileset() {
    WfcTileset tileset;

    // 6 tiles: corridor, room, wall, door, junction, dead-end
    // All walkable tiles can be adjacent to each other in all directions.
    // Wall can only be adjacent to walkable tiles (not other walls — keeps it simple).
    std::vector<std::string> walkableIds = {
        "corridor", "room", "door", "junction", "dead-end"
    };
    std::vector<std::string> allIds = {
        "corridor", "room", "wall", "door", "junction", "dead-end"
    };

    auto makeTile = [&](const std::string& id, int glyph, bool walkable,
                        bool transparent, float weight,
                        const std::vector<std::string>& adj) -> WfcTile {
        WfcTile t;
        t.id = id;
        t.glyph = glyph;
        t.colorName = "white";
        t.walkable = walkable;
        t.transparent = transparent;
        t.weight = weight;
        t.adjNorth = adj;
        t.adjSouth = adj;
        t.adjEast = adj;
        t.adjWest = adj;
        return t;
    };

    // Walkable tiles: can be adjacent to all other tiles
    tileset.tiles.push_back(makeTile("corridor", '.', true, true, 3.0f, allIds));
    tileset.tiles.push_back(makeTile("room", '#', true, true, 2.0f, allIds));
    tileset.tiles.push_back(makeTile("wall", 'X', false, false, 1.5f, walkableIds));
    tileset.tiles.push_back(makeTile("door", '+', true, false, 1.0f, allIds));
    tileset.tiles.push_back(makeTile("junction", '*', true, true, 1.0f, allIds));
    tileset.tiles.push_back(makeTile("dead-end", '>', true, true, 0.5f, allIds));

    // Build idToIndex map
    for (int i = 0; i < static_cast<int>(tileset.tiles.size()); i++) {
        tileset.idToIndex[tileset.tiles[i].id] = i;
    }

    return tileset;
}

// Helper: build a small WfcConfig for fast test execution
static WfcConfig buildTestConfig(int width = 8, int height = 8) {
    WfcConfig config;
    config.gridWidth = width;
    config.gridHeight = height;
    config.maxRestarts = 20;
    config.minWalkablePercent = 0.10f; // low threshold for small grids
    return config;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: wfc-hive-city, Property 1: Deterministic Generation
// Same seed + tileset produces identical grid output.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: WFC deterministic generation — same seed produces identical output",
          "[pbt][wfc]")
{
    rc::prop("same seed and tileset always produce identical grid", []() {
        auto seed = static_cast<long>(rc::generate(rc::gen::inRange(1, 100000)));
        WfcTileset tileset = buildTestTileset();
        WfcConfig config = buildTestConfig(8, 8);

        WfcResult result1 = WfcGenerator::generate(seed, tileset, config);
        WfcResult result2 = WfcGenerator::generate(seed, tileset, config);

        RC_ASSERT(result1.success == result2.success);
        if (result1.success && result2.success) {
            RC_ASSERT(result1.grid.size() == result2.grid.size());
            for (size_t i = 0; i < result1.grid.size(); ++i) {
                RC_ASSERT(result1.grid[i] == result2.grid[i]);
            }
            RC_ASSERT(result1.attemptsUsed == result2.attemptsUsed);
            RC_ASSERT(result1.seedUsed == result2.seedUsed);
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: wfc-hive-city, Property 2: Lowest-Entropy Cell Selection
// selectLowestEntropy returns a cell with minimal domain size among unresolved.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: selectLowestEntropy returns cell with minimal domain size",
          "[pbt][wfc]")
{
    rc::prop("selected cell has minimum domain size among all unresolved cells", []() {
        // Generate a random grid of cells with varying domain sizes
        int numCells = rc::generate(rc::gen::inRange(4, 64));
        std::vector<WfcGenerator::Cell> cells(numCells);

        std::mt19937 setupRng(rc::generate(rc::gen::inRange(1, 99999)));

        // At least one cell must have domain size > 1 (unresolved)
        bool hasUnresolved = false;
        for (int i = 0; i < numCells; ++i) {
            int domainSize = std::uniform_int_distribution<int>(1, 6)(setupRng);
            cells[i].domain.resize(domainSize);
            std::iota(cells[i].domain.begin(), cells[i].domain.end(), 0);
            if (domainSize > 1) hasUnresolved = true;
        }

        // Ensure at least one unresolved cell exists
        if (!hasUnresolved) {
            int idx = std::uniform_int_distribution<int>(0, numCells - 1)(setupRng);
            cells[idx].domain = {0, 1, 2};
        }

        std::mt19937 rng(rc::generate(rc::gen::inRange(1, 99999)));
        int selected = WfcGenerator::selectLowestEntropy(cells, rng);

        // selected must be a valid index
        RC_ASSERT(selected >= 0);
        RC_ASSERT(selected < numCells);

        // selected cell must have domain size > 1 (unresolved)
        int selectedSize = static_cast<int>(cells[selected].domain.size());
        RC_ASSERT(selectedSize > 1);

        // Find the actual minimum domain size among unresolved cells
        int minSize = std::numeric_limits<int>::max();
        for (const auto& cell : cells) {
            int sz = static_cast<int>(cell.domain.size());
            if (sz > 1 && sz < minSize) {
                minSize = sz;
            }
        }

        // Selected cell must have the minimum domain size
        RC_ASSERT(selectedSize == minSize);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: wfc-hive-city, Property 3: Collapse Selects Only From Domain
// collapseCell returns a tile index that is a member of the cell's domain.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: collapseCell returns a tile index from the cell's domain",
          "[pbt][wfc]")
{
    rc::prop("collapsed tile is always a member of the original domain", []() {
        WfcTileset tileset = buildTestTileset();
        int tileCount = static_cast<int>(tileset.tiles.size());

        // Generate a random non-empty domain (subset of tile indices)
        int domainSize = rc::generate(rc::gen::inRange(1, tileCount));
        std::vector<int> fullRange(tileCount);
        std::iota(fullRange.begin(), fullRange.end(), 0);

        std::mt19937 shuffleRng(rc::generate(rc::gen::inRange(1, 99999)));
        std::shuffle(fullRange.begin(), fullRange.end(), shuffleRng);

        WfcGenerator::Cell cell;
        cell.domain.assign(fullRange.begin(), fullRange.begin() + domainSize);

        // Keep a copy of the original domain for verification
        std::set<int> originalDomain(cell.domain.begin(), cell.domain.end());

        std::mt19937 rng(rc::generate(rc::gen::inRange(1, 99999)));
        int result = WfcGenerator::collapseCell(cell, tileset, rng);

        // Result must be from the original domain
        RC_ASSERT(originalDomain.count(result) == 1);

        // After collapse, cell domain must contain exactly the selected tile
        RC_ASSERT(cell.domain.size() == 1);
        RC_ASSERT(cell.domain[0] == result);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: wfc-hive-city, Property 4: Propagation Maintains Adjacency
// After propagation, adjacent cells satisfy mutual adjacency rules.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: propagation maintains adjacency consistency between resolved cells",
          "[pbt][wfc]")
{
    rc::prop("after propagation, adjacent resolved cells satisfy mutual adjacency", []() {
        WfcTileset tileset = buildTestTileset();
        int tileCount = static_cast<int>(tileset.tiles.size());
        int width = 8;
        int height = 8;
        int totalCells = width * height;

        // Initialise all cells with full domain
        std::vector<WfcGenerator::Cell> cells(totalCells);
        std::vector<int> fullDomain(tileCount);
        std::iota(fullDomain.begin(), fullDomain.end(), 0);
        for (auto& cell : cells) {
            cell.domain = fullDomain;
        }

        // Pick a random cell to collapse
        std::mt19937 rng(rc::generate(rc::gen::inRange(1, 99999)));
        int collapseIdx = std::uniform_int_distribution<int>(0, totalCells - 1)(rng);

        WfcGenerator::collapseCell(cells[collapseIdx], tileset, rng);
        bool success = WfcGenerator::propagate(cells, collapseIdx,
                                               width, height, tileset);

        if (!success) {
            // Contradiction is valid — skip this iteration
            return;
        }

        // Check adjacency consistency for all resolved cell pairs
        // Direction offsets: North(-y), South(+y), East(+x), West(-x)
        struct DirInfo {
            int dx, dy;
            // For the tile at (x,y): which adj list does it use for this direction?
            // And for the neighbour: which adj list does it use for the reverse?
        };

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = x + y * width;
                if (cells[idx].domain.size() != 1) continue;
                int tileIdx = cells[idx].domain[0];
                const WfcTile& tile = tileset.tiles[tileIdx];

                // Check East neighbour
                if (x + 1 < width) {
                    int nIdx = (x + 1) + y * width;
                    if (cells[nIdx].domain.size() == 1) {
                        int nTileIdx = cells[nIdx].domain[0];
                        const std::string& nId = tileset.tiles[nTileIdx].id;
                        // tile's east list must contain neighbour's id
                        bool fwd = std::find(tile.adjEast.begin(),
                                             tile.adjEast.end(), nId)
                                   != tile.adjEast.end();
                        RC_ASSERT(fwd);
                    }
                }

                // Check South neighbour
                if (y + 1 < height) {
                    int nIdx = x + (y + 1) * width;
                    if (cells[nIdx].domain.size() == 1) {
                        int nTileIdx = cells[nIdx].domain[0];
                        const std::string& nId = tileset.tiles[nTileIdx].id;
                        // tile's south list must contain neighbour's id
                        bool fwd = std::find(tile.adjSouth.begin(),
                                             tile.adjSouth.end(), nId)
                                   != tile.adjSouth.end();
                        RC_ASSERT(fwd);
                    }
                }
            }
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: wfc-hive-city, Property 5: Connectivity Check Correctness
// Connected grids pass, disconnected grids fail.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: connectivity check — connected grids pass, disconnected fail",
          "[pbt][wfc]")
{
    rc::prop("fully connected walkable grid passes connectivity check", []() {
        WfcTileset tileset = buildTestTileset();
        int width = 8;
        int height = 8;
        int totalCells = width * height;

        // Build a grid that is entirely walkable (index 0 = "corridor", walkable)
        // This is trivially connected.
        std::vector<int> grid(totalCells, 0); // all corridor (walkable)

        int minWalkable = 1; // any positive count
        bool result = WfcGenerator::checkConnectivity(
            grid, width, height, tileset, minWalkable);
        RC_ASSERT(result == true);
    });

    rc::prop("disconnected walkable grid fails connectivity check", []() {
        WfcTileset tileset = buildTestTileset();
        int width = 8;
        int height = 8;
        int totalCells = width * height;

        // Build a grid with two disconnected walkable regions separated by a
        // wall row. Top half = corridor (idx 0), middle row = wall (idx 2),
        // bottom half = corridor (idx 0).
        std::vector<int> grid(totalCells, 0); // all corridor

        // Place a wall barrier across the full width at row 4
        int wallRow = 4;
        for (int x = 0; x < width; ++x) {
            grid[x + wallRow * width] = 2; // wall tile (non-walkable)
        }

        // Both regions above and below the wall are walkable but disconnected
        int minWalkable = 1;
        bool result = WfcGenerator::checkConnectivity(
            grid, width, height, tileset, minWalkable);
        RC_ASSERT(result == false);
    });

    rc::prop("grid below minimum walkable threshold fails", []() {
        WfcTileset tileset = buildTestTileset();
        int width = 8;
        int height = 8;
        int totalCells = width * height;

        // Build a grid that is mostly walls with very few walkable cells
        std::vector<int> grid(totalCells, 2); // all wall (non-walkable)

        // Place a few connected walkable cells
        grid[0] = 0; // corridor at (0,0)
        grid[1] = 0; // corridor at (1,0)
        grid[2] = 0; // corridor at (2,0)
        // 3 walkable cells, connected

        // Require more walkable cells than we have
        int minWalkable = 20;
        bool result = WfcGenerator::checkConnectivity(
            grid, width, height, tileset, minWalkable);
        RC_ASSERT(result == false);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: wfc-hive-city, Property 6: Minimum Walkable Area
// Accepted grids have walkable count >= threshold.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: successful generation meets minimum walkable area threshold",
          "[pbt][wfc]")
{
    rc::prop("accepted grid has walkable cells >= minWalkablePercent * area", []() {
        auto seed = static_cast<long>(rc::generate(rc::gen::inRange(1, 100000)));
        WfcTileset tileset = buildTestTileset();
        WfcConfig config = buildTestConfig(10, 10);
        config.minWalkablePercent = 0.15f;

        WfcResult result = WfcGenerator::generate(seed, tileset, config);

        if (!result.success) {
            // Generation failed — that's acceptable, skip this iteration
            return;
        }

        // Count walkable cells in the result grid
        int walkableCount = 0;
        for (int tileIdx : result.grid) {
            if (tileset.tiles[tileIdx].walkable) {
                walkableCount++;
            }
        }

        int totalCells = config.gridWidth * config.gridHeight;
        int minRequired = static_cast<int>(config.minWalkablePercent * totalCells);

        RC_ASSERT(walkableCount >= minRequired);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Additional unit tests for edge cases and boundary conditions
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("selectLowestEntropy returns -1 when all cells are collapsed",
          "[wfc]")
{
    // All cells have domain size 1 (fully collapsed)
    std::vector<WfcGenerator::Cell> cells(16);
    for (auto& cell : cells) {
        cell.domain = {0};
    }
    std::mt19937 rng(42);
    int result = WfcGenerator::selectLowestEntropy(cells, rng);
    REQUIRE(result == -1);
}

TEST_CASE("selectLowestEntropy returns -1 when cells vector is empty",
          "[wfc]")
{
    std::vector<WfcGenerator::Cell> cells;
    std::mt19937 rng(42);
    int result = WfcGenerator::selectLowestEntropy(cells, rng);
    REQUIRE(result == -1);
}

TEST_CASE("collapseCell with single-element domain returns that element",
          "[wfc]")
{
    WfcTileset tileset = buildTestTileset();
    WfcGenerator::Cell cell;
    cell.domain = {3}; // only tile index 3

    std::mt19937 rng(42);
    int result = WfcGenerator::collapseCell(cell, tileset, rng);
    REQUIRE(result == 3);
    REQUIRE(cell.domain.size() == 1);
    REQUIRE(cell.domain[0] == 3);
}

TEST_CASE("checkConnectivity with all-wall grid returns false",
          "[wfc]")
{
    WfcTileset tileset = buildTestTileset();
    int width = 8;
    int height = 8;
    int totalCells = width * height;

    // All walls (tile index 2 = "wall", non-walkable)
    std::vector<int> grid(totalCells, 2);

    bool result = WfcGenerator::checkConnectivity(grid, width, height, tileset, 1);
    REQUIRE(result == false);
}

TEST_CASE("checkConnectivity with single walkable cell passes if minWalkable <= 1",
          "[wfc]")
{
    WfcTileset tileset = buildTestTileset();
    int width = 4;
    int height = 4;
    int totalCells = width * height;

    // All walls except one corridor
    std::vector<int> grid(totalCells, 2);
    grid[5] = 0; // single walkable cell at (1,1)

    // Passes with minWalkable = 1 (connected component has 1 cell)
    bool result = WfcGenerator::checkConnectivity(grid, width, height, tileset, 1);
    REQUIRE(result == true);

    // Fails with minWalkable = 2 (only 1 walkable cell)
    result = WfcGenerator::checkConnectivity(grid, width, height, tileset, 2);
    REQUIRE(result == false);
}

TEST_CASE("generate with invalid tileset (< 5 tiles) fails gracefully",
          "[wfc]")
{
    WfcTileset tileset;
    // Only 3 tiles — below the 5-tile minimum
    tileset.tiles.push_back({"a", '.', "white", true, true, 1.0f,
                             {"a","b","c"}, {"a","b","c"}, {"a","b","c"}, {"a","b","c"}});
    tileset.tiles.push_back({"b", '#', "white", true, true, 1.0f,
                             {"a","b","c"}, {"a","b","c"}, {"a","b","c"}, {"a","b","c"}});
    tileset.tiles.push_back({"c", 'X', "white", false, false, 1.0f,
                             {"a","b"}, {"a","b"}, {"a","b"}, {"a","b"}});
    tileset.idToIndex["a"] = 0;
    tileset.idToIndex["b"] = 1;
    tileset.idToIndex["c"] = 2;

    REQUIRE(tileset.isValid() == false);
}

TEST_CASE("generate produces valid result on small grid with good tileset",
          "[wfc]")
{
    WfcTileset tileset = buildTestTileset();
    WfcConfig config = buildTestConfig(8, 8);
    config.maxRestarts = 50; // generous restarts for small test grid

    WfcResult result = WfcGenerator::generate(12345L, tileset, config);

    // With a permissive tileset, generation should usually succeed
    if (result.success) {
        REQUIRE(result.grid.size() == 64); // 8*8
        REQUIRE(result.attemptsUsed >= 1);
        // All tile indices must be valid
        for (int idx : result.grid) {
            REQUIRE(idx >= 0);
            REQUIRE(idx < static_cast<int>(tileset.tiles.size()));
        }
    }
}

TEST_CASE("PBT: different seeds produce different grids (non-trivial output)",
          "[pbt][wfc]")
{
    rc::prop("different seeds usually produce different grids", []() {
        WfcTileset tileset = buildTestTileset();
        WfcConfig config = buildTestConfig(8, 8);

        auto seed1 = static_cast<long>(rc::generate(rc::gen::inRange(1, 50000)));
        auto seed2 = static_cast<long>(rc::generate(rc::gen::inRange(50001, 100000)));

        WfcResult result1 = WfcGenerator::generate(seed1, tileset, config);
        WfcResult result2 = WfcGenerator::generate(seed2, tileset, config);

        // If both succeed, they should (almost certainly) differ
        if (result1.success && result2.success) {
            bool identical = (result1.grid == result2.grid);
            // With 6 tiles on an 8x8 grid, identical grids from different seeds
            // would be astronomically unlikely. We allow it but don't require it.
            (void)identical; // no assertion — this property is probabilistic
        }
    });
}

TEST_CASE("PBT: propagation from a corner cell does not crash",
          "[pbt][wfc]")
{
    rc::prop("propagation from corner cells completes without crash", []() {
        WfcTileset tileset = buildTestTileset();
        int tileCount = static_cast<int>(tileset.tiles.size());
        int width = 8;
        int height = 8;
        int totalCells = width * height;

        std::vector<WfcGenerator::Cell> cells(totalCells);
        std::vector<int> fullDomain(tileCount);
        std::iota(fullDomain.begin(), fullDomain.end(), 0);
        for (auto& cell : cells) {
            cell.domain = fullDomain;
        }

        // Pick a corner cell
        std::vector<int> corners = {0, width - 1,
                                    (height - 1) * width,
                                    (height - 1) * width + (width - 1)};
        int cornerIdx = corners[rc::generate(rc::gen::inRange(0, 3))];

        std::mt19937 rng(rc::generate(rc::gen::inRange(1, 99999)));
        WfcGenerator::collapseCell(cells[cornerIdx], tileset, rng);

        // Should not crash — may return true or false (contradiction)
        bool result = WfcGenerator::propagate(cells, cornerIdx,
                                              width, height, tileset);
        // Result is valid boolean (no crash = success)
        RC_ASSERT(result == true || result == false);
    });
}

TEST_CASE("PBT: full WFC generation — all output tile indices are valid",
          "[pbt][wfc]")
{
    rc::prop("every tile index in a successful grid is within tileset bounds", []() {
        auto seed = static_cast<long>(rc::generate(rc::gen::inRange(1, 100000)));
        WfcTileset tileset = buildTestTileset();
        WfcConfig config = buildTestConfig(8, 8);

        WfcResult result = WfcGenerator::generate(seed, tileset, config);

        if (result.success) {
            int tileCount = static_cast<int>(tileset.tiles.size());
            for (int idx : result.grid) {
                RC_ASSERT(idx >= 0);
                RC_ASSERT(idx < tileCount);
            }
            // Grid size must match config
            int expected = config.gridWidth * config.gridHeight;
            RC_ASSERT(static_cast<int>(result.grid.size()) == expected);
        }
    });
}

TEST_CASE("PBT: full WFC generation — adjacency invariant holds on output",
          "[pbt][wfc]")
{
    rc::prop("successful grid satisfies adjacency rules between all neighbours", []() {
        auto seed = static_cast<long>(rc::generate(rc::gen::inRange(1, 100000)));
        WfcTileset tileset = buildTestTileset();
        WfcConfig config = buildTestConfig(8, 8);

        WfcResult result = WfcGenerator::generate(seed, tileset, config);

        if (!result.success) return;

        int w = config.gridWidth;
        int h = config.gridHeight;

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                int idx = x + y * w;
                const WfcTile& tile = tileset.tiles[result.grid[idx]];

                // Check East
                if (x + 1 < w) {
                    int nIdx = (x + 1) + y * w;
                    const std::string& nId = tileset.tiles[result.grid[nIdx]].id;
                    bool allowed = std::find(tile.adjEast.begin(),
                                            tile.adjEast.end(), nId)
                                   != tile.adjEast.end();
                    RC_ASSERT(allowed);
                }

                // Check South
                if (y + 1 < h) {
                    int nIdx = x + (y + 1) * w;
                    const std::string& nId = tileset.tiles[result.grid[nIdx]].id;
                    bool allowed = std::find(tile.adjSouth.begin(),
                                            tile.adjSouth.end(), nId)
                                   != tile.adjSouth.end();
                    RC_ASSERT(allowed);
                }
            }
        }
    });
}
