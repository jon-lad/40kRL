
#include <memory>
#include <list>
#include <sstream>
#include "main.hpp"

Engine engine(layout::SCREEN_WIDTH, layout::SCREEN_HEIGHT);

int main()
{
	engine.load(); // show main menu; starts a new game or restores a save

	while (!TCODConsole::isWindowClosed()) {
		engine.update();
		engine.render();
		TCODConsole::flush();
	}

	engine.save(); // persist state on clean window-close
	TCOD_quit();
	return 0;
}
