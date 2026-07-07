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

struct RangedStats {
	DiceSpec damageDice = {1, 10};  // e.g., "1d10" for laspistol
	int penetration = 0;            // armour penetration value
	int range = 30;                 // max range in tiles
	int rateOfFire = 1;             // shots per attack action
	int clipSize = 6;               // max ammo capacity
	int reloadTime = 1;             // turns to reload (always 1 for now)
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
	std::optional<MeleeStats> meleeStats;      // present for melee weapons
	std::optional<ArmourProfile> armourProfile; // present for armour
	std::optional<RangedStats> rangedStats;    // present for ranged weapons
	int currentAmmo = 0;                       // mutable runtime ammo counter

	Equippable(EquipmentSlot slot, StatModifiers modifiers, float weight, int value);

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};
