# Requirements Document

## Introduction

This spec replaces the existing Attacker/Destructible combat system with a Rogue Trader RPG-style melee combat resolution. The new system uses d100 roll-under mechanics against Weapon Skill (WS), calculates degrees of success/failure, determines hit locations by digit reversal, supports dodge and parry opposed tests, applies damage with per-location armour, and introduces critical hit effects when wounds are exceeded. The Equipment.lua schema is extended to support melee weapon stats (damage dice, penetration, qualities) and per-location armour values. Non-character destructibles (crates, barrels, doors) remain attackable with a simplified auto-hit path.

## Glossary

- **Combat_System**: The melee combat resolution module that replaces the existing Attacker component logic
- **Hit_Roll**: A d100 roll compared against the attacker's effective Weapon Skill to determine success or failure
- **Degrees_of_Success (DoS)**: The number of full tens by which the Hit_Roll succeeded: floor((Effective_WS - roll) / 10)
- **Degrees_of_Failure (DoF)**: The number of full tens by which the Hit_Roll failed: floor((roll - Effective_WS) / 10)
- **Hit_Location_Table**: A mapping from reversed d100 digits to one of six body locations (Head, Body, Left Arm, Right Arm, Left Leg, Right Leg)
- **Effective_WS**: The attacker's base WS characteristic value plus any situational modifiers, clamped to [1, 99]
- **Strength_Bonus (SB)**: The attacker's Strength characteristic divided by 10 (integer division)
- **Toughness_Bonus (TB)**: The defender's Toughness characteristic divided by 10 (integer division)
- **Penetration (Pen)**: A weapon property that reduces effective armour before damage calculation
- **Wounds**: The hit points of a character, tracked on the Destructible component (hp field)
- **Critical_Damage**: Additional damage effects applied when damage exceeds remaining Wounds on a hit location
- **Dodge_Test**: A d100 roll-under test against the defender's Agility (Ag) characteristic
- **Parry_Test**: A d100 roll-under test against the defender's WS characteristic, requiring a melee weapon
- **Equipment_Loader**: The C++ subsystem that reads Equipment.lua and populates EquipmentTemplate instances
- **Destructible_Actor**: An Actor with a Destructible component but no Characteristics component (e.g., crates, barrels, doors)
- **Gui**: The message log system accessed via gui->message() for combat narration

## Requirements

### Requirement 1: Hit Roll Resolution

**User Story:** As a player, I want melee attacks to resolve using a d100 roll against Weapon Skill, so that combat follows Rogue Trader RPG mechanics.

#### Acceptance Criteria

1. WHEN an Actor with a Characteristics component initiates a melee attack against a target with a Characteristics component, THE Combat_System SHALL roll a d100 and compare the result against the attacker's Effective_WS
2. WHEN the d100 roll is less than or equal to the Effective_WS, THE Combat_System SHALL classify the Hit_Roll as a success
3. WHEN the d100 roll is greater than the Effective_WS, THE Combat_System SHALL classify the Hit_Roll as a failure
4. THE Combat_System SHALL clamp the Effective_WS to the range [1, 99] after applying all modifiers

### Requirement 2: Degrees of Success and Failure

**User Story:** As a player, I want to know how well an attack succeeded or failed, so that I can understand the magnitude of the result.

#### Acceptance Criteria

1. WHEN a Hit_Roll succeeds, THE Combat_System SHALL calculate Degrees_of_Success as floor((Effective_WS - roll) / 10)
2. WHEN a Hit_Roll fails, THE Combat_System SHALL calculate Degrees_of_Failure as floor((roll - Effective_WS) / 10)
3. THE Combat_System SHALL guarantee a minimum of zero Degrees_of_Success on any successful hit
4. THE Combat_System SHALL guarantee a minimum of zero Degrees_of_Failure on any failed hit

### Requirement 3: Hit Location Determination

**User Story:** As a player, I want successful hits to target specific body locations, so that combat has tactical variety and armour coverage matters.

#### Acceptance Criteria

1. WHEN a Hit_Roll succeeds, THE Combat_System SHALL reverse the two digits of the d100 roll to produce a hit location value (e.g., a roll of 32 produces location value 23)
2. THE Combat_System SHALL map location values to body locations using the Hit_Location_Table: 01-10 = Head, 11-20 = Right Arm, 21-30 = Left Arm, 31-70 = Body, 71-80 = Right Leg, 81-00 = Left Leg
3. WHEN a single-digit roll occurs (01-09), THE Combat_System SHALL treat the tens digit as 0 for reversal purposes (e.g., roll 05 reverses to 50 = Body)

### Requirement 4: Dodge Test

**User Story:** As a player, I want defenders to attempt dodging attacks, so that Agility provides a defensive benefit in melee combat.

#### Acceptance Criteria

1. WHEN a Hit_Roll succeeds against a target with a Characteristics component, THE Combat_System SHALL offer the target a Dodge_Test by rolling d100 against the target's Agility characteristic
2. WHEN the Dodge_Test succeeds, THE Combat_System SHALL negate one hit plus one additional hit per Degree_of_Success on the Dodge_Test
3. WHEN the Dodge_Test fails, THE Combat_System SHALL proceed to damage calculation without negating any hits
4. THE Combat_System SHALL allow only one Dodge_Test per attack action for the defender

### Requirement 5: Parry Test

**User Story:** As a player, I want defenders with melee weapons to attempt parrying attacks, so that Weapon Skill provides a defensive benefit when armed.

#### Acceptance Criteria

1. WHEN a Hit_Roll succeeds against a target that has a melee weapon equipped in the weapon slot, THE Combat_System SHALL offer the target a Parry_Test by rolling d100 against the target's WS characteristic
2. WHEN the Parry_Test succeeds, THE Combat_System SHALL negate one hit plus one additional hit per Degree_of_Success on the Parry_Test
3. WHEN the Parry_Test fails, THE Combat_System SHALL proceed without negating any hits from the Parry_Test
4. THE Combat_System SHALL allow only one Parry_Test per attack action for the defender
5. WHILE a defender has no melee weapon equipped, THE Combat_System SHALL skip the Parry_Test entirely

### Requirement 6: Damage Calculation

**User Story:** As a player, I want damage to account for weapon stats, attacker strength, and defender toughness and armour, so that equipment and characteristics matter in combat.

#### Acceptance Criteria

1. WHEN a hit is not negated by dodge or parry, THE Combat_System SHALL calculate raw damage as: weapon_damage_roll + Strength_Bonus
2. THE Combat_System SHALL calculate effective armour on the hit location as: max(0, location_armour - weapon_Penetration)
3. THE Combat_System SHALL calculate final damage as: max(0, raw_damage - effective_armour - Toughness_Bonus)
4. WHEN final damage is greater than zero, THE Combat_System SHALL reduce the target's Wounds by the final damage amount
5. WHEN final damage is zero or less, THE Combat_System SHALL not reduce the target's Wounds

### Requirement 7: Critical Hits

**User Story:** As a player, I want devastating blows that exceed a target's remaining health to trigger critical effects, so that combat has dramatic outcomes.

#### Acceptance Criteria

1. WHEN final damage reduces Wounds to zero or below, THE Combat_System SHALL calculate critical damage as the absolute value of remaining negative Wounds
2. WHEN critical damage occurs, THE Combat_System SHALL apply a critical effect based on the hit location and the critical damage magnitude
3. THE Combat_System SHALL trigger the target's death when critical damage reaches or exceeds a severity threshold of 5 on any location
4. THE Combat_System SHALL log a descriptive critical hit message to the Gui indicating the location and severity

### Requirement 8: Equipment Schema Extension for Melee Weapons

**User Story:** As a developer, I want Equipment.lua to support Rogue Trader melee weapon stats, so that weapons define damage dice, penetration, and special qualities.

#### Acceptance Criteria

1. THE Equipment_Loader SHALL recognise an optional "melee" table on weapon entries containing fields: damageDice (string, e.g., "1d10"), penetration (int), and qualities (table of strings)
2. WHEN a weapon entry contains a "melee" table, THE Equipment_Loader SHALL parse damageDice into a count and die size (e.g., "1d10" = 1 die of 10 sides)
3. WHEN a weapon entry contains a "melee" table, THE Equipment_Loader SHALL store the penetration value as an integer on the EquipmentTemplate
4. IF a weapon entry has a "melee" table with an invalid damageDice format, THEN THE Equipment_Loader SHALL log a warning and skip that weapon entry
5. THE Equipment_Loader SHALL treat weapons without a "melee" table as legacy items and assign default melee stats (damageDice = "1d5", penetration = 0, qualities = empty)

### Requirement 9: Equipment Schema Extension for Per-Location Armour

**User Story:** As a developer, I want armour items to define protection values per body location, so that hit location interacts meaningfully with armour coverage.

#### Acceptance Criteria

1. THE Equipment_Loader SHALL recognise an optional "armourLocations" table on body-slot and head-slot entries containing fields: head (int), body (int), leftArm (int), rightArm (int), leftLeg (int), rightLeg (int)
2. WHEN an armour entry contains an "armourLocations" table, THE Equipment_Loader SHALL store per-location values on the EquipmentTemplate
3. WHEN an armour entry omits "armourLocations", THE Equipment_Loader SHALL use the existing defense field as a uniform value for all covered locations based on slot (head slot covers Head only; body slot covers Body, Arms, and Legs)
4. THE Combat_System SHALL sum armour values from all equipped items for the specific hit location when calculating effective armour

### Requirement 10: Non-Character Destructible Attacks

**User Story:** As a player, I want to attack destructible objects like crates and doors without needing opposed tests, so that environmental interaction is straightforward.

#### Acceptance Criteria

1. WHEN an Actor initiates a melee attack against a Destructible_Actor, THE Combat_System SHALL skip the Hit_Roll and treat the attack as an automatic hit
2. WHEN attacking a Destructible_Actor, THE Combat_System SHALL skip Dodge_Test and Parry_Test resolution
3. WHEN attacking a Destructible_Actor, THE Combat_System SHALL calculate damage as: weapon_damage_roll + Strength_Bonus (no armour subtraction, no Toughness_Bonus subtraction)
4. WHEN damage is dealt to a Destructible_Actor, THE Combat_System SHALL reduce the Destructible_Actor's hp directly by the damage amount
5. IF the attacker has no Characteristics component (edge case: destructible vs destructible), THEN THE Combat_System SHALL use a default Strength_Bonus of 3

### Requirement 11: Combat Log Messages

**User Story:** As a player, I want detailed combat messages describing each step of an attack, so that I understand what happened mechanically.

#### Acceptance Criteria

1. WHEN an attack is initiated, THE Gui SHALL display an attack declaration message in the format: "[Attacker] swings at [Target]"
2. WHEN a Hit_Roll succeeds, THE Gui SHALL display a hit message in the format: "Hit! ([DoS] DoS) — [Location]"
3. WHEN a Hit_Roll fails, THE Gui SHALL display a miss message in the format: "[Attacker] misses [Target]"
4. WHEN a Dodge_Test or Parry_Test succeeds, THE Gui SHALL display a defence message in the format: "[Defender] [dodges/parries] [N] hit(s)"
5. WHEN damage is dealt, THE Gui SHALL display a damage message in the format: "[Attacker] deals [N] damage to [Target]'s [Location]"
6. WHEN critical damage occurs, THE Gui SHALL display a critical hit message describing the severity and location
7. WHEN attacking a Destructible_Actor, THE Gui SHALL display a simplified message in the format: "[Attacker] strikes [Target] for [N] damage"

### Requirement 12: Damage Dice Roller

**User Story:** As a developer, I want a reusable dice roller that parses NdX format strings, so that weapon damage and other rolls use a consistent mechanism.

#### Acceptance Criteria

1. THE Combat_System SHALL parse damage dice strings in the format "NdX" where N is the number of dice (1-9) and X is the number of sides per die (1-100)
2. WHEN rolling damage, THE Combat_System SHALL roll N dice each producing a value in [1, X] and return the sum
3. IF a damage dice string does not match the "NdX" format, THEN THE Combat_System SHALL return 0 and log a warning
4. FOR ALL valid NdX strings, parsing then formatting back to a string SHALL produce an equivalent NdX representation (round-trip property)

### Requirement 13: Replace Existing Attacker System

**User Story:** As a developer, I want the new Combat_System to fully replace the existing Attacker::attack() logic, so that there is one unified combat path.

#### Acceptance Criteria

1. THE Combat_System SHALL replace the existing Attacker::attack() method with the new d100 roll-under, hit location, dodge/parry, and damage pipeline
2. THE Combat_System SHALL preserve the injectable rollD100 function on the Attacker component for unit testing
3. THE Combat_System SHALL remove the legacy "power minus defense" damage formula from the attack path
4. THE Combat_System SHALL maintain save/load compatibility by versioning the serialisation format with a new sentinel value
