#include "main.hpp"
#include "ReactionResolver.hpp"

// ─── Helper: check if actor has a melee weapon in the WEAPON slot ───────────

bool hasEquippedMeleeWeapon(Actor* actor) {
	if (!actor->equipment) return false;
	Actor* weapon = actor->equipment->getSlot(EquipmentSlot::WEAPON);
	if (!weapon) return false;
	if (!weapon->equippable) return false;
	return weapon->equippable->meleeStats.has_value();
}

// ─── Helper: determine whether actor is player-controlled ───────────────────

static bool isPlayerControlled(Actor* actor) {
	return (actor == engine.player);
}

// ─── Helper: pick the best reaction for an AI actor ─────────────────────────

static ReactionChoice pickReactionAI(Actor* target, bool canDodge, bool canParry) {
	if (!canDodge && !canParry) return ReactionChoice::SKIP;
	if (!canParry) return ReactionChoice::DODGE;
	if (!canDodge) return ReactionChoice::PARRY;

	// Both available: pick based on higher stat
	int ag = target->characteristics->get(CharId::Ag);
	int ws = target->characteristics->get(CharId::WS);
	return (ws >= ag) ? ReactionChoice::PARRY : ReactionChoice::DODGE;
}

// ─── Helper: pick reaction for the player ───────────────────────────────────
// For now, auto-dodge since we don't have UI prompts yet.

static ReactionChoice pickReactionPlayer(Actor* target, bool canDodge, bool canParry) {
	(void)target;
	(void)canParry;
	if (canDodge) return ReactionChoice::DODGE;
	return ReactionChoice::SKIP;
}

// ─── Main resolution function ───────────────────────────────────────────────

ReactionResult resolveReaction(Actor* target, Actor* attacker, bool isMelee) {
	(void)attacker; // attacker info available for future use (e.g., DoS comparison)

	// 1. Check reaction availability
	if (!target->actionBudget || !target->actionBudget->hasReaction()) {
		return ReactionResult::NO_REACTION;
	}

	// Need characteristics to roll the test
	if (!target->characteristics) {
		return ReactionResult::NO_REACTION;
	}

	// 2. Determine available reactions
	bool canDodge = true; // always available
	bool canParry = isMelee && hasEquippedMeleeWeapon(target);

	// 3. Pick reaction
	ReactionChoice choice;
	if (isPlayerControlled(target)) {
		choice = pickReactionPlayer(target, canDodge, canParry);
	} else {
		choice = pickReactionAI(target, canDodge, canParry);
	}

	if (choice == ReactionChoice::SKIP) {
		return ReactionResult::NO_REACTION;
	}

	// 4. Use the reaction
	target->actionBudget->useReaction();

	// 5. Roll test
	// Use the attacker's rollD100 if available for injectable RNG in tests,
	// otherwise fall back to target's attacker rollD100.
	int roll = 0;
	if (target->attacker && target->attacker->rollD100) {
		roll = target->attacker->rollD100();
	} else {
		// Fallback: use TCODRandom
		roll = TCODRandom::getInstance()->getInt(1, 100);
	}

	if (choice == ReactionChoice::DODGE) {
		int targetAg = target->characteristics->get(CharId::Ag);
		if (roll <= targetAg) {
			// 6. Log success
			engine.gui->message(Colors::reactionEvent, "# dodges the attack!", target->name);
			// 7. Return NEGATED
			return ReactionResult::NEGATED;
		} else {
			// 6. Log failure
			engine.gui->message(Colors::reactionEvent, "# fails to dodge.", target->name);
			return ReactionResult::FAILED;
		}
	}

	if (choice == ReactionChoice::PARRY) {
		int targetWS = target->characteristics->get(CharId::WS);
		if (roll <= targetWS) {
			// 6. Log success
			engine.gui->message(Colors::reactionEvent, "# parries the attack!", target->name);
			// 7. Return NEGATED
			return ReactionResult::NEGATED;
		} else {
			// 6. Log failure
			engine.gui->message(Colors::reactionEvent, "# fails to parry.", target->name);
			return ReactionResult::FAILED;
		}
	}

	return ReactionResult::NO_REACTION;
}
