#pragma once

class Pickable : public Persistent {
public:
	bool pick(std::unique_ptr<Actor> owner, Actor *wearer);
	virtual bool use(Actor* owner, Actor* wearer);
	void drop(Actor* owner, Actor* wearer);
	static std::unique_ptr<Pickable> create(TCODZip& zip);
protected:
	enum class PickableType {
		HEALER = 0,
		LIGHTNING_BOLT,
		CONFUSER,
		FIREBALL
	};
};

/*Pickable Items Below*/

class Healer : public Pickable {
public:
	float amount;

	Healer(float amount);
	bool use(Actor* owner, Actor* wearer);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
};

class LightningBolt : public Pickable {
public:
	float range;
	float damage;
	LightningBolt(float range, float damage);
	bool use(Actor* owner, Actor* wearer);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
};

class Fireball : public LightningBolt {
public:
	Fireball(float range, float damage);
	bool use(Actor* owner, Actor* wearer);
	void save(TCODZip& zip);
};

class Confuser : public Pickable {
public:
	int nbTurns;
	float range;
	Confuser(int nbTurns, float range);
	bool use(Actor* owner, Actor* wearer);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
};