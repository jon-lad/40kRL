#pragma once
#include <array>

class Actor;
class Container;
class Attacker;
class TCODZip;
enum class EquipmentSlot : int;

class Equipment {
public:
	// Returns the item in the given slot, or nullptr if empty.
	Actor* getSlot(EquipmentSlot slot) const;

	// Equips item into its target slot. Returns the previously equipped item
	// (which has been moved back to inventory), or nullptr if slot was empty.
	// Returns nullptr and does nothing if item has no equippable component.
	// If the item has a non-zero skill modifier, calls attacker->addModifier(skill).
	// If swapping out an old item with a non-zero skill modifier, calls
	// attacker->removeModifier(oldSkill) before adding the new one.
	Actor* equip(Actor* item, Container& inventory, Attacker* attacker);

	// Unequips the item in the given slot, moving it back to inventory.
	// If the item has a non-zero skill modifier, calls attacker->removeModifier(skill).
	// Returns false if inventory is full or slot is already empty.
	bool unequip(EquipmentSlot slot, Container& inventory, Attacker* attacker);

	// Returns the sum of all equipped power modifiers.
	float getTotalPowerModifier() const;

	// Returns the sum of all equipped defense modifiers.
	float getTotalDefenseModifier() const;

	// Returns the sum of all equipped maxHp modifiers.
	float getTotalMaxHpModifier() const;

	// Returns the sum of all equipped skill modifiers.
	int getTotalSkillModifier() const;

	// Returns the total weight of all equipped items.
	float getEquippedWeight() const;

	// Returns true if the given actor is currently equipped in any slot.
	bool isEquipped(const Actor* item) const;

	void save(TCODZip& zip);
	void load(TCODZip& zip, Container& inventory);

private:
	std::array<Actor*, 4> slots = {};
};
