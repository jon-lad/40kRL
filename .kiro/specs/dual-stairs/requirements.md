# Requirements Document

## Introduction

This feature replaces the single shared stairs actor with two dedicated stair actors per dungeon level: up-stairs (`<`) and down-stairs (`>`). Each intermediate level contains both stair types, while boundary levels (surface and deepest) contain only the relevant stair. The player spawns on the arrival stair matching their direction of travel, enabling coherent bidirectional vertical navigation.

## Glossary

- **Engine**: The global game state machine that owns the actor list, map, camera, and GUI. Declared as the singleton `engine` in main.cpp.
- **Map**: The class responsible for generating dungeon floor layouts via BSP or Perlin noise, and placing actors within rooms.
- **StairsUp**: The actor rendered as `<` representing a stairway ascending toward the surface.
- **StairsDown**: The actor rendered as `>` representing a stairway descending deeper underground.
- **Player**: The player-controlled actor represented by `@`.
- **BSP_Generator**: The Binary Space Partitioning room generation subsystem within Map.
- **Depth**: An integer representing the current dungeon level, where 0 is the surface and 20 is the deepest starting level.
- **Surface_Level**: The level at depth 0, representing the planet surface.
- **Starting_Level**: The deepest level (depth 20) where the player begins the game.
- **Save_System**: The serialization subsystem in Persistent.cpp responsible for writing and reading game state to/from `game.sav`.

## Requirements

### Requirement 1: Dual Stair Actor Ownership

**User Story:** As a developer, I want the Engine to own two distinct stair actors (up and down), so that the game can track both stairway positions independently.

#### Acceptance Criteria

1. THE Engine SHALL store a pointer named `stairsUp` referencing the up-stairs actor with glyph `<`.
2. THE Engine SHALL store a pointer named `stairsDown` referencing the down-stairs actor with glyph `>`.
3. THE Engine SHALL remove the existing single `stairs` pointer from its interface.

### Requirement 2: Stair Presence on Boundary Levels

**User Story:** As a player, I want boundary levels to only contain stairs that lead somewhere, so that I am not confused by non-functional stair tiles.

#### Acceptance Criteria

1. WHILE the current depth equals 0 (Surface_Level), THE Map SHALL place only StairsDown and SHALL NOT place StairsUp.
2. WHILE the current depth equals 20 (Starting_Level), THE Map SHALL place only StairsUp and SHALL NOT place StairsDown.
3. WHILE the current depth is between 1 and 19 inclusive, THE Map SHALL place both StairsUp and StairsDown.

### Requirement 3: Stair Placement in BSP Rooms

**User Story:** As a player, I want stairs placed in different rooms, so that navigating the level requires exploration.

#### Acceptance Criteria

1. WHEN a BSP level is generated, THE BSP_Generator SHALL place StairsUp in the first room (the room where the Player spawns).
2. WHEN a BSP level is generated, THE BSP_Generator SHALL place StairsDown in the last room.
3. THE BSP_Generator SHALL place StairsUp and StairsDown on different tiles.

### Requirement 4: Player Spawn on Level Transition

**User Story:** As a player, I want to appear on the correct stairs when I change level, so that my arrival position makes spatial sense.

#### Acceptance Criteria

1. WHEN the Player descends via StairsDown, THE Engine SHALL place the Player on the StairsUp position of the new level.
2. WHEN the Player ascends via StairsUp, THE Engine SHALL place the Player on the StairsDown position of the new level.

### Requirement 5: Descend Input Handling

**User Story:** As a player, I want to press `>` to descend when standing on down-stairs, so that the controls match the stair glyph.

#### Acceptance Criteria

1. WHEN the Player presses `>` and the Player position matches StairsDown position, THE Engine SHALL transition to the next deeper level.
2. WHEN the Player presses `>` and the Player position does not match StairsDown position, THE Engine SHALL display the message "There are no stairs here."
3. IF StairsDown is not present on the current level, THEN THE Engine SHALL display the message "There are no stairs here." when the Player presses `>`.

### Requirement 6: Ascend Input Handling

**User Story:** As a player, I want to press `<` to ascend when standing on up-stairs, so that the controls match the stair glyph.

#### Acceptance Criteria

1. WHEN the Player presses `<` and the Player position matches StairsUp position, THE Engine SHALL transition to the next shallower level.
2. WHEN the Player presses `<` and the Player position does not match StairsUp position, THE Engine SHALL display the message "There are no stairs here."
3. IF StairsUp is not present on the current level, THEN THE Engine SHALL display the message "There are no stairs here." when the Player presses `<`.

### Requirement 7: Stair Actor Rendering

**User Story:** As a player, I want stairs to remain visible after discovery, so that I can navigate back to them without re-exploring.

#### Acceptance Criteria

1. THE StairsUp actor SHALL have the `fovOnly` property set to false.
2. THE StairsDown actor SHALL have the `fovOnly` property set to false.
3. THE StairsUp actor SHALL have the `blocks` property set to false.
4. THE StairsDown actor SHALL have the `blocks` property set to false.

### Requirement 8: Save and Load Format

**User Story:** As a player, I want my game to save and restore both stair positions correctly, so that level transitions remain consistent across sessions.

#### Acceptance Criteria

1. WHEN the Save_System saves the game, THE Save_System SHALL serialize StairsUp before StairsDown in the save file.
2. WHEN the Save_System loads a save, THE Save_System SHALL restore StairsUp and StairsDown to their saved positions and glyphs.
3. IF a stair actor is absent on the current level (boundary level), THEN THE Save_System SHALL serialize a sentinel value indicating absence.

### Requirement 9: Depth Change Direction

**User Story:** As a player, I want descending to increase depth and ascending to decrease depth, so that navigation is consistent.

#### Acceptance Criteria

1. WHEN the Player descends via StairsDown, THE Engine SHALL increment the dungeon depth by 1.
2. WHEN the Player ascends via StairsUp, THE Engine SHALL decrement the dungeon depth by 1.

### Requirement 10: Outdoor Level Stair Placement

**User Story:** As a player, I want the surface level stairs placed in a reachable location, so that I can re-enter the dungeon.

#### Acceptance Criteria

1. WHEN an outdoor level is generated at depth 0, THE Map SHALL place StairsDown within the largest connected ground region.
2. WHEN an outdoor level is generated at depth 0, THE Map SHALL NOT place StairsUp.
