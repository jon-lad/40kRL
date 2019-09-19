#include <list>
#include <memory>
#include <iterator>
#include "libtcod.h"
#include "Actor.h"
#include "Map.h"
#include "Engine.h"

Engine::Engine()
{
	TCODConsole::initRoot(80, 50, "Rougelike", false);

	actors.emplace_front(std::make_unique<Actor>(40, 20, '@', TCOD_white));
	player = &*actors.front();
	actors.emplace_back(std::make_unique<Actor>(16, 30, '@', TCOD_yellow));

	map = std::make_unique<Map>(80, 45);
}



void Engine::update()
{
	TCOD_key_t key;
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL);
	switch (key.vk)
	{
	case TCODK_UP:
		if (!map->isWall(player->getX(), player->getY() - 1))
		{
			player->setY(player->getY() - 1);
		} break;
	case TCODK_DOWN:
		if (!map->isWall(player->getX(), player->getY() + 1))
		{
			player->setY(player->getY() + 1);
		} break;
	case TCODK_LEFT:
		if (!map->isWall(player->getX() - 1, player->getY()))
		{
			player->setX(player->getX() - 1);
		}break;
	case TCODK_RIGHT:
		if (!map->isWall(player->getX() + 1, player->getY()))
		{
			player->setX(player->getX() + 1);
		}break;
	default: break;
	}
}

void Engine::render(){
	TCODConsole::root->clear();
	map->render();
	std::list<std::unique_ptr<Actor>>::iterator i;
	for (i = actors.begin(); i != actors.end(); ++i) {
		i->get()->render();
	}
	
}