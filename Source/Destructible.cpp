
#include <sstream>
#include "main.h"

Destructible::Destructible(float maxHp, float defense, std::string_view corpseName, int xp) : maxHp{ maxHp }, hp{ maxHp }, defense{ defense }, corpseName{ corpseName }, xp{ xp } {
	
}

float Destructible::takeDamage(Actor* owner, float damage) {
	damage -= defense;
	if (damage > 0) {
		hp -= damage;
		if (hp <= 0) {
			die(owner);
		}
	}
	else { damage = 0;
	}
	return damage;
}

float Destructible::heal(int amount) {
	hp += amount;
	if (hp > maxHp) {
		amount -= (int)hp - (int)maxHp;
		hp = maxHp;
	}
	return (float)amount;
}

void Destructible::die(Actor* owner) {
	// transform the actor into a corpse!
	owner->setCh('%');
	owner->ai.reset();
	owner->setColor(TCOD_dark_red);
	owner->name = corpseName;
	owner->blocks = false;
	// make sure corpses are drawn before living actors
	engine.sendToBack(owner);
}


MonsterDestructible::MonsterDestructible(float maxHp, float defense, std::string_view corpseName, int xp) :
	Destructible(maxHp, defense, corpseName, xp) {
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, std::string_view corpseName, int xp) :
	Destructible(maxHp, defense, corpseName, xp) {
}

void MonsterDestructible::die(Actor* owner) {
	// transform it into a nasty corpse! it doesn't block, can't be
	// attacked and doesn't move
	std::stringstream ss;
	ss << owner->name << " is dead! You gain " << xp << " xp.";
	engine.gui->message(TCOD_light_grey, ss.str());
	engine.player->destructible->xp += xp;
	Destructible::die(owner);
}

void PlayerDestructible::die(Actor* owner) {
	engine.gui->message(TCOD_red,"You died!");
	Destructible::die(owner);
	engine.gameStatus = Engine::DEFEAT;
}

