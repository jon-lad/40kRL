#include "main.h"

Equippable::Equippable(EquipmentSlot slot, StatModifiers modifiers, float weight, int value)
	: slot{ slot }
	, modifiers{ modifiers }
	, weight{ weight }
	, value{ value }
{
}

void Equippable::save(TCODZip& zip) {
	zip.putInt(static_cast<int>(slot));
	zip.putFloat(modifiers.power);
	zip.putFloat(modifiers.defense);
	zip.putFloat(modifiers.maxHp);
	zip.putInt(modifiers.skill);
	zip.putFloat(weight);
	zip.putInt(value);
}

void Equippable::load(TCODZip& zip) {
	slot = static_cast<EquipmentSlot>(zip.getInt());
	modifiers.power = zip.getFloat();
	modifiers.defense = zip.getFloat();
	modifiers.maxHp = zip.getFloat();
	modifiers.skill = zip.getInt();
	weight = zip.getFloat();
	value = zip.getInt();
}
