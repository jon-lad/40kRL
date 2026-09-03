#pragma once

// ─── Monster AI decision helper (pure, engine-free) ─────────────────────────
// Extends the Skill_Provider / Trait_Provider seam with a pure decision helper
// for the Mob Rule / Cowardly AI-behaviour traits. The helper takes plain values
// (read by the Ai.cpp layer from live actor state) so it never touches engine.*,
// keeping it fully unit- and property-testable without initializing the Engine.
// Feature: npc-skills-talents-wiring (Requirements 8.5, 10.1).

// The action a monster intends to take this turn, chosen from its trait-driven state.
enum class MonsterIntent : int { Approach, Attack, Flee };

// Snapshot of the state the decision depends on. Populated by the Ai.cpp layer.
struct MonsterDecisionContext {
	bool isCowardly       = false; // hasTrait(actor, "Cowardly")
	bool hasMobRule       = false; // hasTrait(actor, "Mob Rule")
	int  currentWounds    = 0;
	int  maxWounds        = 0;
	int  nearbyAllies     = 0;     // allied actors within MOB_RULE_RADIUS
	bool adjacentToPlayer = false;
};

// User-confirmed tuning constants (Req 8.6).
// Flee when current wounds are at or below this fraction of max wounds.
inline constexpr double COWARDLY_FLEE_FRACTION = 0.30;
// A Mob Rule actor is emboldened once it has at least this many nearby allies.
inline constexpr int MOB_RULE_MIN_ALLIES = 2;
// Radius (Chebyshev tiles) within which allies count toward Mob Rule emboldenment.
inline constexpr int MOB_RULE_RADIUS = 5;

// Decides the monster's intended action from its trait-driven context.
// Pure and total: returns a defined value for every input, no engine access.
MonsterIntent decideMonsterAction(const MonsterDecisionContext& ctx);
