#include <memory>
#include <list>
#include "libtcod.h"
#include "Actor.h"
#include "Map.h"
#include "Engine.h"


int main()
{
	std::unique_ptr<Engine> engine = std::make_unique<Engine>();
	while (!TCODConsole::isWindowClosed()) {
		engine->update();
		engine->render();
		TCODConsole::flush();
	}
}