#pragma once

#include <SDL3/SDL.h>

// Default cell dimensions for pixel-to-tile conversion.
// Update these if the font/tile size changes.
constexpr int DEFAULT_CELL_WIDTH = 10;
constexpr int DEFAULT_CELL_HEIGHT = 10;

// Replaces TCOD_key_t — stores the key event state for one frame.
struct KeyState {
    SDL_Keycode key = SDLK_UNKNOWN;  // SDL3 keycode (e.g. SDLK_ESCAPE, SDLK_UP)
    char c = 0;                       // text character if printable
    bool pressed = false;             // was a key pressed this frame
};

// Replaces TCOD_mouse_t — stores the mouse state for one frame.
struct MouseState {
    int cellX = 0, cellY = 0;        // tile coordinates (pixel / cell size)
    int pixelX = 0, pixelY = 0;      // raw pixel coordinates
    bool lbutton_pressed = false;     // left button pressed this frame
    bool rbutton_pressed = false;     // right button pressed this frame
};

// Combined input state replacing TCOD_key_t + TCOD_mouse_t.
struct InputState {
    KeyState key;
    MouseState mouse;
    bool windowClosed = false;        // SDL_EVENT_QUIT received
};

// Drains the SDL3 event queue and populates the given InputState.
// Call once per frame before reading input. Resets per-frame flags
// (key.pressed, lbutton_pressed, rbutton_pressed) at the start of each call.
//
// cellWidth/cellHeight control pixel-to-tile conversion. Pass the font cell
// dimensions used by the console (default 10x10).
void pollInput(InputState& state, int cellWidth = DEFAULT_CELL_WIDTH, int cellHeight = DEFAULT_CELL_HEIGHT);
