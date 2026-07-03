#include <list>
#include <memory>
#include <sstream>
#include <sol/sol.hpp>
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

	pollInput(inputState);

	if (inputState.key.key == SDLK_ESCAPE) {
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
	// Direction is determined by the stairs glyph on the current level.
	// '<' stairs = ascend (depth decreases), '>' stairs = descend (depth increases).
	if (stairs->getGlyph() == '<') {
		dungeonLevel--;
	} else {
		dungeonLevel++;
	}

	gui->message(Colors::healing, "You take a moment to rest and recover your strength.");
	player->destructible->heal(player->destructible->maxHp / 2.0f);

	const bool isOutdoor = (dungeonLevel == 0);

	if (isOutdoor) {
		gui->message(Colors::surfaceMsg, "You emerge from the depths onto the planet surface.");
	} else if (stairs->getGlyph() == '<') {
		gui->message(Colors::damage, "You ascend closer to the surface.");
	} else {
		gui->message(Colors::damage, "You descend deeper underground.");
	}

	map.reset();

	// Remove all actors except the player and stairs.
	for (auto i = actors.begin(); i != actors.end(); ) {
		i = (i->get() != player && i->get() != stairs) ? actors.erase(i) : std::next(i);
	}

	map = std::make_unique<Map>(MAP_WIDTH, MAP_HEIGHT);
	map->init(true, isOutdoor ? LevelType::OUTDOOR : LevelType::BSP);

	// On the surface: stairs go down (dungeon entrance). Underground: stairs go up (toward surface).
	stairs->setGlyph(isOutdoor ? '>' : '<');

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
					TCOD_ColorRGB bright = {
						static_cast<uint8_t>(std::min(255, col.r * 6 / 5)),
						static_cast<uint8_t>(std::min(255, col.g * 6 / 5)),
						static_cast<uint8_t>(std::min(255, col.b * 6 / 5))
					};
					TCOD_console_put_rgb(TCODConsole::root->get_data(), screenX, screenY, 0, &bright, nullptr, TCOD_BKGND_NONE);
				}
			}
		}

		pollInput(inputState);
		auto [worldX, worldY] = camera->getWorldLocation(inputState.mouse.cellX, inputState.mouse.cellY);

		if (map->isInFOV(worldX, worldY)
			&& (maxRange == 0.0f || player->getDistance(worldX, worldY) <= maxRange))
		{
			renderSetBg(TCODConsole::root->get_data(), inputState.mouse.cellX, inputState.mouse.cellY, {255, 255, 255});
			if (inputState.mouse.lbutton_pressed) {
				*x = worldX;
				*y = worldY;
				return true;
			}
			if (inputState.mouse.rbutton_pressed || inputState.key.key != SDLK_UNKNOWN) {
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
	// Load player class stats from Classes.lua (fall back to hardcoded defaults if missing).
	float playerHp      = 30.0f;
	float playerDefense = 2.0f;
	float playerPower   = 5.0f;
	int   playerSkill   = 40;
	int   playerInvSize = 26;

	try {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		lua.script_file("Scripts/Classes.lua");

		sol::table cls = lua["defaultClass"];
		if (cls.valid()) {
			playerHp      = cls.get_or("hp",      playerHp);
			playerDefense = cls.get_or("defense", playerDefense);
			playerPower   = cls.get_or("power",   playerPower);
			playerSkill   = cls.get_or("skill",   playerSkill);
			playerInvSize = cls.get_or("invSize", playerInvSize);
		}
	} catch (const sol::error&) {
		// Classes.lua missing or malformed — use defaults above.
	}

	// Create the player.
	auto newPlayer = std::make_unique<Actor>(0, 0, '@', "Player", Colors::white);
	player = newPlayer.get();
	newPlayer->destructible = std::make_unique<PlayerDestructible>(playerHp, playerDefense, "Your cadaver", 0);
	newPlayer->attacker     = std::make_unique<Attacker>(playerPower, playerSkill);
	newPlayer->ai           = std::make_unique<PlayerAi>();
	newPlayer->container    = std::make_unique<Container>(playerInvSize);
	actors.emplace_front(std::move(newPlayer));

	// Create the stairs (always visible, never blocks). '<' = ascend toward surface.
	auto newStairs = std::make_unique<Actor>(0, 0, '<', "stairs", Colors::white);
	stairs = newStairs.get();
	newStairs->blocks  = false;
	newStairs->fovOnly = false;
	actors.emplace_front(std::move(newStairs));

	map = std::make_unique<Map>(MAP_WIDTH, MAP_HEIGHT);
	map->init(true, LevelType::BSP);
	camera = std::make_unique<Camera>(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
		map->getWidth(), map->getHeight());
	camera->update(player, false);

	gui->message(Colors::uiText, "\n \n \n You awaken deep in the underhive. \n Find your way to the surface!");
	gameStatus = STARTUP;
}

void Engine::term()
{
	actors.clear();
	if (map) { map.reset(); }
	gui->clear();
}
