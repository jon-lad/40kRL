# Requirements Document

## Introduction

This document specifies the requirements for adding doors to dungeon maps in 40kRL. Doors are
interactive map features that can be opened and closed, affecting movement, field-of-view, and
monster pathfinding. They are placed during BSP dungeon generation at room entrances, rendered with
distinct glyphs and colours depending on state, and persist through the save/load system.

Doors are represented as Actor entities with an Openable component, integrating with the existing
component-based architecture. The player opens doors via the 'o' key when adjacent; monsters open
doors automatically when pathing through them.

## Glossary

- **Door**: An Actor entity placed at room entrance tiles that can be in either a closed or open
  state, affecting passability and transparency of its tile.
- **Openable**: A new component attached to a Door Actor that tracks the open/closed state and
  provides methods to transition between states.
- **Closed_Door**: A Door in the closed state. Blocks movement and FOV. Rendered with the '+'
  glyph in a brown/wood colour.
- **Open_Door**: A Door in the open state. Allows movement and FOV. Rendered with the apostrophe
  glyph (') in a darker brown colour.
- **Adjacent**: A tile that is cardinally adjacent (up, down, left, right) to the player's current
  position, at Manhattan distance of exactly 1.
- **Room_Entrance**: A tile where a corridor meets a room boundary during BSP dungeon generation,
  forming a natural chokepoint suitable for door placement.
- **Engine**: The global singleton that owns the game loop, actor list, map, camera, and GUI.
- **Map**: The dungeon floor owning the libtcod walkability/FOV map and tile state array.
- **Actor**: The central game entity with optional components defining its capabilities.
- **TCODMap**: The libtcod spatial data structure tracking per-tile walkability and transparency.
- **BSP_Generator**: The Binary Space Partitioning dungeon generation algorithm that creates rooms
  and corridors.
- **FOV**: Field-of-view computed by libtcod from the player position, determining visible tiles.

---

## Requirements

### Requirement 1: Door Representation

**User Story:** As a developer, I want doors to be Actor entities with an Openable component, so
that they integrate cleanly with the existing component-based architecture and save/load system.

#### Acceptance Criteria

1. THE Door SHALL be an Actor entity with a non-null Openable component that tracks the current
   state as either open or closed.
2. THE Door Actor SHALL have `blocks` set to true when closed and `blocks` set to false when open.
3. THE Door Actor SHALL have `fovOnly` set to false so that explored doors remain visible outside
   the player FOV.
4. WHEN a Door is created, THE Openable component SHALL initialise the Door in the closed state.
5. THE Openable component SHALL implement the Persistent interface, serializing the open/closed
   state via `save(TCODZip&)` and `load(TCODZip&)`.

---

### Requirement 2: Door Rendering

**User Story:** As a player, I want doors to have distinct visual representations for open and
closed states, so that I can identify door state at a glance.

#### Acceptance Criteria

1. WHILE a Door is in the closed state, THE Door SHALL render with the '+' glyph.
2. WHILE a Door is in the open state, THE Door SHALL render with the apostrophe glyph (').
3. WHILE a Door is in the closed state, THE Door SHALL render in a brown/wood-tone colour
   (RGB 150, 100, 50).
4. WHILE a Door is in the open state, THE Door SHALL render in a darker brown colour
   (RGB 100, 65, 30).
5. WHEN a Door tile has been explored but is not currently in the player FOV, THE Door SHALL render
   with a dimmed version of its state-appropriate colour.

---

### Requirement 3: Door Spatial Properties

**User Story:** As a player, I want closed doors to block movement and line-of-sight, so that
doors provide tactical value as barriers and vision blockers.

#### Acceptance Criteria

1. WHILE a Door is in the closed state, THE Map SHALL treat the Door tile as not walkable and not
   transparent in the TCODMap properties.
2. WHILE a Door is in the open state, THE Map SHALL treat the Door tile as walkable and
   transparent in the TCODMap properties.
3. WHEN a Door transitions from closed to open, THE Openable component SHALL update the TCODMap
   properties at the Door position to walkable and transparent, and SHALL trigger a FOV recompute.
4. WHEN a Door transitions from open to closed, THE Openable component SHALL update the TCODMap
   properties at the Door position to not walkable and not transparent, and SHALL trigger a FOV
   recompute.

---

### Requirement 4: Player Door Interaction

**User Story:** As a player, I want to open doors by pressing 'o' when adjacent to them, so that
I can control when to reveal new areas and allow passage.

#### Acceptance Criteria

1. WHEN the player presses 'o' and exactly one closed Door is cardinally adjacent to the player,
   THE PlayerAi SHALL open that Door and advance the game turn.
2. WHEN the player presses 'o' and multiple closed Doors are cardinally adjacent to the player,
   THE PlayerAi SHALL prompt the player to select a direction, then open the Door in the chosen
   direction.
3. WHEN the player presses 'o' and no closed Door is cardinally adjacent to the player, THE Gui
   SHALL display a message indicating there is no door to open.
4. WHEN the player attempts to move into a closed Door tile, THE PlayerAi SHALL block the movement
   and display a message indicating the door is closed.
5. WHEN the player attempts to move into an open Door tile, THE PlayerAi SHALL allow the movement
   as normal.

---

### Requirement 5: Monster Door Interaction

**User Story:** As a player, I want monsters to be able to open doors, so that closed doors
provide temporary rather than permanent protection and monsters can path through the dungeon.

#### Acceptance Criteria

1. WHEN a monster with a non-null Ai component attempts to move into a tile occupied by a closed
   Door, THE MonsterAi SHALL open the Door and consume the monster's turn.
2. WHEN a monster opens a Door, THE Gui SHALL display a message indicating the monster opened the
   door (e.g., "The Ork opens the door").
3. WHEN a Door is open, THE MonsterAi SHALL treat the Door tile as walkable for pathfinding
   purposes.
4. WHILE a Door is closed, THE MonsterAi scent-following algorithm SHALL treat the Door tile as
   impassable.

---

### Requirement 6: Door Placement During Generation

**User Story:** As a player, I want doors placed at room entrances in BSP dungeons, so that
dungeon exploration has natural chokepoints and tactical positioning opportunities.

#### Acceptance Criteria

1. WHEN a BSP dungeon is generated, THE BSP_Generator SHALL identify Room_Entrance tiles where
   corridors connect to rooms.
2. WHEN a Room_Entrance tile is identified, THE BSP_Generator SHALL place a closed Door Actor at
   that position.
3. THE BSP_Generator SHALL place doors only on BSP-type levels; outdoor and WFC levels SHALL not
   receive door placement.
4. WHEN a Door is placed, THE BSP_Generator SHALL set the TCODMap properties at that tile to not
   walkable and not transparent.
5. THE BSP_Generator SHALL place at most one Door per Room_Entrance tile; duplicate doors at the
   same position SHALL not occur.

---

### Requirement 7: Door Save and Load

**User Story:** As a player, I want door state to persist across save and load, so that doors I
have opened remain open when I resume the game.

#### Acceptance Criteria

1. WHEN the game is saved, THE Door Actor SHALL serialize its position, glyph, colour, name,
   blocks flag, and Openable component state.
2. WHEN the game is loaded, THE Door Actor SHALL be reconstructed with the correct open/closed
   state, glyph, colour, and spatial properties matching the saved state.
3. WHEN a Door is loaded in the open state, THE Map SHALL set the TCODMap properties at that
   position to walkable and transparent.
4. WHEN a Door is loaded in the closed state, THE Map SHALL set the TCODMap properties at that
   position to not walkable and not transparent.
5. THE Door serialization format SHALL include a type discriminator so that the static Actor
   factory can reconstruct the Openable component during load.
