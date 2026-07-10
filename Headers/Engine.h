#pragma once

#include <optional>
#include <string>
#include <vector>
#include "Equippable.h"
#include "CharacterData.h"
#include "CharacterGenerator.h"
#include "LevelCache.h"
#include "TargetingContext.h"
#include "WorldMap.h"

// Rarity tier for equipment items — used for weighted random selection during enemy spawning.
enum class ItemTier { COMMON, UNCOMMON, RARE };

// A template for a spawnable equipment item, loaded from Equipment.lua at startup.
struct EquipmentTemplate {
	std::string name;
	int glyph;
	TCODColor color;
	EquipmentSlot slot;
	float weight;
	int value;
	StatModifiers modifiers;
	ItemTier tier = ItemTier::COMMON;
	std::optional<MeleeStats> meleeStats;      // present for weapons
	std::optional<ArmourProfile> armourProfile; // present for armour
	std::optional<RangedStats> rangedStats;    // present for ranged weapons
};

// Parsed from Lua during enemy template loading. Determines how an enemy
// gets its starting equipment and what happens to it on death.
struct EnemyEquipmentConfig {
	std::vector<std::string> equipmentNames;  // explicit item names from "equipment" field
	float dropChance = 1.0f;                  // probability each item drops on death [0.0, 1.0]
	// Tier-based random selection (ignored if equipmentNames is non-empty)
	struct TierWeights {
		float common = 0.70f;
		float uncommon = 0.25f;
		float rare = 0.05f;
	} tierWeights;
	bool useTierSelection = false;  // true if "equipTier" field is present in Lua
};

// A template for a spawnable decoration, loaded from Decorations.lua at startup.
struct DecorationTemplate {
	int glyph;
	std::string name;
	TCODColor color;
	std::string description;
	bool blocks;
	int coverValue;
};

// Direction the player is travelling when using stairs.
enum class StairDirection { UP, DOWN };

// Stores state for the non-blocking inventory menu overlay.
struct InventoryState {
	Actor* owner = nullptr;
	enum class Action { USE, DROP } pendingAction = Action::USE;
};

// Stores state for the pickup selection menu overlay.
struct PickupMenuState {
	std::vector<Actor*> items;
};

// Stores state for the look-mode cursor overlay.
struct LookState {
	int cursorX;
	int cursorY;
};

// Stores state for the character sheet overlay.
struct CharacterSheetState {};

// Stores state for the in-game advance purchase overlay.
struct AdvancesState {
	int selectedIndex = 0;       // cursor position in the advance list
	std::string statusMessage;   // purchase feedback message
};

// Global game state machine. Owns the actor list, map, camera, and GUI.
// There is one instance declared in main.cpp and exposed via extern.
class Engine {
public:
	// Controls which systems run each frame.
	enum GameStatus {
		STARTUP,   // FOV needs recomputing; no AI updates yet
		IDLE,      // waiting for player input; no AI updates
		NEW_TURN,  // player acted; all actors update this frame
		VICTORY,
		DEFEAT,
		TARGETING,  // tile selection in progress
		INVENTORY,  // inventory menu is open
		PICKUP_MENU, // pickup selection menu is open
		LOOK,        // look-mode cursor active
		CHARACTER_SHEET, // character sheet overlay is open
		ADVANCES,        // advance purchase overlay is open
		WORLD_MAP,      // world map overlay is open
		CHARACTER_GEN   // character generation overlay is active
	} gameStatus;

	std::list<std::unique_ptr<Actor>> actors; // all live actors, owned here

	std::unique_ptr<Map> map;

	// Raw-pointer aliases into actors — these actors must remain in the list for the lifetime of the pointers.
	Actor* player;
	Actor* stairsUp;
	Actor* stairsDown;

	int fovRadius;
	float carryingCapacity = 50.0f;
	int screenWidth;
	int screenHeight;

	std::unique_ptr<Camera> camera;
	InputState inputState;
	std::unique_ptr<Gui> gui;
	int dungeonLevel;
	bool debugMode;  // toggled with F12, enables level-skip and other dev tools

	LevelCache levelCache; // LRU cache of serialized level snapshots

	std::vector<EquipmentTemplate> equipmentTemplates; // loaded from Equipment.lua
	std::vector<DecorationTemplate> decorationTemplates; // loaded from Decorations.lua
	std::vector<HomeworldTemplate> homeworldTemplates;   // loaded from Homeworlds.lua
	std::vector<CareerTemplate> careerTemplates;         // loaded from Careers.lua
	std::vector<SkillDefinition> skillDefinitions;       // loaded from Skills.lua
	std::vector<TalentDefinition> talentDefinitions;     // loaded from Talents.lua

	std::optional<TargetingContext> targetingCtx;  // active only during TARGETING state
	std::optional<InventoryState> inventoryState; // active only during INVENTORY state
	std::optional<PickupMenuState> pickupMenuState; // active only during PICKUP_MENU state
	std::optional<LookState> lookState; // active only during LOOK state
	std::optional<CharacterSheetState> characterSheetState; // active only during CHARACTER_SHEET state
	std::optional<AdvancesState> advancesState; // active only during ADVANCES state
	std::optional<WorldMapState> worldMapState; // active only during WORLD_MAP state
	std::optional<CharGenState> charGenState; // active only during CHARACTER_GEN state

	uint32_t worldSeed = 0; // deterministic seed for world map generation, set during init()

	Engine(int screenWidth, int screenHeight);

	// Processes one frame: recompute FOV if needed, read input, update player, update monsters on NEW_TURN.
	void update();

	// Clears the root console, draws the map, draws all visible actors, draws the HUD.
	void render();

	// Moves actor to the front of the actors list so it is drawn beneath living actors.
	void sendToBack(Actor* actor);

	// Returns the nearest living non-player actor within range tiles, or nullptr if none found.
	// Pass range == 0.0f for unlimited range.
	Actor* getClosestMonster(int x, int y, float range) const;

	// Returns the living actor at world position (x, y), or nullptr if the tile is empty.
	Actor* getActorAt(int x, int y) const;

	// Enters targeting mode. Called by TargetSelector instead of pickAtTile.
	void beginTargeting(Actor* item, Actor* owner, float maxRange,
	                    TargetSelector::SelectorType type, Effect* effect,
	                    float aoeRange = 0.0f);

	// Processes one frame of targeting input. Called from update() when TARGETING.
	void updateTargeting();

	// Renders targeting highlights. Called from render() when TARGETING.
	void renderTargeting();

	// Enters inventory display mode. Called when player opens inventory.
	void beginInventory(Actor* owner, InventoryState::Action action);

	// Processes one frame of inventory input. Called from update() when INVENTORY.
	void updateInventory();

	// Renders inventory overlay. Called from render() when INVENTORY.
	void renderInventory();

	// Enters pickup selection mode. Called when player presses 'g' on a multi-item tile.
	void beginPickupMenu(const std::vector<Actor*>& items);

	// Processes one frame of pickup menu input. Called from update() when PICKUP_MENU.
	void updatePickupMenu();

	// Renders pickup menu overlay. Called from render() when PICKUP_MENU.
	void renderPickupMenu();

	// Enters look mode. Called when player presses 'l' in IDLE state.
	void beginLook();

	// Processes one frame of look-mode input. Called from update() when LOOK.
	void updateLook();

	// Renders look-mode cursor highlight. Called from render() when LOOK.
	void renderLook();

	// Enters character sheet display mode. Called when player presses 'c' in IDLE state.
	void beginCharacterSheet();

	// Processes one frame of character sheet input. Called from update() when CHARACTER_SHEET.
	void updateCharacterSheet();

	// Renders character sheet overlay. Called from render() when CHARACTER_SHEET.
	void renderCharacterSheet();

	// Enters advance purchase mode. Called when player presses 'x' in IDLE state.
	void beginAdvances();

	// Processes one frame of advance purchase input. Called from update() when ADVANCES.
	void updateAdvances();

	// Renders advance purchase overlay. Called from render() when ADVANCES.
	void renderAdvances();

	// Enters character generation mode. Called when starting a new game.
	void beginCharGen();

	// Processes one frame of character generation input. Called from update() when CHARACTER_GEN.
	void updateCharGen();

	// Renders character generation overlay. Called from render() when CHARACTER_GEN.
	void renderCharGen();

	// Navigates back to the previous chargen step, resetting state for the step being left.
	// On HOMEWORLD step, does nothing (cannot go back further).
	void charGenGoBack();

	// Enters world map display mode. Called when player presses 'm' in IDLE state.
	void beginWorldMap();

	// Processes one frame of world map input. Called from update() when WORLD_MAP.
	void updateWorldMap();

	// Renders world map overlay. Called from render() when WORLD_MAP.
	void renderWorldMap();

	// Changes depth and generates a new level. Direction determines whether depth increments or decrements.
	void nextLevel(StairDirection direction);

	// Selects a random equipment template matching the given slot, using tier-weighted
	// random selection. Normalizes the tier weights to sum to 1.0, rolls a tier, then
	// picks a random template matching that tier and slot. Returns nullptr if no
	// templates match (logs a warning via gui->message).
	const EquipmentTemplate* selectEquipmentByTier(EquipmentSlot slot, const EnemyEquipmentConfig::TierWeights& weights);

	// Creates the player actor, stairs, and initial map for a new game.
	void init();

	// Lua data loaders — called during init() before player creation.
	void loadHomeworldTemplates();
	void loadCareerTemplates();
	void loadSkillDefinitions();
	void loadTalentDefinitions();

	// Clears all actors and the map. Called before loading a save file or starting a new game.
	void term();

	// Serialize current map + non-player actors into a byte buffer for caching.
	std::vector<char> serializeCurrentLevel() const;

	// Deserialize a byte buffer into the active map + actors.
	// Assigns stairsUp/stairsDown pointers, orders dead actors behind living.
	// Returns false if expected stairs are missing (snapshot is corrupted).
	bool deserializeLevel(const std::vector<char>& snapshot);

	void save();
	void load();
};

extern Engine engine;
