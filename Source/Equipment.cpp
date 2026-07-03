#include "main.h"

Actor* Equipment::getSlot(EquipmentSlot slot) const {
	return slots[static_cast<int>(slot)];
}

Actor* Equipment::equip(Actor* item, Container& inventory, Attacker* attacker) {
	if (!item || !item->equippable) {
		return nullptr;
	}

	EquipmentSlot targetSlot = item->equippable->slot;
	int slotIndex = static_cast<int>(targetSlot);
	Actor* previous = slots[slotIndex];

	// If slot is occupied, remove old item's skill modifier
	if (previous && previous->equippable) {
		int oldSkill = previous->equippable->modifiers.skill;
		if (oldSkill != 0 && attacker) {
			attacker->removeModifier(oldSkill);
		}
		// Old item stays in inventory (already there per design)
	}

	// Apply new item's skill modifier
	int newSkill = item->equippable->modifiers.skill;
	if (newSkill != 0 && attacker) {
		attacker->addModifier(newSkill);
	}

	// Set slot pointer to new item
	slots[slotIndex] = item;

	return previous;
}

bool Equipment::unequip(EquipmentSlot slot, Container& inventory, Attacker* attacker) {
	int slotIndex = static_cast<int>(slot);
	Actor* item = slots[slotIndex];

	if (!item) {
		return false;
	}

	// Remove item's skill modifier if non-zero
	if (item->equippable) {
		int skill = item->equippable->modifiers.skill;
		if (skill != 0 && attacker) {
			attacker->removeModifier(skill);
		}
	}

	// Item remains in inventory (already there), just clear the slot pointer
	slots[slotIndex] = nullptr;
	return true;
}

float Equipment::getTotalPowerModifier() const {
	float total = 0.0f;
	for (const auto& item : slots) {
		if (item && item->equippable) {
			total += item->equippable->modifiers.power;
		}
	}
	return total;
}

float Equipment::getTotalDefenseModifier() const {
	float total = 0.0f;
	for (const auto& item : slots) {
		if (item && item->equippable) {
			total += item->equippable->modifiers.defense;
		}
	}
	return total;
}

float Equipment::getTotalMaxHpModifier() const {
	float total = 0.0f;
	for (const auto& item : slots) {
		if (item && item->equippable) {
			total += item->equippable->modifiers.maxHp;
		}
	}
	return total;
}

int Equipment::getTotalSkillModifier() const {
	int total = 0;
	for (const auto& item : slots) {
		if (item && item->equippable) {
			total += item->equippable->modifiers.skill;
		}
	}
	return total;
}

float Equipment::getEquippedWeight() const {
	float total = 0.0f;
	for (const auto& item : slots) {
		if (item && item->equippable) {
			total += item->equippable->weight;
		}
	}
	return total;
}

bool Equipment::isEquipped(const Actor* item) const {
	for (const auto& slot : slots) {
		if (slot == item) {
			return true;
		}
	}
	return false;
}

void Equipment::save(TCODZip& zip, const Container& inventory) {
	for (int i = 0; i < static_cast<int>(EquipmentSlot::COUNT); i++) {
		if (slots[i]) {
			zip.putInt(1); // slot occupied
			// Find index of this item in the inventory list
			int index = 0;
			for (auto it = inventory.inventory.begin(); it != inventory.inventory.end(); ++it, ++index) {
				if (it->get() == slots[i]) break;
			}
			zip.putInt(index);
		} else {
			zip.putInt(0); // slot empty
		}
	}
}

void Equipment::load(TCODZip& zip, Container& inventory) {
	for (int i = 0; i < static_cast<int>(EquipmentSlot::COUNT); i++) {
		int occupied = zip.getInt();
		if (occupied) {
			int index = zip.getInt();
			// Find the nth item in inventory
			auto it = inventory.inventory.begin();
			std::advance(it, index);
			if (it != inventory.inventory.end()) {
				slots[i] = it->get();
			} else {
				slots[i] = nullptr;
			}
		} else {
			slots[i] = nullptr;
		}
	}
}
