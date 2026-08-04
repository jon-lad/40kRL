
#include <sstream>
#include <random>
#include <algorithm>
#include <numeric>
#include <cstring>
#include "main.h"
#include "CriticalEffects.h"
#include "DiceRoller.h"
#include "ReactionResolver.h"

Attacker::Attacker(float power, int skillValue)
	: power{ power }
	, skillValue{ clampSkill(skillValue) }
	, rollD100{ defaultRoll }
	, rollDie{ nullptr }
{
}

int Attacker::clampSkill(int value) {
	return std::max(MIN_SKILL, std::min(MAX_SKILL, value));
}

int Attacker::defaultRoll() {
	static std::mt19937 engine{ std::random_device{}() };
	static std::uniform_int_distribution<int> dist(1, 100);
	return dist(engine);
}

int Attacker::computeThreshold() const {
	int sum = std::accumulate(modifiers.begin(), modifiers.end(), 0);
	int raw = skillValue + sum;
	return std::max(MIN_SKILL, std::min(MAX_SKILL, raw));
}

void Attacker::addModifier(int mod) {
	modifiers.push_back(mod);
}

void Attacker::removeModifier(int mod) {
	auto it = std::find(modifiers.begin(), modifiers.end(), mod);
	if (it != modifiers.end()) {
		modifiers.erase(it);
	}
}

void Attacker::clearModifiers() {
	modifiers.clear();
}

void Attacker::attack(Actor* owner, Actor* target)
{
	if (target->destructible && !target->destructible->isDead()) {
		if (target->characteristics) {
			resolveCharacterAttack(owner, target);
		} else {
			resolveDestructibleAttack(owner, target);
		}
	} else {
		engine.gui->message(Colors::uiText, "# attacks # in vain.",
			owner->name, target->name);
	}
}

void Attacker::save(TCODZip& zip) {
	zip.putInt(ATTACKER_SAVE_V3);  // sentinel: -2 (V3 format marker)
	zip.putFloat(power);
	zip.putInt(skillValue);
	zip.putInt(static_cast<int>(modifiers.size()));
	for (int mod : modifiers) {
		zip.putInt(mod);
	}
}

void Attacker::load(TCODZip& zip) {
	int marker = zip.getInt();
	if (marker == ATTACKER_SAVE_V3) {
		// V3: power + skillValue + modifiers
		power = zip.getFloat();
		skillValue = clampSkill(zip.getInt());
		int modCount = zip.getInt();
		modifiers.clear();
		for (int i = 0; i < modCount; i++) {
			modifiers.push_back(zip.getInt());
		}
	} else if (marker == ATTACKER_SAVE_V2) {
		// V2: power + skillValue (no modifiers)
		power = zip.getFloat();
		skillValue = clampSkill(zip.getInt());
	} else {
		// V1: marker is the 4 bytes of old power float
		std::memcpy(&power, &marker, sizeof(float));
		skillValue = DEFAULT_SKILL;
	}
}

void Attacker::resolveCharacterAttack(Actor* owner, Actor* target) {
	// ── Attack declaration ──
	engine.gui->message(Colors::uiText, "# swings at #.",
		owner->name, target->name);

	// ── Compute effective WS ──
	const int baseWS = owner->characteristics->get(CharId::WS);
	const int modSum = std::accumulate(modifiers.begin(), modifiers.end(), 0);

	// ── Aim bonus (from ActionBudget) ──
	const int aimBonus = owner->actionBudget ? owner->actionBudget->getAimBonus() : 0;

	// ── Proficiency penalty (weapon types) ──
	int profPenalty = 0;
	if (owner->equipment) {
		Actor* weaponItem = owner->equipment->getSlot(EquipmentSlot::WEAPON);
		if (weaponItem && weaponItem->equippable && weaponItem->equippable->weaponGroup) {
			profPenalty = proficiencyModifier(owner, *weaponItem->equippable->weaponGroup);
		}
	}
	const int effectiveWS = std::max(1, std::min(99, baseWS + modSum + profPenalty + aimBonus));

	// ── Roll d100 ──
	const int roll = rollD100();

	// ── Consume aim bonus (whether hit or miss, it's spent on this attack) ──
	if (owner->actionBudget) {
		owner->actionBudget->consumeAimBonus();
	}

	// ── Classify hit/miss ──
	if (roll > effectiveWS) {
		// Miss
		engine.gui->message(Colors::uiText, "# misses #.",
			owner->name, target->name);
		return;
	}

	// ── Hit: compute DoS ──
	const int dos = std::max(0, (effectiveWS - roll) / 10);

	// ── Determine hit location ──
	const HitLocation loc = HitLocationTable::resolve(roll);

	// ── Log hit ──
	engine.gui->message(Colors::damage, "Hit! (# DoS) — #.",
		dos, HitLocationTable::name(loc));

	// ── Reaction check: offer Dodge/Parry before applying damage ──
	if (target->actionBudget && target->actionBudget->hasReaction()) {
		ReactionResult result = resolveReaction(target, owner, true); // true = melee
		if (result == ReactionResult::NEGATED) {
			return; // hit negated, skip damage
		}
	}

	// ── Damage Calculation (Task 7.4) ──
	// Get weapon MeleeStats
	MeleeStats weaponStats{ DiceSpec{1, 5}, 0, {} }; // default unarmed
	if (owner->equipment) {
		Actor* weaponItem = owner->equipment->getSlot(EquipmentSlot::WEAPON);
		if (weaponItem && weaponItem->equippable && weaponItem->equippable->meleeStats) {
			weaponStats = *weaponItem->equippable->meleeStats;
		}
	}

	// Calculate Strength Bonus
	const int sb = owner->characteristics->bonus(CharId::S);

	// Roll raw damage
	const int rawDamage = DiceRoller::roll(weaponStats.damageDice, rollDie) + sb;

	// Calculate effective armour at hit location
	int locArmour = 0;
	if (target->equipment) {
		locArmour = target->equipment->getArmourAtLocation(loc);
	}
	const int effectiveArmour = std::max(0, locArmour - weaponStats.penetration);

	// Calculate Toughness Bonus
	const int tb = target->characteristics->bonus(CharId::T);

	// Calculate final damage
	const int finalDamage = std::max(0, rawDamage - effectiveArmour - tb);

	// If no effective damage, log and return
	if (finalDamage <= 0) {
		engine.gui->message(Colors::uiText, "...but it has no effect!");
		return;
	}

	// ── Critical Hit Logic (Task 7.5) ──
	const float currentHp = target->destructible->hp;

	// Apply damage directly to hp (bypass legacy defense reduction)
	target->destructible->hp -= static_cast<float>(finalDamage);

	if (target->destructible->hp <= 0) {
		// Critical hit: damage exceeded remaining wounds
		const int critMagnitude = finalDamage - static_cast<int>(currentHp);
		const auto critEffect = CriticalEffects::resolve(loc, critMagnitude);

		engine.gui->message(Colors::damage,
			"Critical Hit on #! #",
			HitLocationTable::name(loc), critEffect.description);

		if (!critEffect.fatal && critMagnitude <= 4) {
			// Non-fatal critical hit: apply injury via cumulative magnitude system
			if (!target->injuryTracker) {
				target->injuryTracker = std::make_unique<InjuryTracker>();
			}
			bool survived = target->injuryTracker->applyCrit(target, loc, critMagnitude);
			if (!survived) {
				// Cumulative magnitude reached fatal threshold (10)
				auto fatalEffect = CriticalEffects::resolve(loc, 10);
				engine.gui->message(Colors::damage, "Critical overload! #", fatalEffect.description);
				target->destructible->die(target);
			} else {
				// Injury applied; target survives at HP=1 for low-magnitude crits
				if (critMagnitude < 3) {
					target->destructible->hp = 1;
				}
				engine.gui->message(Colors::damage,
					"# suffers a critical injury to the #!",
					target->name, HitLocationTable::name(loc));
			}
		} else {
			// Fatal crit (magnitude >= 5 or fatal effect) — kill target
			target->destructible->die(target);
		}
	} else {
		// Normal damage — target survives
		engine.gui->message(Colors::damage, "# deals # damage to #'s #.",
			owner->name, finalDamage, target->name, HitLocationTable::name(loc));
	}
}

void Attacker::resolveDestructibleAttack(Actor* owner, Actor* target) {
	// ── Get weapon MeleeStats ──
	MeleeStats weaponStats{ DiceSpec{1, 5}, 0, {} }; // default unarmed
	if (owner->equipment) {
		Actor* weaponItem = owner->equipment->getSlot(EquipmentSlot::WEAPON);
		if (weaponItem && weaponItem->equippable && weaponItem->equippable->meleeStats) {
			weaponStats = *weaponItem->equippable->meleeStats;
		}
	}

	// ── Calculate Strength Bonus ──
	int sb = 3; // default when attacker has no Characteristics
	if (owner->characteristics) {
		sb = owner->characteristics->bonus(CharId::S);
	}

	// ── Roll weapon damage ──
	const int damage = DiceRoller::roll(weaponStats.damageDice, rollDie) + sb;

	// ── Apply damage directly (no armour, no TB) ──
	target->destructible->takeDamage(target, static_cast<float>(damage));

	// ── Log simplified message ──
	engine.gui->message(Colors::damage, "# strikes # for # damage.",
		owner->name, target->name, damage);
}
