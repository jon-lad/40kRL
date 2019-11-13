#pragma once

class Attacker : public Persistent {
public:

	float power; //hit points given

	Attacker(float power);
	void attack(Actor* owner, Actor* target);
	void save(TCODZip& zip);
	void load(TCODZip& zip);

};