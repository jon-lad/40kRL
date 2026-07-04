
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

// ─── MonsterDestructible ─────────────────────────────────────────────────────

MonsterDestructible::MonsterDestructible(float maxHp, float defense, std::string_view corpseName, int xp)
	: Destructible(maxHp, defense, corpseName, xp)
{}

void MonsterDestructible::die(Actor* owner)
{
	std::stringstream ss;
	ss << owner->name << " is dead! You gain " << xp << " xp.";
	engine.gui->message(Colors::uiText, ss.str());
	engine.player->destructible->xp += xp;
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
