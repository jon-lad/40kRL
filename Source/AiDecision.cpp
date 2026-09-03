#include "AiDecision.hpp"

// ─── Monster AI decision helper (pure, engine-free) ─────────────────────────
// TDD red stub: returns a trivial default so the project links and tests compile
// while failing until the real logic lands in task 12.1.
// Feature: npc-skills-talents-wiring (Requirements 8.5, 10.1, 10.7).

MonsterIntent decideMonsterAction(const MonsterDecisionContext& ctx) {
	(void)ctx; // stub: real decision logic implemented in task 12.1
	return MonsterIntent::Approach;
}
