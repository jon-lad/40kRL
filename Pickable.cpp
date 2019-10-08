#include <memory>
#include "main.h"

bool Pickable::pick(std::unique_ptr<Actor> owner, Actor* wearer) {
	if (wearer->container && wearer->container->add(std::move(owner))) {
		return true;
	}
	return false;
}

bool Pickable::use(Actor *owner, Actor* wearer) {
	if (wearer->container) {
		for (auto i = wearer->container->inventory.begin(); i != wearer->container->inventory.end();) {
			if (i->get() == owner) {
				i = wearer->container->inventory.erase(i);
				return true;
			}
			else { ++i; }
		}
	}
	return false;
}

Healer::Healer(float amount) :amount{ amount }{}

bool Healer::use(Actor* owner, Actor* wearer) {
	if (wearer->destructible) {
		float amountHealed = wearer->destructible->heal((int)amount);
		if (amountHealed > 0) {
			engine.gui->message(TCODColor::lightGrey, "%s healed %g HP.", wearer->name, amountHealed);
			return Pickable::use(std::move(owner), wearer);
		}

	}
	return false;
}