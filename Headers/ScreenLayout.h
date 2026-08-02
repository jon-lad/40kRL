#pragma once

#include "LayoutRect.h"
#include "Constants.h"

// Precomputed screen panel rectangles derived from layout constants.
// All values are in character-cell coordinates.
struct ScreenLayout {
    LayoutRect viewport;       // Central map viewport
    LayoutRect rightSidebar;   // Fixed right sidebar
    LayoutRect leftSidebar;    // Optional left sidebar (only valid when enabled)
    LayoutRect messageLog;     // Message log panel in HUD area
    LayoutRect skillBar;       // Skill shortcut bar below message log
    LayoutRect hudBars;        // HP/XP bars in bottom-left HUD area

    // Compute the default screen layout from the layout namespace constants.
    static ScreenLayout compute() {
        ScreenLayout sl{};

        sl.viewport = {
            layout::VIEWPORT_X,
            0,
            layout::VIEWPORT_WIDTH,
            layout::VIEWPORT_HEIGHT
        };

        sl.rightSidebar = {
            layout::SCREEN_WIDTH - layout::RIGHT_SIDEBAR_WIDTH,
            0,
            layout::RIGHT_SIDEBAR_WIDTH,
            layout::SCREEN_HEIGHT
        };

        sl.leftSidebar = {
            0,
            0,
            layout::LEFT_SIDEBAR_WIDTH,
            layout::SCREEN_HEIGHT
        };

        sl.messageLog = {
            layout::VIEWPORT_X,
            layout::VIEWPORT_HEIGHT,
            layout::VIEWPORT_WIDTH,
            layout::MSG_LOG_HEIGHT
        };

        sl.skillBar = {
            layout::VIEWPORT_X,
            layout::VIEWPORT_HEIGHT + layout::MSG_LOG_HEIGHT + 1,
            layout::VIEWPORT_WIDTH,
            layout::SKILL_BAR_HEIGHT
        };

        sl.hudBars = {
            0,
            layout::VIEWPORT_HEIGHT,
            layout::VIEWPORT_X > 0 ? layout::VIEWPORT_X : constants::BAR_WIDTH,
            layout::HUD_HEIGHT
        };

        return sl;
    }
};
