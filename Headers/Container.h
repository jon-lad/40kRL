#pragma once
#include <list>

class Container : public Persistent {
public:
	int size; //max number of actors 0 unlimited
	std::list<std::unique_ptr<Actor>> inventory;

	Container(int size);
	bool add(std::unique_ptr<Actor> actor);
	void remove(Actor* actor);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
};