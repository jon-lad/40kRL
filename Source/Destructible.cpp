
#include <sstream>
#include "main.h"

Destructible::Destructible(float maxHp, float defense, std::string_view corpseName, int xp)
	: maxHp{ maxHp }, hp{ maxHp }, defense{ defense }, corpseName{ corpseName }, xp{ xp }
{}

float Destructible::takeDamage(Actor* owner, float damage)
{
	float equipDefenseBonus = owner->equipment ? owner->equipment->getTotalDefenseModifier() : 0.0f;
	damage -= (defense + equipDefenseBonus);
	if (damage > 0) {
		hp -= damage;
		if (hp <= 0) { die(owner); }
	} else {
		damage = 0;
	}
	return damage;
}

float Destructible::heal(float amount)
{
	hp += amount;
	if (hp > maxHp) {
		amount -= static_cast<int>(hp) - static_cast<int>(maxHp);
		hp = maxHp;
	}
	return static_cast<float>(amount);
}

void Destructible::die(Actor* owner)
{
	owner->setGlyph('%');
	owner->ai.reset();
	owner->setColor(Colors::darkRed);
	owner->name   = corpseName;
	owner->blocks = false;
	engine.sendToBack(owner); // draw corpses beneath living actors
}

// ─── Loot Drop ───────────────────────────────────────────────────────────────

void dropEnemyEquipment(Actor* enemy)
{
	if (!enemy || !enemy->equipment) {
		return;
	}

	TCODRandom* rng = TCODRandom::getInstance();
	const float dropChance = enemy->equipment->dropChance;
	const auto& slots = enemy->equipment->getSlots();

	for (const auto* item : slots) {
		if (!item || !item->equippable) {
			continue;
		}

		// Roll against dropChance
		float roll = rng->getFloat(0.0f, 1.0f);
		if (roll >= dropChance) {
			continue; // Drop failed, discard item
		}

		// Create a Pickable_Actor on the map copying the equipped item's properties
		auto droppedItem = std::make_unique<Actor>(
			enemy->getX(), enemy->getY(),
			item->getGlyph(), item->name, item->getColor());
		droppedItem->blocks = false;
		droppedItem->fovOnly = true;

		// Add Pickable component with weight and value from the equippable
		droppedItem->pickable = std::make_shared<Pickable>(nullptr, nullptr);
		droppedItem->pickable->weight = item->equippable->weight;
		droppedItem->pickable->value  = item->equippable->value;

		// Add Equippable component (same slot, same stat modifiers, same weight, value)
		droppedItem->equippable = std::make_shared<Equippable>(
			item->equippable->slot,
			item->equippable->modifiers,
			item->equippable->weight,
			item->equippable->value);
		droppedItem->equippable->meleeStats = item->equippable->meleeStats;
		droppedItem->equippable->armourProfile = item->equippable->armourProfile;

		// Add to engine actors list and send to back (draw beneath living actors)
		Actor* droppedPtr = droppedItem.get();
		engine.actors.push_back(std::move(droppedItem));
		engine.sendToBack(droppedPtr);
	}
}

// ─── MonsterDestructible ─────────────────────────────────────────────────────

MonsterDestructible::MonsterDestructible(float maxHp, float defense, std::string_view corpseName, int xp)
	: Destructible(maxHp, defense, corpseName, xp)
{}

void MonsterDestructible::die(Actor* owner)
{
	// Drop equipped loot before the actor becomes a corpse
	dropEnemyEquipment(owner);

	std::stringstream ss;
	ss << owner->name << " is dead! You gain " << xp << " xp.";
	engine.gui->message(Colors::uiText, ss.str());
	engine.player->destructible->xp += xp;

	// Add XP to career progression pool and evaluate rank-up.
	if (engine.player->career) {
		engine.player->career->xpPool += xp;

		// Find the matching career template for rank evaluation.
		const CareerTemplate* careerTpl = nullptr;
		for (const auto& ct : engine.careerTemplates) {
			if (ct.name == engine.player->career->careerName) {
				careerTpl = &ct;
				break;
			}
		}

		if (careerTpl) {
			int oldRank = engine.player->career->currentRank;
			engine.player->career->evaluateRankUp(*careerTpl);
			int newRank = engine.player->career->currentRank;

			if (newRank > oldRank) {
				// Find the rank title for the new rank.
				std::string rankTitle;
				for (const auto& rd : careerTpl->ranks) {
					if (rd.rankNumber == newRank) {
						rankTitle = rd.rankTitle;
						break;
					}
				}
				engine.gui->message(Colors::yellow,
					"You have advanced to Rank #: #!",
					newRank, rankTitle);
			}
		}
	}

	Destructible::die(owner);
}

// ─── PlayerDestructible ──────────────────────────────────────────────────────

PlayerDestructible::PlayerDestructible(float maxHp, float defense, std::string_view corpseName, int xp)
	: Destructible(maxHp, defense, corpseName, xp)
{}

void PlayerDestructible::die(Actor* owner)
{
	engine.gui->message(Colors::damage, "You died!");
	Destructible::die(owner);
	engine.gameStatus = Engine::DEFEAT;
}
