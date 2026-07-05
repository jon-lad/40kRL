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
	, stairsUp{ nullptr }
	, stairsDown{ nullptr }
	, fovRadius{ DEFAULT_FOV_RADIUS }
	, screenWidth{ screenWidth }
	, screenHeight{ screenHeight }
	, dungeonLevel{ 20 }
	, debugMode{ false }
{
	TCODConsole::initRoot(screenWidth, screenHeight, "40kRL", false);
	SDL_StartTextInput(SDL_GetKeyboardFocus());
	gui = std::make_unique<Gui>();
}

void Engine::update()
{
	if (gameStatus == STARTUP) { map->computeFOV(); gameStatus = IDLE; }

	pollInput(inputState);

	// Handle targeting state — skip all normal game logic.
	if (gameStatus == TARGETING) {
		updateTargeting();
		return;
	}

	// Handle inventory state — skip all normal game logic.
	if (gameStatus == INVENTORY) {
		updateInventory();
		return;
	}

	gameStatus = IDLE;

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

	// Render targeting overlay (highlights + cursor) when in TARGETING state.
	if (gameStatus == TARGETING) {
		renderTargeting();
	}

	// Render inventory overlay when in INVENTORY state.
	if (gameStatus == INVENTORY) {
		renderInventory();
	}

	gui->render();
}

void Engine::nextLevel(StairDirection direction)
{
	if (direction == StairDirection::DOWN) {
		dungeonLevel++;
	} else {
		dungeonLevel--;
	}

	gui->message(Colors::healing, "You take a moment to rest and recover your strength.");
	player->destructible->heal(player->destructible->maxHp / 2.0f);

	const bool isOutdoor = (dungeonLevel == 0);

	if (isOutdoor) {
		gui->message(Colors::surfaceMsg, "You emerge from the depths onto the planet surface.");
	} else if (direction == StairDirection::UP) {
		gui->message(Colors::damage, "You ascend closer to the surface.");
	} else {
		gui->message(Colors::damage, "You descend deeper underground.");
	}

	// Destroy old map and remove all actors except player.
	map.reset();
	for (auto i = actors.begin(); i != actors.end(); ) {
		i = (i->get() != player) ? actors.erase(i) : std::next(i);
	}
	stairsUp = nullptr;
	stairsDown = nullptr;

	// Create stair actors for the new depth BEFORE map->init() so that
	// createRoom() can set their positions during BSP generation.
	const bool needsUp   = (dungeonLevel > 0);
	const bool needsDown = (dungeonLevel < 20);

	if (needsUp) {
		auto newUp = std::make_unique<Actor>(0, 0, '<', "stairs up", Colors::white);
		stairsUp = newUp.get();
		newUp->blocks = false;
		newUp->fovOnly = false;
		actors.emplace_front(std::move(newUp));
	}
	if (needsDown) {
		auto newDown = std::make_unique<Actor>(0, 0, '>', "stairs down", Colors::white);
		stairsDown = newDown.get();
		newDown->blocks = false;
		newDown->fovOnly = false;
		actors.emplace_front(std::move(newDown));
	}

	// Generate new map — createRoom() sets stair positions during BSP generation.
	map = std::make_unique<Map>(MAP_WIDTH, MAP_HEIGHT);
	map->init(true, isOutdoor ? LevelType::OUTDOOR : LevelType::BSP);

	// Place player on arrival stair (BSP levels — outdoor handled by placeOutdoorActors).
	if (!isOutdoor) {
		if (direction == StairDirection::DOWN && stairsUp) {
			player->setX(stairsUp->getX());
			player->setY(stairsUp->getY());
		} else if (direction == StairDirection::UP && stairsDown) {
			player->setX(stairsDown->getX());
			player->setY(stairsDown->getY());
		}
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

void Engine::beginTargeting(Actor* item, Actor* owner, float maxRange,
                            TargetSelector::SelectorType type, Effect* effect,
                            float aoeRange)
{
	// Validate pointers — abort without state change if any are null.
	if (!item) {
		gui->message(Colors::damage, "Warning: beginTargeting called with null item.");
		return;
	}
	if (!owner) {
		gui->message(Colors::damage, "Warning: beginTargeting called with null owner.");
		return;
	}
	if (!effect) {
		gui->message(Colors::damage, "Warning: beginTargeting called with null effect.");
		return;
	}

	// Populate targeting context and transition to TARGETING state.
	targetingCtx = TargetingContext{ item, owner, maxRange, type, effect, aoeRange };
	gameStatus = TARGETING;

	gui->message(Colors::uiText, "Left-click to confirm, right-click or ESC to cancel.");
}

void Engine::renderTargeting()
{
	if (!targetingCtx) return;

	const float maxRange = targetingCtx->maxRange;

	// DEBUG: unconditional marker to confirm renderTargeting is being called
	renderSetBg(TCODConsole::root->get_data(), 0, 0, {255, 0, 0}); // red pixel at top-left

	// Brighten all in-range, in-FOV tiles to indicate they are selectable.
	for (int cx = 0; cx < map->getWidth(); cx++) {
		for (int cy = 0; cy < map->getHeight(); cy++) {
			if (map->isInFOV(cx, cy)
				&& (maxRange == 0.0f || player->getDistance(cx, cy) <= maxRange))
			{
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

	// Show cursor highlight on the tile under the mouse, only if valid (in FOV + in range).
	int mouseCellX = inputState.mouse.cellX;
	int mouseCellY = inputState.mouse.cellY;
	auto [worldX, worldY] = camera->getWorldLocation(mouseCellX, mouseCellY);

	if (map->isInFOV(worldX, worldY)
		&& (maxRange == 0.0f || player->getDistance(worldX, worldY) <= maxRange))
	{
		renderSetBg(TCODConsole::root->get_data(), mouseCellX, mouseCellY, {255, 255, 255});
	}
}

void Engine::updateTargeting()
{
	if (!targetingCtx) {
		// Safety: should not be called without a valid context.
		gameStatus = IDLE;
		return;
	}

	// DEBUG: Check if mouse events are being received
	if (inputState.mouse.lbutton_pressed) {
		gui->message(Colors::yellow, "DEBUG: Left-click detected at cell (#,#)", 
			std::to_string(inputState.mouse.cellX), std::to_string(inputState.mouse.cellY));
	}

	// --- Cancellation: ESC key or right-click ---
	if ((inputState.key.key == SDLK_ESCAPE && inputState.key.pressed)
		|| inputState.mouse.rbutton_pressed)
	{
		gui->message(Colors::uiText, "Targeting cancelled.");
		targetingCtx = std::nullopt;
		gameStatus = IDLE;
		return;
	}

	// --- Confirmation: left-click ---
	if (inputState.mouse.lbutton_pressed) {
		// Convert mouse cell coords to world coords.
		auto [worldX, worldY] = camera->getWorldLocation(inputState.mouse.cellX, inputState.mouse.cellY);

		// Validate tile: must be in FOV.
		if (!map->isInFOV(worldX, worldY)) {
			gui->message(Colors::damage, "Target out of sight.");
			return; // ignore click, remain in TARGETING
		}

		// Validate tile: must be in range (maxRange == 0 means unlimited).
		if (targetingCtx->maxRange > 0.0f
			&& player->getDistance(worldX, worldY) > targetingCtx->maxRange)
		{
			gui->message(Colors::damage, "Target out of range.");
			return; // ignore click, remain in TARGETING
		}

		// --- Stale-pointer check: verify item still exists in owner's inventory ---
		bool itemFound = false;
		if (targetingCtx->owner && targetingCtx->owner->container) {
			for (const auto& slot : targetingCtx->owner->container->inventory) {
				if (slot.get() == targetingCtx->item) {
					itemFound = true;
					break;
				}
			}
		}
		if (!itemFound) {
			gui->message(Colors::damage, "Targeting cancelled — item no longer available.");
			targetingCtx = std::nullopt;
			gameStatus = IDLE;
			return;
		}

		// --- SELECTED_MONSTER: require a living non-player actor on tile ---
		if (targetingCtx->type == TargetSelector::SelectorType::SELECTED_MONSTER) {
			Actor* target = getActorAt(worldX, worldY);
			if (!target || target == player) {
				return; // no valid target, remain in TARGETING
			}
			// Apply effect to the targeted actor.
			targetingCtx->effect->applyTo(target);
		}
		// --- SELECTED_RANGE: AoE around confirmed tile ---
		else if (targetingCtx->type == TargetSelector::SelectorType::SELECTED_RANGE) {
			for (const auto& actorPtr : actors) {
				Actor* actor = actorPtr.get();
				if (actor != player
					&& actor->destructible
					&& !actor->destructible->isDead()
					&& actor->getDistance(worldX, worldY) <= targetingCtx->aoeRange)
				{
					targetingCtx->effect->applyTo(actor);
				}
			}
		}

		// --- Successful confirmation: remove item from owner's inventory ---
		if (targetingCtx->owner && targetingCtx->owner->container) {
			targetingCtx->owner->container->remove(targetingCtx->item);
		}

		// Clear context and advance turn.
		targetingCtx = std::nullopt;
		gameStatus = NEW_TURN;
	}
}

void Engine::beginInventory(Actor* owner, InventoryState::Action action)
{
	if (!owner || !owner->container) {
		gui->message(Colors::damage, "Warning: beginInventory called with invalid owner.");
		return;
	}
	inventoryState = InventoryState{ owner, action };
	gameStatus = INVENTORY;
}

void Engine::updateInventory()
{
	if (!inventoryState) {
		// Safety: should not be called without a valid state.
		gameStatus = IDLE;
		return;
	}

	// --- Cancellation: ESC key ---
	if (inputState.key.key == SDLK_ESCAPE && inputState.key.pressed) {
		inventoryState = std::nullopt;
		gameStatus = IDLE;
		return;
	}

	// --- Item selection: a-z key ---
	if (inputState.key.pressed && inputState.key.c >= 'a' && inputState.key.c <= 'z') {
		const int itemIndex = inputState.key.c - 'a';
		Actor* owner = inventoryState->owner;

		if (itemIndex < static_cast<int>(owner->container->inventory.size())) {
			// Get the item at that index.
			auto it = owner->container->inventory.begin();
			std::advance(it, itemIndex);
			Actor* item = it->get();

			if (item) {
				if (inventoryState->pendingAction == InventoryState::Action::USE) {
					// Equippable items get equipped; others are used normally.
					if (item->equippable && owner->equipment) {
						Actor* previous = owner->equipment->equip(item, owner->container.get(), owner->attacker.get());
						if (previous) {
							gui->message(Colors::uiText, "You unequip the # and equip the #.", previous->name, item->name);
						} else {
							gui->message(Colors::uiText, "You equip the #.", item->name);
						}
						gameStatus = NEW_TURN;
					} else {
						// use() may initiate TARGETING (returns false) or apply immediately (returns true).
						// If targeting was initiated, gameStatus is already TARGETING — don't override.
						if (item->pickable->use(item, owner)) {
							gameStatus = NEW_TURN;
						}
						// If use returned false and gameStatus == TARGETING, leave it.
					}
				} else {
					// DROP action
					item->pickable->drop(item, owner);
					gameStatus = NEW_TURN;
				}
			}

			inventoryState = std::nullopt;
			return;
		}
		// Invalid index — ignore, remain in INVENTORY.
	}
}

void Engine::renderInventory()
{
	if (!inventoryState) return;

	static constexpr int INVENTORY_WIDTH  = 50;
	static constexpr int INVENTORY_HEIGHT = 28;
	static TCODConsole inventoryConsole(INVENTORY_WIDTH, INVENTORY_HEIGHT);

	inventoryConsole.setDefaultForeground(Colors::menuFrame);
	inventoryConsole.printFrame(0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, true, TCOD_BKGND_DEFAULT, "inventory");

	inventoryConsole.setDefaultForeground(Colors::white);
	int shortcutKey = 'a';
	int row = 1;
	for (const auto& itemPtr : inventoryState->owner->container->inventory) {
		if (itemPtr) {
			inventoryConsole.printf(2, row, "(%c) %s", shortcutKey, itemPtr->name.c_str());
			row++;
			shortcutKey++;
		}
	}

	TCODConsole::blit(&inventoryConsole, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT,
		TCODConsole::root,
		screenWidth  / 2 - INVENTORY_WIDTH  / 2,
		screenHeight / 2 - INVENTORY_HEIGHT / 2);
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

	// Create stairsUp (always visible, never blocks). Starting level is depth 20 (deepest),
	// so only up-stairs exist — stairsDown remains nullptr.
	auto newStairsUp = std::make_unique<Actor>(0, 0, '<', "stairs up", Colors::white);
	stairsUp = newStairsUp.get();
	newStairsUp->blocks  = false;
	newStairsUp->fovOnly = false;
	actors.emplace_front(std::move(newStairsUp));

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
