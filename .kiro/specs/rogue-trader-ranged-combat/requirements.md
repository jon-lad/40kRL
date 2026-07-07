# Requirements Document

## Introduction

This spec adds a ranged combat system to the existing Rogue Trader RPG melee combat. The player presses 's' to enter shooting mode (which reuses the existing tile-targeting cursor), selects a target, and resolves the shot using d100 roll-under mechanics against Ballistic Skill (BS). The player presses 'r' to reload their ranged weapon. Equipment.lua is extended with ranged weapon stats (damage dice, penetration, range, rate of fire, clip size, reload time). A new RangedAi is introduced for enemies that prefer to shoot at range and fall back to melee when cornered, while melee-focused enemies retain the existing MonsterAi chase-and-attack behaviour. Ammunition is tracked per-weapon as a current clip count, and reload consumes a turn.

## Glossary

- **Ranged_Combat_System**: The ranged attack resolution module that handles shooting, hit rolls, damage, and ammunition
- **Ballistic_Skill (BS)**: The characteristic value used for ranged hit rolls, stored in the Characteristics component at CharId::BS
- **Effective_BS**: The shooter's base BS characteristic value plus any situational modifiers, clamped to [1, 99]
- **Ranged_Hit_Roll**: A d100 roll compared against the shooter's Effective_BS to determine success or failure
- **Rate_of_Fire (RoF)**: The number of shots a weapon fires in a single attack action (single = 1, semi-auto = 2-3, full-auto = 4+)
- **Clip_Size**: The maximum number of shots a ranged weapon can hold before requiring a reload
- **Current_Ammo**: The number of shots remaining in the weapon's clip before it must be reloaded
- **Reload_Action**: A full-turn action that restores Current_Ammo to Clip_Size
- **Weapon_Range**: The maximum effective range in tiles for a ranged weapon
- **Line_of_Sight (LoS)**: An unobstructed straight line between shooter and target, computed using Bresenham or similar algorithm on the tile map
- **RangedStats**: A data structure on Equippable containing damageDice, penetration, range, rateOfFire, clipSize, and reloadTime for ranged weapons
- **RangedAi**: An AI behaviour for enemies equipped with ranged weapons that prefers shooting at range and closes to melee when adjacent or out of ammo
- **MonsterAi**: The existing melee-focused AI behaviour that chases and attacks in melee
- **Targeting_Mode**: The existing TARGETING game state (from the tile-targeting spec) reused for ranged target selection
- **Gui**: The message log system accessed via gui->message() for combat narration
- **Equipment_Loader**: The C++ subsystem that reads Equipment.lua and populates EquipmentTemplate instances
- **Destructible_Actor**: An Actor with a Destructible component but no Characteristics component (e.g., crates, barrels, doors)

## Requirements

### Requirement 1: Shoot Action Input

**User Story:** As a player, I want to press 's' to shoot my ranged weapon at a target, so that I can attack enemies from a distance.

#### Acceptance Criteria

1. WHEN the player presses 's' while in IDLE state and has a ranged weapon equipped in the weapon slot, THE Ranged_Combat_System SHALL transition the game to Targeting_Mode with maximum range set to the equipped weapon's Weapon_Range
2. WHEN the player presses 's' while in IDLE state and has no ranged weapon equipped, THE Gui SHALL display the message "You have no ranged weapon equipped."
3. WHEN the player presses 's' while in IDLE state and the equipped ranged weapon has zero Current_Ammo, THE Gui SHALL display the message "Your weapon is empty. Press 'r' to reload."
4. WHILE in Targeting_Mode initiated by the shoot action, THE Ranged_Combat_System SHALL only allow selection of tiles containing a living actor within Line_of_Sight and within Weapon_Range
5. WHEN the player confirms a valid target in Targeting_Mode, THE Ranged_Combat_System SHALL resolve the ranged attack against the targeted actor and advance the turn

### Requirement 2: Reload Action Input

**User Story:** As a player, I want to press 'r' to reload my ranged weapon, so that I can replenish ammunition and continue shooting.

#### Acceptance Criteria

1. WHEN the player presses 'r' while in IDLE state and has a ranged weapon equipped with Current_Ammo less than Clip_Size, THE Ranged_Combat_System SHALL set Current_Ammo to Clip_Size and advance the turn
2. WHEN the player presses 'r' while in IDLE state and has a ranged weapon equipped with Current_Ammo equal to Clip_Size, THE Gui SHALL display the message "Your weapon is already fully loaded."
3. WHEN the player presses 'r' while in IDLE state and has no ranged weapon equipped, THE Gui SHALL display the message "You have no ranged weapon to reload."
4. WHEN a reload is performed, THE Gui SHALL display the message "[Actor] reloads [Weapon_Name]."

### Requirement 3: Ranged Hit Roll Resolution

**User Story:** As a player, I want ranged attacks to resolve using a d100 roll against Ballistic Skill, so that shooting follows Rogue Trader RPG mechanics.

#### Acceptance Criteria

1. WHEN a ranged attack is resolved against a target with a Characteristics component, THE Ranged_Combat_System SHALL roll a d100 and compare the result against the shooter's Effective_BS
2. WHEN the d100 roll is less than or equal to the Effective_BS, THE Ranged_Combat_System SHALL classify the Ranged_Hit_Roll as a success
3. WHEN the d100 roll is greater than the Effective_BS, THE Ranged_Combat_System SHALL classify the Ranged_Hit_Roll as a failure
4. THE Ranged_Combat_System SHALL clamp the Effective_BS to the range [1, 99] after applying all modifiers
5. THE Ranged_Combat_System SHALL calculate Degrees_of_Success as floor((Effective_BS - roll) / 10) on a successful hit
6. THE Ranged_Combat_System SHALL calculate Degrees_of_Failure as floor((roll - Effective_BS) / 10) on a failed hit

### Requirement 4: Ranged Hit Location Determination

**User Story:** As a player, I want ranged hits to strike specific body locations, so that armour coverage matters for ranged combat as it does for melee.

#### Acceptance Criteria

1. WHEN a Ranged_Hit_Roll succeeds, THE Ranged_Combat_System SHALL determine the hit location using the same digit-reversal method as the melee Combat_System (reversed d100 roll mapped through the Hit_Location_Table)
2. THE Ranged_Combat_System SHALL use the existing HitLocationTable::resolve function to map reversed digits to a HitLocation enum value

### Requirement 5: Ranged Dodge Test

**User Story:** As a player, I want targets to have a chance to dodge ranged attacks, so that Agility provides a defensive benefit against shooting.

#### Acceptance Criteria

1. WHEN a Ranged_Hit_Roll succeeds against a target with a Characteristics component, THE Ranged_Combat_System SHALL offer the target a Dodge_Test by rolling d100 against the target's Agility characteristic
2. WHEN the Dodge_Test succeeds, THE Ranged_Combat_System SHALL negate one hit plus one additional hit per Degree_of_Success on the Dodge_Test
3. WHEN the Dodge_Test fails, THE Ranged_Combat_System SHALL proceed to damage calculation without negating any hits
4. THE Ranged_Combat_System SHALL allow only one Dodge_Test per ranged attack action for the defender
5. THE Ranged_Combat_System SHALL NOT offer a Parry_Test against ranged attacks

### Requirement 6: Ranged Damage Calculation

**User Story:** As a player, I want ranged damage to account for weapon stats, target armour, and target toughness, so that equipment and characteristics matter.

#### Acceptance Criteria

1. WHEN a ranged hit is not negated by dodge, THE Ranged_Combat_System SHALL calculate raw damage as: weapon_damage_roll + 0 (ranged weapons do not add Strength_Bonus)
2. THE Ranged_Combat_System SHALL calculate effective armour on the hit location as: max(0, location_armour - weapon_Penetration)
3. THE Ranged_Combat_System SHALL calculate final damage as: max(0, raw_damage - effective_armour - Toughness_Bonus)
4. WHEN final damage is greater than zero, THE Ranged_Combat_System SHALL reduce the target's Wounds by the final damage amount
5. WHEN final damage is zero or less, THE Ranged_Combat_System SHALL not reduce the target's Wounds
6. WHEN final damage reduces Wounds to zero or below, THE Ranged_Combat_System SHALL trigger critical hit effects using the same critical hit system as melee combat

### Requirement 7: Ammunition Tracking

**User Story:** As a player, I want my ranged weapon to consume ammunition when I shoot, so that I must manage ammo and reload strategically.

#### Acceptance Criteria

1. WHEN a ranged attack is resolved, THE Ranged_Combat_System SHALL reduce Current_Ammo by the Rate_of_Fire of the weapon (consuming one shot per hit attempt in the burst)
2. WHEN Current_Ammo reaches zero, THE Ranged_Combat_System SHALL prevent further ranged attacks until the weapon is reloaded
3. THE Ranged_Combat_System SHALL display Current_Ammo and Clip_Size information in the Gui when a ranged weapon is equipped
4. WHEN a weapon is first equipped or spawned, THE Ranged_Combat_System SHALL set Current_Ammo to Clip_Size (weapon starts fully loaded)

### Requirement 8: Line of Sight Validation

**User Story:** As a player, I want ranged attacks to require a clear line between me and the target, so that terrain and obstacles matter for shooting.

#### Acceptance Criteria

1. WHEN a target tile is selected in Targeting_Mode for a ranged attack, THE Ranged_Combat_System SHALL verify Line_of_Sight exists between the shooter and the target using a straight-line tile check
2. WHEN Line_of_Sight is blocked by a wall tile along the path, THE Ranged_Combat_System SHALL reject the target selection and remain in Targeting_Mode
3. THE Ranged_Combat_System SHALL use the existing FOV data to determine valid target tiles (a tile must be in the player's FOV and within Weapon_Range)

### Requirement 9: Equipment Schema Extension for Ranged Weapons

**User Story:** As a developer, I want Equipment.lua to support Rogue Trader ranged weapon stats, so that weapons define damage, range, ammo, and fire modes.

#### Acceptance Criteria

1. THE Equipment_Loader SHALL recognise an optional "ranged" table on weapon entries containing fields: damageDice (string, e.g., "1d10"), penetration (int), range (int, in tiles), rateOfFire (int), clipSize (int), and reloadTime (int, in turns)
2. WHEN a weapon entry contains a "ranged" table, THE Equipment_Loader SHALL parse all fields and store them as a RangedStats struct on the EquipmentTemplate
3. IF a weapon entry has a "ranged" table with an invalid damageDice format, THEN THE Equipment_Loader SHALL log a warning and skip that weapon entry
4. THE Equipment_Loader SHALL treat weapons that have both a "melee" and a "ranged" table as valid (pistol-type weapons usable in both modes)
5. WHEN a weapon has only a "ranged" table and no "melee" table, THE Equipment_Loader SHALL assign default melee stats for pistol-whip attacks (damageDice = "1d5", penetration = 0, qualities = empty)

### Requirement 10: Ranged AI Behaviour

**User Story:** As a player, I want enemies with ranged weapons to shoot at me from a distance, so that combat has tactical variety beyond melee.

#### Acceptance Criteria

1. WHILE a RangedAi-controlled enemy has Line_of_Sight to the player and is within Weapon_Range and has Current_Ammo greater than zero, THE RangedAi SHALL resolve a ranged attack against the player
2. WHILE a RangedAi-controlled enemy is adjacent to the player (distance less than 2 tiles), THE RangedAi SHALL resolve a melee attack against the player using the existing melee Combat_System
3. WHILE a RangedAi-controlled enemy has Line_of_Sight to the player but is beyond Weapon_Range, THE RangedAi SHALL move toward the player to close distance
4. WHILE a RangedAi-controlled enemy has no Line_of_Sight to the player, THE RangedAi SHALL follow the scent trail toward the player (same as MonsterAi)
5. WHILE a RangedAi-controlled enemy has zero Current_Ammo and is not adjacent to the player, THE RangedAi SHALL spend one turn performing a Reload_Action to restore Current_Ammo to Clip_Size
6. THE RangedAi SHALL use the same Effective_BS calculation (BS + modifiers, clamped to [1, 99]) as the player when resolving ranged attacks

### Requirement 11: Melee AI Preservation

**User Story:** As a developer, I want melee-focused enemies to retain their existing AI behaviour, so that adding ranged combat does not disrupt existing gameplay.

#### Acceptance Criteria

1. THE MonsterAi SHALL continue to chase the player using FOV line-of-sight and scent tracking without modification
2. THE MonsterAi SHALL continue to resolve melee attacks when adjacent to the player without modification
3. WHEN an enemy is defined without a ranged weapon in Enemies.lua, THE Engine SHALL assign MonsterAi to that enemy

### Requirement 12: Enemy AI Assignment

**User Story:** As a developer, I want enemy AI type to be determined by their equipment, so that new ranged enemies can be defined in Lua without C++ changes.

#### Acceptance Criteria

1. WHEN an enemy is spawned with a ranged weapon listed in their equipment field, THE Engine SHALL assign RangedAi to that enemy
2. WHEN an enemy is spawned without any ranged weapon in their equipment field, THE Engine SHALL assign MonsterAi to that enemy
3. THE Engine SHALL determine whether an equipment item is a ranged weapon by checking for the presence of a "ranged" table in the EquipmentTemplate

### Requirement 13: Ranged Combat Log Messages

**User Story:** As a player, I want detailed combat messages for ranged attacks, so that I understand what happened mechanically.

#### Acceptance Criteria

1. WHEN a ranged attack is initiated, THE Gui SHALL display a message in the format: "[Shooter] fires at [Target]"
2. WHEN a Ranged_Hit_Roll succeeds, THE Gui SHALL display a message in the format: "Hit! ([DoS] DoS) — [Location]"
3. WHEN a Ranged_Hit_Roll fails, THE Gui SHALL display a message in the format: "[Shooter] misses [Target]"
4. WHEN a Dodge_Test succeeds against a ranged attack, THE Gui SHALL display a message in the format: "[Defender] dodges [N] hit(s)"
5. WHEN ranged damage is dealt, THE Gui SHALL display a message in the format: "[Shooter] deals [N] damage to [Target]'s [Location]"
6. WHEN a ranged attack is attempted with zero Current_Ammo (AI edge case), THE Gui SHALL display the message "[Actor]'s weapon clicks empty"

### Requirement 14: Ranged Attack Against Destructible Actors

**User Story:** As a player, I want to shoot destructible objects like crates and doors without needing hit rolls, so that environmental interaction works at range.

#### Acceptance Criteria

1. WHEN a ranged attack targets a Destructible_Actor, THE Ranged_Combat_System SHALL skip the Ranged_Hit_Roll and treat the attack as an automatic hit
2. WHEN shooting a Destructible_Actor, THE Ranged_Combat_System SHALL skip the Dodge_Test
3. WHEN shooting a Destructible_Actor, THE Ranged_Combat_System SHALL calculate damage as: weapon_damage_roll (no armour subtraction, no Toughness_Bonus subtraction)
4. WHEN damage is dealt to a Destructible_Actor via ranged attack, THE Ranged_Combat_System SHALL reduce the Destructible_Actor's hp directly by the damage amount
5. WHEN shooting a Destructible_Actor, THE Ranged_Combat_System SHALL still consume one ammo from Current_Ammo

### Requirement 15: Ranged Weapon Serialization

**User Story:** As a developer, I want ranged weapon state (current ammo) to persist across save and load, so that the player's ammo count is preserved.

#### Acceptance Criteria

1. THE Ranged_Combat_System SHALL serialize Current_Ammo for each equipped ranged weapon during the save operation
2. THE Ranged_Combat_System SHALL deserialize Current_Ammo for each equipped ranged weapon during the load operation
3. THE Ranged_Combat_System SHALL maintain save/load compatibility by versioning the Equippable serialisation format with a new sentinel value
4. FOR ALL valid Equippable states containing RangedStats and Current_Ammo, serializing then deserializing SHALL produce an equivalent state (round-trip property)

