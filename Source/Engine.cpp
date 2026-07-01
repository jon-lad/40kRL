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
	, dungeonLevel{ 20 }
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

void Engine::nextLevel(int direction)
{
	dungeonLevel += direction;

	gui->message(TCOD_light_violet, "You take a moment to rest and recover your strength.");
	player->destructible->heal(static_cast<int>(player->destructible->maxHp / 2));

	if (dungeonLevel == 0) {
		gui->message(TCOD_light_green, "You emerge from the depths onto the planet surface.");
	} else if (direction < 0) {
		gui->message(TCOD_red, "You ascend closer to the surface.");
	} else {
		gui->message(TCOD_red, "You descend deeper underground.");
	}

	const bool isOutdoor = (dungeonLevel == 0);

	map.reset();

	// Remove all actors except the player and stairs.
	for (auto i = actors.begin(); i != actors.end(); ) {
		if (i->get() != player && i->get() != stairsUp && i->get() != stairsDown) {
			i = actors.erase(i);
		} else {
			++i;
		}
	}

	map = std::make_unique<Map>(MAP_WIDTH, MAP_HEIGHT);
	map->init(true, isOutdoor ? LevelType::OUTDOOR : LevelType::BSP);

	// The stairs you came from should be at the player's starting position.
	// If you ascended (direction -1), the down-stairs align with the player.
	// If you descended (direction +1), the up-stairs align with the player.
	if (direction < 0) {
		stairsDown->setX(player->getX());
		stairsDown->setY(player->getY());
	} else {
		stairsUp->setX(player->getX());
		stairsUp->setY(player->getY());
	}

	// Hide stairsUp on surface (depth 0 — can't go higher) by moving off-map.
	if (isOutdoor) {
		stairsUp->setX(-1);
		stairsUp->setY(-1);
	}

	camera->mapWidth  = map->getWidth();
	camera->mapHeight = map->getHeight();
	camera->update(player, isOutdoor);
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

	// Create up-stairs (ascend toward surface). Always visible, never blocks.
	auto newStairsUp = std::make_unique<Actor>(0, 0, '<', "stairs up", TCOD_white);
	stairsUp = newStairsUp.get();
	newStairsUp->blocks  = false;
	newStairsUp->fovOnly = false;
	actors.emplace_front(std::move(newStairsUp));

	// Create down-stairs (descend deeper). Always visible, never blocks.
	auto newStairsDown = std::make_unique<Actor>(0, 0, '>', "stairs down", TCOD_white);
	stairsDown = newStairsDown.get();
	newStairsDown->blocks  = false;
	newStairsDown->fovOnly = false;
	actors.emplace_front(std::move(newStairsDown));

	map = std::make_unique<Map>(MAP_WIDTH, MAP_HEIGHT);
	map->init(true, LevelType::BSP);
	camera = std::make_unique<Camera>(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
		map->getWidth(), map->getHeight());
	camera->update(player, false);

	gui->message(TCODColor::lightGrey, "\n \n \n You awaken deep in the underhive. \n Find your way to the surface!");
	gameStatus = STARTUP;
}

void Engine::term()
{
	actors.clear();
	if (map) { map.reset(); }
	gui->clear();
}
