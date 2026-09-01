#pragma once

// CP437 codepoint constants for box-drawing and symbol characters.
// With TCOD_FONT_LAYOUT_CP437, we use the raw CP437 index directly.
// Values in range [0, 255] map to tile position (codepoint % 16, codepoint / 16).
namespace CharConst {
    // Double box-drawing characters (CP437 codepoints 179-218)
    constexpr int DCROSS = 206;          // ╬ Double cross
    constexpr int DTEES = 203;           // ╦ Double tee south
    constexpr int DTEEN = 202;           // ╩ Double tee north
    constexpr int DTEEE = 204;           // ╠ Double tee east
    constexpr int DTEEW = 185;           // ╣ Double tee west
    constexpr int DNE = 200;             // ╚ Double corner north-east
    constexpr int DNW = 188;             // ╝ Double corner north-west
    constexpr int DSE = 187;             // ╗ Double corner south-east
    constexpr int DSW = 201;             // ╔ Double corner south-west
    constexpr int DVLINE = 186;          // ║ Double vertical line
    constexpr int DHLINE = 205;          // ═ Double horizontal line

    // Miscellaneous CP437 symbols
    constexpr int RADIO_UNSET = 9;       // ○ Radio button unset (CP437 codepoint 9)
    constexpr int SPADE = 6;             // ♠ Spade suit (CP437 codepoint 6)
}
