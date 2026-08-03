# Requirements Document

## Introduction

Extend the existing Lua-driven Equipment system with comprehensive weapon classification metadata. Every weapon gains a Size Classification, a Weapon Group (proficiency category), and a Damage Type. Ranged weapons additionally store a range stat in tiles. These classifications drive downstream mechanics: proficiency penalties, critical-hit table selection, and combat-mode restrictions. The system is based on the Rogue Trader RPG weapon taxonomy.

## Glossary

- **Equipment_Loader**: The C++ subsystem (using sol2) that reads `Scripts/Equipment.lua` and creates in-memory weapon/armour templates at game initialization.
- **Weapon_Entry**: A single Lua table within the `equipment` array in `Equipment.lua` that defines a weapon or armour piece.
- **Size_Classification**: An enum categorizing every weapon into one of five physical form-factor classes (Melee, Pistol, Basic, Heavy, Thrown).
- **Weapon_Group**: An enum categorizing every weapon by its technology/firing mechanism, used for proficiency checks.
- **Damage_Type**: An enum categorizing the kind of harm a weapon inflicts, used to select the appropriate critical-hit table.
- **Proficiency_Penalty**: A −20 modifier applied to attack rolls when a character lacks the corresponding Weapon_Group talent.
- **Range_Stat**: An integer field on ranged weapons representing maximum effective distance in tiles.
- **Equippable_Component**: The C++ `Equippable` class on an Actor that stores weapon stats, armour profiles, and classification metadata.
- **Combat_Mode**: Whether a weapon is used in melee or ranged mode for a given attack action.

## Requirements

### Requirement 1: Size Classification Enum

**User Story:** As a developer, I want every weapon to carry a Size Classification, so that combat rules can enforce mode restrictions (e.g., Basic weapons cannot fire in close combat).

#### Acceptance Criteria

1. THE Equipment_Loader SHALL recognise exactly five Size_Classification values: "Melee", "Pistol", "Basic", "Heavy", and "Thrown".
2. WHEN a Weapon_Entry contains a `sizeClass` field with a valid value, THE Equipment_Loader SHALL store the corresponding Size_Classification on the Equippable_Component.
3. WHEN a Weapon_Entry contains a `sizeClass` field with an unrecognised value, THE Equipment_Loader SHALL log a warning and skip the entry.
4. WHEN a Weapon_Entry omits the `sizeClass` field entirely, THE Equipment_Loader SHALL log a warning and skip the entry.

### Requirement 2: Weapon Group Enum

**User Story:** As a developer, I want every weapon to carry a Weapon Group tag, so that the proficiency system can apply a −20 penalty when a character lacks training.

#### Acceptance Criteria

1. THE Equipment_Loader SHALL recognise exactly nine Weapon_Group values: "Las", "SP", "Bolt", "Melta", "Plasma", "Flame", "Primitive", "Launcher", and "Exotic".
2. WHEN a Weapon_Entry contains a `weaponGroup` field with a valid value, THE Equipment_Loader SHALL store the corresponding Weapon_Group on the Equippable_Component.
3. WHEN a Weapon_Entry contains a `weaponGroup` field with an unrecognised value, THE Equipment_Loader SHALL log a warning and skip the entry.
4. WHEN a Weapon_Entry omits the `weaponGroup` field and the entry has melee or ranged stats, THE Equipment_Loader SHALL log a warning and skip the entry.
5. WHEN a Weapon_Entry has no melee or ranged stats (non-weapon equipment such as armour), THE Equipment_Loader SHALL accept a missing `weaponGroup` field without warning.

### Requirement 3: Damage Type Enum

**User Story:** As a developer, I want every weapon to carry a Damage Type, so that the critical-hit system can select the correct injury table when a target drops below zero wounds.

#### Acceptance Criteria

1. THE Equipment_Loader SHALL recognise exactly four Damage_Type values: "E" (Energy), "X" (Explosive), "I" (Impact), and "R" (Rending).
2. WHEN a Weapon_Entry contains a `damageType` field with a valid value, THE Equipment_Loader SHALL store the corresponding Damage_Type on the Equippable_Component.
3. WHEN a Weapon_Entry contains a `damageType` field with an unrecognised value, THE Equipment_Loader SHALL log a warning and skip the entry.
4. WHEN a Weapon_Entry omits the `damageType` field and the entry has melee or ranged stats, THE Equipment_Loader SHALL log a warning and skip the entry.
5. WHEN a Weapon_Entry has no melee or ranged stats (non-weapon equipment), THE Equipment_Loader SHALL accept a missing `damageType` field without warning.

### Requirement 4: Range Stat on Ranged Weapons

**User Story:** As a developer, I want the range field within a weapon's `ranged` table to represent maximum effective distance in tiles, so that the ranged combat system can enforce distance limits.

#### Acceptance Criteria

1. THE Equipment_Loader SHALL read the `range` field from the `ranged` table as a positive integer representing distance in tiles.
2. WHEN the `range` field is present and greater than zero, THE Equipment_Loader SHALL store the value on the RangedStats of the Equippable_Component.
3. WHEN the `range` field is zero or negative, THE Equipment_Loader SHALL log a warning and skip the entry.
4. WHEN the `range` field is absent from a `ranged` table, THE Equipment_Loader SHALL apply a default value of 30 tiles.

### Requirement 5: Classification Storage on Equippable Component

**User Story:** As a developer, I want the Size Classification, Weapon Group, and Damage Type stored on the Equippable component in C++, so that combat, UI, and proficiency systems can query them at runtime.

#### Acceptance Criteria

1. THE Equippable_Component SHALL expose a `sizeClass` field of type Size_Classification.
2. THE Equippable_Component SHALL expose a `weaponGroup` field of type optional Weapon_Group (empty for non-weapon items).
3. THE Equippable_Component SHALL expose a `damageType` field of type optional Damage_Type (empty for non-weapon items).
4. WHEN a weapon is equipped, THE Equippable_Component SHALL retain the classification metadata without modification.

### Requirement 6: Proficiency Penalty Application

**User Story:** As a player, I want to suffer a −20 penalty to attack rolls when wielding a weapon whose group I lack training in, so that specialisation matters.

#### Acceptance Criteria

1. WHEN a character attacks with a weapon whose Weapon_Group does not match any of the character's proficiency talents, THE combat system SHALL apply a −20 modifier to the attack roll.
2. WHEN a character attacks with a weapon whose Weapon_Group matches one of the character's proficiency talents, THE combat system SHALL apply no proficiency penalty.
3. THE Proficiency_Penalty SHALL stack additively with other situational modifiers already present on the Attacker.

### Requirement 7: Size Classification Combat Restrictions

**User Story:** As a player, I want combat mode restrictions based on weapon size, so that pistols work in melee while rifles do not.

#### Acceptance Criteria

1. WHILE a character is adjacent to a target, THE combat system SHALL allow Pistol-class weapons to fire ranged attacks.
2. WHILE a character is adjacent to a target, THE combat system SHALL prevent Basic-class and Heavy-class weapons from firing ranged attacks.
3. WHEN a character attempts to fire a Basic-class or Heavy-class weapon at an adjacent target, THE combat system SHALL display a message indicating the weapon cannot be used at this range.
4. THE combat system SHALL allow Melee-class weapons to attack only adjacent targets.
5. THE combat system SHALL allow Thrown-class weapons to target any tile within their range stat.

### Requirement 8: Equipment.lua Schema Update

**User Story:** As a content author, I want clear Lua fields for weapon classification, so that I can define new weapons with correct metadata.

#### Acceptance Criteria

1. THE Weapon_Entry schema SHALL include optional string fields: `sizeClass`, `weaponGroup`, and `damageType`.
2. WHEN a Weapon_Entry defines melee or ranged stats, THE Equipment_Loader SHALL require `sizeClass`, `weaponGroup`, and `damageType` fields to be present.
3. THE Equipment_Loader SHALL validate all classification fields before creating the weapon template.
4. WHEN validation fails for any classification field, THE Equipment_Loader SHALL log a descriptive warning naming the weapon and the invalid field, then skip the entire entry.

### Requirement 9: Serialization of Classification Data

**User Story:** As a player, I want weapon classifications to persist across save and load, so that proficiency and combat rules remain consistent after reloading.

#### Acceptance Criteria

1. WHEN saving an Equippable_Component that has classification fields, THE serialization system SHALL write Size_Classification, Weapon_Group, and Damage_Type to the save file.
2. WHEN loading an Equippable_Component, THE serialization system SHALL restore Size_Classification, Weapon_Group, and Damage_Type from the save file.

### Requirement 10: Existing Weapons Retrofitted

**User Story:** As a developer, I want all existing weapons in Equipment.lua to carry correct classification metadata, so that the new systems apply uniformly from day one.

#### Acceptance Criteria

1. THE Combat_Knife entry SHALL have sizeClass "Melee", weaponGroup "Primitive", and damageType "R".
2. THE Chainsword entry SHALL have sizeClass "Melee", weaponGroup "Primitive", and damageType "R".
3. THE Power_Sword entry SHALL have sizeClass "Melee", weaponGroup "Primitive", and damageType "E".
4. THE Laspistol entry SHALL have sizeClass "Pistol", weaponGroup "Las", and damageType "E".
5. THE Autogun entry SHALL have sizeClass "Basic", weaponGroup "SP", and damageType "I".
6. THE Choppa entry SHALL have sizeClass "Melee", weaponGroup "Primitive", and damageType "R".
7. THE Slugga entry SHALL have sizeClass "Pistol", weaponGroup "SP", and damageType "I".
8. THE Shoota entry SHALL have sizeClass "Basic", weaponGroup "SP", and damageType "I".
9. THE Big_Choppa entry SHALL have sizeClass "Melee", weaponGroup "Primitive", and damageType "I".
10. THE Power_Klaw entry SHALL have sizeClass "Melee", weaponGroup "Exotic", and damageType "I".
