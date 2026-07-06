# Requirements Document

## Introduction

This feature introduces the nine standard Rogue Trader RPG characteristics (WS, BS, S, T, Ag, Int, Per, WP, Fel) as a component on all actors — both the player and enemies. Characteristics are integer values (range 01–99) with a derived bonus (value ÷ 10, rounded down). Player characteristics are loaded from Classes.lua, enemy characteristics from Enemies.lua, and all values persist through the existing save/load system. A character sheet overlay displays the player's characteristics on demand. The existing Attacker/Destructible combat fields remain functional; a future combat-rework spec will wire the new characteristics into damage and hit calculations.

## Glossary

- **Characteristics**: The nine Rogue Trader RPG stats (WS, BS, S, T, Ag, Int, Per, WP, Fel) stored as integers in the range 1–99.
- **Characteristic_Bonus**: The integer quotient of a characteristic value divided by 10, rounded toward zero (e.g. a value of 43 yields bonus 4).
- **Characteristics_Component**: A C++ component attached to an Actor that stores all nine characteristic values.
- **Character_Sheet**: A full-screen overlay UI that displays the player's name, all nine characteristics, and their bonuses.
- **Engine**: The global game state machine that owns actors, processes input, and drives the render loop.
- **Actor**: Any entity on the map (player, enemy, item, corpse, stairs) identified by optional components.
- **Classes_Lua**: The Lua script (Scripts/Classes.lua) that defines player class templates including characteristics.
- **Enemies_Lua**: The Lua script (Scripts/Enemies.lua) that defines enemy templates including characteristics.
- **Save_Archive**: The TCODZip-based binary archive used for persisting game state across sessions.

## Requirements

### Requirement 1: Characteristics Component Definition

**User Story:** As a developer, I want a reusable component that stores all nine Rogue Trader characteristics, so that any actor can carry a full stat block.

#### Acceptance Criteria

1. THE Characteristics_Component SHALL store nine integer fields: WS, BS, S, T, Ag, Int, Per, WP, and Fel.
2. THE Characteristics_Component SHALL constrain each characteristic value to the range 1–99 inclusive.
3. WHEN a characteristic value outside the range 1–99 is assigned, THE Characteristics_Component SHALL clamp the value to the nearest bound (1 or 99).
4. THE Characteristics_Component SHALL compute the Characteristic_Bonus for a given characteristic as the integer quotient of the characteristic value divided by 10, rounded toward zero.

### Requirement 2: Player Characteristics from Lua

**User Story:** As a designer, I want to define player characteristics in Classes.lua, so that stat values are data-driven and easy to tune.

#### Acceptance Criteria

1. WHEN the Engine initializes a new game, THE Engine SHALL read the nine characteristic values from the selected class entry in Classes_Lua.
2. WHEN a class entry in Classes_Lua omits one or more characteristic fields, THE Engine SHALL assign a default value of 30 for each missing characteristic.
3. WHEN the Engine creates the player Actor, THE Engine SHALL attach a Characteristics_Component populated with the values read from Classes_Lua.

### Requirement 3: Enemy Characteristics from Lua

**User Story:** As a designer, I want to define enemy characteristics in Enemies.lua per template, so that each enemy type has distinct stat profiles.

#### Acceptance Criteria

1. WHEN the Engine spawns an enemy Actor, THE Engine SHALL read the nine characteristic values from the matching enemy template in Enemies_Lua.
2. WHEN an enemy template in Enemies_Lua omits one or more characteristic fields, THE Engine SHALL assign a default value of 20 for each missing characteristic.
3. WHEN the Engine creates an enemy Actor, THE Engine SHALL attach a Characteristics_Component populated with the values read from Enemies_Lua.

### Requirement 4: Characteristics Serialization

**User Story:** As a player, I want my character's stats to persist across save and load, so that progress is not lost between sessions.

#### Acceptance Criteria

1. WHEN the Engine saves game state to the Save_Archive, THE Characteristics_Component SHALL write all nine characteristic values to the archive in a fixed order (WS, BS, S, T, Ag, Int, Per, WP, Fel).
2. WHEN the Engine loads game state from the Save_Archive, THE Characteristics_Component SHALL read all nine characteristic values from the archive in the same fixed order.
3. FOR ALL valid Characteristics_Component instances, saving then loading SHALL produce a Characteristics_Component with values equal to the original (round-trip property).

### Requirement 5: Character Sheet Overlay Activation

**User Story:** As a player, I want to open a character sheet by pressing 'c', so that I can review my stats during gameplay.

#### Acceptance Criteria

1. WHILE the Engine is in IDLE state, WHEN the player presses the 'c' key, THE Engine SHALL transition to CHARACTER_SHEET state.
2. WHILE the Engine is in CHARACTER_SHEET state, WHEN the player presses the Escape key, THE Engine SHALL transition to IDLE state.
3. WHILE the Engine is in CHARACTER_SHEET state, WHEN the player presses the 'c' key, THE Engine SHALL transition to IDLE state.
4. WHILE the Engine is in CHARACTER_SHEET state, THE Engine SHALL not process player movement or other gameplay input.

### Requirement 6: Character Sheet Rendering

**User Story:** As a player, I want to see my name, all nine characteristics, and their bonuses displayed clearly, so that I can make informed gameplay decisions.

#### Acceptance Criteria

1. WHILE the Engine is in CHARACTER_SHEET state, THE Character_Sheet SHALL render as a centered frame overlay on top of the game view.
2. THE Character_Sheet SHALL display the player Actor's name at the top of the frame.
3. THE Character_Sheet SHALL display each of the nine characteristics as a labeled row containing the abbreviated name (WS, BS, S, T, Ag, Int, Per, WP, Fel), the numeric value, and the Characteristic_Bonus.
4. THE Character_Sheet SHALL display all nine characteristics in a fixed order: WS, BS, S, T, Ag, Int, Per, WP, Fel.

### Requirement 7: Coexistence with Existing Combat Fields

**User Story:** As a developer, I want the new characteristics to exist alongside the current Attacker and Destructible components, so that existing combat functionality remains operational until the combat-rework spec replaces it.

#### Acceptance Criteria

1. THE Actor SHALL retain the existing Attacker component (power, skillValue) and Destructible component (hp, maxHp, defense) without modification.
2. WHEN the Engine spawns a player or enemy Actor, THE Engine SHALL attach both the Characteristics_Component and the existing Attacker and Destructible components.
3. THE existing combat system SHALL continue to use Attacker.power, Attacker.skillValue, and Destructible.defense for hit and damage calculations without referencing the Characteristics_Component.
