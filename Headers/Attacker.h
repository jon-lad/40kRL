#pragma once

//actor can attack
class Attacker : public Persistent {
public:

	double power; //hit points given

	Attacker(double power);
	void attack(Actor* owner, Actor* target); // attack target
	void save(TCODZip& zip); //save attacker data to zip
	void load(TCODZip& zip); //load attacker data to zip

};