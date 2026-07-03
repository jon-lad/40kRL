# Requirements Document

## Introduction

This document specifies requirements for an equipment system in 40kRL. The system allows the player to equip items into dedicated body slots (weapon, offhand, head, body), gaining stat bonuses from equipped gear. Phase 1 focuses on melee weapons that increase attack power, with the architecture designed to support armor and accessory slots in future phases. Items gain weight and gold value properties, and the player is subject to a carrying capacity limit.

## Glossary

- **Equipment_System**: The subsystem responsible for managing equipment slots, equip/unequip actions, and stat modifier calculations.
- **Equippable_Component**: A component attached to an Actor that marks the item as equippable, defining its target slot and stat modifiers.
- **Equipment_Slot**: A named position on the player where one item can be worn or wielded. Valid slots are: weapon, offhand, head, body.
- **Stat_Modifier**: A numeric bonus or penalty applied to a base stat (power, defense, maxHp, skill) while an item is equipped.
- **Skill_Modifier**: An integer bonus or penalty applied to the Attacker's hit chance modifier vector. Positive values increase hit chance; negative values decrease it. The modifier is added via addModifier() on equip and removed via removeModifier() on unequip.
- **Carrying_Capacity**: The maximum total weight the player can carry across all inventory and equipped items.
- **Item_Weight**: A non-negative float representing how heavy an item is in abstract weight units.
- **Item_Value**: A non-negative integer representing the gold worth of an item (used by future shop system).
- **Effective_Power**: The Attacker power value after adding all weapon Stat_Modifiers from equipped items.
- **Equipment_Menu**: A UI overlay that displays the player's currently equipped items organized by Equipment_Slot.
- **Lua_Equipment_Definition**: A table in a Lua script file that defines an equippable item's properties (glyph, name, color, slot, weight, value, stat modifiers).

## Requirements

### Requirement 1: Equippable Component

**User Story:** As a player, I want certain items to be equippable so that I can wear or wield them for stat bonuses.

#### Acceptance Criteria

1. THE Equippable_Component SHALL store a target Equipment_Slot, a list of Stat_Modifiers, an Item_Weight, and an Item_Value.
2. WHEN an Actor has both a Pickable_Component and an Equippable_Component, THE Equipment_System SHALL treat that Actor as an equippable item.
3. THE Equippable_Component SHALL support Stat_Modifiers for power, defense, maxHp, and skill properties.

### Requirement 2: Equipment Slots

**User Story:** As a player, I want distinct equipment slots so that I can equip different gear to different body parts.

#### Acceptance Criteria

1. THE Equipment_System SHALL define exactly four Equipment_Slots: weapon, offhand, head, body.
2. THE Equipment_System SHALL allow at most one item equipped per Equipment_Slot at any time.
3. WHEN an item is equipped to an occupied Equipment_Slot, THE Equipment_System SHALL unequip the previously equipped item and return that item to the player inventory before equipping the new item.

### Requirement 3: Equip Action

**User Story:** As a player, I want to equip items from my inventory so that I can gain their stat bonuses.

#### Acceptance Criteria

1. WHEN the player selects an equippable item from the inventory menu and chooses to equip it, THE Equipment_System SHALL move that item from the inventory into the appropriate Equipment_Slot.
2. WHEN an item is equipped, THE Equipment_System SHALL apply all of that item's Stat_Modifiers to the player's stats.
3. IF the player attempts to equip an item to a slot that does not match the item's target Equipment_Slot, THEN THE Equipment_System SHALL reject the action and display an error message.

### Requirement 4: Unequip Action

**User Story:** As a player, I want to unequip items so that I can swap gear or free up weight.

#### Acceptance Criteria

1. WHEN the player selects an equipped item and chooses to unequip it, THE Equipment_System SHALL move that item back to the player inventory.
2. WHEN an item is unequipped, THE Equipment_System SHALL remove all of that item's Stat_Modifiers from the player's stats.
3. IF the player's inventory is full when unequipping, THEN THE Equipment_System SHALL reject the unequip action and display a message indicating the inventory is full.

### Requirement 5: Effective Power Calculation

**User Story:** As a player, I want my equipped weapon to increase my attack damage so that gear progression feels meaningful.

#### Acceptance Criteria

1. WHEN the player attacks a target, THE Attacker component SHALL calculate Effective_Power as base power plus the sum of all power Stat_Modifiers from equipped items.
2. WHEN no weapon is equipped, THE Attacker component SHALL use the base power value with no modifier.
3. THE Equipment_System SHALL recalculate Effective_Power immediately when any item is equipped or unequipped.
4. WHEN an item with a skill Stat_Modifier is equipped, THE Equipment_System SHALL add that modifier to the Attacker's modifiers vector using addModifier.
5. WHEN an item with a skill Stat_Modifier is unequipped, THE Equipment_System SHALL remove that modifier from the Attacker's modifiers vector using removeModifier.

### Requirement 6: Equipment Menu

**User Story:** As a player, I want a dedicated equipment menu so that I can see what I have equipped at a glance.

#### Acceptance Criteria

1. WHEN the player presses the equipment menu key, THE Equipment_Menu SHALL display all four Equipment_Slots with the name of the item in each slot or "empty" if no item is equipped.
2. THE Equipment_Menu SHALL allow the player to select an equipped item and choose to unequip it.
3. THE Equipment_Menu SHALL close when the player presses escape or completes an action.

### Requirement 7: Item Weight and Carrying Capacity

**User Story:** As a player, I want items to have weight and a carrying limit so that inventory management adds a strategic element.

#### Acceptance Criteria

1. THE Equipment_System SHALL assign an Item_Weight to every item (equippable and non-equippable).
2. THE Equipment_System SHALL define a Carrying_Capacity for the player as a configurable value.
3. WHEN the player attempts to pick up an item, THE Equipment_System SHALL calculate the total weight of all carried items (inventory plus equipped) and reject the pickup if adding the item would exceed the Carrying_Capacity.
4. IF a pickup is rejected due to exceeding Carrying_Capacity, THEN THE Equipment_System SHALL display a message indicating the item is too heavy to carry.

### Requirement 8: Item Gold Value

**User Story:** As a designer, I want items to store a gold value so that a future shop system can reference item worth without code changes.

#### Acceptance Criteria

1. THE Equippable_Component SHALL store an Item_Value as a non-negative integer.
2. THE Equipment_System SHALL assign an Item_Value to every item (equippable and non-equippable) defaulting to zero if unspecified.
3. THE Item_Value SHALL have no gameplay effect in this phase beyond being stored and displayable.

### Requirement 9: Lua Equipment Definitions

**User Story:** As a designer, I want to define weapons in Lua so that new equipment can be added without recompiling the game.

#### Acceptance Criteria

1. THE Equipment_System SHALL load equippable item definitions from a Lua script file at game initialization.
2. WHEN a Lua_Equipment_Definition is loaded, THE Equipment_System SHALL create an item Actor with the specified glyph, name, color, Equipment_Slot, Item_Weight, Item_Value, and Stat_Modifiers.
3. THE Lua_Equipment_Definition SHALL support specifying any combination of power, defense, maxHp, and skill Stat_Modifiers.
4. WHEN a Lua_Equipment_Definition contains invalid data (missing required fields, unknown slot name, negative weight), THE Equipment_System SHALL log a warning and skip that definition.

### Requirement 10: Equipment Persistence

**User Story:** As a player, I want my equipment to persist across save and load so that I do not lose progress.

#### Acceptance Criteria

1. WHEN the game is saved, THE Equipment_System SHALL serialize all equipped items and their slot assignments to the save file.
2. WHEN the game is loaded, THE Equipment_System SHALL restore all equipped items to their correct Equipment_Slots and reapply Stat_Modifiers.
3. WHEN the game is loaded, THE Equipment_System SHALL restore Item_Weight and Item_Value for all items in the inventory and equipment.

### Requirement 11: Skill Modifier Integration

**User Story:** As a player, I want equipment to affect my hit chance so that weapon optics and heavy armor create meaningful trade-offs in combat accuracy.

#### Acceptance Criteria

1. WHEN an item with a skill Stat_Modifier is equipped, THE Equipment_System SHALL call addModifier on the player's Attacker component with the skill modifier value.
2. WHEN an item with a skill Stat_Modifier is unequipped, THE Equipment_System SHALL call removeModifier on the player's Attacker component with the skill modifier value.
3. WHEN multiple items with skill Stat_Modifiers are equipped simultaneously, THE Equipment_System SHALL add each modifier independently to the Attacker's modifiers vector.
4. WHEN an item with a skill Stat_Modifier value of zero is equipped, THE Equipment_System SHALL not add a modifier to the Attacker's modifiers vector.
5. THE Equipment_System SHALL support both positive skill Stat_Modifiers (accuracy bonus) and negative skill Stat_Modifiers (accuracy penalty) on equippable items.
