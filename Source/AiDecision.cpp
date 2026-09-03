#include "AiDecision.hpp"

#include <cmath>

// ─── Monster AI decision helper (pure, engine-free) ─────────────────────────
// Chooses a monster's intended action from a snapshot of its trait-driven state.
// Pure and total: no engine access, defined result for every input.
// Feature: npc-skills-talents-wiring (Requirements 8.2, 8.3, 8.4, 8.5).

MonsterIntent decideMonsterAction(const MonsterDecisionContext& ctx) {
	// Mob Rule actors gain courage from a nearby pack (Req 8.4).
	const bool emboldened = ctx.hasMobRule && ctx.nearbyAllies >= MOB_RULE_MIN_ALLIES;

	// Cowardly actors flee once wounds drop to or below the flee fraction (Req 8.3).
	const int fleeThreshold =
		static_cast<int>(std::floor(ctx.maxWounds * COWARDLY_FLEE_FRACTION));

	if (ctx.isCowardly && !emboldened && ctx.currentWounds <= fleeThreshold) {
		return MonsterIntent::Flee;
	}

	// Otherwise close in or engage the player (Req 8.2).
	return ctx.adjacentToPlayer ? MonsterIntent::Attack : MonsterIntent::Approach;
}
