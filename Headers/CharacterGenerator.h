#pragma once

#include "Characteristics.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Stores state for the character generation modal overlay.
struct CharGenState {
	enum class Step { HOMEWORLD, CAREER, ADVANCES, DONE };
	Step currentStep = Step::HOMEWORLD;

	int selectedIndex = 0;     // cursor position in current list
	int scrollOffset = 0;      // for scrollable lists

	// Accumulated choices (indices into Engine template vectors)
	int homeworldIndex = -1;
	int careerIndex = -1;

	// Temporary characteristics built up during chargen (base 25 for all)
	Characteristics workingChars{25};

	// Working skills/talents/traits from homeworld + career grants
	std::unordered_map<std::string, int> workingSkills;
	std::unordered_set<std::string> workingTalents;
	std::vector<std::string> workingTraits;

	// XP for initial advance purchases (granted by career at creation)
	int startingXp = 0;
	int spentXp = 0;

	// Temporary status message for purchase feedback (shown briefly)
	std::string statusMessage;
};
