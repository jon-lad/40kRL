# Requirements Document

## Introduction

This document specifies requirements for equipping enemies (NPCs) with items from the existing Equipment.lua templates and dropping those items as loot when the enemy dies. This extends the equipment system (originally player-only) to enemy actors, making enemy combat stats equipment-driven and providing a loot acquisition loop. Equipment is tiered (common, uncommon, rare) — enemies generally carry common-tier gear, with a small chance of rare drops that can shift the player's build. Faction/species restrictions on equipment usage are explicitly deferred to a future phase — any actor can use any equipment in this phase.

## Glossary

- **Enemy_Actor**: A non-player Actor with an Ai component and a Destructible component that is hostile to the player.
- **Equipment_System**: The subsystem managing equipment slots, equip/unequip actions, and stat modifier calculations (existing, originally player-only).
- **Enemy_Equipment**: An Equipment instance attached to an Enemy_Actor that tracks which items occupy each Equipment_Slot on that enemy.
- **Equipment_Template**: A data record loaded from Equipment.lua defining an item's name, glyph, color, slot, weight, value, and stat modifiers.
- **Enemy_Template**: A data record in Enemies.lua defining an enemy type's base stats, appearance, and optionally its starting equipment loadout.
- **Loot_Drop**: The process of converting an equipped item on a dying Enemy_Actor into a Pickable Actor placed on the map at the corpse location.
- **Drop_Chance**: A per-item probability (0.0 to 1.0) that determines whether a specific equipped item is dropped on death.
- **Effective_Power**: The Attacker power value after adding all power stat modifiers from equipped items.
- **Effective_Defense**: The Destructible defense value after adding all defense stat modifiers from equipped items.
- **Equipment_Slot**: A named position where one item can be worn or wielded. Valid slots: weapon, offhand, head, body.
- **Stat_Modifier**: A numeric bonus or penalty applied to a base stat (power, defense, maxHp, skill) while an item is equipped.
- **Pickable_Actor**: An Actor on the map with a Pickable component that the player can walk over and pick up.
- **Item_Tier**: A rarity classification assigned to each Equipment_Template. Valid tiers are: common, uncommon, rare. Higher tiers have stronger stat modifiers and are less frequently assigned to enemies.
- **Tier_Weight**: A per-tier probability weight used when randomly selecting equipment for an enemy from a pool of valid items for a given slot.

## Requirements

### Requirement 1: Enemy Equipment Component

**User Story:** As a designer, I want enemies to have an Equipment component so that they can hold items in equipment slots just like the player.

#### Acceptance Criteria

1. WHEN an Enemy_Actor is spawned with a configured equipment loadout, THE Equipment_System SHALL create an Equipment instance on that Enemy_Actor.
2. THE Enemy_Equipment SHALL support the same four Equipment_Slots as the player: weapon, offhand, head, body.
3. THE Enemy_Equipment SHALL hold at most one item per Equipment_Slot at any time.

### Requirement 2: Enemy Spawning with Equipment

**User Story:** As a designer, I want to configure enemy equipment loadouts in Lua so that I can balance encounters without recompiling.

#### Acceptance Criteria

1. THE Enemy_Template SHALL support an optional "equipment" field containing a list of Equipment_Template names to equip on spawn.
2. WHEN an Enemy_Actor is spawned and the Enemy_Template specifies an equipment list, THE Equipment_System SHALL create item Actors from the named Equipment_Templates and equip them into the appropriate Equipment_Slots on the Enemy_Actor.
3. IF an Enemy_Template references an Equipment_Template name that does not exist in the loaded templates, THEN THE Equipment_System SHALL log a warning and skip that item without aborting enemy creation.
4. IF an Enemy_Template specifies multiple items targeting the same Equipment_Slot, THEN THE Equipment_System SHALL equip only the last item listed for that slot and log a warning about the conflict.
5. WHEN an Enemy_Template does not specify an equipment field, THE Equipment_System SHALL spawn the Enemy_Actor with no equipment and no Equipment instance.

### Requirement 3: Enemy Combat Stat Modifiers

**User Story:** As a player, I want enemies with better equipment to be stronger in combat so that loot drops feel proportional to encounter difficulty.

#### Acceptance Criteria

1. WHEN an Enemy_Actor with equipped items attacks a target, THE Attacker component SHALL calculate Effective_Power as the enemy base power plus the sum of all power Stat_Modifiers from the Enemy_Equipment.
2. WHEN an Enemy_Actor with equipped items takes damage, THE Destructible component SHALL use Effective_Defense as the enemy base defense plus the sum of all defense Stat_Modifiers from the Enemy_Equipment.
3. WHEN an Enemy_Actor has equipped items with skill Stat_Modifiers, THE Attacker component SHALL include those modifiers in the enemy hit chance calculation via the existing modifier system.
4. WHEN an Enemy_Actor has no equipment, THE Attacker component SHALL use the base power and base skill values with no equipment modifier.

### Requirement 4: Equipment Drop on Death

**User Story:** As a player, I want slain enemies to drop their equipment so that I can loot gear from defeated foes.

#### Acceptance Criteria

1. WHEN an Enemy_Actor dies, THE Equipment_System SHALL evaluate each item in the Enemy_Equipment for dropping.
2. WHEN an equipped item's Drop_Chance roll succeeds, THE Equipment_System SHALL create a Pickable_Actor on the map at the Enemy_Actor corpse position with the same name, glyph, color, Equipment_Slot, weight, value, and Stat_Modifiers as the equipped item.
3. WHEN an equipped item's Drop_Chance roll fails, THE Equipment_System SHALL discard that item without placing it on the map.
4. THE Equipment_System SHALL evaluate each equipped item independently using its own Drop_Chance value.
5. WHEN multiple items are dropped from a single Enemy_Actor, THE Equipment_System SHALL place all dropped items at the same map position as the corpse.

### Requirement 5: Drop Chance Configuration

**User Story:** As a designer, I want to configure drop chances per enemy template so that loot rarity is tunable without code changes.

#### Acceptance Criteria

1. THE Enemy_Template SHALL support an optional "dropChance" field as a float value between 0.0 and 1.0 inclusive.
2. WHEN a "dropChance" is specified in the Enemy_Template, THE Equipment_System SHALL use that value as the probability for each equipped item to drop on death.
3. WHEN a "dropChance" is not specified in the Enemy_Template, THE Equipment_System SHALL default to a Drop_Chance of 1.0 (all equipped items always drop).
4. IF a "dropChance" value is outside the range 0.0 to 1.0, THEN THE Equipment_System SHALL clamp the value to the nearest bound (0.0 or 1.0) and log a warning.

### Requirement 6: Dropped Loot Pickup Integration

**User Story:** As a player, I want to pick up dropped enemy equipment the same way I pick up other items so that the loot experience is seamless.

#### Acceptance Criteria

1. THE Pickable_Actor created from a Loot_Drop SHALL be fully compatible with the existing pickup system (Pickable::pick, carrying capacity check, inventory management).
2. THE Pickable_Actor created from a Loot_Drop SHALL have an Equippable component so that the player can equip the looted item.
3. WHEN the player walks over a tile containing a dropped Pickable_Actor, THE Equipment_System SHALL allow the player to pick it up subject to the existing carrying capacity rules.
4. THE Loot_Drop items SHALL be visually indistinguishable from items spawned directly from Equipment_Templates (same glyph, color, name).

### Requirement 7: Lua Enemy Equipment Definition

**User Story:** As a designer, I want to define enemy equipment loadouts in Enemies.lua alongside other enemy stats so that all enemy configuration remains in one place.

#### Acceptance Criteria

1. THE Enemies.lua script SHALL support an "equipment" field in each enemy definition as a list of strings referencing Equipment_Template names.
2. THE Enemies.lua script SHALL support a "dropChance" field in each enemy definition as a float.
3. WHEN the Enemies.lua file is loaded, THE Equipment_System SHALL validate that all referenced Equipment_Template names exist in the loaded equipment templates.
4. THE Equipment_System SHALL load Equipment.lua before Enemies.lua to ensure all Equipment_Templates are available for reference.

### Requirement 8: Item Tier System

**User Story:** As a player, I want enemy equipment to generally be weaker than what I find in the world, with a small chance of rare drops that could change my playstyle, so that loot feels rewarding without trivializing progression.

#### Acceptance Criteria

1. THE Equipment_Template SHALL support a "tier" field with valid values: "common", "uncommon", "rare".
2. WHEN a "tier" field is not specified in an Equipment_Template, THE Equipment_System SHALL default to "common".
3. WHEN an Enemy_Template specifies equipment by tier rather than by name, THE Equipment_System SHALL randomly select an item from the available templates matching the requested slot and tier.
4. THE Enemy_Template SHALL support an optional "equipTier" field that specifies a weighted probability for which tier of equipment the enemy spawns with. Default weights SHALL be: common = 70%, uncommon = 25%, rare = 5%.
5. WHEN an Enemy_Template specifies both a named "equipment" list and an "equipTier" field, THE named list SHALL take precedence and "equipTier" SHALL be ignored.
6. THE Equipment_System SHALL ensure that common-tier items have generally lower stat modifiers than uncommon-tier items, and uncommon-tier items have generally lower stat modifiers than rare-tier items (enforced by Lua data authoring, not by code validation).
7. THE Item_Tier SHALL be a designer-facing concept only — the tier is NOT displayed to the player. Players discover item quality by comparing stat modifiers themselves.

