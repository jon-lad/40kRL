#include <list>
#include <memory>
#include "main.h"

Engine::Engine(int screenWidth, int screenHeight) :gameStatus{ STARTUP }, player{ NULL }, FOVRadius{ 10 }, screenWidth{ screenWidth }, screenHeight{ screenHeight }, level{1}
{
	map.reset();
	TCODConsole::initRoot(screenWidth, screenHeight, "Rougelike", false);
	gui = std::make_unique<Gui>();
}

void Engine::update()
{
	if (gameStatus == STARTUP) { map->computeFOV(); }
	gameStatus = IDLE;
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);
	if (lastKey.vk == TCODK_ESCAPE) {
		save();
		load();
	}
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
		if (!i->get()->fovOnly && map->isExplored(i->get()->getX(), i->get()->getY() )||map->isInFOV(i->get()->getX(), i->get()->getY())) {
			i->get()->render();
		}
	}
	// show the player's stats
	gui->render();
}

void Engine::nextLevel() {
	level++;
	gui->message(TCOD_light_violet, "You take a moment to rest and recover your strength");
	player->destructible->heal((int)(player->destructible->maxHp / 2));
	gui->message(TCOD_red, " After a rare moment of peace you descend\n deeper into the dungeon");
	map.reset();
	for (auto i = actors.begin(); i != actors.end(); ) {
		if (i->get() != player && i->get() != stairs) {
			i = actors.erase(i);
		}
		else { ++i; }
	}
	//create a new map
	map = std::make_unique<Map>(80, 43);
	map->init(true);
	gameStatus = STARTUP;
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
			if (mouse.rbutton_pressed || lastKey.c != TCODK_NONE) {
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

void Engine::init() {
	std::unique_ptr<Actor>newPlayer = std::make_unique<Actor>(0, 0, '@', "Player", TCOD_white);
	player = newPlayer.get();
	newPlayer->destructible = std::make_unique<PlayerDestructible>(30.0f, 2.0f, "Your cadaver", 0);
	newPlayer->attacker = std::make_unique<Attacker>(5.0f);
	newPlayer->ai = std::make_unique<PlayerAi>();
	newPlayer->container = std::make_unique<Container>(26);
	actors.emplace_front(std::move(newPlayer));
	std::unique_ptr<Actor>newStair = std::make_unique<Actor>(0, 0, '>', "stairs", TCOD_white);
	stairs = newStair.get();
	newStair->blocks = false;
	newStair->fovOnly = false;
	actors.emplace_front(std::move(newStair));
	
	
	map = std::make_unique<Map>(80, 43);
	map->init(true);
	
	gui->message(TCODColor::lightGrey, "\n \n \n Hello friend. \n welcome to the underhive!");
	gameStatus = STARTUP;
}

void Engine::term() {
	actors.clear();
	if (map) map.reset();
	gui->clear();
}

