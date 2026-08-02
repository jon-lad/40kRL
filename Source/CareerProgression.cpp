
#include "main.h"

#include <algorithm>

// Maps a characteristic abbreviation string to the corresponding CharId.
// Returns CharId::COUNT if the name is not recognized.
static CharId charIdFromName(const std::string& name)
{
	// Order: WS=0, BS=1, S=2, T=3, Ag=4, Int=5, Per=6, WP=7, Fel=8
	if (name == "WS")  return CharId::WS;
	if (name == "BS")  return CharId::BS;
	if (name == "S")   return CharId::S;
	if (name == "T")   return CharId::T;
	if (name == "Ag")  return CharId::Ag;
	if (name == "Int") return CharId::Int;
	if (name == "Per") return CharId::Per;
	if (name == "WP")  return CharId::WP;
	if (name == "Fel") return CharId::Fel;
	return CharId::COUNT;  // not found
}

bool CareerProgression::canPurchase(const AdvanceEntry& advance) const
{
	if (availableXp() < advance.cost) {
		return false;
	}

	switch (advance.type) {
	case AdvanceEntry::Type::SKILL: {
		auto it = skills.find(advance.name);
		if (it != skills.end() && it->second >= 2) {
			return false;  // already at max rank (+20)
		}
		break;
	}
	case AdvanceEntry::Type::TALENT: {
		if (talents.count(advance.name) > 0) {
			return false;  // talent already acquired
		}
		break;
	}
	case AdvanceEntry::Type::CHARACTERISTIC:
		// No additional restriction beyond XP cost
		break;
	}

	return true;
}

bool CareerProgression::purchase(const AdvanceEntry& advance, Characteristics& chars)
{
	if (!canPurchase(advance)) {
		return false;
	}

	// Deduct XP cost
	spentXp += advance.cost;

	switch (advance.type) {
	case AdvanceEntry::Type::CHARACTERISTIC: {
		CharId charId = charIdFromName(advance.name);
		if (charId != CharId::COUNT) {
			int current = chars.get(charId);
			int newValue = std::clamp(current + advance.amount, 1, 99);
			chars.set(charId, newValue);
		}
		break;
	}
	case AdvanceEntry::Type::SKILL: {
		auto it = skills.find(advance.name);
		if (it == skills.end()) {
			// Skill not yet in map — add at rank 0 (Trained), then increment
			skills[advance.name] = 1;
		}
		else {
			it->second += 1;
		}
		break;
	}
	case AdvanceEntry::Type::TALENT: {
		talents.insert(advance.name);
		break;
	}
	}

	return true;
}

void CareerProgression::evaluateRankUp(const CareerTemplate& career)
{
	for (const auto& rank : career.ranks) {
		if (spentXp >= rank.xpThreshold && rank.rankNumber > currentRank) {
			currentRank = rank.rankNumber;
		}
	}
}

void CareerProgression::save(TCODZip& zip)
{
	// Stub — full implementation in task 3.3
}

void CareerProgression::load(TCODZip& zip)
{
	// Stub — full implementation in task 3.3
}
