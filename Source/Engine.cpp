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
	, debugMode{ false }
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

		TCODConsole::flush();
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

const EquipmentTemplate* Engine::selectEquipmentByTier(EquipmentSlot slot, const EnemyEquipmentConfig::TierWeights& weights)
{
	// Normalize weights so they sum to 1.0
	float totalWeight = weights.common + weights.uncommon + weights.rare;
	if (totalWeight <= 0.0f) {
		gui->message(Colors::damage, "Warning: tier weights sum to zero, cannot select equipment.");
		return nullptr;
	}
	float normCommon   = weights.common / totalWeight;
	float normUncommon = weights.uncommon / totalWeight;
	// normRare is the remainder (1.0 - normCommon - normUncommon)

	// Roll a random float to select a tier
	TCODRandom* rng = TCODRandom::getInstance();
	float tierRoll = rng->getFloat(0.0f, 1.0f);

	ItemTier selectedTier;
	if (tierRoll < normCommon) {
		selectedTier = ItemTier::COMMON;
	} else if (tierRoll < normCommon + normUncommon) {
		selectedTier = ItemTier::UNCOMMON;
	} else {
		selectedTier = ItemTier::RARE;
	}

	// Filter equipmentTemplates by the selected tier AND the target slot
	std::vector<const EquipmentTemplate*> candidates;
	for (const auto& tmpl : equipmentTemplates) {
		if (tmpl.slot == slot && tmpl.tier == selectedTier) {
			candidates.push_back(&tmpl);
		}
	}

	if (candidates.empty()) {
		gui->message(Colors::damage, "Warning: no equipment templates for slot+tier combination.");
		return nullptr;
	}

	// Randomly pick one from the matching candidates
	int pick = rng->getInt(0, static_cast<int>(candidates.size()) - 1);
	return candidates[pick];
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

	// Load global config from Config.lua.
	try {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		lua.script_file("Scripts/Config.lua");

		sol::table config = lua["config"];
		if (config.valid()) {
			carryingCapacity = config.get_or("carryingCapacity", 50.0f);
		}
	} catch (const sol::error&) {
		// Config.lua missing or malformed — use defaults.
	}

	// Load equipment templates from Equipment.lua.
	// NOTE: This MUST run before map->init() because enemy spawning (Map::addMonster)
	// references equipmentTemplates to assign gear to enemies.
	try {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		lua.script_file("Scripts/Equipment.lua");

		sol::table equipTable = lua["equipment"];
		if (equipTable.valid()) {
			for (size_t i = 1; i <= equipTable.size(); i++) {
				sol::table entry = equipTable[i];

				// Required fields
				std::string emptyStr;
				std::string name     = entry.get_or("name", emptyStr);
				std::string glyphStr = entry.get_or("glyph", emptyStr);
				std::string colorName = entry.get_or("color", emptyStr);
				std::string slotName = entry.get_or("slot", emptyStr);
				float negWeight = -1.0f;
				float weight    = entry.get_or("weight", negWeight);

				// Validate required fields
				if (name.empty() || glyphStr.empty() || colorName.empty() || slotName.empty()) {
					gui->message(Colors::damage, "Equipment.lua: skipping entry # — missing required field.", static_cast<int>(i));
					continue;
				}
				if (weight < 0.0f) {
					gui->message(Colors::damage, "Equipment.lua: skipping '#' — weight must be >= 0.", name);
					continue;
				}

				// Parse slot
				EquipmentSlot slot;
				if (slotName == "weapon")       slot = EquipmentSlot::WEAPON;
				else if (slotName == "offhand") slot = EquipmentSlot::OFFHAND;
				else if (slotName == "head")    slot = EquipmentSlot::HEAD;
				else if (slotName == "body")    slot = EquipmentSlot::BODY;
				else {
					gui->message(Colors::damage, "Equipment.lua: skipping '#' — invalid slot '#'.", name, slotName);
					continue;
				}

				// Optional fields
				int defaultValue = 0;
				float defaultFloat = 0.0f;
				int defaultInt = 0;
				int value      = entry.get_or("value", defaultValue);
				float power    = entry.get_or("power", defaultFloat);
				float defense  = entry.get_or("defense", defaultFloat);
				float maxHp    = entry.get_or("maxHp", defaultFloat);
				int skill      = entry.get_or("skill", defaultInt);

				// Resolve color
				TCODColor color = Colors::colorFromName(colorName);

				// Parse optional tier field (default: COMMON)
				std::string tierStr = entry.get_or("tier", std::string("common"));
				ItemTier tier = ItemTier::COMMON;
				if (tierStr == "uncommon")     tier = ItemTier::UNCOMMON;
				else if (tierStr == "rare")    tier = ItemTier::RARE;

				// Create template
				EquipmentTemplate tmpl;
				tmpl.name      = name;
				tmpl.glyph     = static_cast<int>(glyphStr[0]);
				tmpl.color     = color;
				tmpl.slot      = slot;
				tmpl.weight    = weight;
				tmpl.value     = value;
				tmpl.modifiers = { power, defense, maxHp, skill };
				tmpl.tier      = tier;

				equipmentTemplates.push_back(tmpl);
			}
		}
	} catch (const sol::error&) {
		// Equipment.lua missing or malformed — no equipment templates loaded.
	}

	// Create the player.
	auto newPlayer = std::make_unique<Actor>(0, 0, '@', "Player", Colors::white);
	player = newPlayer.get();
	newPlayer->destructible = std::make_unique<PlayerDestructible>(playerHp, playerDefense, "Your cadaver", 0);
	newPlayer->attacker     = std::make_unique<Attacker>(playerPower, playerSkill);
	newPlayer->ai           = std::make_unique<PlayerAi>();
	newPlayer->container    = std::make_unique<Container>(playerInvSize);
	newPlayer->equipment    = std::make_unique<Equipment>();
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
