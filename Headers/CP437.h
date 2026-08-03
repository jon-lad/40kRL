#pragma once

#include <utility>

// CP437 tileset mapping utilities.
// The tileset is a 16x16 grid of characters arranged by CP437 codepoint.
// Tile position for a given codepoint = (codepoint % 16, codepoint / 16).

namespace cp437 {

// Valid CP437 codepoint range
inline constexpr int MIN_CODEPOINT = 0;
inline constexpr int MAX_CODEPOINT = 255;
inline constexpr int FALLBACK_CODEPOINT = 63; // '?' character

// Known actor glyph codepoints (CP437 standard)
inline constexpr int GLYPH_PLAYER      = 64;  // '@'
inline constexpr int GLYPH_STAIRS_UP   = 60;  // '<'
inline constexpr int GLYPH_STAIRS_DOWN = 62;  // '>'
inline constexpr int GLYPH_DOOR_CLOSED = 43;  // '+'
inline constexpr int GLYPH_DOOR_OPEN   = 47;  // '/'
inline constexpr int GLYPH_FLOOR       = 46;  // '.' period
inline constexpr int GLYPH_WALL        = 35;  // '#'
inline constexpr int GLYPH_POTION      = 33;  // '!'
inline constexpr int GLYPH_SCROLL      = 63;  // '?'

// Returns true if the codepoint is a valid CP437 index [0, 255].
inline constexpr bool isValidCodepoint(int codepoint) {
    return codepoint >= MIN_CODEPOINT && codepoint <= MAX_CODEPOINT;
}

// Maps a CP437 codepoint to tile coordinates in the 16x16 grid.
// Returns (column, row) where column = codepoint % 16, row = codepoint / 16.
// If the codepoint is outside [0, 255], remaps to FALLBACK_CODEPOINT ('?').
inline constexpr std::pair<int, int> tileCoords(int codepoint) {
    if (!isValidCodepoint(codepoint)) {
        codepoint = FALLBACK_CODEPOINT;
    }
    return { codepoint % 16, codepoint / 16 };
}

// Sanitizes a glyph value: returns codepoint if valid, otherwise FALLBACK_CODEPOINT.
inline constexpr int sanitizeGlyph(int glyph) {
    return isValidCodepoint(glyph) ? glyph : FALLBACK_CODEPOINT;
}

} // namespace cp437
