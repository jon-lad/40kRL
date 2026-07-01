#include "InputHandler.h"

void pollInput(InputState& state, int cellWidth, int cellHeight) {
    // Reset per-frame flags so callers see only this frame's events.
    state.key.pressed = false;
    state.key.key = SDLK_UNKNOWN;
    state.key.c = 0;
    state.mouse.lbutton_pressed = false;
    state.mouse.rbutton_pressed = false;
    state.windowClosed = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                state.windowClosed = true;
                break;

            case SDL_EVENT_KEY_DOWN:
                state.key.pressed = true;
                state.key.key = event.key.key;
                // Derive a printable character from the keycode if it maps
                // to a basic ASCII character (a-z, 0-9, punctuation).
                if (event.key.key >= SDLK_SPACE && event.key.key <= SDLK_z) {
                    state.key.c = static_cast<char>(event.key.key & 0xFF);
                } else {
                    state.key.c = 0;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                state.mouse.pixelX = static_cast<int>(event.motion.x);
                state.mouse.pixelY = static_cast<int>(event.motion.y);
                if (cellWidth > 0 && cellHeight > 0) {
                    state.mouse.cellX = state.mouse.pixelX / cellWidth;
                    state.mouse.cellY = state.mouse.pixelY / cellHeight;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    state.mouse.lbutton_pressed = true;
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    state.mouse.rbutton_pressed = true;
                }
                // Also update pixel/cell position from button event coords.
                state.mouse.pixelX = static_cast<int>(event.button.x);
                state.mouse.pixelY = static_cast<int>(event.button.y);
                if (cellWidth > 0 && cellHeight > 0) {
                    state.mouse.cellX = state.mouse.pixelX / cellWidth;
                    state.mouse.cellY = state.mouse.pixelY / cellHeight;
                }
                break;

            default:
                break;
        }
    }
}
