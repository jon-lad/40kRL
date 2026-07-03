
#include <sstream>
#include <random>
#include <algorithm>
#include <numeric>
#include "main.h"

Attacker::Attacker(float power, int skillValue)
	: power{ power }
	, skillValue{ clampSkill(skillValue) }
	, rollD100{ defaultRoll }
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
		// ── Hit check ──
		const int threshold = computeThreshold();
		const int roll = rollD100();

		if (roll > threshold) {
			// Miss — log and return early
			engine.gui->message(Colors::uiText, "# attacks # but misses.",
				owner->name, target->name);
			return;
		}

		// ── Existing damage logic (unchanged) ──
		const float effectiveDamage = power - target->destructible->defense;
		if (effectiveDamage > 0) {
			const TCODColor messageColor = (owner == engine.player) ? Colors::damage : Colors::uiText;
			engine.gui->message(messageColor, "# attacks # for # damage.",
				owner->name, target->name, effectiveDamage);
		} else {
			engine.gui->message(Colors::uiText, "# attacks # but it has no effect!",
				owner->name, target->name);
		}
		target->destructible->takeDamage(target, power);
	} else {
		engine.gui->message(Colors::uiText, "# attacks # in vain.",
			owner->name, target->name);
	}
}
