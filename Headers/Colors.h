#pragma once
// 40kRL game colour palette.
// Game-themed names for all colours used in rendering. Values currently match
// the libtcod 1.x definitions; a future revision could align these with
// Games Workshop / Citadel paint names for flavour.

#include "libtcod.hpp"
#include <string>

namespace Colors {
    // ─── UI / system ────────────────────────────────────────────────────
    constexpr TCODColor white{255, 255, 255};
    constexpr TCODColor black{0, 0, 0};
    constexpr TCODColor uiText{191, 191, 191};       // default HUD text
    constexpr TCODColor uiHighlight{255, 255, 63};   // highlighted menu items

    // ─── Actors ─────────────────────────────────────────────────────────
    constexpr TCODColor orkSkin{63, 127, 63};        // Ork body (desaturated green)
    constexpr TCODColor nobArmour{0, 63, 0};         // Nob (darker green)
    constexpr TCODColor playerGlyph{255, 255, 255};  // player '@' colour

    // ─── Items ──────────────────────────────────────────────────────────
    constexpr TCODColor healthPotion{127, 0, 255};   // potion '!' (violet)
    constexpr TCODColor scroll{255, 255, 63};        // scroll '#' (light yellow)
    constexpr TCODColor stairsGlyph{255, 255, 255};  // stairs '<' / '>'

    // ─── Effects / messages ─────────────────────────────────────────────
    constexpr TCODColor healing{159, 63, 255};       // heal message (light violet)
    constexpr TCODColor damage{255, 0, 0};           // damage / danger (red)
    constexpr TCODColor damageLight{255, 63, 63};    // light damage flash
    constexpr TCODColor damageDark{127, 0, 0};       // dark damage backdrop
    constexpr TCODColor darkRed{95, 0, 0};           // corpse tint (dark red)
    constexpr TCODColor confusion{63, 0, 127};       // confusion effect (darker violet)
    constexpr TCODColor levelUp{255, 191, 127};      // level-up message (lighter orange)
    constexpr TCODColor surfaceMsg{63, 255, 63};     // surface/outdoor message (light green)
    constexpr TCODColor yellow{255, 255, 0};         // level-up announcement
    constexpr TCODColor menuFrame{200, 180, 50};     // menu frame/title colour

    // ─── Map rendering ──────────────────────────────────────────────────
    constexpr TCODColor lightBlue{63, 63, 255};      // generic light blue (if needed)
    constexpr TCODColor orange{255, 127, 0};         // generic orange (if needed)
    constexpr TCODColor cyan{0, 255, 255};           // generic cyan (if needed)
    constexpr TCODColor brown{150, 100, 50};         // hab-unit / rusty metal tones
    constexpr TCODColor gold{255, 215, 0};           // chapel / ornamental trim

    // ─── Legacy aliases (for incremental migration; remove once all
    //     call sites use game-themed names above) ────────────────────────
    constexpr TCODColor desaturatedGreen = orkSkin;
    constexpr TCODColor darkerGreen      = nobArmour;
    constexpr TCODColor lightGrey        = uiText;
    constexpr TCODColor red              = damage;
    constexpr TCODColor violet           = healthPotion;
    constexpr TCODColor lightYellow      = scroll;
    constexpr TCODColor lightViolet      = healing;
    constexpr TCODColor lightRed         = damageLight;
    constexpr TCODColor darkerRed        = damageDark;
    constexpr TCODColor lighterOrange    = levelUp;
    constexpr TCODColor darkerViolet     = confusion;
    constexpr TCODColor lightGreen       = surfaceMsg;

    // ─── Shared colour-name resolver ────────────────────────────────────
    // Maps Lua colour-name strings to TCODColor values.
    // Returns black for unrecognised names (used as sentinel for invalid colours).
    inline TCODColor colorFromName(const std::string& name)
    {
        if (name == "white")            return white;
        if (name == "desaturatedGreen") return orkSkin;
        if (name == "darkerGreen")      return nobArmour;
        if (name == "lightBlue")        return lightBlue;
        if (name == "orange")           return orange;
        if (name == "lightGreen")       return surfaceMsg;
        if (name == "violet")           return healthPotion;
        if (name == "lightYellow")      return scroll;
        if (name == "lightGrey")        return lightGrey;
        if (name == "lighterOrange")    return lighterOrange;
        if (name == "darkGrey")         return TCODColor{95, 95, 95};
        if (name == "red")              return damage;
        if (name == "darkRed")          return darkRed;
        if (name == "cyan")             return cyan;
        if (name == "brown")            return brown;
        if (name == "gold")             return gold;
        return black;
    }
}
