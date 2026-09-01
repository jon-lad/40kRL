#pragma once

// Forward declarations
class Effect;

// ─── Target selection ────────────────────────────────────────────────────────

// Determines which actors are affected when a Pickable item is used.
// The five modes cover every targeting pattern in the game.
class TargetSelector : public Persistent {
public:
	enum class SelectorType {
		SELF,             // targets only the actor carrying the item
		CLOSEST_MONSTER,  // targets the nearest living non-player actor within range
		SELECTED_MONSTER, // player clicks on a specific monster within range
		WEARER_RANGE,     // all living non-player actors within range of the wearer
		SELECTED_RANGE    // all living non-player actors within range of a clicked tile
	};

	TargetSelector(SelectorType type, float range);

	// Fills list with the selected targets for immediate-resolution selectors (SELF, CLOSEST_MONSTER, WEARER_RANGE).
	// For SELECTED_MONSTER/SELECTED_RANGE: initiates targeting mode and returns false (effect not yet applied).
	// Returns true if targets were resolved immediately, false if targeting mode was started.
	bool selectTargets(Actor* wearer, Actor* itemActor, Effect* effect, TCODList<Actor*>& list);

	float getRange() const { return range; }
	SelectorType getType() const { return type; }

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

protected:
	SelectorType type;
	float range; // maximum selection range in tiles (0 = unlimited for CLOSEST_MONSTER)
};

// ─── Effects ─────────────────────────────────────────────────────────────────

// Abstract outcome applied to each target when a Pickable item is used.
class Effect : public Persistent {
public:
	// Serialisation discriminator written before each Effect payload.
	enum class EffectType {
		HEALTH    = 0,
		CHANGE_AI = 1
	};

	// Returns true if the effect was successfully applied.
	virtual bool applyTo(Actor* actor) = 0;

	// Factory: reads the type discriminator and constructs the correct subclass.
	static std::unique_ptr<Effect> create(TCODZip& zip);
};

// ─── Pickable ────────────────────────────────────────────────────────────────

// Gives an actor the ability to be picked up, used, and dropped.
// Behaviour is fully defined by the TargetSelector + Effect pair — no subclassing needed for new items.
class Pickable : public Persistent {
public:
	Pickable(std::unique_ptr<TargetSelector> selector, std::unique_ptr<Effect> effect);

	float weight = 0.0f;
	int   value  = 0;

	// Moves owner into wearer's inventory. Returns false if the inventory is full.
	bool pick(std::unique_ptr<Actor> owner, Actor* wearer);

	// Selects targets, applies the effect, and removes the item from inventory on success.
	// Returns true if the effect was applied to at least one target.
	bool use(Actor* owner, Actor* wearer);

	// Moves the item from wearer's inventory back onto the map at the wearer's position.
	void drop(Actor* owner, Actor* wearer);

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

protected:
	std::unique_ptr<TargetSelector> selector;
	std::unique_ptr<Effect>         effect;
};

// ─── Concrete Effects ────────────────────────────────────────────────────────

// Heals or damages the target.
// Positive amount → heals via Effects.lua; negative amount → direct damage via takeDamage().
class HealthEffect : public Effect {
public:
	float       amount;
	std::string message;  // message template; # is replaced with actor name / damage value
	TCODColor   textCol;

	HealthEffect(float amount, std::string_view message, const TCODColor& textCol);

	bool applyTo(Actor* actor) override;
	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};

// Replaces the target's current AI with a TemporaryAi for a fixed number of turns.
class AiChangeEffect : public Effect {
public:
	std::unique_ptr<TemporaryAi> newAi;
	std::string message;
	TCODColor   textCol;

	AiChangeEffect(std::unique_ptr<TemporaryAi> newAi, std::string_view message, const TCODColor& textCol);

	bool applyTo(Actor* actor) override;
	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};