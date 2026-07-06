# Requirements Document

## Introduction

This feature adds two related capabilities to the 40kRL roguelike: a "look" command that lets the player inspect any visible tile to read descriptions of actors on it, and a set of data-driven room decorations (crates, barrels, consoles, pillars, rubble, etc.) that populate generated rooms to make them visually richer. Decorations are defined in a Lua script following the same pattern as Enemies.lua and Items.lua, are cosmetic for now, but are designed to support future cover and interaction mechanics.

## Glossary

- **Engine**: The global game state machine that owns the actor list, map, camera, GUI, and processes frames.
- **Actor**: Any entity that exists on the map, identified by its glyph, name, colour, and optional components.
- **Look_Mode**: A game state in which the player moves a cursor over the map to inspect tiles and read descriptions of visible actors.
- **Decoration**: A non-blocking Actor placed in rooms during map generation that has a glyph, name, colour, and description but no gameplay-affecting components (no Ai, Attacker, Destructible, or Pickable).
- **Decoration_Registry**: The collection of decoration templates loaded from Scripts/Decorations.lua at startup, used to spawn decoration Actors during map generation.
- **Description**: A short textual string associated with an Actor that is displayed when the player examines it via Look_Mode.
- **Cursor**: A highlighted tile position controlled by the player during Look_Mode, used to select which tile to inspect.
- **Room**: A rectangular area carved out by the BSP map generator in which actors are placed.
- **GUI**: The message log and HUD overlay that displays text feedback to the player.

## Requirements

### Requirement 1: Enter and Exit Look Mode

**User Story:** As a player, I want to press a key to enter look mode and move a cursor around the map, so that I can inspect what is on any visible tile.

#### Acceptance Criteria

1. WHEN the player presses the 'l' key during the IDLE game state, THE Engine SHALL transition to Look_Mode and place the Cursor at the player's current position.
2. WHILE in Look_Mode, THE Engine SHALL highlight the Cursor tile with a background colour that differs from both the default floor and wall background colours.
3. WHILE in Look_Mode, WHEN the player presses a movement key (arrow keys or vi-keys), THE Engine SHALL move the Cursor one tile in the corresponding direction, clamped to the map boundaries (0 to map width−1 horizontally, 0 to map height−1 vertically).
4. WHILE in Look_Mode, WHEN the player presses Escape or 'l', THE Engine SHALL exit Look_Mode, remove the Cursor highlight, and return to the IDLE game state.
5. WHILE in Look_Mode, THE Engine SHALL NOT advance the game turn or run AI updates.

### Requirement 2: Display Actor Descriptions

**User Story:** As a player, I want to see descriptions of actors on the tile I'm inspecting, so that I can understand what I'm looking at.

#### Acceptance Criteria

1. WHILE in Look_Mode, WHEN the Cursor is positioned on a tile within the player's field of view, THE GUI SHALL display the name and description of each Actor occupying that tile in the HUD panel.
2. WHILE in Look_Mode, WHEN the Cursor is positioned on a tile outside the player's field of view but previously explored, THE GUI SHALL display "You can't see that clearly from here."
3. WHILE in Look_Mode, WHEN the Cursor is positioned on a tile that has not been explored, THE GUI SHALL clear the description area and display no information.
4. WHILE in Look_Mode, WHEN multiple Actors occupy the Cursor tile, THE GUI SHALL list each Actor's name and description separated by line breaks, ordered by their position in the Engine actor list (front to back).
5. WHILE in Look_Mode, WHEN no Actor occupies the Cursor tile and the tile is in the field of view, THE GUI SHALL display a terrain description: "Floor" for walkable tiles, "Wall" for non-walkable non-transparent tiles, "Tree" for tree terrain, or "Water" for water terrain.
6. WHILE in Look_Mode, WHEN a dead Actor (corpse) occupies the Cursor tile within the player's field of view, THE GUI SHALL include the corpse's name in the displayed list.
7. WHILE in Look_Mode, WHEN the Cursor is positioned on the player's own tile, THE GUI SHALL exclude the player Actor from the displayed list.

### Requirement 3: Actor Description Field

**User Story:** As a developer, I want each Actor to carry an optional description string, so that the look command has text to display for any actor.

#### Acceptance Criteria

1. THE Actor SHALL store a description string field with a maximum length of 200 characters that defaults to an empty string.
2. WHEN an Actor's description field is an empty string and the player inspects the Actor, THE GUI SHALL display only the Actor's name.
3. WHEN an Actor's description field is non-empty, THE GUI SHALL display the Actor's name followed by a colon separator and the description text.
4. WHEN an Actor is saved, THE system SHALL persist the description field so that loading restores the same description value that was present at save time.

### Requirement 4: Decoration Data Script

**User Story:** As a developer, I want decorations to be defined in a Lua script (Scripts/Decorations.lua), so that new decoration types can be added without recompiling.

#### Acceptance Criteria

1. WHEN Engine::init() executes, THE Decoration_Registry SHALL load decoration templates from Scripts/Decorations.lua before map generation begins.
2. THE Decorations.lua script SHALL define each decoration template with the following fields: glyph (single-character string), name (string, maximum 30 characters), colour (colour name string matching a Colors::colorFromName lookup), description (string, maximum 120 characters), and blocks (boolean indicating whether the decoration blocks movement).
3. IF Scripts/Decorations.lua fails to load or contains a parse error, THEN THE Engine SHALL log a warning message via the GUI and continue map generation without spawning decorations.
4. IF a decoration entry is missing a required field or contains an invalid colour name, THEN THE Decoration_Registry SHALL skip that entry, log a warning via the GUI identifying the entry index, and continue loading remaining entries.
5. THE Decorations.lua script SHALL contain at least eight Warhammer 40K-themed decoration entries (examples: ammo crate, fuel barrel, cogitator console, rockcrete pillar, rubble pile, comm-relay, aquila shrine, barricade).

### Requirement 5: Decoration Spawning During Map Generation

**User Story:** As a player, I want rooms to contain decorations when I enter a new level, so that the dungeon feels more visually interesting.

#### Acceptance Criteria

1. WHEN the map generator creates a room with actors enabled, THE Map SHALL spawn between zero and the configured maximum number of Decoration actors into the room, selecting templates from the Decoration_Registry.
2. THE Map SHALL place each Decoration on a walkable tile that is not already occupied by another blocking Actor.
3. IF no walkable unoccupied tile is available within the room during decoration placement, THEN THE Map SHALL skip that decoration placement without error.
4. THE Map SHALL select decoration types randomly from the Decoration_Registry for each placement. IF the Decoration_Registry contains no templates, THEN THE Map SHALL skip decoration spawning for that room without error.
5. THE Map SHALL respect a configurable maximum number of decorations per room, sourced from Scripts/Config.lua (field: maxRoomDecorations, default: 3). IF the configured value is less than zero, THEN THE Map SHALL treat it as zero.
6. THE Map SHALL NOT spawn decorations in the first room (player spawn room) to keep the starting area clear.

### Requirement 6: Decoration Actor Properties

**User Story:** As a developer, I want decorations to be standard Actors with specific component configuration, so that the existing rendering and actor systems handle them without special cases.

#### Acceptance Criteria

1. THE Decoration Actor SHALL have its blocks field set to the value specified by the decoration template (true for blocking decorations like pillars, false for non-blocking ones like rubble).
2. THE Decoration Actor SHALL have no Ai, Attacker, Destructible, or Pickable components.
3. THE Decoration Actor SHALL have fovOnly set to true so it renders only when visible.
4. THE Decoration Actor SHALL store the description from its Lua template in the Actor description field.
5. WHEN a Decoration has blocks set to true, THE Map SHALL treat the Decoration tile as impassable for pathfinding and player movement.

### Requirement 7: Outdoor Level Decoration Handling

**User Story:** As a player, I want outdoor levels to also contain decorations appropriate to the terrain, so that the environment feels consistent.

#### Acceptance Criteria

1. WHEN an outdoor level is generated with actors enabled, THE Map SHALL spawn decorations on ground tiles within the largest connected region.
2. THE Map SHALL respect a configurable decoration count for outdoor levels, sourced from Scripts/Config.lua (field: outdoorDecorationCount, default: 8).
3. THE Map SHALL place outdoor decorations only on ground-type terrain tiles that are walkable and unoccupied.

### Requirement 8: Future-Proofing for Cover System

**User Story:** As a developer, I want the decoration data structure to support an optional cover_value field, so that a future cover system can use existing decorations without schema changes.

#### Acceptance Criteria

1. THE Decorations.lua script SHALL support an optional cover_value numeric field on each decoration template (integer 0-100, where 0 means no cover).
2. WHEN a decoration template omits the cover_value field, THE Decoration_Registry SHALL default cover_value to 0.
3. THE Decoration Actor SHALL store the cover_value from its template for future use, with no gameplay effect in this iteration.
