#pragma once
class Destructible : public Persistent {
public:
	float maxHp; // maximum Health points
	float hp; // current health points
	float defense; //hit point deflected
	std::string corpseName; // the actors name once dead
	int xp; //xp for killing monster or player xp

	Destructible(float maxHp, float defense, std::string_view corpseName, int xp);
	
	inline bool isDead() {
		return hp <= 0;
	}

	float takeDamage(Actor* owner, float damage);
	float heal(int amount);

	virtual void die(Actor* owner);

	virtual void save(TCODZip& zip);
	virtual void load(TCODZip& zip);
	static std::unique_ptr<Destructible> create(TCODZip& zip);
	
protected:
	enum class DestructibleType {
		MONSTER = 0,
		PLAYER
	};
};

class MonsterDestructible : public Destructible {
public:
	MonsterDestructible(float maxHp, float defense, std::string_view corpseName, int xp);
	void die(Actor* owner);
	void save(TCODZip& zip);
	
};

class PlayerDestructible : public Destructible {
public:
	PlayerDestructible(float maxHp, float defense, std::string_view corpseName, int xp);
	void die(Actor* owner);
	void save(TCODZip& zip);
	
};


