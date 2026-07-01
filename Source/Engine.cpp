#include <list>
#include <memory>
#include <sstream>
#include "main.h"

static constexpr int DEFAULT_FOV_RADIUS   = 10;
static constexpr int MAP_WIDTH            = 160;
static constexpr int MAP_HEIGHT           = 86;
static constexpr int VIEWPORT_WIDTH       = 80;
static constexpr int VIEWPORT_HEIGHT      = 43;

Engine::Engine(int screenWidth, int screenHeight)
	: gameStatus{ STARTUP }
	, player{ nullptr }
	, stairs{ nullptr }
	, fovRadius{ DEFAULT_FOV_RADIUS }
	, screenWidth{ screenWidth }
	, screenHeight{ screenHeight }
	, dungeonLevel{ 1 }
{
	TCODConsole::initRoot(screenWidth, screenHeight, "40kRL", false);
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
		map->currentScentValue++;
		for (auto i = actors.begin(); i != actors.end(); ) {
			if (i->get() == nullptr) {
				i = actors.erase(i);
			} else if (i->get() != player) {
				i->get()->update();
				++i;
			} else {
				++i;
			}
		}
	}
}

void Engine::render()
{
	TCODConsole::root->clear();
	map->render();

	for (const auto& actorPtr : actors) {
		Actor* actor = actorPtr.get();
		const bool isVisible  = map->isInFOV(actor->getX(), actor->getY());
		const bool isExplored = !actor->fovOnly && map->isExplored(actor->getX(), actor->getY());
		if (isVisible || isExplored) {
			actor->render();
		}
	}

	gui->render();
}

void Engine::nextLevel()
{
	dungeonLevel++;
	gui->message(TCOD_light_violet, "You take a moment to rest and recover your strength.");
	player->destructible->heal(static_cast<int>(player->destructible->maxHp / 2));
	gui->message(TCOD_red, "After a rare moment of peace you descend deeper into the dungeon.");

	map.reset();

	// Remove all actors except the player and stairs.
	for (auto i = actors.begin(); i != actors.end(); ) {
		i = (i->get() != player && i->get() != stairs) ? actors.erase(i) : std::next(i);
	}

	map = std::make_unique<Map>(MAP_WIDTH, MAP_HEIGHT);
	map->init(true);
	camera->mapWidth  = map->getWidth();
	camera->mapHeight = map->getHeight();
	camera->update(player, map->getLevelType() == LevelType::OUTDOOR);
	gameStatus = STARTUP;
}

Actor* Engine::getClosestMonster(int x, int y, float range) const
{
	Actor* closest     = nullptr;
	float  bestDistance = 1e6f;

	for (const auto& actorPtr : actors) {
		Actor* actor = actorPtr.get();
		if (actor != player && actor->destructible && !actor->destructible->isDead()) {
			const float distance = actor->getDistance(x, y);
			if (distance < bestDistance && (range == 0.0f || distance <= range)) {
				bestDistance = distance;
				closest      = actor;
			}
		}
	}
	return closest;
}

Actor* Engine::getActorAt(int x, int y) const
{
	for (const auto& actorPtr : actors) {
		Actor* actor = actorPtr.get();
		if (actor->getX() == x && actor->getY() == y
			&& actor->destructible && !actor->destructible->isDead())
		{
			return actor;
		}
	}
	return nullptr;
}

bool Engine::pickAtTile(int* x, int* y, float maxRange)
{
	while (!TCODConsole::isWindowClosed()) {
		render();

		// Brighten all in-range, in-FOV tiles to indicate they are selectable.
		for (int cx = 0; cx < map->getWidth(); cx++) {
			for (int cy = 0; cy < map->getHeight(); cy++) {
				if (map->isInFOV(cx, cy) && (maxRange == 0.0f || player->getDistance(cx, cy) <= maxRange)) {
					auto [screenX, screenY] = camera->apply(cx, cy);
					TCODColor col = TCODConsole::root->getCharForeground(screenX, screenY);
					TCODConsole::root->setCharForeground(screenX, screenY, col * 1.2f);
				}
			}
		}

		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS | TCOD_EVENT_MOUSE, &lastKey, &mouse);
		auto [worldX, worldY] = camera->getWorldLocation(mouse.cx, mouse.cy);

		if (map->isInFOV(worldX, worldY)
			&& (maxRange == 0.0f || player->getDistance(worldX, worldY) <= maxRange))
		{
			TCODConsole::root->setCharBackground(mouse.cx, mouse.cy, TCOD_white);
			if (mouse.lbutton_pressed) {
				*x = worldX;
				*y = worldY;
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

void Engine::sendToBack(Actor* actor)
{
	for (auto i = actors.begin(); i != actors.end(); ++i) {
		if (i->get() == actor) {
			actors.splice(actors.begin(), actors, i);
			return;
		}
	}
}

void Engine::init()
{
	// Create the player.
	auto newPlayer = std::make_unique<Actor>(0, 0, '@', "Player", TCOD_white);
	player = newPlayer.get();
	newPlayer->destructible = std::make_unique<PlayerDestructible>(30.0f, 2.0f, "Your cadaver", 0);
	newPlayer->attacker     = std::make_unique<Attacker>(5.0f);
	newPlayer->ai           = std::make_unique<PlayerAi>();
	newPlayer->container    = std::make_unique<Container>(26);
	actors.emplace_front(std::move(newPlayer));

	// Create the stairs (always visible, never blocks).
	auto newStairs = std::make_unique<Actor>(0, 0, '>', "stairs", TCOD_white);
	stairs = newStairs.get();
	newStairs->blocks  = false;
	newStairs->fovOnly = false;
	actors.emplace_front(std::move(newStairs));

	map = std::make_unique<Map>(MAP_WIDTH, MAP_HEIGHT);
	map->init(true);
	camera = std::make_unique<Camera>(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
		map->getWidth(), map->getHeight());
	camera->update(player, false);

	gui->message(TCODColor::lightGrey, "\n \n \n Hello friend. \n Welcome to the underhive!");
	gameStatus = STARTUP;
}

void Engine::term()
{
	actors.clear();
	if (map) { map.reset(); }
	gui->clear();
}
