
#include <memory>
#include <list>
#include "main.h"

Container::Container(int size) :size{ size } {}

bool Container::add(std::unique_ptr<Actor> actor) {
	if (inventory.size() == size) {
		return false;
	}
	inventory.emplace_back(std::move(actor));
	return true;
}

void Container::remove(Actor* actor) {
	for (auto &item : inventory) {
		auto i = inventory.begin();
		auto e = inventory.end();
		while (i != e) {
			if (i->get() == actor) {
				i = inventory.erase(i);
			}
			else { i++; }
		}
	}
}
