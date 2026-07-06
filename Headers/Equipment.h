#pragma once
#include <array>
#include <memory>
#include <vector>
#include "HitLocation.h"

class Actor;
class Container;
class Attacker;
class TCODZip;
enum class EquipmentSlot : int;

class Equipment {
public:
	// Returns the item in the given slot, or nullptr if empty.
	Actor* getSlot(EquipmentSlot slot) const;

	// Returns a const reference to the internal slots array for iteration.
	const std::array<Actor*, 4>& getSlots() const { return slots; }

	// Equips item into its target slot. Returns the previously equipped item
	// (which has been moved back to inventory), or nullptr if slot was empty.
	// Returns nullptr and does nothing if item has no equippable component.
	// If the item has a non-zero skill modifier, calls attacker->addModifier(skill).
	// If swapping out an old item with a non-zero skill modifier, calls
	// attacker->removeModifier(oldSkill) before adding the new one.
	// inventory can be nullptr (enemy path) — inventory management is skipped.
	Actor* equip(Actor* item, Container* inventory, Attacker* attacker);

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

	// Returns the sum of armour values from all equipped items for the given HitLocation.
	int getArmourAtLocation(HitLocation loc) const;

	// Returns the total weight of all equipped items.
	float getEquippedWeight() const;

	// Returns true if the given actor is currently equipped in any slot.
	bool isEquipped(const Actor* item) const;

	void save(TCODZip& zip, const Container& inventory);
	void load(TCODZip& zip, Container& inventory);

	// Owns item Actors for non-player entities that don't use Container.
	// Player equipment does NOT use this (items live in Container).
	std::vector<std::unique_ptr<Actor>> ownedItems;

	// Probability each equipped item drops on enemy death. Default 1.0 = always drop.
	float dropChance = 1.0f;

private:
	std::array<Actor*, 4> slots = {};
};
