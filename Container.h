#pragma once
#include <list>

class Container {
public:
	int size; //max number of actors 0 unlimited
	std::list<std::unique_ptr<Actor>> inventory;

	Container(int size);
	bool add(std::unique_ptr<Actor> actor);
	void remove(Actor* actor);
};