
#include "main.hpp"

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
	// Write order is the load-order contract (see design: Serialization Write/Read Order).
	// Scalars first, then each collection as count-then-elements.
	zip.putString(homeworldName.c_str());
	zip.putString(careerName.c_str());
	zip.putInt(currentRank);
	zip.putInt(xpPool);
	zip.putInt(spentXp);

	// Skills: count, then (name, rank) pairs. Map iteration order is unspecified,
	// which is fine — load reconstructs by insertion and equality is membership-based.
	zip.putInt(static_cast<int>(skills.size()));
	for (const auto& entry : skills) {
		zip.putString(entry.first.c_str());
		zip.putInt(entry.second);
	}

	// Talents: count, then each name.
	zip.putInt(static_cast<int>(talents.size()));
	for (const auto& name : talents) {
		zip.putString(name.c_str());
	}

	// Traits: count, then each name in vector order (sequence preserved).
	zip.putInt(static_cast<int>(traits.size()));
	for (const auto& trait : traits) {
		zip.putString(trait.c_str());
	}
}

void CareerProgression::load(TCODZip& zip)
{
	// Clear collections first so a reused instance does not accumulate stale entries.
	skills.clear();
	talents.clear();
	traits.clear();

	// Strings may come back as nullptr from TCODZip::getString — guard every read.
	const char* homeworld = zip.getString();
	homeworldName = homeworld ? homeworld : "";
	const char* career = zip.getString();
	careerName = career ? career : "";

	currentRank = zip.getInt();
	xpPool = zip.getInt();
	spentXp = zip.getInt();

	// Skills: count, then (name, rank) pairs.
	const int skillCount = zip.getInt();
	for (int i = 0; i < skillCount; ++i) {
		const char* name = zip.getString();
		const std::string skillName = name ? name : "";
		const int rank = zip.getInt();
		skills[skillName] = rank;
	}

	// Talents: count, then each name.
	const int talentCount = zip.getInt();
	for (int i = 0; i < talentCount; ++i) {
		const char* name = zip.getString();
		talents.insert(name ? name : "");
	}

	// Traits: count, then each name (order preserved).
	const int traitCount = zip.getInt();
	for (int i = 0; i < traitCount; ++i) {
		const char* name = zip.getString();
		traits.push_back(name ? name : "");
	}
}
