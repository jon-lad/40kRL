#pragma once

#include <string>

// sol::table is a template alias (basic_table_core<...>), not a plain class, so it
// cannot be forward-declared with `class table;`. sol's lightweight forward header
// provides the correct declaration without pulling in the full sol2 implementation.
#include <sol/forward.hpp>

class Actor;
class CareerProgression;

// The Skill_Provider: free functions that read an actor's stat-block uniformly
// for player and NPC. All accessors are pure with respect to the engine (no
// engine.* access) and null-safe (an actor with no career reads as untrained /
// talentless), which keeps them fully unit- and property-testable.

// Sentinel returned by getSkillRank when the actor lacks the skill (Untrained).
inline constexpr int SKILL_UNTRAINED = -1;

// Maximum skill rank stored on a CareerProgression (0 = Trained, 1 = +10, 2 = +20).
inline constexpr int MAX_SKILL_RANK = 2;

// Returns the actor's rank for the named skill, or SKILL_UNTRAINED if the actor
// has no career component or no entry for that skill. Null-safe (nullptr -> untrained).
int getSkillRank(const Actor* actor, const std::string& skill);

// Returns true iff the actor has a career component with an entry for the named skill.
bool hasSkill(const Actor* actor, const std::string& skill);

// Returns true iff the actor has a career component whose talent set contains `talent`.
// Null-safe (nullptr -> false).
bool hasTalent(const Actor* actor, const std::string& talent);

// ─── Trait_Provider: pure, null-safe helpers over CareerProgression::traits ─
// Extend the Skill_Provider seam. Every helper returns a defined neutral value
// for a null actor or an actor with no career, and performs zero engine.* access.

// Size categories (RT-Bestiary, Size Categories table).
enum class SizeCategory : int {
	Puny = 0,
	Scrawny,
	Average,
	Hulking,
	Enormous,
	Massive
};

// Exact byte-for-byte match against CareerProgression::traits.
// Null-safe: nullptr / no career / empty traits -> false. (Req 8.1, 9.1)
bool hasTrait(const Actor* actor, const std::string& name);

// +3 if actor has "Brutal Charge", else 0 (incl. null / no career). (Req 5.1)
int brutalChargeBonus(const Actor* actor);

// true iff actor has "Sturdy"; side-effect-free, idempotent. (Req 6.1, 6.2)
bool isImmuneToKnockdown(const Actor* actor);

// Parses "Size (<Category>)" trait; unknown / missing / null -> Average. (Req 7.1, 7.2, 7.6)
SizeCategory getSizeCategory(const Actor* actor);

// Maps size category to its to-hit modifier. (Req 7.3)
int sizeToHitModifier(SizeCategory category);

// ─── Awareness surprise helper (pure; dependency A1) ────────────────────────

// RT-CoreMechanics §4 surprise bonus applied to a non-surprised attacker vs a
// surprised target. Exposed for a future surprise round. (Req 3.7)
inline constexpr int SURPRISE_ATTACK_BONUS = 30;

// clamp(perception + awarenessModifier, 0, 100); awarenessModifier = rank*10 when
// Awareness present, -20 when absent. Null-safe. (Req 3.1-3.4)
int surpriseAvoidanceTarget(int perception, int awarenessRank, bool hasAwareness);

// true = "not surprised", false = "surprised"; auto-1 not surprised, auto-100
// surprised. (Req 3.5, 3.6)
bool surpriseAvoided(int roll, int surpriseTarget);

// Parses the optional skills / talents / traits tables from a Lua enemy entry into
// the given CareerProgression. Factored out of Map.cpp addActor so it can be driven
// by tests with an in-memory sol::state (engine-free).
void populateStatBlockFromLua(CareerProgression& career, const sol::table& entry);
