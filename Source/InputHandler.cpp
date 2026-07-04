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
                // key.c will be populated by TEXT_INPUT event if available.
                // For non-text keys, derive basic ASCII from keycode.
                if (event.key.key >= SDLK_A && event.key.key <= SDLK_Z) {
                    // Lowercase letter (shift handling comes from TEXT_INPUT)
                    state.key.c = static_cast<char>(event.key.key - SDLK_A + 'a');
                } else if (event.key.key >= SDLK_0 && event.key.key <= SDLK_9) {
                    state.key.c = static_cast<char>(event.key.key - SDLK_0 + '0');
                } else {
                    state.key.c = 0;
                }
                break;

            case SDL_EVENT_TEXT_INPUT:
                // TEXT_INPUT gives us the actual typed character including shift.
                // This correctly maps Shift+. to '>' and Shift+, to '<'.
                if (event.text.text[0] != 0) {
                    state.key.c = event.text.text[0];
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
