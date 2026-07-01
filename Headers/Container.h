#pragma once
#include <list>

// Gives an actor an inventory: a list of owned items (other Actors).
// size == 0 means unlimited capacity.
class Container : public Persistent {
public:
	int size; // maximum number of items (0 = unlimited)
	std::list<std::unique_ptr<Actor>> inventory;

	explicit Container(int size);

	// Adds the actor to the inventory. Returns false if the inventory is full.
	bool add(std::unique_ptr<Actor> actor);

	// Removes the actor from the inventory by pointer identity.
	void remove(Actor* actor);

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};