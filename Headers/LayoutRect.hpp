#pragma once

// A simple axis-aligned rectangle used for screen layout geometry.
struct LayoutRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    // Returns true if this rectangle overlaps with another (exclusive boundaries).
    bool intersects(const LayoutRect& other) const {
        return x < other.x + other.width && x + width > other.x
            && y < other.y + other.height && y + height > other.y;
    }
};
