#pragma once
#include <list>

//actor can contain other actors
class Container : public Persistent {
public:
	int size; //max number of actors 0 unlimited
	std::list<std::unique_ptr<Actor>> inventory;//actors in container

	Container(int size);
	bool add(std::unique_ptr<Actor> actor);//addd actor to container
	void remove(Actor* actor); //remove actor from container
	void save(TCODZip& zip); //save container and contained actors to zip
	void load(TCODZip& zip); //load container from zip
};