#include <memory>
#include <string>
#include <sstream>
#include <sol/sol.hpp>
#include "main.h"

// ─── Pickable ────────────────────────────────────────────────────────────────

Pickable::Pickable(std::unique_ptr<TargetSelector> selector, std::unique_ptr<Effect> effect)
	: selector{ std::move(selector) }, effect{ std::move(effect) }
{}

bool Pickable::pick(std::unique_ptr<Actor> owner, Actor* wearer)
{
	if (!wearer->container) return false;

	// Check carrying capacity
	float totalWeight = 0.0f;
	for (const auto& item : wearer->container->inventory) {
		totalWeight += item->getWeight();
	}
	float itemWeight = owner->getWeight();
	if (engine.carryingCapacity > 0 && totalWeight + itemWeight > engine.carryingCapacity) {
		engine.gui->message(Colors::uiText, "Too heavy to carry.");
		return false;
	}

	if (wearer->container->add(std::move(owner))) {
		return true;
	}
	return false; // inventory full
}

bool Pickable::use(Actor* owner, Actor* wearer)
{
	if (!effect) return false; // safety guard for equipment items

	TCODList<Actor*> targets;
	if (selector) {
		selector->selectTargets(wearer, targets);
	} else {
		targets.push(wearer); // no selector means self-targeting
	}

	bool anySucceeded = false;
	for (Actor* target : targets) {
		if (effect->applyTo(target)) {
			anySucceeded = true;
		}
	}

	if (anySucceeded) {
		// Remove the consumed item from the wearer's inventory.
		if (wearer->container) {
			for (auto i = wearer->container->inventory.begin();
				 i != wearer->container->inventory.end(); )
			{
				if (i->get() == owner) {
					i = wearer->container->inventory.erase(i);
				} else {
					++i;
				}
			}
		}
	}
	return anySucceeded;
}

void Pickable::drop(Actor* owner, Actor* wearer)
{
	if (!wearer->container) { return; }

	owner->setX(wearer->getX());
	owner->setY(wearer->getY());

	std::stringstream msg;
	msg << wearer->name << " drops a " << owner->name << ".";
	engine.gui->message(Colors::uiText, msg.str());

	for (auto i = wearer->container->inventory.begin();
		 i != wearer->container->inventory.end(); )
	{
		if (i->get() == owner) {
			engine.actors.emplace_front(std::move(*i));
			i = wearer->container->inventory.erase(i);
		} else {
			++i;
		}
	}
}

// ─── HealthEffect ────────────────────────────────────────────────────────────

HealthEffect::HealthEffect(float amount, std::string_view message, const TCODColor& textCol)
	: amount{ amount }, message{ message }, textCol{ textCol }
{}

bool HealthEffect::applyTo(Actor* actor)
{
	if (!actor->destructible) { return false; }

	if (amount > 0) {
		// Positive amount: heal via Lua script so designers can tune values without recompiling.
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		auto destructibleType = lua.new_usertype<Destructible>("Destructible",
			sol::constructors<Destructible(float, float, std::string, int)>());
		auto actorType = lua.new_usertype<Actor>("Actor",
			sol::constructors<Actor(int, int, int, std::string, TCODColor)>());
		actorType["name"]          = &Actor::name;
		actorType["destructible"]  = &Actor::destructible;
		destructibleType["heal"]   = &Destructible::heal;
		lua["act"]    = actor;
		lua["amount"] = amount;

		try {
			const float pointsHealed = lua.script_file("Scripts/Effects.lua");
			lua.collect_garbage();
			if (pointsHealed > 0) {
				if (!message.empty()) {
					engine.gui->message(textCol, message, actor->name, pointsHealed);
				}
				return true;
			}
		} catch (const sol::error& e) {
			engine.gui->message(Colors::damage, "Lua error in Effects.lua: #", std::string(e.what()));
			lua.collect_garbage();
		}
		return false;

	} else {
		// Negative amount: direct damage.
		const float effectiveDamage = -amount - actor->destructible->defense;
		if (!message.empty() && effectiveDamage > 0) {
			engine.gui->message(textCol, message, actor->name, effectiveDamage);
		}
		return actor->destructible->takeDamage(actor, -amount) > 0;
	}
}

// ─── AiChangeEffect ──────────────────────────────────────────────────────────

AiChangeEffect::AiChangeEffect(std::unique_ptr<TemporaryAi> newAi,
	std::string_view message, const TCODColor& textCol)
	: newAi{ std::move(newAi) }, message{ message }, textCol{ textCol }
{}

bool AiChangeEffect::applyTo(Actor* actor)
{
	newAi->applyToActor(std::move(newAi), actor);
	if (!message.empty()) {
		engine.gui->message(textCol, message, actor->name);
	}
	return true;
}

// ─── TargetSelector ──────────────────────────────────────────────────────────

TargetSelector::TargetSelector(SelectorType type, float range)
	: type{ type }, range{ range }
{}

void TargetSelector::selectTargets(Actor* wearer, TCODList<Actor*>& list)
{
	switch (type) {

	case SelectorType::SELF:
		list.push(wearer);
		break;

	case SelectorType::CLOSEST_MONSTER: {
		Actor* closest = engine.getClosestMonster(wearer->getX(), wearer->getY(), range);
		if (closest) { list.push(closest); }
		break;
	}

	case SelectorType::SELECTED_MONSTER: {
		int x, y;
		engine.gui->message(Colors::cyan, "Left-click to select an enemy, or right-click to cancel.");
		if (engine.pickAtTile(&x, &y, range)) {
			Actor* target = engine.getActorAt(x, y);
			if (target) { list.push(target); }
		}
		break;
	}

	case SelectorType::WEARER_RANGE:
		for (const auto& actorPtr : engine.actors) {
			Actor* actor = actorPtr.get();
			if (actor->destructible && !actor->destructible->isDead()
				&& actor->getDistance(wearer->getX(), wearer->getY()) <= range)
			{
				list.push(actor);
			}
		}
		break;

	case SelectorType::SELECTED_RANGE: {
		int x, y;
		engine.gui->message(Colors::cyan, "Left-click to select a tile, or right-click to cancel.");
		if (engine.pickAtTile(&x, &y)) {
			for (const auto& actorPtr : engine.actors) {
				Actor* actor = actorPtr.get();
				if (actor->destructible && !actor->destructible->isDead()
					&& actor->getDistance(x, y) <= range)
				{
					list.push(actor);
				}
			}
		}
		break;
	}
	}

	if (list.isEmpty()) {
		engine.gui->message(Colors::uiText, "No enemy is close enough.");
	}
}
