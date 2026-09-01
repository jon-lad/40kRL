#pragma once

#include <string>
#include <vector>

// Represents a single entry in the skill shortcut bar at the bottom of the HUD.
// Each frame, the player's active combat skills are queried and a vector of
// SkillBarEntry is built for rendering.
struct SkillBarEntry {
    std::string name;
    char keybinding;       // '1' through '9'
    bool available;        // false if on cooldown or prerequisites unmet
};

// Pure color-selection helper for the skill bar. Returns a dimmed color
// when the skill is unavailable, normal color otherwise.
// This function is defined inline so tests can exercise it without libtcod.
namespace SkillBarColors {
    // These values match Colors::white and the dimmed grey from the design doc.
    // The actual render path uses TCODColor directly, but these constants serve
    // as the canonical logic definition.
    inline constexpr int NORMAL_R = 255, NORMAL_G = 255, NORMAL_B = 255;
    inline constexpr int DIMMED_R = 128, DIMMED_G = 128, DIMMED_B = 128;
}
