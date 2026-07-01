
#include <sstream>
#include "main.h"

Attacker::Attacker(float power) : power{ power } {}

void Attacker::attack(Actor* owner, Actor* target)
{
	if (target->destructible && !target->destructible->isDead()) {
		const float effectiveDamage = power - target->destructible->defense;
		if (effectiveDamage > 0) {
			const TCODColor messageColor = (owner == engine.player) ? TCODColor::red : TCODColor::lightGrey;
			engine.gui->message(messageColor, "# attacks # for # damage.",
				owner->name, target->name, effectiveDamage);
		} else {
			engine.gui->message(TCODColor::lightGrey, "# attacks # but it has no effect!",
				owner->name, target->name);
		}
		target->destructible->takeDamage(target, power);
	} else {
		engine.gui->message(TCODColor::lightGrey, "# attacks # in vain.",
			owner->name, target->name);
	}
}
