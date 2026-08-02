# Requirements Document

## Introduction

This feature implements a character generation flow for the 40kRL roguelike based on the Rogue Trader RPG NPC creation rules. The player selects a homeworld and career path, which determine starting characteristics, skills, traits, and what advances are available at each rank. A skills and talents system is introduced — non-combat skills are recorded on the character but their mechanical effects are deferred to a future spec. The player spends XP to purchase characteristic advances, skills, and talents from their career path's rank table, with purchases restricted to the current rank (no buying ahead, no elite advances). The existing XP bar in the HUD is reworked to display current XP versus spent XP rather than the old level-up progress bar.

## Glossary

- **Character_Generator**: The modal UI flow presented before gameplay begins that guides the player through homeworld selection, career path selection, and initial advance purchases.
- **Homeworld**: A selectable origin that modifies starting characteristic values, grants starting skills and traits, and is defined in Lua data.
- **Career_Path**: A selectable profession (e.g. Rogue Trader, Void-Master, Arch-Militant) that determines which advances are available at each rank.
- **Rank**: A numeric tier (starting at Rank 1) within a Career_Path that gates which advances the character may purchase. Higher ranks unlock after spending a threshold of XP.
- **Advance**: A purchasable improvement — either a characteristic increase, a skill acquisition/upgrade, or a talent acquisition — available at a specific rank for a specific XP cost.
- **XP_Pool**: The total experience points a character has earned. Divided into Spent_XP (used on advances) and Available_XP (remaining to spend).
- **Spent_XP**: The cumulative XP the character has invested in advances.
- **Available_XP**: The portion of XP_Pool not yet spent on advances (XP_Pool minus Spent_XP).
- **Skill**: A trained ability with a rank (Trained, +10, +20). Non-combat skills are recorded but produce no mechanical effect in this spec.
- **Talent**: A special ability or passive bonus purchased from the career path's rank table.
- **Characteristic_Advance**: An advance that increases one of the nine characteristics (WS, BS, S, T, Ag, Int, Per, WP, Fel) by a fixed amount.
- **XP_Bar**: The HUD element that displays the character's experience point status.
- **Homeworlds_Lua**: The Lua data file (Scripts/Homeworlds.lua) defining homeworld templates.
- **Careers_Lua**: The Lua data file (Scripts/Careers.lua) defining career paths, ranks, and advance tables.
- **Skills_Lua**: The Lua data file (Scripts/Skills.lua) defining all available skills and their properties.
- **Talents_Lua**: The Lua data file (Scripts/Talents.lua) defining all available talents and their properties.
- **Engine**: The global game state machine that owns actors, processes input, and drives the render loop.
- **Actor**: Any entity on the map (player, enemy, item, corpse, stairs) identified by optional components.
- **Characteristics_Component**: The existing C++ component attached to an Actor that stores all nine characteristic values.

## Requirements

### Requirement 1: Homeworld Selection

**User Story:** As a player, I want to select a homeworld during character generation, so that my character's starting characteristics, skills, and traits reflect their origin.

#### Acceptance Criteria

1. WHEN the Character_Generator begins, THE Character_Generator SHALL present a list of all homeworlds defined in Homeworlds_Lua.
2. WHEN the player selects a homeworld, THE Character_Generator SHALL apply the homeworld's characteristic modifiers to the character's base characteristics.
3. WHEN the player selects a homeworld, THE Character_Generator SHALL grant the homeworld's starting skills to the character.
4. WHEN the player selects a homeworld, THE Character_Generator SHALL grant the homeworld's starting traits to the character.
5. THE Character_Generator SHALL display a summary of the selected homeworld's modifiers and grants before advancing to the next step.

### Requirement 2: Career Path Selection

**User Story:** As a player, I want to select a career path during character generation, so that my character has a defined progression through ranks and advances.

#### Acceptance Criteria

1. WHEN the player has confirmed a homeworld, THE Character_Generator SHALL present a list of all career paths defined in Careers_Lua.
2. WHEN the player selects a career path, THE Character_Generator SHALL assign the character to Rank 1 of that career path.
3. WHEN the player selects a career path, THE Character_Generator SHALL grant any Rank 1 starting skills or talents defined for that career path.
4. THE Character_Generator SHALL display a summary of the selected career path and its Rank 1 benefits before advancing to the next step.

### Requirement 3: Characteristic Generation

**User Story:** As a player, I want my starting characteristics to be determined by base values modified by homeworld, so that different origins produce distinct stat profiles.

#### Acceptance Criteria

1. THE Character_Generator SHALL assign base characteristic values of 25 for all nine characteristics (WS, BS, S, T, Ag, Int, Per, WP, Fel) before homeworld modifiers are applied.
2. WHEN homeworld modifiers are applied, THE Characteristics_Component SHALL clamp all resulting values to the range 1–99 inclusive.
3. THE Character_Generator SHALL display all nine final starting characteristic values and their bonuses after homeworld selection.

### Requirement 4: Skills System

**User Story:** As a player, I want a skills system that records my trained skills and their ranks, so that my character's non-combat proficiencies are tracked for future use.

#### Acceptance Criteria

1. THE Engine SHALL maintain a collection of skills on the player Actor, where each entry records the skill name and its current rank (Trained, +10, or +20).
2. WHEN a skill is granted by homeworld or career path, THE Engine SHALL add that skill at Trained rank to the player's skill collection.
3. WHEN the player purchases a skill advance, THE Engine SHALL increase the skill's rank by one step (Trained to +10, or +10 to +20).
4. IF the player attempts to purchase a skill advance beyond +20, THEN THE Engine SHALL reject the purchase and display a message indicating the skill is at maximum rank.
5. THE Engine SHALL record non-combat skills without implementing their mechanical effects.

### Requirement 5: Talents System

**User Story:** As a player, I want a talents system that records my acquired talents, so that special abilities from my career path are tracked.

#### Acceptance Criteria

1. THE Engine SHALL maintain a collection of talents on the player Actor, where each entry records the talent name.
2. WHEN a talent is granted by career path starting benefits, THE Engine SHALL add that talent to the player's talent collection.
3. WHEN the player purchases a talent advance, THE Engine SHALL add that talent to the player's talent collection.
4. IF the player attempts to purchase a talent already in the collection, THEN THE Engine SHALL reject the purchase and display a message indicating the talent is already acquired.

### Requirement 6: XP Spending on Advances

**User Story:** As a player, I want to spend XP to purchase characteristic advances, skills, and talents from my career path's rank table, so that I can improve my character over time.

#### Acceptance Criteria

1. WHEN the player opens the advance purchase interface, THE Engine SHALL display all advances available at the character's current rank from Careers_Lua.
2. WHEN the player selects an advance, THE Engine SHALL display the advance's XP cost and confirm the purchase before deducting XP.
3. WHEN the player confirms a purchase, THE Engine SHALL deduct the advance's XP cost from Available_XP and add it to Spent_XP.
4. IF the player's Available_XP is less than the advance's XP cost, THEN THE Engine SHALL reject the purchase and display a message indicating insufficient XP.
5. WHEN a characteristic advance is purchased, THE Engine SHALL increase the corresponding characteristic value by the amount defined in the advance entry.
6. THE Engine SHALL restrict displayed advances to the character's current rank only (no buying ahead to higher ranks).

### Requirement 7: Rank Tracking

**User Story:** As a player, I want the game to track my current rank so that I can see what advances are available and know when I qualify for the next rank.

#### Acceptance Criteria

1. THE Engine SHALL store the character's current rank as an integer starting at 1.
2. WHEN Spent_XP reaches or exceeds the XP threshold for the next rank as defined in Careers_Lua, THE Engine SHALL advance the character to the next rank.
3. WHEN the character advances to a new rank, THE Engine SHALL make the new rank's advances available for purchase.
4. WHEN the character advances to a new rank, THE Engine SHALL display a message notifying the player of the rank increase.

### Requirement 8: XP Bar Rework

**User Story:** As a player, I want the XP bar to show my current XP and spent XP, so that I can see at a glance how much XP I have available to spend.

#### Acceptance Criteria

1. THE XP_Bar SHALL display Available_XP and XP_Pool in the format "XP: [Available_XP] / [XP_Pool]".
2. THE XP_Bar SHALL display the character's current rank number alongside the XP values.
3. THE XP_Bar SHALL replace the existing level-based XP progress bar in the HUD.
4. WHEN the character earns XP, THE XP_Bar SHALL update to reflect the new XP_Pool and Available_XP values.

### Requirement 9: Character Generation Flow

**User Story:** As a player, I want to go through a guided character generation flow when starting a new game, so that I can make meaningful choices about my character before gameplay begins.

#### Acceptance Criteria

1. WHEN the player starts a new game, THE Engine SHALL enter the Character_Generator state before placing the player on the map.
2. THE Character_Generator SHALL present steps in the following order: homeworld selection, career path selection, initial advance purchases.
3. WHEN the player completes all character generation steps, THE Engine SHALL create the player Actor with the configured characteristics, skills, talents, and career path, then transition to normal gameplay.
4. THE Character_Generator SHALL allow the player to navigate back to a previous step before finalizing.

### Requirement 10: Lua Data Definitions

**User Story:** As a designer, I want homeworlds, career paths, skills, and talents defined in Lua data files, so that content is data-driven and easy to modify without recompiling.

#### Acceptance Criteria

1. THE Engine SHALL load homeworld definitions from Homeworlds_Lua at startup, including name, characteristic modifiers, starting skills, and starting traits.
2. THE Engine SHALL load career path definitions from Careers_Lua at startup, including name, rank thresholds, and advance tables for each rank.
3. THE Engine SHALL load skill definitions from Skills_Lua at startup, including name and whether the skill is classified as combat or non-combat.
4. THE Engine SHALL load talent definitions from Talents_Lua at startup, including name and description.
5. IF a Lua data file contains an invalid entry (missing required fields), THEN THE Engine SHALL log a warning and skip that entry without crashing.

### Requirement 11: Character Generation Persistence

**User Story:** As a player, I want my homeworld, career path, rank, skills, and talents to persist through save and load, so that my character progression is not lost.

#### Acceptance Criteria

1. WHEN the Engine saves game state, THE Engine SHALL serialize the character's homeworld name, career path name, current rank, Spent_XP, skill collection, and talent collection to the save archive.
2. WHEN the Engine loads game state, THE Engine SHALL deserialize and restore the character's homeworld name, career path name, current rank, Spent_XP, skill collection, and talent collection from the save archive.
3. FOR ALL valid character states, saving then loading SHALL produce a character state with values equal to the original (round-trip property).

### Requirement 12: No Elite Advances or Buying Ahead

**User Story:** As a designer, I want to restrict advancement to the current rank's table without elite advances, so that progression follows the intended power curve.

#### Acceptance Criteria

1. THE Engine SHALL only display advances from the character's current rank in the advance purchase interface.
2. THE Engine SHALL not provide any mechanism to purchase advances from ranks higher than the character's current rank.
3. THE Engine SHALL not provide any mechanism to purchase elite advances.

### Requirement 13: Remove Legacy Leveling System

**User Story:** As a designer, I want the old level-based XP progression system removed entirely, so that character advancement uses only the Rogue Trader rank-based XP spending system defined in this spec.

#### Acceptance Criteria

1. THE Engine SHALL remove the xpLevel variable and getNextLevelXp function from the PlayerAi class.
2. THE Engine SHALL remove the LEVEL_UP_BASE and LEVEL_UP_FACTOR constants from the Ai source file.
3. THE Engine SHALL remove all automatic stat increase logic triggered by accumulating XP past a threshold.
4. THE Engine SHALL remove all level-up detection checks from the PlayerAi update loop.
5. THE Engine SHALL remove all level-up notification messages (including the "Your battle skills grow stronger" message and associated colour constants used exclusively for level-up).
6. THE Engine SHALL remove the XP progress bar that displays current XP as a fraction of the next-level threshold.
7. WHEN an enemy is killed, THE Engine SHALL add the awarded XP directly to the character's XP_Pool without triggering any level-up evaluation.
8. THE Engine SHALL update save and load serialization to exclude the xpLevel field.
9. THE Engine SHALL remove any remaining references to the legacy leveling system so that no dead code or unused variables persist in the codebase.
