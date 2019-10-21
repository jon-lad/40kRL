#include <memory>
#include <sstream>
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
void Pickable::drop(Actor* owner, Actor* wearer) {
	if (wearer->container) {
		owner->setX(wearer->getX());
		owner->setY(wearer->getY());
		std::stringstream ss;
		ss << wearer->name << " drops a " << owner->name << ".";
		engine.gui->message(TCOD_light_grey, ss.str());
		
		for (auto i = wearer->container->inventory.begin(); i != wearer->container->inventory.end(); ) {
			if (i->get() == owner) {
				engine.actors.emplace_front(std::move(*i));
				i = wearer->container->inventory.erase(i);
			}
			else { ++i; }
		}
	}
}
/*Pickable items below*/

/*Healer*/
Healer::Healer(float amount) :amount{ amount }{}

bool Healer::use(Actor* owner, Actor* wearer) {
	if (wearer->destructible) {
		float amountHealed = wearer->destructible->heal((int)amount);
		if (amountHealed > 0) {
			std::stringstream ss;
			ss << wearer->name << " healed " << amountHealed << " Hp.";
			engine.gui->message(TCODColor::lightGrey, ss.str());
			return Pickable::use(std::move(owner), wearer);
		}

	}
	return false;
}

/*Lightning bolt*/

LightningBolt::LightningBolt(float range, float damage) :range{ range }, damage{ damage }{}

bool LightningBolt::use(Actor* owner, Actor* wearer) {
	Actor* closestMonster = engine.getClosestMonster(wearer->getX(), wearer->getY(), range);
	if (!closestMonster) {
		engine.gui->message(TCODColor::lightGrey, "No monster is close enough to strike!");
		return false;
	}
	//hit closest monster for <damage> hit points
	std::stringstream ss;
	ss << "A lightningbolt hit the " << closestMonster->name << " with loud thunder!\nThe damage is " << closestMonster->destructible->takeDamage(closestMonster, damage) << " hit points.";
	engine.gui->message(TCOD_light_blue, ss.str());
	return Pickable::use(owner, wearer);
}
/*Fireball*/
Fireball::Fireball(float range, float damage) :LightningBolt(range, damage) {}

bool Fireball::use(Actor* owner, Actor* wearer) {
	engine.gui->message(TCOD_cyan, "Left click on tile for the fireball \n or right click to cancel.");
	int x, y;
	if (!engine.pickAtTile(&x, &y)) {
		return false;
	}
	//Burn everything in <range> (including Player)
	std::stringstream ss;
	ss << "The fireball explodes burning everything in " << range << " tiles.";
	engine.gui->message(TCOD_orange, ss.str());
	ss.str(std::string());
	for (auto i = engine.actors.begin(); i != engine.actors.end(); ++i) {
		if (i->get()->destructible && !i->get()->destructible->isDead() && i->get()->getDistance(x, y) <= range) {
			ss << "The " << i->get()->name << " gets burned for " << damage << " hit points.";
			engine.gui->message(TCOD_orange, ss.str());
			i->get()->destructible->takeDamage(i->get(), damage);
		}
	}
	return Pickable::use(owner, wearer);
}

//Confuser

Confuser::Confuser(int nbTurns, float range):nbTurns{nbTurns},range{range}{}

bool Confuser::use(Actor* owner, Actor* wearer) {
	engine.gui->message(TCOD_cyan, "Left click an enemy to confuse it \n or right click to cancel");
	int x, y;
	if (!engine.pickAtTile(&x, &y, range)) {
		return false;
	}
	Actor* actor = engine.getActor(x, y);
	if (!actor) {
		return false;
	}
	//confuse the monster for <nbTurns> turns
	std::unique_ptr<ConfusedMonsterAi> confusedAi = std::make_unique<ConfusedMonsterAi>(nbTurns, std::move(actor->ai));
	actor->ai = std::move(confusedAi);
	std::stringstream ss;
	ss << "The eyes of the " << actor->name << " look vacant \nas he starts to stumble around!";
	engine.gui->message(TCOD_light_green, ss.str());
	return Pickable::use(owner, wearer);
}