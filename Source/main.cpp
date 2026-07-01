
#include <memory>
#include <list>
#include <sstream>
#include "main.h"

Engine engine(80, 50);

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
