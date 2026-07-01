# Requirements Document

## Introduction

This document specifies the requirements for adding Perlin-noise-based outdoor map areas to 40kRL.
When the player reaches dungeon level 20, the game transitions from BSP-generated dungeon floors to
open outdoor terrain generated using libtcod's built-in Perlin noise (`TCODNoise`). The outdoor area
features varied terrain types — walkable ground, impassable trees/walls, and impassable water —
determined by noise thresholds. All existing systems (FOV, scent, camera, rendering, Lua-driven
enemy/item spawning) continue to function on outdoor levels.

## Glossary

- **Engine**: The global singleton that owns the game loop, the actor list, the map, the camera, and
  the current dungeon level counter.
- **Map**: The class that owns tile data, the libtcod FOV/walkability map, and the seeded RNG; it
  generates level layouts and handles spatial queries.
- **Outdoor_Level**: A map level generated using Perlin noise instead of BSP trees; characterised by
  open terrain, natural obstacles (trees, water), and outdoor enemy encounters.
- **BSP_Level**: A map level generated using Binary Space Partitioning, producing rooms connected by
  corridors; the default generation method for dungeon levels 1–19.
- **Perlin_Generator**: The subsystem that uses `TCODNoise` with a deterministic seed to produce a
  2D noise field, which is then mapped to terrain types via configurable thresholds.
- **Terrain_Type**: A classification of a map tile based on its Perlin noise value. Types include
  ground (walkable, transparent), tree (impassable, opaque), and water (impassable, transparent).
- **Noise_Threshold**: A floating-point boundary value that determines which Perlin noise values map
  to which Terrain_Type.
- **TCODNoise**: The libtcod class that provides coherent noise generation (Perlin, simplex, etc.).
- **Tile**: The per-cell state struct storing `explored` and `scent` values.
- **Camera**: The viewport offset that maps world coordinates to screen coordinates, centred on the
  player.
- **FOV**: Field-of-View, computed by libtcod's `TCODMap::computeFov`.
- **Scent**: The per-tile tracking value used by MonsterAi to follow the player when out of FOV.
- **Outdoor_Enemies**: A set of enemy definitions loaded from Lua, distinct from dungeon enemy
  tables, spawned on Outdoor_Level maps.

---

## Requirements

### Requirement 1: Level Transition Trigger

**User Story:** As a player, I want to emerge into an outdoor area after clearing dungeon level 19,
so that the game world feels larger and more varied.

#### Acceptance Criteria

1. WHEN `Engine::nextLevel` increments `dungeonLevel` to 20, THE Engine SHALL generate an
   Outdoor_Level instead of a BSP_Level.
2. WHEN `dungeonLevel` is 20 or greater, THE Engine SHALL generate an Outdoor_Level for each
   subsequent call to `Engine::nextLevel`.
3. WHEN transitioning to an Outdoor_Level, THE Engine SHALL display a narrative message indicating
   the player has reached the surface.
4. THE Engine SHALL use the same `Map` dimensions (160×86) for Outdoor_Level maps as for BSP_Level
   maps.

---

### Requirement 2: Perlin Noise Terrain Generation

**User Story:** As a player, I want outdoor levels to look natural and varied, so that exploration
feels different from the repetitive dungeon corridors.

#### Acceptance Criteria

1. WHEN generating an Outdoor_Level, THE Perlin_Generator SHALL create a `TCODNoise` instance with
   2 dimensions and a seed derived from the Map's deterministic `seed` field.
2. THE Perlin_Generator SHALL sample the noise field at each tile coordinate (x, y) to obtain a
   floating-point value in the range [-1.0, 1.0].
3. WHEN the noise value for a tile is above the ground Noise_Threshold, THE Perlin_Generator SHALL
   mark that tile as ground (walkable and transparent in the TCODMap).
4. WHEN the noise value for a tile is between the water Noise_Threshold and the ground
   Noise_Threshold, THE Perlin_Generator SHALL mark that tile as tree (not walkable and not
   transparent in the TCODMap).
5. WHEN the noise value for a tile is below the water Noise_Threshold, THE Perlin_Generator SHALL
   mark that tile as water (not walkable but transparent in the TCODMap).
6. FOR ALL Outdoor_Level maps generated with the same seed, THE Perlin_Generator SHALL produce
   identical terrain layouts (deterministic generation).
7. THE Perlin_Generator SHALL guarantee that at least one connected region of ground tiles exists
   large enough to place the player, stairs, and a minimum number of enemies.

---

### Requirement 3: Terrain Rendering

**User Story:** As a player, I want to visually distinguish outdoor terrain types from dungeon walls
and floors, so that I can plan movement through the outdoor area.

#### Acceptance Criteria

1. WHEN rendering a ground tile on an Outdoor_Level within FOV, THE Map SHALL display the ground
   glyph ('.') with an outdoor ground colour distinct from the dungeon floor colour.
2. WHEN rendering a tree tile on an Outdoor_Level within FOV, THE Map SHALL display a tree glyph
   (character code for a tree symbol) with a green foreground colour.
3. WHEN rendering a water tile on an Outdoor_Level within FOV, THE Map SHALL display a water glyph
   ('~') with a blue foreground colour.
4. WHEN rendering explored-but-not-in-FOV tiles on an Outdoor_Level, THE Map SHALL display the same
   glyphs as in-FOV tiles but with darker colour variants.
5. THE Map SHALL not use box-drawing wall characters for tree tiles on Outdoor_Level maps; tree
   tiles SHALL use the tree glyph regardless of adjacent tile types.

---

### Requirement 4: Player and Stairs Placement

**User Story:** As a player, I want to start on valid ground when entering an outdoor level and have
stairs available to continue progressing, so that I am not stuck.

#### Acceptance Criteria

1. WHEN generating an Outdoor_Level with actors, THE Map SHALL place the player on a walkable
   ground tile.
2. WHEN generating an Outdoor_Level with actors, THE Map SHALL place stairs on a walkable ground
   tile that is not the player's starting tile.
3. THE Map SHALL place stairs at a minimum distance of 40 tiles (Euclidean) from the player's
   starting position.
4. IF no valid ground tile meeting the distance constraint exists for stairs placement, THEN THE Map
   SHALL place stairs on the walkable ground tile furthest from the player.

---

### Requirement 5: World-Based Enemy Spawning

**User Story:** As a player, I want enemies on both dungeon and outdoor levels to be determined by
the world I am on, so that the enemy pool feels thematic to the planet rather than tied to whether
I am underground or on the surface.

#### Acceptance Criteria

1. WHEN generating any level (BSP_Level or Outdoor_Level) with actors, THE Map SHALL spawn enemies
   using a single world-specific Lua enemy table loaded from `Scripts/Enemies.lua`.
2. THE `Scripts/Enemies.lua` table SHALL define all enemies available on the current world; THE same
   table SHALL be used regardless of whether the level is a dungeon or outdoor map.
3. WHEN spawning enemies on an Outdoor_Level, THE Map SHALL distribute them across walkable ground
   tiles that are not occupied by the player or stairs.
4. THE Map SHALL spawn between a configurable minimum and maximum number of enemies per room
   (dungeon) or per outdoor region, regardless of level type.
5. WHEN spawning items on an Outdoor_Level, THE Map SHALL use `Scripts/Items.lua` with the same
   spawn pattern as dungeon levels.
6. THE architecture SHALL support future extension where switching worlds (galactic travel) swaps
   the active Lua enemy and item scripts; however this feature is out of scope for this spec.

---

### Requirement 6: FOV, Scent, and Camera Compatibility

**User Story:** As a player, I want FOV, monster tracking, and camera scrolling to work identically
on outdoor levels, so that gameplay remains consistent.

#### Acceptance Criteria

1. WHEN `Map::computeFOV` is called on an Outdoor_Level, THE Map SHALL compute FOV using the same
   libtcod `computeFov` call with the same `fovRadius` as on BSP_Level maps.
2. WHILE on an Outdoor_Level, THE Map SHALL update per-tile scent values on each FOV recomputation
   using the same attenuation formula as on BSP_Level maps.
3. WHILE on an Outdoor_Level, THE MonsterAi SHALL track the player via scent gradient when the
   player is outside the monster's FOV, identical to dungeon behaviour.
4. WHILE on an Outdoor_Level, THE Camera SHALL centre on the player and clamp to map bounds using
   the same logic as on BSP_Level maps.
5. THE Map SHALL set `TCODMap` properties (transparency and walkability) for each Outdoor_Level tile
   so that the existing `isWall`, `canWalk`, `isInFOV`, and `isExplored` queries return correct
   values without modification.

---

### Requirement 7: Serialisation of Outdoor Levels

**User Story:** As a player, I want to save and resume my game on an outdoor level without losing
progress or terrain state.

#### Acceptance Criteria

1. WHEN saving a game on an Outdoor_Level, THE Map SHALL write a level-type indicator to the save
   archive before writing tile data.
2. WHEN loading a save file, THE Map SHALL read the level-type indicator and reconstruct the
   Outdoor_Level terrain using the stored seed (deterministic regeneration) rather than storing
   every tile's terrain type.
3. WHEN loading a save file for an Outdoor_Level, THE Map SHALL restore per-tile `explored` and
   `scent` state from the archive identically to BSP_Level loading.
4. THE save format change SHALL be backward-compatible: save files from before this feature (which
   lack a level-type indicator) SHALL be interpreted as BSP_Level maps.

---

### Requirement 8: Noise Threshold Configuration

**User Story:** As a developer tuning outdoor level generation, I want terrain thresholds to be
configurable without recompiling, so that I can iterate on level aesthetics quickly.

#### Acceptance Criteria

1. THE Perlin_Generator SHALL read Noise_Threshold values from a Lua configuration source
   (`Scripts/Config.lua` or a dedicated outdoor config section).
2. WHEN threshold configuration values are not present in the Lua source, THE Perlin_Generator
   SHALL use compiled default values (ground threshold: -0.1, water threshold: -0.5).
3. THE Perlin_Generator SHALL accept an octave count and lacunarity parameter from configuration,
   with compiled defaults of 4 octaves and 2.0 lacunarity.
4. WHEN configuration values are outside valid ranges (thresholds not in [-1.0, 1.0], octaves < 1),
   THE Perlin_Generator SHALL clamp values to valid bounds and log a warning via Gui.
