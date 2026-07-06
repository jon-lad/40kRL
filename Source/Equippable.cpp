#include "main.h"
#include "DiceRoller.h"

static constexpr int EQUIPPABLE_SAVE_V2 = -1; // sentinel for V2 format with MeleeStats/ArmourProfile

Equippable::Equippable(EquipmentSlot slot, StatModifiers modifiers, float weight, int value)
	: slot{ slot }
	, modifiers{ modifiers }
	, weight{ weight }
	, value{ value }
{
}

void Equippable::save(TCODZip& zip) {
	zip.putInt(EQUIPPABLE_SAVE_V2);  // V2 sentinel
	zip.putInt(static_cast<int>(slot));
	zip.putFloat(modifiers.power);
	zip.putFloat(modifiers.defense);
	zip.putFloat(modifiers.maxHp);
	zip.putInt(modifiers.skill);
	zip.putFloat(weight);
	zip.putInt(value);

	// MeleeStats
	zip.putInt(meleeStats.has_value() ? 1 : 0);
	if (meleeStats) {
		zip.putString(DiceRoller::format(meleeStats->damageDice).c_str());
		zip.putInt(meleeStats->penetration);
		zip.putInt(static_cast<int>(meleeStats->qualities.size()));
		for (const auto& q : meleeStats->qualities) {
			zip.putString(q.c_str());
		}
	}

	// ArmourProfile
	zip.putInt(armourProfile.has_value() ? 1 : 0);
	if (armourProfile) {
		for (int i = 0; i < static_cast<int>(HitLocation::COUNT); i++) {
			zip.putInt(armourProfile->values[i]);
		}
	}
}

void Equippable::load(TCODZip& zip) {
	int first = zip.getInt();
	if (first == EQUIPPABLE_SAVE_V2) {
		// V2 format: has MeleeStats and ArmourProfile
		slot = static_cast<EquipmentSlot>(zip.getInt());
		modifiers.power = zip.getFloat();
		modifiers.defense = zip.getFloat();
		modifiers.maxHp = zip.getFloat();
		modifiers.skill = zip.getInt();
		weight = zip.getFloat();
		value = zip.getInt();

		// MeleeStats
		int hasMelee = zip.getInt();
		if (hasMelee) {
			MeleeStats ms;
			const char* diceStr = zip.getString();
			auto parsed = DiceRoller::parse(diceStr ? diceStr : "");
			ms.damageDice = parsed.value_or(DiceSpec{1, 5});
			ms.penetration = zip.getInt();
			int qualCount = zip.getInt();
			for (int i = 0; i < qualCount; i++) {
				const char* q = zip.getString();
				if (q) ms.qualities.push_back(q);
			}
			meleeStats = ms;
		}

		// ArmourProfile
		int hasArmour = zip.getInt();
		if (hasArmour) {
			ArmourProfile ap;
			for (int i = 0; i < static_cast<int>(HitLocation::COUNT); i++) {
				ap.values[i] = zip.getInt();
			}
			armourProfile = ap;
		}
	} else {
		// V1 format: first int is the slot value
		slot = static_cast<EquipmentSlot>(first);
		modifiers.power = zip.getFloat();
		modifiers.defense = zip.getFloat();
		modifiers.maxHp = zip.getFloat();
		modifiers.skill = zip.getInt();
		weight = zip.getFloat();
		value = zip.getInt();
		// No MeleeStats or ArmourProfile in old saves
	}
}
