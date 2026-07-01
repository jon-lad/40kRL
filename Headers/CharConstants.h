#pragma once

// Unicode codepoint replacements for removed TCOD_CHAR_* constants (libtcod 1.16+).
// Values correspond to Code Page 437 / Unicode Box Drawing block characters.
namespace CharConst {
    constexpr int DCROSS = 0x256C;       // ╬ Double cross
    constexpr int DTEES = 0x2566;        // ╦ Double tee south
    constexpr int DTEEN = 0x2569;        // ╩ Double tee north
    constexpr int DTEEE = 0x2560;        // ╠ Double tee east
    constexpr int DTEEW = 0x2563;        // ╣ Double tee west
    constexpr int DNE = 0x255A;          // ╚ Double corner north-east
    constexpr int DNW = 0x255D;          // ╝ Double corner north-west
    constexpr int DSE = 0x2557;          // ╗ Double corner south-east
    constexpr int DSW = 0x2554;          // ╔ Double corner south-west
    constexpr int DVLINE = 0x2551;       // ║ Double vertical line
    constexpr int DHLINE = 0x2550;       // ═ Double horizontal line
    constexpr int RADIO_UNSET = 0x25CB;  // ○ Radio button unset
    constexpr int SPADE = 0x2660;        // ♠ Spade suit
}
