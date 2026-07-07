#pragma once

// Number of turns worth of scent a monster can track before losing the trail.
static constexpr int SCENT_THRESHOLD = 20;

// Abstract base for all AI behaviours. Subclass and implement update() to define
// how an actor decides what to do each turn.
class Ai : public Persistent {
public:
	virtual void update(Actor* owner) = 0;

	// Factory: reads the type discriminator from the archive and constructs the correct subclass.
	static std::unique_ptr<Ai> create(TCODZip& zip);

protected:
	// Serialisation discriminator written before each Ai payload.
	enum class AiType {
		PLAYER          = 0,
		MONSTER         = 1,
		CONFUSED_MONSTER = 2,
		RANGED_MONSTER  = 3
	};
};

// Wraps another Ai for a fixed number of turns, then restores the original.
// Used as the base for status-effect behaviours such as confusion.
class TemporaryAi : public Ai {
public:
	explicit TemporaryAi(int turnsRemaining);

	// Decrements the turn counter and restores oldAi when it reaches zero.
	void update(Actor* owner) override;

	// Legacy transfer path — only safe when the caller immediately moves this into owner->ai.
	virtual void applyTo(Actor* actor);

	// Safe transfer: takes exclusive ownership of self and installs it as owner->ai,
	// saving the current ai into oldAi first.
	void applyToActor(std::unique_ptr<TemporaryAi> self, Actor* actor);

	static auto create(TCODZip& zip);
	virtual void save(TCODZip& zip) {}
	virtual void load(TCODZip& zip) {}

protected:
	int turnsRemaining;
	std::shared_ptr<Ai> oldAi; // the ai to restore when the effect expires
};

class PlayerAi : public Ai {
public:
	int xpLevel;

	PlayerAi();

	// Returns the XP total required to reach the next level.
	int getNextLevelXp();

	void update(Actor* owner) override;

	// Handles single-character action keys (g=pickup, i=inventory, d=drop, >=stairs).
	void handleActionKey(Actor* owner, int ascii);

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

protected:
	// Moves to (targetX, targetY) or attacks the living actor there.
	// Returns true if the player actually moved (FOV needs recomputing).
	bool moveOrAttack(Actor* owner, int targetX, int targetY);
};

class MonsterAi : public Ai {
public:
	void update(Actor* owner) override;

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

protected:
	// Attacks if adjacent; moves toward (targetX, targetY) using FOV line-of-sight
	// or scent tracking when the player is out of sight.
	void moveOrAttack(Actor* owner, int targetX, int targetY);
};

// Ranged AI for enemies with ranged weapons. Prefers shooting at range,
// falls back to melee when adjacent or moves closer when out of range.
class RangedAi : public Ai {
public:
	void update(Actor* owner) override;

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

private:
	void shoot(Actor* owner, Actor* target);
	void reload(Actor* owner);
	void moveToward(Actor* owner, int targetX, int targetY);
	void followScent(Actor* owner);
};

// Wanders randomly for turnsRemaining turns, then reverts to the monster's normal AI.
class ConfusedMonsterAi : public TemporaryAi {
public:
	explicit ConfusedMonsterAi(int turnsRemaining);

	void update(Actor* owner) override;
	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};

