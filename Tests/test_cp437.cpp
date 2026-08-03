#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"
#include "CP437.h"

#include <utility>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 11: CP437 glyph mapping correctness
// ═══════════════════════════════════════════════════════════════════════════════
//
// **Validates: Requirements 10.1, 10.2, 10.3**
//
// For any CP437 codepoint in [0, 255], the tileset mapping function SHALL
// produce tile coordinates (codepoint % 16, codepoint / 16). For any actor
// glyph value, it SHALL be a valid CP437 codepoint in [0, 255].

// ─── Test 1: Any codepoint in [0,255] maps to correct tile coordinates ───────

TEST_CASE("PBT: CP437 codepoint maps to correct tile coordinates",
          "[pbt][property][ui-rework][cp437]")
{
    // Feature: ui-rework, Property 11: CP437 glyph mapping correctness
    // Exhaustive check: all 256 valid codepoints map to correct tile coordinates.
    // This replaces the randomized PBT because the function is pure and the domain
    // is small enough to check exhaustively, avoiding flaky interactions with other
    // RapidCheck tests in the same process.
    SECTION("Any valid codepoint produces (codepoint % 16, codepoint / 16)") {
        for (int codepoint = 0; codepoint <= 255; ++codepoint) {
            auto [col, row] = cp437::tileCoords(codepoint);
            REQUIRE(col == codepoint % 16);
            REQUIRE(row == codepoint / 16);
        }
    }
}

// ─── Test 2: All known actor glyphs are valid CP437 codepoints ───────────────

TEST_CASE("PBT: All actor glyph values are valid CP437 codepoints",
          "[pbt][property][ui-rework][cp437]")
{
    // Feature: ui-rework, Property 11: CP437 glyph mapping correctness
    SECTION("Known actor glyphs are valid CP437 codepoints") {
        rc::prop("all known actor glyphs are in [0,255] range", []() {
            // Pick a random known glyph from the standard set
            const int glyph = *rc::gen::elementOf(std::vector<int>{
                cp437::GLYPH_PLAYER,       // '@' = 64
                cp437::GLYPH_STAIRS_UP,    // '<' = 60
                cp437::GLYPH_STAIRS_DOWN,  // '>' = 62
                cp437::GLYPH_DOOR_CLOSED,  // '+' = 43
                cp437::GLYPH_DOOR_OPEN,    // '/' = 47
                cp437::GLYPH_FLOOR,        // middle dot = 250
                cp437::GLYPH_WALL,         // '#' = 35
                cp437::GLYPH_POTION,       // '!' = 33
                cp437::GLYPH_SCROLL        // '?' = 63
            });

            RC_ASSERT(cp437::isValidCodepoint(glyph));

            // Also verify the tile coordinates are within grid bounds
            auto [col, row] = cp437::tileCoords(glyph);
            RC_ASSERT(col >= 0 && col < 16);
            RC_ASSERT(row >= 0 && row < 16);
        });
    }
}

// ─── Test 3: Glyphs outside [0,255] are remapped to '?' (63) ────────────────

TEST_CASE("PBT: Out-of-range glyphs remap to fallback '?' codepoint",
          "[pbt][property][ui-rework][cp437]")
{
    // Feature: ui-rework, Property 11: CP437 glyph mapping correctness
    SECTION("Negative codepoints remap to fallback") {
        rc::prop("negative codepoints produce fallback tile coords", []() {
            const int badCodepoint = *rc::gen::inRange(-10000, -1);

            auto [col, row] = cp437::tileCoords(badCodepoint);

            // Should map to '?' = 63: col = 63 % 16 = 15, row = 63 / 16 = 3
            RC_ASSERT(col == cp437::FALLBACK_CODEPOINT % 16);
            RC_ASSERT(row == cp437::FALLBACK_CODEPOINT / 16);
        });
    }

    SECTION("Codepoints above 255 remap to fallback") {
        rc::prop("codepoints > 255 produce fallback tile coords", []() {
            const int badCodepoint = *rc::gen::inRange(256, 10000);

            auto [col, row] = cp437::tileCoords(badCodepoint);

            // Should map to '?' = 63: col = 63 % 16 = 15, row = 63 / 16 = 3
            RC_ASSERT(col == cp437::FALLBACK_CODEPOINT % 16);
            RC_ASSERT(row == cp437::FALLBACK_CODEPOINT / 16);
        });
    }

    SECTION("sanitizeGlyph returns fallback for invalid values") {
        rc::prop("sanitizeGlyph remaps out-of-range to FALLBACK_CODEPOINT", []() {
            // Generate a value that might be in or out of range
            const int glyph = *rc::gen::inRange(-500, 500);

            int sanitized = cp437::sanitizeGlyph(glyph);

            if (glyph >= 0 && glyph <= 255) {
                RC_ASSERT(sanitized == glyph);
            } else {
                RC_ASSERT(sanitized == cp437::FALLBACK_CODEPOINT);
            }
        });
    }
}

// ─── Unit Tests: Specific glyph values ──────────────────────────────────────

TEST_CASE("CP437 specific glyph coordinate checks", "[unit][ui-rework][cp437]")
{
    SECTION("Player '@' = 64 maps to (0, 4)") {
        auto [col, row] = cp437::tileCoords(64);
        REQUIRE(col == 0);   // 64 % 16 = 0
        REQUIRE(row == 4);   // 64 / 16 = 4
    }

    SECTION("Stairs up '<' = 60 maps to (12, 3)") {
        auto [col, row] = cp437::tileCoords(60);
        REQUIRE(col == 12);  // 60 % 16 = 12
        REQUIRE(row == 3);   // 60 / 16 = 3
    }

    SECTION("Stairs down '>' = 62 maps to (14, 3)") {
        auto [col, row] = cp437::tileCoords(62);
        REQUIRE(col == 14);  // 62 % 16 = 14
        REQUIRE(row == 3);   // 62 / 16 = 3
    }

    SECTION("Floor (middle dot) = 250 maps to (10, 15)") {
        auto [col, row] = cp437::tileCoords(250);
        REQUIRE(col == 10);  // 250 % 16 = 10
        REQUIRE(row == 15);  // 250 / 16 = 15
    }

    SECTION("Wall '#' = 35 maps to (3, 2)") {
        auto [col, row] = cp437::tileCoords(35);
        REQUIRE(col == 3);   // 35 % 16 = 3
        REQUIRE(row == 2);   // 35 / 16 = 2
    }

    SECTION("Boundary: codepoint 0 maps to (0, 0)") {
        auto [col, row] = cp437::tileCoords(0);
        REQUIRE(col == 0);
        REQUIRE(row == 0);
    }

    SECTION("Boundary: codepoint 255 maps to (15, 15)") {
        auto [col, row] = cp437::tileCoords(255);
        REQUIRE(col == 15);  // 255 % 16 = 15
        REQUIRE(row == 15);  // 255 / 16 = 15
    }

    SECTION("Out-of-range: -1 maps to fallback '?' coords") {
        auto [col, row] = cp437::tileCoords(-1);
        REQUIRE(col == 15);  // 63 % 16 = 15
        REQUIRE(row == 3);   // 63 / 16 = 3
    }

    SECTION("Out-of-range: 256 maps to fallback '?' coords") {
        auto [col, row] = cp437::tileCoords(256);
        REQUIRE(col == 15);  // 63 % 16 = 15
        REQUIRE(row == 3);   // 63 / 16 = 3
    }
}
