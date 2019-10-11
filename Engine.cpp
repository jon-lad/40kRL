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

Actor* Engine::getClosestMonster(int x, int y, float range) {
	Actor* closest = NULL;
	float bestDistance = 1E6f;
	for (auto i = actors.begin(); i != actors.end(); ++i) {
		if (i->get() != player && i->get()->destructible && !i->get()->destructible->isDead()) {
			float distance = i->get()->getDistance(x, y);
			if (distance < bestDistance && (distance <= range || range == 0.0f)) {
				bestDistance = distance;
				closest = i->get();
			}
		}
	}
	return closest;
}

Actor* Engine::getActor(int x, int y)const {
	for (auto i = actors.begin(); i != actors.end(); ++i)
	{
		if (i->get()->getX()== x && i->get()->getY()== y && i->get()->destructible && !i->get()->destructible->isDead()) {
			return i->get();
		}
	}
	return NULL;
}

bool Engine::pickAtTile(int* x, int* y, float maxRange) {
	while (!TCODConsole::isWindowClosed()) {
		render();
		for (int cx = 0; cx < map->getWidth(); cx++) {
			for (int cy = 0; cy < map->getHeight(); cy++){
				if (map->isInFOV(cx, cy) && (maxRange == 0 || player->getDistance(cx, cy) <= maxRange)) {
					TCODColor col = TCODConsole::root->getCharBackground(cx, cy);
					col = col*1.2f;
					TCODConsole::root->setCharBackground(cx, cy, col);
				}
			}
		}
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);
		if (map->isInFOV(mouse.cx, mouse.cy) && (maxRange==0 || player->getDistance(mouse.cx,mouse.cy)<= maxRange)) {
			TCODConsole::root->setCharBackground(mouse.cx, mouse.cy, TCOD_white);
			if (mouse.lbutton_pressed) {
				*x = mouse.cx;
				*y = mouse.cy;
				return true;
			}
			if (mouse.rbutton_pressed || lastKey.vk != TCODK_NONE) {
				return false;
			}
		}
		TCODConsole::flush();
	}
	return false;
}

void Engine::sendToBack(Actor* actor) {
	auto i = actors.begin(); 
	for (i; i != actors.end(); ++i) {
	
		if ( actor == i->get() ) {
			actors.splice(actors.begin(), actors, i);
			
			
		}
	}
}