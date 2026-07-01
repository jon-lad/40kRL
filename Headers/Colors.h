#pragma once
// Project color palette — constexpr replacements for deprecated TCODColor named
// constants and TCOD_* C-style color macros removed in libtcod 2.x.
// Values match the libtcod 1.x colour definitions exactly.

#include "libtcod.hpp"

namespace Colors {
    // TCODColor::* named static member replacements
    constexpr TCODColor desaturatedGreen{63, 127, 63};
    constexpr TCODColor darkerGreen{0, 63, 0};
    constexpr TCODColor lightBlue{63, 63, 255};
    constexpr TCODColor orange{255, 127, 0};
    constexpr TCODColor lightGreen{63, 255, 63};
    constexpr TCODColor lightGrey{191, 191, 191};
    constexpr TCODColor red{255, 0, 0};
    constexpr TCODColor black{0, 0, 0};
    constexpr TCODColor white{255, 255, 255};

    // TCOD_* C-style color macro replacements
    constexpr TCODColor violet{127, 0, 255};
    constexpr TCODColor lightYellow{255, 255, 63};
    constexpr TCODColor lightViolet{159, 63, 255};
    constexpr TCODColor lightRed{255, 63, 63};
    constexpr TCODColor darkerRed{127, 0, 0};
    constexpr TCODColor lighterOrange{255, 191, 127};
    constexpr TCODColor darkerViolet{63, 0, 127};
    constexpr TCODColor cyan{0, 255, 255};
}
