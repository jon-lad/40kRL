#pragma once

#include "libtcod.hpp"

// Inline rendering helpers wrapping TCOD_console_put_rgb (libtcod 2.x API).
// These replace the deprecated per-cell TCODConsole methods (setChar,
// setCharForeground, setCharBackground) with the modern C function.

/// Sets glyph and foreground colour at (x, y). Background is unchanged.
inline void renderPutChar(TCOD_Console* con, int x, int y, int ch, TCOD_ColorRGB fg)
{
    TCOD_console_put_rgb(con, x, y, ch, &fg, nullptr, TCOD_BKGND_NONE);
}

/// Sets glyph, foreground, and background colour at (x, y).
inline void renderPutCharBg(TCOD_Console* con, int x, int y, int ch, TCOD_ColorRGB fg, TCOD_ColorRGB bg)
{
    TCOD_console_put_rgb(con, x, y, ch, &fg, &bg, TCOD_BKGND_SET);
}

/// Sets only the background colour at (x, y). Existing character and foreground are preserved.
inline void renderSetBg(TCOD_Console* con, int x, int y, TCOD_ColorRGB bg)
{
    TCOD_console_put_rgb(con, x, y, 0, nullptr, &bg, TCOD_BKGND_SET);
}
