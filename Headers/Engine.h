#pragma once

#include <string>
#include <vector>
#include "Equippable.h"

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
		DEFEAT
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

	// Enters interactive tile-selection mode. Highlights reachable tiles and waits for a
	// mouse click. Writes the chosen world position into *x and *y.
	// Returns true if a tile was selected, false if cancelled.
	bool pickAtTile(int* x, int* y, float maxRange = 0.0f);

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
