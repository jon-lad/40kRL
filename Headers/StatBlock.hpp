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

// Parses the optional skills / talents / traits tables from a Lua enemy entry into
// the given CareerProgression. Factored out of Map.cpp addActor so it can be driven
// by tests with an in-memory sol::state (engine-free).
void populateStatBlockFromLua(CareerProgression& career, const sol::table& entry);
