#include <memory>
#include <list>
#include "main.h"

Container::Container(int size) : size{ size } {}

bool Container::add(std::unique_ptr<Actor> actor)
{
	if (size > 0 && static_cast<int>(inventory.size()) >= size) {
		return false; // inventory full
	}
	inventory.emplace_back(std::move(actor));
	return true;
}

void Container::remove(Actor* actor)
{
	for (auto i = inventory.begin(); i != inventory.end(); ) {
		if (i->get() == actor) {
			i = inventory.erase(i);
		} else {
			++i;
		}
	}
}
