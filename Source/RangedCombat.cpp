#include <sstream>
#include <algorithm>
#include <numeric>
#include <random>
#include "main.h"
#include "RangedCombat.h"
#include "CriticalEffects.h"
#include "DiceRoller.h"

namespace {
	int defaultRollD100() {
		static std::mt19937 eng{ std::random_device{}() };
		static std::uniform_int_distribution<int> dist(1, 100);
		return dist(eng);
	}

	int defaultRollDie(int sides) {
		static std::mt19937 eng{ std::random_device{}() };
		std::uniform_int_distribution<int> dist(1, sides);
		return dist(eng);
	}
}

namespace RangedCombat {

RangedResult resolveCharacterAttack(const RangedContext& ctx) {
	RangedResult result;

	// ── Determine RNG functions ──
	auto roll100 = ctx.rollD100 ? ctx.rollD100 : defaultRollD100;
	auto rollDie = ctx.rollDie ? ctx.rollDie : defaultRollDie;

	// ── Compute Effective BS ──
	const int baseBS = ctx.shooter->characteristics->get(CharId::BS);
	// Sum attacker modifiers (from attacker component, if present)
	int modSum = 0;
	if (ctx.shooter->attacker) {
		modSum = std::accumulate(
			ctx.shooter->attacker->modifiers.begin(),
			ctx.shooter->attacker->modifiers.end(), 0);
	}
	const int effectiveBS = std::max(1, std::min(99, baseBS + modSum));

	// ── Roll d100 ──
	const int roll = roll100();

	// ── Classify hit/miss ──
	if (roll > effectiveBS) {
		// Miss
		result.hit = false;
		result.doF = std::max(0, (roll - effectiveBS) / 10);
		return result;
	}

	// ── Hit ──
	result.hit = true;
	result.doS = std::max(0, (effectiveBS - roll) / 10);

	// ── Determine hit location via digit reversal ──
	result.location = HitLocationTable::resolve(roll);

	// ── Dodge Test (stub for task 2.3 — full implementation later) ──
	{
		const int targetAg = ctx.target->characteristics->get(CharId::Ag);
		const int dodgeRoll = roll100();
		if (dodgeRoll <= targetAg) {
			const int dodgeDoS = std::max(0, (targetAg - dodgeRoll) / 10);
			const int hitsNegated = 1 + dodgeDoS;
			(void)hitsNegated; // used for multi-hit; single shot always negated
			result.dodged = true;
			return result;
		}
	}
	// NOTE: No Parry test against ranged attacks (Requirement 5.5)

	// ── Damage Calculation (stub for task 2.4 — full implementation later) ──
	{
		// Raw damage = weapon dice roll only (no Strength Bonus for ranged)
		const int rawDamage = DiceRoller::roll(ctx.weaponStats.damageDice, rollDie);

		// Effective armour at hit location
		int locArmour = 0;
		if (ctx.target->equipment) {
			locArmour = ctx.target->equipment->getArmourAtLocation(result.location);
		}
		const int effectiveArmour = std::max(0, locArmour - ctx.weaponStats.penetration);

		// Toughness Bonus
		const int tb = ctx.target->characteristics->bonus(CharId::T);

		// Final damage
		result.finalDamage = std::max(0, rawDamage - effectiveArmour - tb);
	}

	return result;
}

RangedResult resolveDestructibleAttack(const RangedContext& ctx) {
	RangedResult result;

	// ── Determine RNG functions ──
	auto rollDie = ctx.rollDie ? ctx.rollDie : defaultRollDie;

	// ── Auto-hit (no roll, no dodge) ──
	result.hit = true;
	result.location = HitLocation::BODY;

	// ── Damage = dice roll only (no armour, no TB) ──
	result.finalDamage = DiceRoller::roll(ctx.weaponStats.damageDice, rollDie);

	return result;
}

void resolve(Actor* shooter, Actor* target,
             std::function<int()> rollD100,
             std::function<int(int)> rollDie) {

	// ── Validate target is alive ──
	if (!target->destructible || target->destructible->isDead()) {
		engine.gui->message(Colors::uiText, "# fires at # in vain.",
			shooter->name, target->name);
		return;
	}

	// ── Get weapon stats from equipped weapon ──
	RangedStats weaponStats;
	int currentAmmo = 0;
	Equippable* weaponEquippable = nullptr;

	if (shooter->equipment) {
		Actor* weaponItem = shooter->equipment->getSlot(EquipmentSlot::WEAPON);
		if (weaponItem && weaponItem->equippable && weaponItem->equippable->rangedStats) {
			weaponEquippable = weaponItem->equippable.get();
			weaponStats = *weaponEquippable->rangedStats;
			currentAmmo = weaponEquippable->currentAmmo;
		}
	}

	// ── Check ammo ──
	if (currentAmmo <= 0) {
		engine.gui->message(Colors::uiText, "#'s weapon clicks empty.",
			shooter->name);
		return;
	}

	// ── Build context ──
	RangedContext ctx;
	ctx.shooter = shooter;
	ctx.target = target;
	ctx.weaponStats = weaponStats;
	ctx.currentAmmo = currentAmmo;
	ctx.rollD100 = rollD100;
	ctx.rollDie = rollDie;

	// ── Log attack initiation ──
	engine.gui->message(Colors::uiText, "# fires at #.",
		shooter->name, target->name);

	// ── Dispatch based on target type ──
	RangedResult result;
	if (target->characteristics) {
		result = resolveCharacterAttack(ctx);
	} else {
		result = resolveDestructibleAttack(ctx);
	}

	// ── Ammo consumption ──
	const int ammoToConsume = std::min(weaponStats.rateOfFire, currentAmmo);
	result.ammoConsumed = ammoToConsume;
	if (weaponEquippable) {
		weaponEquippable->currentAmmo -= ammoToConsume;
	}

	// ── Log results and apply wounds ──
	if (target->characteristics) {
		// Character attack path
		if (!result.hit) {
			engine.gui->message(Colors::uiText, "# misses #.",
				shooter->name, target->name);
		} else if (result.dodged) {
			engine.gui->message(Colors::uiText, "Hit! (# DoS) — #.",
				result.doS, HitLocationTable::name(result.location));
			engine.gui->message(Colors::uiText, "# dodges 1 hit(s).",
				target->name);
		} else {
			engine.gui->message(Colors::damage, "Hit! (# DoS) — #.",
				result.doS, HitLocationTable::name(result.location));

			if (result.finalDamage <= 0) {
				engine.gui->message(Colors::uiText, "...but it has no effect!");
			} else {
				// Apply wound
				const float currentHp = target->destructible->hp;
				target->destructible->hp -= static_cast<float>(result.finalDamage);

				if (target->destructible->hp <= 0) {
					// Critical hit
					const int critMagnitude = result.finalDamage - static_cast<int>(currentHp);
					const auto critEffect = CriticalEffects::resolve(result.location, critMagnitude);

					engine.gui->message(Colors::damage,
						"Critical Hit on #! #",
						HitLocationTable::name(result.location), critEffect.description);

					target->destructible->die(target);
					result.targetKilled = true;
				} else {
					engine.gui->message(Colors::damage, "# deals # damage to #'s #.",
						shooter->name, result.finalDamage, target->name,
						HitLocationTable::name(result.location));
				}
			}
		}
	} else {
		// Destructible attack path
		if (result.finalDamage > 0) {
			target->destructible->takeDamage(target, static_cast<float>(result.finalDamage));
			engine.gui->message(Colors::damage, "# shoots # for # damage.",
				shooter->name, target->name, result.finalDamage);

			if (target->destructible->isDead()) {
				result.targetKilled = true;
			}
		}
	}
}

} // namespace RangedCombat
