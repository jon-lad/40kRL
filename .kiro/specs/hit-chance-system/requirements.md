# Requirements Document

## Introduction

This feature introduces a hit/miss chance mechanic to the 40kRL combat system, replacing the current unconditional damage model. The system is inspired by the Rogue Trader RPG's d100 percentile resolution mechanic, where attackers roll a d100 and must score equal to or under their effective skill percentage to land a hit. Both the player and enemies use the same "roll under" resolution system. The mechanic integrates with the existing Attacker component and is configurable through Lua scripts, allowing per-actor skill percentages and modifier support from equipment or situational effects.

## Glossary

- **Hit_Chance_System**: The subsystem responsible for determining whether an attack connects with its target before damage is applied, using a d100 percentile "roll under" mechanic inspired by the Rogue Trader RPG.
- **Rogue_Trader_RPG**: A tabletop role-playing game set in the Warhammer 40,000 universe that uses a d100 (percentile) system for skill resolution. Actors have percentage-based skills and must roll equal to or under their skill value on a d100 to succeed.
- **Skill_Value**: An integer from 1 to 99 representing an actor's combat proficiency as a percentage. In the Rogue Trader RPG, this corresponds to Weapon Skill (WS) for melee attacks or Ballistic Skill (BS) for ranged attacks. A Skill_Value of 35 means the actor has a 35% base chance to hit.
- **Weapon_Skill (WS)**: The Rogue Trader RPG term for melee combat proficiency, expressed as a percentage. Used as the Skill_Value for melee attacks.
- **Ballistic_Skill (BS)**: The Rogue Trader RPG term for ranged combat proficiency, expressed as a percentage. Used as the Skill_Value for ranged attacks.
- **Hit_Roll**: A uniformly distributed random integer from 1 to 100 (inclusive) generated for each attack attempt, representing a d100 (percentile die) roll.
- **Hit_Threshold**: The effective Skill_Value after modifiers are applied. The attack hits if Hit_Roll is less than or equal to the Hit_Threshold. This is the Rogue Trader RPG "roll under" mechanic.
- **Modifier**: A signed integer added to or subtracted from the base Skill_Value to produce the effective Hit_Threshold (e.g., +10 for flanking, -20 for cover).
- **Attacker_Component**: The existing C++ class (`Attacker`) that owns the `power` field and the `attack()` method.
- **Message_Log**: The GUI message panel managed by `Gui::message()` that displays combat feedback to the player.
- **Config_Script**: The Lua file (`Config.lua`) that holds global game configuration values loaded at startup.
- **Enemy_Script**: The Lua file (`Enemies.lua`) that defines enemy templates including their stats.
- **D100_Percentile_System**: A resolution mechanic where a random number from 1 to 100 is rolled and compared against a target percentage. Success occurs when the roll is equal to or less than the target number.

## Requirements

### Requirement 1: Skill Value Attribute

**User Story:** As a game designer, I want each attacking actor to have a configurable percentage-based skill value (inspired by the Rogue Trader RPG's WS/BS characteristics), so that different enemies and the player can have distinct accuracy levels.

#### Acceptance Criteria

1. THE Attacker_Component SHALL store a Skill_Value as an integer field with a valid range of 1 to 99, clamping any provided value that falls outside this range to the nearest bound (1 or 99)
2. WHEN an Attacker_Component is constructed without an explicit Skill_Value, THE Attacker_Component SHALL default the Skill_Value to 40, representing a typical combatant's percentage chance to hit
3. THE Config_Script SHALL expose an optional `playerSkill` integer field that sets the player's initial Skill_Value; IF the `playerSkill` field is absent from the Config_Script, THEN THE Attacker_Component SHALL use the default Skill_Value of 40 for the player
4. THE Enemy_Script SHALL accept an optional `skill` integer field per enemy template that sets that enemy type's Skill_Value as a percentage; IF the `skill` field is omitted from an enemy template, THEN THE Attacker_Component SHALL use the default Skill_Value of 40 for that enemy type

### Requirement 2: Hit Roll Resolution

**User Story:** As a player, I want attacks to succeed or fail based on a d100 roll-under mechanic (as in the Rogue Trader RPG), so that combat has tension and unpredictability.

#### Acceptance Criteria

1. WHEN an attack is initiated by any Actor with an Attacker_Component, THE Hit_Chance_System SHALL generate a Hit_Roll as a uniformly distributed random integer from 1 to 100 inclusive, representing a d100 percentile die roll
2. WHEN the Hit_Roll is less than or equal to the Hit_Threshold (the effective Skill_Value after modifiers), THE Hit_Chance_System SHALL allow the attack to proceed to damage calculation as defined in the Attacker_Component
3. WHEN the Hit_Roll is greater than the Hit_Threshold, THE Hit_Chance_System SHALL prevent damage from being dealt to the target and SHALL log a miss message to the Message_Log indicating the attacker's name and that the attack missed
4. THE Hit_Chance_System SHALL compute the Hit_Threshold as the effective Skill_Value (base Skill_Value plus the sum of all applied modifiers), clamped to the range 1 to 99, so that no attack is ever a guaranteed hit or guaranteed miss
5. IF an attacking Actor's Skill_Value is not set or falls outside the range 1 to 99, THEN THE Hit_Chance_System SHALL treat the Skill_Value as 40 (producing a 40% base hit chance)
6. THE Hit_Chance_System SHALL produce a hit probability equal to (Hit_Threshold / 100) for any given effective Skill_Value, yielding 1% at Hit_Threshold 1 through 99% at Hit_Threshold 99

### Requirement 3: Miss Feedback

**User Story:** As a player, I want to see a message when an attack misses, so that I understand why no damage was dealt.

#### Acceptance Criteria

1. WHEN the player's attack misses, THE Message_Log SHALL display a message containing the attacker's name, the target's name, and the word "misses"
2. WHEN an enemy's attack misses the player, THE Message_Log SHALL display a message containing the enemy's name, the player's name, and the word "misses"
3. WHEN the player's attack misses, THE Message_Log SHALL display the miss message using a color that is not Colors::damage
4. WHEN an enemy's attack misses the player, THE Message_Log SHALL display the miss message using a color that is not Colors::damage

### Requirement 4: Symmetric Application

**User Story:** As a player, I want both the player and enemies to use the same d100 roll-under hit resolution, so that combat feels fair and consistent.

#### Acceptance Criteria

1. WHEN any Actor with an Attacker_Component initiates an attack, THE Hit_Chance_System SHALL resolve the hit roll using a single code path that accepts the attacker's Skill_Value as input, regardless of whether the attacker is the player or an enemy
2. THE Hit_Chance_System SHALL use the attacking Actor's own Skill_Value (an integer in the range 1 to 99 inclusive) and any applicable modifiers as the basis for computing the Hit_Threshold for each d100 roll
3. IF a d100 hit roll results in a miss (Hit_Roll greater than Hit_Threshold), THEN THE Hit_Chance_System SHALL skip damage application for that attack and THE Message_Log SHALL display a message indicating the attacker missed the target

### Requirement 5: Modifier Support

**User Story:** As a game designer, I want the hit chance system to support additive and subtractive modifiers to the Skill_Value (e.g., +10 for flanking, -20 for cover), so that equipment and situational effects can alter accuracy.

#### Acceptance Criteria

1. WHEN an attack is initiated, THE Hit_Chance_System SHALL compute an effective Skill_Value (the Hit_Threshold) as (base Skill_Value + sum of all modifiers currently applied to the attacker), where each modifier is a signed integer that may be positive or negative, before performing the d100 roll
2. THE Hit_Chance_System SHALL clamp the effective Skill_Value to the valid range of 1 to 99 after applying modifiers, so that no modifier combination can produce a guaranteed hit (100%) or a guaranteed miss (0%)
3. IF no modifiers are currently applied to the attacker, THEN THE Hit_Chance_System SHALL use the base Skill_Value unchanged as the Hit_Threshold for the d100 roll

### Requirement 6: Save and Load Persistence

**User Story:** As a player, I want my character's skill value to persist across save and load cycles, so that progress is not lost.

#### Acceptance Criteria

1. WHEN the game is saved, THE Attacker_Component SHALL serialize the Skill_Value as an integer to the save file using the existing TCODZip sequential archive format
2. WHEN the game is loaded, THE Attacker_Component SHALL deserialize the Skill_Value from the save file and restore it to the same integer value that was saved
3. WHEN a save file from a previous version without Skill_Value is loaded, THE Attacker_Component SHALL assign the default Skill_Value of 40 to maintain backward compatibility

### Requirement 7: Integration with Existing Attack Flow

**User Story:** As a developer, I want the d100 hit chance check to occur before damage calculation in the existing attack method, so that the change is minimally invasive to the current architecture.

#### Acceptance Criteria

1. WHEN `Attacker::attack()` is called and the target has a non-null `Destructible` component that is not dead, THE Hit_Chance_System SHALL perform the d100 hit roll before any damage calculation or defense reduction
2. WHEN the d100 hit roll succeeds (Hit_Roll less than or equal to Hit_Threshold), THE Attacker_Component SHALL proceed with the existing damage formula (effective_damage = power - target.defense) and log the result as it does currently
3. WHEN the d100 hit roll fails (Hit_Roll greater than Hit_Threshold), THE Attacker_Component SHALL skip all damage calculation, leave the target's HP and defense unchanged, and log a miss message instead
4. IF the target's `Destructible` component is null or the target is already dead, THEN THE Attacker_Component SHALL follow the existing behavior without performing a hit roll
