#pragma once

#include <array>
#include <string>
#include <vector>

// Loaded from Scripts/Homeworlds.lua
struct HomeworldTemplate {
	std::string name;
	std::array<int, 9> characteristicMods;  // indexed by CharId, can be negative
	std::vector<std::string> startingSkills; // skill names granted at Trained
	std::vector<std::string> startingTraits; // trait names (recorded, not mechanically active)
};

// A single advance entry within a rank table
struct AdvanceEntry {
	enum class Type { CHARACTERISTIC, SKILL, TALENT };
	Type type;
	std::string name;       // CharId abbreviation, skill name, or talent name
	int cost;               // XP cost to purchase
	int amount = 5;         // for CHARACTERISTIC: amount to add (default +5)
};

// One rank's data within a career path
struct RankDefinition {
	int rankNumber;         // 1-based
	std::string rankTitle;  // display name (e.g. "Void-Master")
	int xpThreshold;        // cumulative Spent_XP required to enter this rank
	std::vector<AdvanceEntry> advances;
	std::vector<std::string> startingSkills;  // granted free at rank entry (Rank 1 only)
	std::vector<std::string> startingTalents; // granted free at rank entry (Rank 1 only)
};

// Loaded from Scripts/Careers.lua
struct CareerTemplate {
	std::string name;
	std::vector<RankDefinition> ranks;  // ordered by rankNumber
};

// Loaded from Scripts/Skills.lua
struct SkillDefinition {
	std::string name;
	bool isCombat;  // true = combat skill, false = non-combat (deferred mechanics)
};

// Loaded from Scripts/Talents.lua
struct TalentDefinition {
	std::string name;
	std::string description;
};
