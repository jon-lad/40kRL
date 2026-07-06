#pragma once

#include <vector>
#include <functional>

// Gives an actor the ability to deal melee damage.
// Damage formula: effective_damage = power - target.defense
// Hit check: d100 roll-under against effective skill (base + modifiers, clamped 1-99)
class Attacker : public Persistent {
public:
	float power; // raw attack power before the target's defence is applied
	int skillValue; // [1, 99], percentage chance to hit (d100 roll-under)
	std::vector<int> modifiers; // situational modifiers (signed)
	std::function<int()> rollD100; // injectable RNG, default: uniform [1,100]
	std::function<int(int)> rollDie; // injectable RNG for weapon damage dice, takes sides, returns [1, sides]

	explicit Attacker(float power, int skillValue = 40);

	// Deals damage from owner to target and logs the result to the message log.
	void attack(Actor* owner, Actor* target);

	// Computes effective threshold: clamp(skillValue + sum(modifiers), 1, 99)
	int computeThreshold() const;

	// Adds a modifier (positive = bonus, negative = penalty)
	void addModifier(int mod);

	// Removes the first occurrence of a modifier with the given value
	void removeModifier(int mod);

	// Clears all modifiers
	void clearModifiers();

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

private:
	static constexpr int DEFAULT_SKILL = 40;
	static constexpr int MIN_SKILL = 1;
	static constexpr int MAX_SKILL = 99;
	static constexpr int ATTACKER_SAVE_V2 = -1; // sentinel for V2 save format
	static constexpr int ATTACKER_SAVE_V3 = -2; // sentinel for V3 save format (melee combat)

	static int clampSkill(int value);
	static int defaultRoll(); // uniform_int_distribution [1, 100]

	void resolveCharacterAttack(Actor* owner, Actor* target);
	void resolveDestructibleAttack(Actor* owner, Actor* target);
};
