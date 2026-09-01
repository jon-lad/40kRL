#pragma once

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
