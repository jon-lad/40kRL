#pragma once

#include "Persistent.h"
#include "DiceRoller.h"
#include "HitLocation.h"
#include <array>
#include <optional>
#include <string>
#include <vector>

enum class EquipmentSlot : int {
	WEAPON = 0,
	OFFHAND = 1,
	HEAD = 2,
	BODY = 3,
	COUNT = 4
};

struct StatModifiers {
	float power = 0.0f;
	float defense = 0.0f;
	float maxHp = 0.0f;
	int skill = 0;
};

struct MeleeStats {
	DiceSpec damageDice = {1, 5};  // default "1d5"
	int penetration = 0;
	std::vector<std::string> qualities;  // e.g., "Balanced", "Tearing"
};

struct ArmourProfile {
	std::array<int, static_cast<int>(HitLocation::COUNT)> values = {};
	// Indexed by HitLocation enum
};

class Equippable : public Persistent {
public:
	EquipmentSlot slot;
	StatModifiers modifiers;
	float weight = 0.0f;
	int value = 0;
	std::optional<MeleeStats> meleeStats;      // present for weapons
	std::optional<ArmourProfile> armourProfile; // present for armour

	Equippable(EquipmentSlot slot, StatModifiers modifiers, float weight, int value);

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};
