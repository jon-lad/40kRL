#include <list>
#include <memory>
#include "main.h"

Engine::Engine(int screenWidth, int screenHeight) :gameStatus{ STARTUP }, FOVRadius{ 10 }, screenWidth{ screenWidth }, screenHeight{screenHeight}
{
	TCODConsole::initRoot(screenWidth, screenHeight, "Rougelike", false);
	actors.emplace_front(std::make_unique<Actor>(40, 20, '@',"Player", TCOD_white));
	player = &*actors.front();
	player->destructible = std::make_unique<Destructible>(30.0f, 2.0f, "Your cadaver");
	player->attacker = std::make_unique<Attacker>(5.0f);
	player->ai = std::make_unique<PlayerAi>();
	player->container = std::make_unique<Container>(26);
	map = std::make_unique<Map>(80, 43);
	gui = std::make_unique<Gui>();
	gui->message(TCODColor::lightGrey, "");
	gui->message(TCODColor::lightGrey, "");
	gui->message(TCODColor::lightGrey, "");
	gui->message(TCODColor::lightGrey, "");
	gui->message(TCODColor::lightGrey, "Welcome friend");
}

void Engine::update()
{
	if (gameStatus == STARTUP) { map->computeFOV(); }
	gameStatus = IDLE;
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);
	player->update();
	if (gameStatus == NEW_TURN) {
		for (std::list<std::unique_ptr<Actor>>::iterator i = actors.begin(); i != actors.end(); ) {
			if (i->get() == NULL) {
				i = actors.erase(i);
			}
			else if (i->get() != player) {
				i->get()->update();
				++i;
			}
			else { ++i; }
		}
	}
}

	


void Engine::render(){
	TCODConsole::root->clear();
	map->render();
	std::list<std::unique_ptr<Actor>>::iterator i;
	for (i = actors.begin(); i != actors.end(); ++i) {
		if (map->isInFOV(i->get()->getX(), i->get()->getY())) {
			i->get()->render();
		}
	}
	// show the player's stats
	gui->render();
}

void Engine::sendToBack(Actor* actor) {
	std::list<std::unique_ptr<Actor>>::iterator i = actors.begin(); 
	for (i; i != actors.end(); ++i) {
	
		if ( actor == i->get() ) {
			actors.splice(actors.begin(), actors, i);
			
			
		}
	}
}