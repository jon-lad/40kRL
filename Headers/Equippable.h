#pragma once

#include "Persistent.h"

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

class Equippable : public Persistent {
public:
	EquipmentSlot slot;
	StatModifiers modifiers;
	float weight = 0.0f;
	int value = 0;

	Equippable(EquipmentSlot slot, StatModifiers modifiers, float weight, int value);

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;
};
