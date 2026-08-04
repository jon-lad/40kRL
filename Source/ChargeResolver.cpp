#include "main.h"
#include "ChargeResolver.h"
#include <vector>
#include <utility>
#include <cmath>

namespace {
    // Simple Bresenham line algorithm producing all tiles from (x0,y0) to (x1,y1).
    void bresenhamLine(int x0, int y0, int x1, int y1, std::vector<std::pair<int,int>>& points) {
        int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        while (true) {
            points.push_back({x0, y0});
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }
}

namespace ChargeResolver {

    ChargeResult compute(int startX, int startY, int targetX, int targetY,
                         int agilityBonus, const Map& map) {
        ChargeResult result;

        // Compute max charge range
        int maxRange = agilityBonus * 3;
        if (maxRange <= 0) return result;

        // Generate Bresenham line from start to target
        std::vector<std::pair<int,int>> line;
        bresenhamLine(startX, startY, targetX, targetY, line);

        // Walk the line tile-by-tile, skipping the starting tile
        int tilesTraversed = 0;
        for (size_t i = 1; i < line.size(); ++i) {
            int x = line[i].first;
            int y = line[i].second;

            // If we've reached the target tile itself
            if (x == targetX && y == targetY) {
                // If start was already adjacent (only 1 tile traversed to reach target),
                // that means we're on the target — charge requires minimum distance of 2
                if (tilesTraversed == 0) {
                    // Start was adjacent to target — invalid charge
                    return result;
                }
                // We reached the target without finding an adjacent stop tile via Bresenham.
                // The last tile before this one should be our end position.
                // (This case is handled by the adjacency check below on previous iterations)
                return result;
            }

            // Check walkability
            if (!map.canWalk(x, y)) {
                // Path is blocked
                return result;
            }

            tilesTraversed++;

            // Check if we've exceeded max range
            if (tilesTraversed > maxRange) {
                return result;
            }

            // Check if this tile is adjacent to the target (Chebyshev distance == 1)
            int dx = std::abs(x - targetX);
            int dy = std::abs(y - targetY);
            if (dx <= 1 && dy <= 1) {
                // We've reached a tile adjacent to the target
                result.valid = true;
                result.endX = x;
                result.endY = y;
                result.tilesTraversed = tilesTraversed;
                return result;
            }
        }

        // If we exhausted the line without finding an adjacent tile, charge is invalid
        return result;
    }

}
