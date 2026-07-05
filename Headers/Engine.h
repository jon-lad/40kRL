#pragma once

#include <optional>
#include <string>
#include <vector>
#include "Equippable.h"
#include "TargetingContext.h"

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

// Direction the player is travelling when using stairs.
enum class StairDirection { UP, DOWN };

// Stores state for the non-blocking inventory menu overlay.
struct InventoryState {
	Actor* owner = nullptr;
	enum class Action { USE, DROP } pendingAction = Action::USE;
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
		INVENTORY   // inventory menu is open
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

	std::vector<EquipmentTemplate> equipmentTemplates; // loaded from Equipment.lua

	std::optional<TargetingContext> targetingCtx;  // active only during TARGETING state
	std::optional<InventoryState> inventoryState; // active only during INVENTORY state

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

	// Changes depth and generates a new level. Direction determines whether depth increments or decrements.
	void nextLevel(StairDirection direction);

	// Selects a random equipment template matching the given slot, using tier-weighted
	// random selection. Normalizes the tier weights to sum to 1.0, rolls a tier, then
	// picks a random template matching that tier and slot. Returns nullptr if no
	// templates match (logs a warning via gui->message).
	const EquipmentTemplate* selectEquipmentByTier(EquipmentSlot slot, const EnemyEquipmentConfig::TierWeights& weights);

	// Creates the player actor, stairs, and initial map for a new game.
	void init();

	// Clears all actors and the map. Called before loading a save file or starting a new game.
	void term();

	void save();
	void load();
};

extern Engine engine;
