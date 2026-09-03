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

// ─── Trait_Provider: pure, null-safe helpers over CareerProgression::traits ─
// All helpers are engine-free and treat a null actor / careerless actor as
// traitless (neutral result), keeping them unit- and property-testable.

bool hasTrait(const Actor* actor, const std::string& name) {
	if (!actor || !actor->career) return false;
	const auto& traits = actor->career->traits;
	// Exact byte-for-byte match; no normalization of case or whitespace.
	return std::find(traits.begin(), traits.end(), name) != traits.end();
}

int brutalChargeBonus(const Actor* actor) {
	return hasTrait(actor, "Brutal Charge") ? 3 : 0;
}

bool isImmuneToKnockdown(const Actor* actor) {
	return hasTrait(actor, "Sturdy");
}

SizeCategory getSizeCategory(const Actor* actor) {
	if (!actor || !actor->career) return SizeCategory::Average;

	// Case-sensitive parse of the exact form "Size (<Category>)". Any unknown
	// category, malformed string, or missing trait falls back to Average.
	static const std::pair<const char*, SizeCategory> kSizes[] = {
		{ "Size (Puny)",     SizeCategory::Puny },
		{ "Size (Scrawny)",  SizeCategory::Scrawny },
		{ "Size (Average)",  SizeCategory::Average },
		{ "Size (Hulking)",  SizeCategory::Hulking },
		{ "Size (Enormous)", SizeCategory::Enormous },
		{ "Size (Massive)",  SizeCategory::Massive },
	};

	for (const auto& trait : actor->career->traits) {
		for (const auto& entry : kSizes) {
			if (trait == entry.first) return entry.second;
		}
	}
	return SizeCategory::Average;
}

int sizeToHitModifier(SizeCategory category) {
	switch (category) {
		case SizeCategory::Puny:     return -30;
		case SizeCategory::Scrawny:  return -10;
		case SizeCategory::Average:  return 0;
		case SizeCategory::Hulking:  return 10;
		case SizeCategory::Enormous: return 20;
		case SizeCategory::Massive:  return 30;
	}
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
