#include <memory>
#include <string>
#include <sstream>
#include <sol/sol.hpp>
#include "main.h"

Pickable::Pickable(std::unique_ptr<TargetSelector> selector, 
	std::unique_ptr<Effect> effect, Equipment::EquipLocation equpiLocation)
	:selector{ std::move(selector) }, effect{ std::move(effect) }, equipLocation{ equipLocation }
{}

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

//Equip item and apply effect
bool Pickable::equip(std::unique_ptr<Actor> owner, Actor* wearer) {
	//if wearer can equp items and this item equipable
	if (wearer->equipment && wearer->equipment->equip(std::move(owner), wearer))
	{ 
		//apply effect as defined in Effects.lua, canceled on removal
		owner->pickable->effect->applyTo(wearer); 
		return true;
	}
	return false;
}
/*Effects below*/

/*Health*/
HealthEffect::HealthEffect(float amount, std::string_view message, const TCODColor& textCol) :amount{ amount }, message{ std::string(message) }, textCol{ textCol }{}

bool HealthEffect::applyTo(Actor* actor) {
	if (!actor->destructible) return false;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	//Setting Actor as userdatata so it can be used in lua scripts
	sol::usertype<Actor> actor_type = lua.new_usertype<Actor>
		("Actor", sol::constructors <Actor(int, int, int, std::string, TCODColor)>());
	
	
	actor_type["name"]         = &Actor::name;
	actor_type["destructible"] = &Actor::destructible;
	actor_type["ai"]           = &Actor::ai;
	
	//setting destructible as user type to use in lua scripts
	sol::usertype<Destructible> destructible_type = lua.new_usertype<Destructible>
		("Destructible", sol::constructors<Destructible(float, float, std::string, int)>());

	destructible_type["heal"] = &Destructible::heal;
	
	//passing arguments to lua script
	lua["act"] = actor;
	lua["amount"] = amount;
	

	if(amount > 0)
	{
		float pointsHealed = lua.script_file("Scripts/Effects.lua");
		
		if (pointsHealed > 0) {
			if (!message.empty()) {
				engine.gui->message(textCol, message, std::move(actor->name), std::move(pointsHealed));
			}
			lua.collect_garbage();
			return true;
		}
	}
	else {
		if (!message.empty() && (-amount - actor->destructible->defense) > 0) {
			engine.gui->message(textCol, message, std::move(actor->name),std::move(-amount - actor->destructible->defense));
		}
		if (actor->destructible->takeDamage(actor, -amount) > 0) {
			lua.collect_garbage();
			return true;
		}

	}
	lua.collect_garbage();
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