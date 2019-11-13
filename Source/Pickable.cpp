#include <memory>
#include <string>
#include <sstream>
#include <sol/sol.hpp>
#include "main.h"

Pickable::Pickable(std::unique_ptr<TargetSelector> selector, std::unique_ptr <Effect> effect) :selector{ std::move(selector) }, effect{ std::move(effect) }{ }

bool Pickable::pick(std::unique_ptr<Actor> owner, Actor* wearer) {
	if (wearer->container && wearer->container->add(std::move(owner))) {
		return true;
	}
	return false;
}

bool Pickable::use(Actor* owner, Actor* wearer) {
	TCODList<Actor*> list;
	if (selector) {
		selector->selectTargets(wearer, list);
	}
	else {
		list.push(wearer);
	}
	bool succeed = false;
	for (auto& actor: list) {
		if (effect->applyTo(actor)) {
			succeed = true;
		}
	}
	if(succeed){
		if (wearer->container) {
			for (auto i = wearer->container->inventory.begin(); i != wearer->container->inventory.end();) {
				if (i->get() == owner) {
					i = wearer->container->inventory.erase(i);
				}
				else { ++i; }
			}
		}
	}
	return succeed;
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
/*Effects below*/

/*Health*/
HealthEffect::HealthEffect(float amount, std::string_view message, const TCODColor& textCol) :amount{ amount }, message{ std::string(message) }, textCol{ textCol }{}

bool HealthEffect::applyTo(Actor* actor) {
	sol::state lua;
	lua.open_libraries(sol::lib::base);
	if (!actor->destructible) return false;
	if(amount > 0)
	{
		float pointsHealed = actor->destructible->heal((int)amount);
		sol::usertype<Actor> actor_type = lua.new_usertype<Actor>("actorT", sol::constructors <Actor(int , int, int, std::string, TCODColor)>());
		actor_type["name"] = &Actor::name;
		lua["act"] = actor;

		lua.script_file("Scripts/Effects.lua");
		if (pointsHealed > 0) {
			if (!message.empty()) {
				engine.gui->message(textCol, message, std::move(actor->name), std::move(pointsHealed));
			}
			return true;
		}
	}
	else {
		if (!message.empty() && (-amount - actor->destructible->defense) > 0) {
			engine.gui->message(textCol, message, std::move(actor->name),std::move(-amount - actor->destructible->defense));
		}
		if (actor->destructible->takeDamage(actor, -amount) > 0) {
			return true;
		}

	}
	return false;
}


//Change AI
AiChangeEffect::AiChangeEffect(std::unique_ptr<TemporaryAi> newAi, std::string_view message, const TCODColor& textCol):
	message{ std::string(message) }, textCol{ textCol } {
	this->newAi = std::move(newAi);
}

bool AiChangeEffect::applyTo(Actor* actor) {
	newAi->applyTo(actor);
	if (!message.empty()) {
		engine.gui->message(textCol, message, std::move(actor->name));
	}
	return true;
}

//Target Selector


TargetSelector::TargetSelector(SelectorType type, float range):type { type }, range{ range } {}

void TargetSelector::selectTargets(Actor* wearer, TCODList<Actor*>& list) {
	switch (type) {
		case SelectorType::SELF: {
			list.push(wearer);
		}
		break;
		case SelectorType::CLOSEST_MONSTER:
		{
			Actor* closestMonster = engine.getClosestMonster(wearer->getX(), wearer->getY(), range);
			if (closestMonster) {
				list.push(closestMonster);
			}
		}
		break;
		case SelectorType::SELECTED_MONSTER:
		{
			int x, y;
			engine.gui->message(TCOD_cyan, "Left click to select an an enemy,\nor right click to cancel.");
			if (engine.pickAtTile(&x, &y, range)) {
				Actor* actor = engine.getActor(x, y);
				if (actor) {
					list.push(actor);
				}
			}

		}
		break;
		case SelectorType::WEARER_RANGE:
		{
			for (auto i = engine.actors.begin(); i != engine.actors.end(); ++i) {
				Actor* actor = i->get();
				if (actor->destructible && !actor->destructible->isDead() && actor->getDistance(wearer->getX(),wearer->getY())<= range) {
					list.push(actor);
				}
			} 
		}
		break;
		case SelectorType::SELECTED_RANGE:
		{
			int x, y;
			engine.gui->message(TCOD_cyan, "Left-click to select an an tile,\nor right-click to cancel.");
			if (engine.pickAtTile(&x, &y)) {
				for (auto i = engine.actors.begin(); i != engine.actors.end(); ++i) {
					Actor* actor = i->get();
					if (actor->destructible && !actor->destructible->isDead() && actor->getDistance(x, y) <= range) {
						list.push(actor);
					}
				}
			}
		}
		break;
	}
	if (list.isEmpty()) {
		engine.gui->message(TCOD_light_grey, "No enemy is close enough.");
	}
}