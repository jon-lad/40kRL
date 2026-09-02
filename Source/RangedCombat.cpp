#include <sstream>
#include <algorithm>
#include <numeric>
#include <random>
#include "main.hpp"
#include "RangedCombat.hpp"
#include "CriticalEffects.hpp"
#include "DiceRoller.hpp"
#include "WeaponTypes.hpp"
#include "ReactionResolver.hpp"

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
	// Aim bonus (from ActionBudget)
	const int aimBonus = ctx.shooter->actionBudget ? ctx.shooter->actionBudget->getAimBonus() : 0;
	const int effectiveBS = std::max(1, std::min(99, baseBS + modSum + aimBonus));

	// ── Roll d100 ──
	const int roll = roll100();

	// ── Consume aim bonus (whether hit or miss, it's spent on this attack) ──
	if (ctx.shooter->actionBudget) {
		ctx.shooter->actionBudget->consumeAimBonus();
	}

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

	// ── Reaction check: offer Dodge before applying damage (ranged) ──
	if (ctx.target->actionBudget && ctx.target->actionBudget->hasReaction()) {
		ReactionResult reaction = resolveReaction(ctx.target, ctx.shooter, false); // false = ranged
		if (reaction == ReactionResult::NEGATED) {
			result.dodged = true;
			return result;
		}
	}
	// NOTE: No Parry test against ranged attacks (Requirement 6.5)

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
		const bool earlyVis = (shooter == engine.player) || engine.map->isInFOV(shooter->getX(), shooter->getY());
		if (earlyVis) {
			const TCODColor earlyColor = (shooter == engine.player) ? Colors::playerAction : Colors::enemyAction;
			engine.gui->message(earlyColor, "# fires at # in vain.",
				shooter->name, target->name);
		}
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

	// ── Size classification combat-mode check ──
	if (weaponEquippable && weaponEquippable->sizeClass) {
		int dx = std::abs(shooter->getX() - target->getX());
		int dy = std::abs(shooter->getY() - target->getY());
		int distance = std::max(dx, dy); // Chebyshev distance
		int range = weaponStats.range;
		auto modeCheck = checkCombatMode(*weaponEquippable->sizeClass, distance, range);
		if (!modeCheck.allowed) {
			const bool modeVis = (shooter == engine.player) || engine.map->isInFOV(shooter->getX(), shooter->getY());
			if (modeVis) {
				engine.gui->message(Colors::uiText, modeCheck.message);
			}
			return;
		}
	}

	// ── Check ammo ──
	if (currentAmmo <= 0) {
		const bool ammoVis = (shooter == engine.player) || engine.map->isInFOV(shooter->getX(), shooter->getY());
		if (ammoVis) {
			const TCODColor ammoColor = (shooter == engine.player) ? Colors::playerAction : Colors::enemyAction;
			engine.gui->message(ammoColor, "#'s weapon clicks empty.",
				shooter->name);
		}
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

	// ── Determine visibility for FOV-gated messaging ──
	const bool isPlayer = (shooter == engine.player);
	const bool visibleToPlayer = isPlayer || engine.map->isInFOV(shooter->getX(), shooter->getY());
	const TCODColor actionColor = isPlayer ? Colors::playerAction : Colors::enemyAction;

	// ── Log attack initiation (suppress if attacker is outside FOV) ──
	if (visibleToPlayer) {
		engine.gui->message(actionColor, "# fires at #.",
			shooter->name, target->name);
	}

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
			if (visibleToPlayer) {
				engine.gui->message(actionColor, "# misses #.",
					shooter->name, target->name);
			}
		} else if (result.dodged) {
			if (visibleToPlayer) {
				engine.gui->message(Colors::damage, "Hit! (# DoS) — #.",
					result.doS, HitLocationTable::name(result.location));
				engine.gui->message(Colors::reactionEvent, "# dodges 1 hit(s).",
					target->name);
			}
		} else {
			if (visibleToPlayer) {
				engine.gui->message(Colors::damage, "Hit! (# DoS) — #.",
					result.doS, HitLocationTable::name(result.location));
			}

			if (result.finalDamage <= 0) {
				if (visibleToPlayer) {
					engine.gui->message(Colors::uiText, "...but it has no effect!");
				}
			} else {
				// ── Thread cause-of-death (Req 1.6, 1.7) ──
				// When the victim is the player, record the shooter's name so that if this
				// wound triggers PlayerDestructible::die(), the outcome is "Slain by <name>".
				if (shooter && target == engine.player) {
					engine.pendingCauseOfDeath_ = shooter->name;
				}

				// Apply wound
				target->destructible->hp -= static_cast<float>(result.finalDamage);

				if (target->destructible->hp <= 0) {
					// Dead outright
					const int critMagnitude = 10;
					const auto critEffect = CriticalEffects::resolve(result.location, critMagnitude);

					if (visibleToPlayer) {
						engine.gui->message(Colors::damage,
							"Critical Hit on #! #",
							HitLocationTable::name(result.location), critEffect.description);
					}
					target->destructible->die(target);
					result.targetKilled = true;
				} else if (target->destructible->hp <= 9.0f) {
					// Crit territory: HP 1-9. Magnitude = 10 - hp.
					const int critMagnitude = 10 - static_cast<int>(target->destructible->hp);
					const auto critEffect = CriticalEffects::resolve(result.location, critMagnitude);

					if (visibleToPlayer) {
						engine.gui->message(Colors::damage,
							"Critical Hit on #! #",
							HitLocationTable::name(result.location), critEffect.description);
					}

					if (critEffect.fatal) {
						target->destructible->die(target);
						result.targetKilled = true;
					} else {
						if (!target->injuryTracker) {
							target->injuryTracker = std::make_unique<InjuryTracker>();
						}
						bool survived = target->injuryTracker->applyCrit(target, result.location, critMagnitude);
						if (!survived) {
							target->destructible->die(target);
							result.targetKilled = true;
						} else if (visibleToPlayer) {
							engine.gui->message(Colors::damage,
								"# suffers a critical injury to the #!",
								target->name, HitLocationTable::name(result.location));
						}
					}
				} else {
					if (visibleToPlayer) {
						engine.gui->message(Colors::damage, "# deals # damage to #'s #.",
							shooter->name, result.finalDamage, target->name,
							HitLocationTable::name(result.location));
					}
				}
			}
		}
	} else {
		// Destructible attack path
		if (result.finalDamage > 0) {
			target->destructible->takeDamage(target, static_cast<float>(result.finalDamage));
			if (visibleToPlayer) {
				engine.gui->message(Colors::damage, "# shoots # for # damage.",
					shooter->name, target->name, result.finalDamage);
			}

			if (target->destructible->isDead()) {
				result.targetKilled = true;
			}
		}
	}
}

} // namespace RangedCombat
