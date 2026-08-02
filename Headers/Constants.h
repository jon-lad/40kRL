#pragma once

namespace constants {
	// HUD panel at the bottom of the screen
	inline static constexpr auto PANEL_HEIGHT     = 7;
	inline static constexpr auto BAR_WIDTH        = 20;
	inline static constexpr auto MSG_X            = BAR_WIDTH + 2; // message log starts after the bars
	inline static constexpr auto MSG_HEIGHT       = PANEL_HEIGHT - 2;

	// Pause menu overlay
	inline static constexpr auto PAUSE_MENU_WIDTH  = 30;
	inline static constexpr auto PAUSE_MENU_HEIGHT = 15;
}

// ─── New screen layout constants (ui-rework) ─────────────────────────────────
namespace layout {
	inline constexpr int SCREEN_WIDTH         = 120;
	inline constexpr int SCREEN_HEIGHT        = 50;
	inline constexpr int RIGHT_SIDEBAR_WIDTH  = 24;
	inline constexpr int LEFT_SIDEBAR_WIDTH   = 20;
	inline constexpr bool LEFT_SIDEBAR_ENABLED = false;
	inline constexpr int HUD_HEIGHT           = 8;
	inline constexpr int MSG_LOG_HEIGHT       = 6;
	inline constexpr int SKILL_BAR_HEIGHT     = 1;

	// Derived
	inline constexpr int VIEWPORT_X      = LEFT_SIDEBAR_ENABLED ? LEFT_SIDEBAR_WIDTH : 0;
	inline constexpr int VIEWPORT_WIDTH  = SCREEN_WIDTH - RIGHT_SIDEBAR_WIDTH
	                                       - (LEFT_SIDEBAR_ENABLED ? LEFT_SIDEBAR_WIDTH : 0);
	inline constexpr int VIEWPORT_HEIGHT = SCREEN_HEIGHT - HUD_HEIGHT;

	// Tileset dimensions (CP437 16x16 grid)
	inline constexpr int TILESET_CHAR_WIDTH  = 16;
	inline constexpr int TILESET_CHAR_HEIGHT = 16;
}