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
            {
                state.key.pressed = true;
                state.key.key = event.key.key;

                // Derive the printable character from the keycode + shift state.
                // SDL3 keycodes for printable keys correspond to the unshifted ASCII value.
                SDL_Keymod mod = SDL_GetModState();
                bool shifted = (mod & SDL_KMOD_SHIFT) != 0;

                SDL_Keycode k = event.key.key;
                char c = 0;

                if (k >= SDLK_A && k <= SDLK_Z) {
                    c = shifted ? static_cast<char>('A' + (k - SDLK_A))
                                : static_cast<char>('a' + (k - SDLK_A));
                } else if (k >= SDLK_0 && k <= SDLK_9) {
                    if (shifted) {
                        // US keyboard layout shifted digits
                        const char shiftedDigits[] = ")!@#$%^&*(";
                        c = shiftedDigits[k - SDLK_0];
                    } else {
                        c = static_cast<char>('0' + (k - SDLK_0));
                    }
                } else {
                    // Punctuation keys — map unshifted and shifted variants
                    switch (k) {
                        case SDLK_PERIOD:     c = shifted ? '>' : '.'; break;
                        case SDLK_COMMA:      c = shifted ? '<' : ','; break;
                        case SDLK_SLASH:      c = shifted ? '?' : '/'; break;
                        case SDLK_SEMICOLON:  c = shifted ? ':' : ';'; break;
                        case SDLK_APOSTROPHE: c = shifted ? '"' : '\''; break;
                        case SDLK_MINUS:      c = shifted ? '_' : '-'; break;
                        case SDLK_EQUALS:     c = shifted ? '+' : '='; break;
                        case SDLK_LEFTBRACKET:  c = shifted ? '{' : '['; break;
                        case SDLK_RIGHTBRACKET: c = shifted ? '}' : ']'; break;
                        case SDLK_BACKSLASH:  c = shifted ? '|' : '\\'; break;
                        case SDLK_GRAVE:      c = shifted ? '~' : '`'; break;
                        case SDLK_SPACE:      c = ' '; break;
                        default: c = 0; break;
                    }
                }
                state.key.c = c;
                break;
            }

            case SDL_EVENT_TEXT_INPUT:
                // TEXT_INPUT overrides with the actual OS-level character (handles
                // non-US layouts, dead keys, etc). Only use if we get one.
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
