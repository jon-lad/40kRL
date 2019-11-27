
#include <sstream>
#include "main.h"

Attacker::Attacker(double power) : power{ power } {};

void Attacker::attack(Actor* owner, Actor* target) {
	if (target->destructible && !target->destructible->isDead()) {
		
		if (power - target->destructible->defense > 0) {
			engine.gui->message(owner == engine.player ? TCODColor::red : TCODColor::lightGrey, "# attacks # for # damage.", std::move(owner->name), std::move(target->name), std::move(power - target->destructible->defense));
		}
		else {
			engine.gui->message(TCODColor::lightGrey, "# attacks # but it has no effect!", std::move(owner->name), std::move(target->name));
		}
		target->destructible->takeDamage(target, power); 
	}
	else {
		engine.gui->message(TCODColor::lightGrey, "# attacks # in vain.", std::move(owner->name),std::move(target->name));
	}
}

