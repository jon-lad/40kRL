#pragma once

class Pickable {
public:
	bool pick(std::unique_ptr<Actor> owner, Actor *wearer);
	virtual bool use(Actor* owner, Actor* wearer);
	void drop(Actor* owner, Actor* wearer);
};

/*Pickable Items Below*/

class Healer : public Pickable {
public:
	float amount;

	Healer(float amount);
	bool use(Actor* owner, Actor* wearer);
};

class LightningBolt : public Pickable {
public:
	float range;
	float damage;
	LightningBolt(float range, float damage);
	bool use(Actor* owner, Actor* wearer);
};

class Fireball : public LightningBolt {
public:
	Fireball(float range, float damage);
	bool use(Actor* owner, Actor* wearer);
};

class Confuser : public Pickable {
public:
	int nbTurns;
	float range;
	Confuser(int nbTurns, float range);
	bool use(Actor* owner, Actor* wearer);
};