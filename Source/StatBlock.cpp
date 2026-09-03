#include "main.hpp"
#include "StatBlock.hpp"
#include "Actor.hpp"
#include "CareerProgression.hpp"

#include <sol/sol.hpp>
#include <algorithm>

// ─── Skill_Provider: uniform accessors over an actor's stat-block ───────────
// All helpers are null-safe (nullptr actor / no career -> untrained / talentless)
// and never touch engine.*, keeping them fully unit- and property-testable.

int getSkillRank(const Actor* actor, const std::string& skill) {
	if (!actor || !actor->career) return SKILL_UNTRAINED;
	auto it = actor->career->skills.find(skill);
	return (it != actor->career->skills.end()) ? it->second : SKILL_UNTRAINED;
}

bool hasSkill(const Actor* actor, const std::string& skill) {
	if (!actor || !actor->career) return false;
	return actor->career->skills.count(skill) > 0;
}

bool hasTalent(const Actor* actor, const std::string& talent) {
	if (!actor || !actor->career) return false;
	return actor->career->talents.count(talent) > 0;
}

// ─── Trait_Provider stubs (TDD red) ─────────────────────────────────────────
// Trivial stub bodies so the project links; real logic lands in tasks 9.1/9.2.

bool hasTrait(const Actor* /*actor*/, const std::string& /*name*/) {
	return false;
}

int brutalChargeBonus(const Actor* /*actor*/) {
	return 0;
}

bool isImmuneToKnockdown(const Actor* /*actor*/) {
	return false;
}

SizeCategory getSizeCategory(const Actor* /*actor*/) {
	return SizeCategory::Average;
}

int sizeToHitModifier(SizeCategory /*category*/) {
	return 0;
}

// ─── Awareness surprise helper stubs (TDD red) ──────────────────────────────

int surpriseAvoidanceTarget(int /*perception*/, int /*awarenessRank*/, bool /*hasAwareness*/) {
	return 0;
}

bool surpriseAvoided(int /*roll*/, int /*surpriseTarget*/) {
	return false;
}

// ─── Lua stat-block parsing (engine-free; driven by tests and Map.cpp) ──────

void populateStatBlockFromLua(CareerProgression& career, const sol::table& entry) {
	// Skills: map of name -> integer rank, clamped to [0, MAX_SKILL_RANK].
	sol::optional<sol::table> skillsTbl = entry["skills"];
	if (skillsTbl) {
		for (auto& kv : *skillsTbl) {
			std::string name = kv.first.as<std::string>();
			int rank = kv.second.as<int>();
			rank = std::clamp(rank, 0, MAX_SKILL_RANK);
			career.skills[name] = rank;
		}
	}

	// Talents: 1-based array of strings (order-independent set).
	sol::optional<sol::table> talentsTbl = entry["talents"];
	if (talentsTbl) {
		for (size_t i = 1; i <= talentsTbl->size(); ++i) {
			sol::optional<std::string> t = (*talentsTbl)[i];
			if (t) career.talents.insert(*t);
		}
	}

	// Traits: 1-based array of strings; preserve declared order.
	sol::optional<sol::table> traitsTbl = entry["traits"];
	if (traitsTbl) {
		for (size_t i = 1; i <= traitsTbl->size(); ++i) {
			sol::optional<std::string> tr = (*traitsTbl)[i];
			if (tr) career.traits.push_back(*tr);
		}
	}
}
