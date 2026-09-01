#pragma once

class Map;

struct ChargeResult {
    bool valid = false;
    int endX = 0, endY = 0;  // tile to stop on (adjacent to target)
    int tilesTraversed = 0;
};

namespace ChargeResolver {
    // Computes whether a charge from (startX,startY) to a tile adjacent to
    // (targetX,targetY) is valid. Checks:
    //   1. Distance <= agilityBonus * 3
    //   2. Straight-line path (Bresenham) is unobstructed
    //   3. Final tile is walkable and adjacent to target
    // Returns a ChargeResult with valid=true and the end position if successful.
    ChargeResult compute(int startX, int startY, int targetX, int targetY,
                         int agilityBonus, const Map& map);
}
