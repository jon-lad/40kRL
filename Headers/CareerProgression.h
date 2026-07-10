#pragma once

#include "Persistent.h"
#include "CharacterData.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Characteristics;

// Tracks career path progression, XP spending, skills, and talents.
// Attached to the player Actor after character generation.
class CareerProgression : public Persistent {
public:
	// Identifiers (stored as strings for serialization)
	std::string homeworldName;
	std::string careerName;

	// Rank tracking
	int currentRank = 1;

	// XP
	int xpPool = 0;       // total XP earned
	int spentXp = 0;      // total XP spent on advances

	int availableXp() const { return xpPool - spentXp; }

	// Skills: maps skill name -> rank (0 = Trained, 1 = +10, 2 = +20)
	std::unordered_map<std::string, int> skills;

	// Talents: set of acquired talent names
	std::unordered_set<std::string> talents;

	// Traits: list of trait names (no mechanical effect this spec)
	std::vector<std::string> traits;

	// Purchase logic
	bool canPurchase(const AdvanceEntry& advance) const;
	bool purchase(const AdvanceEntry& advance, Characteristics& chars);

	// Rank evaluation
	void evaluateRankUp(const CareerTemplate& career);

	// Persistence
	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};
