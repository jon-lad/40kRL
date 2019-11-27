#pragma once
#include "libtcod.h"

namespace constants {

	//Gui Constants
	inline constexpr auto PANEL_HEIGHT{ 7 };// height of info panel
	inline constexpr auto BAR_WIDTH{ 20 };//width of healt and xp bars
	inline constexpr auto MSG_X{ BAR_WIDTH + 2 }; // x psition of log
	inline constexpr auto MSG_HEIGHT{ PANEL_HEIGHT - 2 };// log height
	inline constexpr auto PAUSE_MENU_WIDTH{ 30 }; //width menu when lvling up
	inline constexpr auto PAUSE_MENU_HEIGHT{ 15 }; // height of menu when leveling up

	//Map Constants
	inline constexpr auto ROOM_MAX_SIZE{ 12 };
	inline constexpr auto ROOM_MIN_SIZE{ 6 };

	inline constexpr auto MAX_ROOM_MONSTERS{ 3 };
	inline constexpr auto MAX_ROOM_ITEMS{ 2 };

	inline constexpr auto ground{ '.' };

	//Map Colors
	inline const TCODColor darkWall{ 0,0,100 };
	inline const TCODColor darkGround{ 50,50,150 };
	inline const TCODColor lightWall{ 130,110,50 };
	inline const TCODColor lightGround{ 200,180,50 };

	//Map Scroll Constatnts
	inline constexpr auto SCREEN_BORDER{ 10 };
	inline constexpr auto MAP_BORDER{ 10 };
}