#pragma once

// Gives an actor health points, a defence value, and death behaviour.
class Destructible : public Persistent {
public:
	float maxHp;
	float hp;
	float defense;       // damage reduction applied before each hit
	std::string corpseName; // name this actor gets after dying
	int xp;              // XP awarded to the player for killing this actor (or the player's own XP pool)

	Destructible(float maxHp, float defense, std::string_view corpseName, int xp);

	bool isDead() const { return hp <= 0; }

	// Applies damage after defence reduction. Calls die() if hp drops to zero or below.
	// Returns the effective damage dealt (0 if blocked entirely).
	float takeDamage(Actor* owner, float damage);

	// Restores up to amount HP, capped at maxHp.
	// Returns the actual amount healed.
	float heal(float amount);

	// Transforms owner into a corpse (changes glyph, disables blocking, resets AI).
	virtual void die(Actor* owner);

	virtual void save(TCODZip& zip) override;
	virtual void load(TCODZip& zip) override;

	// Factory: reads the type discriminator from the archive and constructs the correct subclass.
	static std::unique_ptr<Destructible> create(TCODZip& zip);

protected:
	// Serialisation discriminator written before each Destructible payload.
	enum class DestructibleType {
		MONSTER = 0,
		PLAYER  = 1
	};
};

class MonsterDestructible : public Destructible {
public:
	MonsterDestructible(float maxHp, float defense, std::string_view corpseName, int xp);

	// Awards XP to the player and logs a death message before calling the base die().
	void die(Actor* owner) override;
	void save(TCODZip& zip) override;
};

class PlayerDestructible : public Destructible {
public:
	PlayerDestructible(float maxHp, float defense, std::string_view corpseName, int xp);

	// Logs "You died!", calls base die(), and sets gameStatus to DEFEAT.
	void die(Actor* owner) override;
	void save(TCODZip& zip) override;
};


