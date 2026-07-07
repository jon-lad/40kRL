# Requirements Document

## Introduction

This document specifies the requirements for a world map feature in 40kRL. The world map provides
a strategic navigation layer between the existing dungeon gameplay and future location types. The
player can open the world map as a modal overlay (via the 'm' key), view Perlin-noise-generated
terrain at a scale of 10km × 10km per tile, and fast-travel to marked locations such as hive cities.
Fast-travelling to a hive city transitions the game to a WFC-generated hive level (specified
separately). The world map integrates with the existing game state machine by adding a new
GameStatus (WORLD_MAP) and follows the established overlay rendering approach used by the inventory,
character sheet, and look mode.

## Glossary

- **Engine**: The global singleton that owns the game loop, the actor list, the map, the camera, and
  the current game state.
- **World_Map**: A modal overlay displaying the strategic-scale map of the planet surface, with
  terrain generated via Perlin noise and marked locations for fast travel. Each tile represents a
  10km × 10km area.
- **World_Map_Overlay**: The TCODConsole used to render the world map as a blit overlay on top of
  the main game view, following the same pattern as the inventory and character sheet overlays.
- **World_Tile**: A single cell on the world map representing a 10km × 10km region, classified by
  biome type derived from Perlin noise values.
- **Biome**: A terrain classification for a World_Tile, determined by Perlin noise thresholds.
  Biomes include wasteland, ash desert, toxic swamp, dead forest, and hive city.
- **Hive_City**: A marked location on the world map representing a massive urban structure. The
  player can fast-travel to a hive city, which transitions gameplay to a WFC-generated level
  (specified in a separate document).
- **Fast_Travel**: The action of selecting a marked location on the world map and immediately
  transitioning the player to that location's associated level.
- **Player_Position**: The World_Tile that the player currently occupies on the world map, indicated
  by a distinct marker glyph.
- **Perlin_Generator**: The subsystem that uses `TCODNoise` with a deterministic seed to produce
  noise values that determine biome classification for each World_Tile.
- **Cursor**: A navigable selection indicator on the world map overlay, controlled by arrow keys or
  vi-keys, used to inspect tiles and select fast-travel destinations.
- **GameStatus**: The Engine's state machine enum that controls which systems run each frame.
  WORLD_MAP is a new value in this enum.
- **InputHandler**: The SDL3-based input polling system that translates key events into game actions.

---

## Requirements

### Requirement 1: World Map Overlay Activation

**User Story:** As a player, I want to open the world map with a single keypress, so that I can
quickly view the strategic map without leaving the game screen.

#### Acceptance Criteria

1. WHEN the player presses the 'm' key WHILE the Engine GameStatus is IDLE, THE Engine SHALL
   transition GameStatus to WORLD_MAP and display the World_Map_Overlay.
2. WHEN the player presses Escape or 'm' WHILE the Engine GameStatus is WORLD_MAP, THE Engine
   SHALL transition GameStatus back to IDLE and dismiss the World_Map_Overlay.
3. WHILE the Engine GameStatus is WORLD_MAP, THE Engine SHALL not process player movement, monster
   AI, or turn-based game logic.
4. THE World_Map_Overlay SHALL render on top of the existing game view using the TCODConsole blit
   pattern, consistent with the inventory and character sheet overlays.
5. THE Engine SHALL store world map state in an optional `WorldMapState` struct, active only during
   WORLD_MAP status, consistent with the existing `InventoryState` and `LookState` patterns.

---

### Requirement 2: World Map Terrain Generation

**User Story:** As a player, I want the world map to display varied terrain that looks like a
natural planetary surface, so that exploration feels geographically meaningful.

#### Acceptance Criteria

1. WHEN the World_Map is first generated, THE Perlin_Generator SHALL create a `TCODNoise` instance
   with 2 dimensions and a deterministic seed derived from the Map's `seed` field.
2. THE Perlin_Generator SHALL sample noise at each tile coordinate (x, y), scaling coordinates by a
   configurable noise scale factor (default: 0.05), to produce a floating-point value in the range
   [-1.0, 1.0].
3. WHEN the noise value for a tile is above the ground Noise_Threshold (default: -0.1), THE
   Perlin_Generator SHALL classify that tile as Terrain_Type ground (walkable, transparent). WHEN
   the noise value is between the water Noise_Threshold (default: -0.5) and the ground
   Noise_Threshold, THE Perlin_Generator SHALL classify that tile as Terrain_Type tree (not
   walkable, not transparent). WHEN the noise value is below the water Noise_Threshold, THE
   Perlin_Generator SHALL classify that tile as Terrain_Type water (not walkable, transparent).
4. THE World_Map SHALL use the same dimensions as all other levels: 160 tiles wide × 86 tiles tall,
   as configured via `mapWidth` and `mapHeight` in `Scripts/Config.lua`.
5. FOR ALL World_Maps generated with the same seed, THE Perlin_Generator SHALL produce identical
   Terrain_Type layouts (deterministic generation).
6. THE Perlin_Generator SHALL use a configurable number of noise octaves (default: 4, minimum: 1)
   and lacunarity (default: 2.0) to produce terrain variation at multiple spatial frequencies.
7. THE Perlin_Generator SHALL read noise parameters (octave count, lacunarity, noise scale, ground
   threshold, water threshold) from `Scripts/Config.lua`, with the compiled defaults specified in
   criteria 2, 3, and 6 used if values are absent.
8. THE Perlin_Generator SHALL guarantee that at least one connected region of ground tiles exists
   large enough to place the player, stairs, and the configured minimum number of enemies
   (default: 6).
9. IF configuration threshold values are outside the valid range [-1.0, 1.0] or octave count is
   less than 1, THEN THE Perlin_Generator SHALL clamp values to valid bounds and log a warning via
   Gui.

---

### Requirement 3: Biome Classification and Rendering

**User Story:** As a player, I want to visually distinguish different terrain types on the world
map, so that I can identify regions of interest and plan travel routes.

#### Acceptance Criteria

1. THE World_Map SHALL classify each World_Tile into one of the following biomes based on Perlin
   noise thresholds (ordered from lowest to highest noise value): toxic swamp, dead forest, ash
   desert, wasteland.
2. THE World_Map SHALL render each biome with a distinct glyph and colour combination: toxic swamp
   ('~', green), dead forest (0x06 tree glyph, dark grey), ash desert ('.', tan/sandy), wasteland
   (',', brown).
3. THE World_Map SHALL render Hive_City locations with a unique glyph (0x7F or '#', distinct from
   all biome terrain glyphs) and a bright white colour to ensure marked locations stand out above
   biome terrain.
4. THE World_Map SHALL render the Player_Position with the '@' glyph in high-contrast white,
   drawn on top of the underlying biome tile. WHEN the player is on a Hive_City tile, the '@'
   glyph SHALL take rendering precedence over the Hive_City glyph.
5. THE World_Map_Overlay SHALL display a single-line border (using box-drawing characters) around
   the map area and a title header "World Map" centred on the top border.
6. THE World_Map_Overlay SHALL display contextual information (biome name and location name if
   present) for the tile currently under the Cursor in a status line rendered on the bottom border
   row of the overlay.

---

### Requirement 4: Cursor Navigation

**User Story:** As a player, I want to move a cursor across the world map to inspect tiles and
select destinations, so that I can make informed travel decisions.

#### Acceptance Criteria

1. WHEN the World_Map_Overlay is displayed, THE Cursor SHALL start at the Player_Position and THE
   World_Map_Overlay SHALL immediately display the biome name (and location name if present) for
   the Player_Position tile in the status line.
2. WHEN the player presses an arrow key or vi-key (h/j/k/l/y/u/b/n) WHILE GameStatus is
   WORLD_MAP, THE Cursor SHALL move one tile in the corresponding direction.
3. IF the Cursor is at a World_Map boundary and the player presses a key that would move it beyond
   that boundary, THEN THE Cursor SHALL remain at its current position and no status line update
   SHALL occur.
4. WHEN the Cursor moves to a new tile, THE World_Map_Overlay SHALL update the status line to show
   the biome name of the tile under the Cursor. IF a marked location exists on that tile, THEN THE
   status line SHALL additionally display the location name after the biome name.
5. WHILE GameStatus is WORLD_MAP, THE Cursor SHALL be rendered with a background colour different
   from the underlying tile's background colour, so that the Cursor tile is visually
   distinguishable from all adjacent tiles.

---

### Requirement 5: Hive City Placement

**User Story:** As a player, I want hive cities to be placed at meaningful locations on the world
map, so that fast-travel destinations feel intentionally positioned rather than random noise.

#### Acceptance Criteria

1. WHEN generating the World_Map, THE generator SHALL place a configurable number of Hive_City
   locations (default: 3, read from `Scripts/Config.lua`) on the map.
2. THE generator SHALL place Hive_City locations with a minimum Euclidean separation distance
   between each pair of cities (default: 15 tiles) to prevent clustering.
3. THE generator SHALL place Hive_City locations only on tiles classified as wasteland or ash desert
   biome (cities are built on solid ground; toxic swamp and dead forest are excluded).
4. THE generator SHALL assign each Hive_City a unique name drawn from a Lua-configurable name
   table in `Scripts/Config.lua`. IF the name table has fewer entries than the configured city
   count, THE generator SHALL append numeric suffixes to duplicate names (e.g., "Hive Primus 2").
5. IF the generator cannot place the configured number of Hive_City locations while satisfying
   the separation and biome constraints after 100 placement attempts per city, THEN THE generator
   SHALL place as many cities as possible and log a warning via Gui indicating how many could not
   be placed.

---

### Requirement 6: Fast Travel

**User Story:** As a player, I want to select a hive city on the world map and travel there
immediately, so that I can access different gameplay areas without traversing every tile.

#### Acceptance Criteria

1. WHEN the player presses Enter or Return WHILE the Cursor is on a Hive_City tile and GameStatus
   is WORLD_MAP, THE Engine SHALL initiate fast travel to that Hive_City.
2. WHEN fast travel is initiated, THE Engine SHALL dismiss the World_Map_Overlay, cache the current
   dungeon level via LevelCache, update the Player_Position to the destination Hive_City tile,
   generate the destination level, and set GameStatus to STARTUP so that FOV is recomputed on the
   new level.
3. WHEN fast-travelling to a Hive_City, THE Engine SHALL generate a new level using the WFC
   generation method associated with hive cities (specified in a separate document).
4. IF the WFC generator is not yet implemented, THEN THE Engine SHALL generate a BSP_Level as a
   placeholder when fast-travelling to a Hive_City.
5. WHEN fast travel is initiated, THE Engine SHALL display a message via Gui containing the
   destination Hive_City name, indicating the player is travelling to that city.
6. WHEN the player presses Enter or Return WHILE the Cursor is on a tile that is not a marked
   location, THE World_Map SHALL display a message indicating no destination is available at that
   tile.
7. WHEN the player presses Enter or Return WHILE the Cursor is on the Hive_City tile that matches
   the current Player_Position, THE Engine SHALL display a message indicating the player is already
   at that location and SHALL NOT initiate fast travel.

---

### Requirement 7: World Map State Persistence

**User Story:** As a player, I want the world map to persist across game sessions, so that
discovered locations and my position are preserved when I save and load.

#### Acceptance Criteria

1. WHEN saving the game, THE Engine SHALL serialize the following world map data to the save archive
   in order: the world seed (unsigned 32-bit integer), the Player_Position on the world map (x and
   y coordinates as integers), the Hive_City count (integer), and for each Hive_City its x
   coordinate, y coordinate, and name (as a length-prefixed string, maximum 64 characters).
2. WHEN loading a save file that contains world map data, THE Engine SHALL reconstruct the World_Map
   terrain from the stored seed (deterministic regeneration), restore the Player_Position, and
   restore the Hive_City list (positions and names) from the archive without re-running the
   placement algorithm.
3. IF a save file predates the world map feature (no world map data marker present), THEN THE
   Engine SHALL generate a new World_Map using a seed derived from the current `dungeonLevel`
   value, place the Player_Position at the first Hive_City in the generated list, and log an
   informational message via Gui indicating a new world map was created.
4. THE Engine SHALL write a presence marker (a boolean or magic byte) before world map data in the
   save archive, so that save files without world map data are detected and loaded without error.
5. IF the world map data in a save file is truncated or contains an invalid Hive_City count
   (greater than 20 or negative), THEN THE Engine SHALL discard the partial world map data,
   generate a fresh World_Map from a seed derived from the current game state, and log a warning
   via Gui.

---

### Requirement 8: World Map Generation Seeding

**User Story:** As a developer, I want world map generation to be deterministic and reproducible
from a seed, so that I can debug terrain issues and test specific map configurations.

#### Acceptance Criteria

1. WHEN creating a new game, THE Engine SHALL generate a world seed as an unsigned 32-bit integer
   using the system clock (milliseconds since epoch, truncated to 32 bits).
2. THE Perlin_Generator SHALL use the world seed to produce identical World_Map terrain on every
   generation call with that seed, across all platforms using the same libtcod version.
3. THE Hive_City placement algorithm SHALL use the world seed to produce identical city positions
   and name assignments on every generation call with that seed.
4. WHERE debug mode is active (F12 toggled), THE Engine SHALL display the current world seed as a
   decimal integer in the world map status line below the biome information.
5. WHERE debug mode is active (F12 toggled), WHEN the player types a numeric seed value (up to 10
   digits) followed by Enter while the world map overlay is displayed, THE Engine SHALL regenerate
   the World_Map using the entered seed and log a message confirming the seed override.

---

### Requirement 9: Integration with Game State Machine

**User Story:** As a developer, I want the world map to integrate cleanly with the existing Engine
state machine, so that adding a new overlay does not break existing modal states.

#### Acceptance Criteria

1. THE Engine SHALL add WORLD_MAP as a new value in the GameStatus enum, following the same pattern
   as INVENTORY, LOOK, and CHARACTER_SHEET.
2. THE Engine SHALL implement `beginWorldMap()`, `updateWorldMap()`, and `renderWorldMap()` methods
   following the established begin/update/render pattern for modal states.
3. WHILE GameStatus is any value other than IDLE (including INVENTORY, LOOK, CHARACTER_SHEET,
   TARGETING, PICKUP_MENU, STARTUP, NEW_TURN, VICTORY, and DEFEAT), THE Engine SHALL not allow
   transition to WORLD_MAP (the 'm' key is ignored in non-IDLE states).
4. THE Engine SHALL process world map input (cursor movement, fast travel, dismiss) exclusively
   within `updateWorldMap()`, preventing input bleed to other systems.
5. THE `renderWorldMap()` method SHALL use `TCODConsole::blit` to composite the World_Map_Overlay
   onto the root console, consistent with other overlay rendering methods.
6. WHEN `beginWorldMap()` is called, THE Engine SHALL store world map state in a
   `std::optional<WorldMapState>` member, and WHEN the overlay is dismissed, THE Engine SHALL reset
   the optional to `std::nullopt`, consistent with the existing `inventoryState` and `lookState`
   lifecycle.
