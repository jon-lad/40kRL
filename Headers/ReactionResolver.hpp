#pragma once

#include <string>
#include <vector>

class Actor;

// Result of a reaction attempt (Dodge or Parry).
enum class ReactionResult {
	NEGATED,      // reaction succeeded — hit is negated
	FAILED,       // reaction attempted but failed the test
	NO_REACTION   // no reaction available or target chose to skip
};

// The type of defensive reaction chosen.
enum class ReactionChoice {
	DODGE,
	PARRY,
	SKIP
};

// Resolves a defensive reaction for the target against an incoming attack.
// Checks reaction availability, determines available options, picks the best
// one (AI) or auto-dodges (player, until UI prompt is implemented), rolls the
// test, and returns the outcome.
// isMelee: true for melee attacks, false for ranged attacks.
ReactionResult resolveReaction(Actor* target, Actor* attacker, bool isMelee);

// Returns true if the actor has a melee weapon equipped in the WEAPON slot.
bool hasEquippedMeleeWeapon(Actor* actor);

// ─── Dodge pure helpers (engine-independent, unit- and property-testable) ───

// Computes the effective Dodge target number (before roll comparison).
//   hasDodgeSkill == true  -> clamp(agility + dodgeRank * 10, 0, 100)
//   hasDodgeSkill == false -> clamp(agility - 20,            0, 100)
// The result is always within [0, 100]. Pure and engine-independent; the
// formula has no per-actor-type branch, so identical Agility + Dodge rank
// yield identical thresholds for player and NPC. (Requirements 1.2, 1.3, 1.8)
int computeDodgeTarget(int agility, int dodgeRank, bool hasDodgeSkill);

// Resolves a d100 roll-under test against a Dodge target.
//   roll == 1   -> true  (auto-success)
//   roll == 100 -> false (auto-fail)
//   otherwise   -> roll <= dodgeTarget
// Pure and engine-independent. (Requirements 1.1, 1.6, 1.7)
bool dodgeSucceeds(int roll, int dodgeTarget);

// ─── Parry pure helpers (engine-independent, unit- and property-testable) ──
// NOTE: These are currently TDD-red stubs (return 0 / false). Real logic is
// implemented in task 10.1. They mirror the Dodge helper pair above.

// Rank-based parry bonus.
//   hasParrySkill == true  -> parryRank * 10 (+0 / +10 / +20 for rank 0/1/2)
//   hasParrySkill == false -> -20 (untrained skill penalty, RT-CoreMechanics §3)
// Never applies both the rank bonus and the untrained penalty.
// Pure and engine-independent. (Requirements 1.1, 1.2, 1.3)
int parryBonus(int parryRank, bool hasParrySkill);

// Sum of the applicable Balanced (+10) and Unbalanced (-10) contributions from
// the equipped melee weapon's qualities list; 0 for an empty/absent list.
// Null-safe over the list. Pure and engine-independent. (Requirements 2.1, 2.2, 2.3, 2.5)
int parryQualityModifier(const std::vector<std::string>& qualities);

// Returns true iff the qualities list contains "Unwieldy", which makes Parry
// unavailable and takes precedence over any other quality. Pure. (Requirement 2.4)
bool parryUnavailableFromQualities(const std::vector<std::string>& qualities);

// Final clamped parry target number: clamp(weaponSkill + parryBonus + qualityModifier, 0, 100).
// Pure and engine-independent. (Requirements 2.7, 1.4)
int computeParryTarget(int weaponSkill, int parryBonus, int qualityModifier);

// Resolves a d100 roll-under test against a Parry target.
//   roll == 1   -> true  (auto-success)
//   roll == 100 -> false (auto-fail)
//   otherwise   -> roll <= parryTarget
// Pure and engine-independent. (Requirements 1.5, 1.6, 1.7)
bool parrySucceeds(int roll, int parryTarget);
