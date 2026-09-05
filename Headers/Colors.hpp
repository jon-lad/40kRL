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
    constexpr TCODColor menuHighlightAlt{255, 191, 127}; // menu selected-item highlight (lighter orange)
    constexpr TCODColor surfaceMsg{63, 255, 63};     // surface/outdoor message (light green)
    constexpr TCODColor yellow{255, 255, 0};         // debug / system announcement
    constexpr TCODColor menuFrame{200, 180, 50};     // menu frame/title colour

    // ─── Map rendering ──────────────────────────────────────────────────
    constexpr TCODColor lightBlue{63, 63, 255};      // generic light blue (if needed)
    constexpr TCODColor orange{255, 127, 0};         // generic orange (if needed)
    constexpr TCODColor cyan{0, 255, 255};           // generic cyan (if needed)
    constexpr TCODColor brown{150, 100, 50};         // hab-unit / rusty metal tones
    constexpr TCODColor gold{255, 215, 0};           // chapel / ornamental trim

    // ─── Faction / NPC palette (bestiary-npcs) ──────────────────────────
    // Extra distinct hues so every NPC can carry a unique (glyph, color)
    // pair while staying faction-themed. See colorFromName below.
    constexpr TCODColor crimson{178, 34, 52};        // Tyranid / blood tones
    constexpr TCODColor purple{128, 0, 128};         // Warp / daemonic
    constexpr TCODColor metalGrey{160, 160, 170};    // Necron / servitor metal
    constexpr TCODColor lightCyan{128, 255, 255};    // Tau / energy
    constexpr TCODColor darkGreen{0, 100, 0};        // xenos flora / Kroot
    constexpr TCODColor pink{255, 105, 180};         // Harlequin / Dark Eldar accent
    constexpr TCODColor teal{0, 128, 128};           // Eldar wraithbone accent
    constexpr TCODColor tan{210, 180, 140};          // Kroot hide / bone
    constexpr TCODColor magenta{255, 0, 255};        // Slaanesh / warp accent
    constexpr TCODColor steelBlue{70, 130, 180};     // Eldar / Tau armour
    constexpr TCODColor boneWhite{227, 218, 190};    // Necron / bone
    constexpr TCODColor darkOrange{200, 90, 0};      // Ork specialist / fire

    // ─── Doors ──────────────────────────────────────────────────────────
    constexpr TCODColor doorClosed{150, 100, 50};    // closed door (brown/wood tone)
    constexpr TCODColor doorOpen{100, 65, 30};       // open door (darker brown)

    // ─── Action system ─────────────────────────────────────────────────
    constexpr TCODColor playerAction{180, 220, 255};  // player action log (light blue)
    constexpr TCODColor enemyAction{255, 180, 100};   // enemy action log (orange)
    constexpr TCODColor reactionEvent{200, 255, 200}; // reaction log (light green)
    constexpr TCODColor apFull{100, 255, 100};        // AP pip filled (bright green)
    constexpr TCODColor apEmpty{80, 80, 80};          // AP pip empty (dark grey)

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
    constexpr TCODColor lighterOrange    = menuHighlightAlt;
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
        if (name == "crimson")          return crimson;
        if (name == "purple")           return purple;
        if (name == "metalGrey")        return metalGrey;
        if (name == "lightCyan")        return lightCyan;
        if (name == "darkGreen")        return darkGreen;
        if (name == "pink")             return pink;
        if (name == "teal")             return teal;
        if (name == "tan")              return tan;
        if (name == "magenta")          return magenta;
        if (name == "steelBlue")        return steelBlue;
        if (name == "boneWhite")        return boneWhite;
        if (name == "darkOrange")       return darkOrange;
        return black;
    }
}
