#include "main.hpp"
#include "StatBlock.hpp"

#include <sol/sol.hpp>

// STUB implementations. Real logic is implemented in a later task.

int getSkillRank(const Actor* actor, const std::string& skill) {
	(void)actor;
	(void)skill;
	return SKILL_UNTRAINED;
}

bool hasSkill(const Actor* actor, const std::string& skill) {
	(void)actor;
	(void)skill;
	return false;
}

bool hasTalent(const Actor* actor, const std::string& talent) {
	(void)actor;
	(void)talent;
	return false;
}

void populateStatBlockFromLua(CareerProgression& career, const sol::table& entry) {
	(void)career;
	(void)entry;
	// no-op stub
}
