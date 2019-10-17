
#include <sstream>
#include "main.h"

Attacker::Attacker(float power) : power{ power } {};

void Attacker::attack(Actor* owner, Actor* target) {
	if (target->destructible && !target->destructible->isDead()) {
		
		if (power - target->destructible->defense > 0) {
			std::stringstream ss;
			ss << owner->name << " attacks " << target->name << " for "<< power - target->destructible->defense;
			engine.gui->message(owner == engine.player ? TCOD_red: TCODColor::lightGrey, ss.str() );
		}
		else {
			std::stringstream ss;
			ss << owner->name << " attacks " << target->name << " but it has no effect!";
			engine.gui->message(TCODColor::lightGrey,ss.str());
		}
		target->destructible->takeDamage(target, power); 
	}
	else {
		std::stringstream ss;
		ss << owner->name << " attacks " << target->name << " in vain.";
		engine.gui->message(TCODColor::lightGrey, ss.str());
	}
}

