# Requirements Document

## Introduction

This document specifies the requirements for a Wave Function Collapse (WFC) level generator that
produces hive city interiors in 40kRL. When the player fast-travels to a hive city from the world
map, the WFC generator creates a procedural hive interior composed of logical tile types (hab-units,
corridors, shafts, markets, manufactorums, sump-pools, etc.) with adjacency rules encoding
Warhammer 40k environmental logic. The generator operates as a standalone module that accepts a Lua-
defined tileset and map dimensions, outputs a 2D grid of resolved tiles, and guarantees full
connectivity. The Map class translates the resolved grid into TCODMap walkability/transparency data
and spawns actors. WFC is added as a new LevelType alongside the existing BSP and OUTDOOR types,
reusing the same 160×86 map dimensions and 10m × 10m per-tile scale as existing levels.

## Glossary

- **WFC_Generator**: The standalone module that implements the Wave Function Collapse algorithm.
  It accepts a tileset definition and grid dimensions as input and produces a fully-resolved 2D
  grid of tile assignments as output.
- **WFC_Tileset**: A Lua-defined collection of logical tile types with their adjacency rules,
  glyph, colour, walkability, and transparency properties. Loaded from `Scripts/WfcTiles.lua`.
- **WFC_Tile**: A single logical tile type within a WFC_Tileset. Each tile has a unique string
  identifier, a display glyph, a foreground colour, walkability, transparency, and directional
  adjacency rules specifying which other tile types may appear in each cardinal direction.
- **Adjacency_Rules**: Per-tile, per-direction lists of allowed neighbour tile identifiers. The
  four cardinal directions are north, south, east, and west. A tile placement is valid only if
  both tiles mutually permit each other in the corresponding directions.
- **Cell**: A single position in the WFC output grid. Before collapse, a cell holds a set of
  possible tile types (its domain). After collapse, a cell holds exactly one resolved tile type.
- **Entropy**: The number of remaining possible tile types in a cell's domain. A cell with
  entropy 1 is already collapsed. A cell with entropy 0 is a contradiction.
- **Collapse**: The act of selecting a single tile type from a cell's domain (by weighted random
  choice) and fixing that cell to that tile type.
- **Propagation**: After a cell is collapsed, the process of removing incompatible tile types
  from neighbouring cells' domains based on adjacency rules, recursively until no further
  reductions are possible.
- **Contradiction**: A state where a cell's domain becomes empty (no valid tile can be placed).
  The generator must recover from contradictions via backtracking or restart.
- **Connectivity_Check**: A flood-fill verification pass after grid resolution that confirms all
  walkable cells form a single connected component reachable by the player.
- **Map**: The class that owns the libtcod TCODMap, tile state array, and generation dispatch.
  It translates WFC output into walkability/transparency data.
- **LevelType**: The enum that selects the generation algorithm. WFC is added as a new value
  alongside BSP and OUTDOOR.
- **Hive_Layer**: A conceptual vertical section of a hive city (underhive, mid-hive, upper-hive).
  Different layers use different WFC_Tilesets with the same WFC_Generator. Layer support is a
  future enhancement; the initial implementation uses a single default tileset.

---

## Requirements

### Requirement 1: LevelType Enum Extension

**User Story:** As a developer, I want a WFC value in the LevelType enum, so that the Map class
can dispatch to the WFC generation path when creating hive city levels.

#### Acceptance Criteria

1. THE LevelType enum SHALL include a WFC value (integer value 2) following the existing BSP (0)
   and OUTDOOR (1) values.
2. WHEN `Map::init` is called with `LevelType::WFC`, THE Map SHALL invoke `initWfc(bool withActors)`
   to generate the level using the WFC_Generator.
3. THE Map SHALL serialize and deserialize the WFC LevelType value correctly, maintaining backward
   compatibility with save files that contain only BSP and OUTDOOR levels.

---

### Requirement 2: WFC Tileset Definition in Lua

**User Story:** As a developer, I want WFC tile types and adjacency rules defined in Lua, so that
I can iterate on hive city layouts without recompiling C++ code.

#### Acceptance Criteria

1. THE WFC_Generator SHALL load tile definitions from `Scripts/WfcTiles.lua` at game
   initialization, following the same sol2 loading pattern used by `Scripts/Decorations.lua`.
2. WHEN loading tile definitions, THE WFC_Generator SHALL require each WFC_Tile entry to specify:
   a string `id` (unique identifier), an integer or string `glyph` (display character), a string
   `color` (foreground colour name matching the existing colour lookup), a boolean `walkable`, a
   boolean `transparent`, and a table `adjacency` containing four directional lists (`north`,
   `south`, `east`, `west`) of allowed neighbour tile IDs.
3. IF a WFC_Tile entry is missing a required field or references a nonexistent tile ID in its
   adjacency lists, THEN THE WFC_Generator SHALL skip that entry and log a warning via Gui
   identifying the invalid tile and the specific validation failure.
4. THE WFC_Generator SHALL validate that adjacency rules are symmetric: if tile A allows tile B
   to its east, tile B must allow tile A to its west. IF asymmetric rules are detected, THEN THE
   WFC_Generator SHALL log a warning identifying the asymmetric pair and auto-correct by adding
   the missing reciprocal entry.
5. THE WFC_Tileset SHALL support a minimum of 5 and a maximum of 50 tile type definitions. IF
   fewer than 5 valid tiles remain after validation, THEN THE WFC_Generator SHALL fall back to
   BSP generation and log an error via Gui.
6. WHEN loading tile definitions, THE WFC_Generator SHALL read an optional `weight` field
   (floating-point, default 1.0) on each WFC_Tile entry that biases tile selection probability
   during collapse (higher weight means more likely to be chosen).

---

### Requirement 3: WFC Algorithm Core

**User Story:** As a developer, I want the WFC algorithm to resolve a grid of tile assignments
respecting adjacency constraints, so that generated hive cities have coherent spatial logic.

#### Acceptance Criteria

1. WHEN `initWfc` is called, THE WFC_Generator SHALL initialize a grid of 160×86 cells, each
   cell's domain containing all valid tile types from the loaded WFC_Tileset.
2. THE WFC_Generator SHALL select the cell with the lowest entropy (fewest remaining possibilities)
   as the next cell to collapse. IF multiple cells share the lowest entropy, THE WFC_Generator
   SHALL break ties by selecting randomly among them using the Map's seeded RNG.
3. WHEN collapsing a cell, THE WFC_Generator SHALL choose a tile type from the cell's domain using
   weighted random selection based on each tile's `weight` value, normalized to sum to 1.0 across
   the remaining domain.
4. AFTER collapsing a cell, THE WFC_Generator SHALL propagate constraints to all neighbouring
   cells by removing tile types from their domains that are not permitted by the collapsed tile's
   adjacency rules in the corresponding direction. Propagation SHALL continue recursively to
   affected neighbours until no further domain reductions occur.
5. IF propagation causes any cell's domain to become empty (contradiction), THEN THE WFC_Generator
   SHALL attempt recovery by restarting generation with a new RNG state derived from the original
   seed plus an attempt counter.
6. THE WFC_Generator SHALL attempt a maximum of 10 restarts before falling back to BSP generation
   and logging a warning via Gui indicating WFC generation failed after exhausting retry attempts.
7. FOR ALL grids generated with the same seed and tileset, THE WFC_Generator SHALL produce
   identical tile assignments (deterministic generation).

---

### Requirement 4: Connectivity Guarantee

**User Story:** As a player, I want every generated hive city level to be fully connected, so
that I can reach all areas without being blocked by impassable terrain.

#### Acceptance Criteria

1. AFTER the WFC_Generator produces a fully-resolved grid, THE WFC_Generator SHALL perform a
   flood-fill connectivity check starting from the first walkable cell (scanning left-to-right,
   top-to-bottom).
2. IF the connectivity check finds that all walkable cells belong to a single connected component
   (using four-directional adjacency), THE WFC_Generator SHALL accept the grid as valid.
3. IF the connectivity check finds multiple disconnected walkable regions, THE WFC_Generator SHALL
   reject the grid and attempt a restart (counting toward the 10-restart maximum defined in
   Requirement 3).
4. THE connectivity check SHALL use four-directional (cardinal) adjacency for determining
   walkable connectivity, consistent with the existing pathfinding model used by TCODMap.
5. WHEN a grid passes the connectivity check, THE WFC_Generator SHALL guarantee that the total
   number of walkable cells is at least 20% of the total grid area (160×86 = 13,760 cells,
   minimum 2,752 walkable cells), ensuring the level has sufficient open space for gameplay.

---

### Requirement 5: Map Integration and Rendering

**User Story:** As a player, I want WFC-generated hive city levels to render correctly and
support FOV, pathfinding, and actor placement, so that gameplay works identically to BSP levels.

#### Acceptance Criteria

1. AFTER the WFC_Generator produces a valid grid, THE Map SHALL set each TCODMap cell's walkability
   and transparency based on the resolved WFC_Tile's `walkable` and `transparent` properties.
2. THE Map SHALL render each resolved WFC_Tile using its defined `glyph` and `color` values,
   following the same explored/visible rendering rules as BSP and OUTDOOR levels (dim colour for
   explored-but-not-visible tiles, full colour for visible tiles).
3. WHEN `initWfc` is called with `withActors` set to true, THE Map SHALL place the player on a
   random walkable cell, place stairs up and stairs down on distinct walkable cells at least 20
   cells apart (Euclidean distance), and spawn monsters and items using the existing
   `addMonster` and `addItem` methods.
4. THE Map SHALL store the resolved WFC tile type ID for each cell in a parallel array (indexed
   as `x + y * width`), enabling WFC-specific rendering and future save/load of WFC level state.
5. WHEN rendering a WFC level, THE Map SHALL invoke a `renderWfc()` method that draws each visible
   or explored cell using the tile's glyph and colour, consistent with the `renderOutdoor()`
   pattern for outdoor levels.
6. THE Map SHALL support FOV computation and pathfinding on WFC levels using the same TCODMap
   instance, with walkability and transparency set from the WFC tile properties.

---

### Requirement 6: WFC Configuration

**User Story:** As a developer, I want WFC generation parameters to be configurable in Lua, so
that I can tune map density, retry limits, and tile weights without recompiling.

#### Acceptance Criteria

1. THE WFC_Generator SHALL read the following parameters from `Scripts/Config.lua`: `wfcMaxRestarts`
   (integer, default 10), `wfcMinWalkablePercent` (float, default 0.20),
   `wfcMinStairDistance` (integer, default 20).
2. IF `wfcMaxRestarts` is less than 1, THEN THE WFC_Generator SHALL clamp it to 1 and log a
   warning via Gui.
3. IF `wfcMinWalkablePercent` is outside the range [0.05, 0.95], THEN THE WFC_Generator SHALL
   clamp it to the nearest bound and log a warning via Gui.
4. THE WFC_Generator SHALL read an optional `wfcSeed` override from `Scripts/Config.lua`. IF
   present, THE WFC_Generator SHALL use that seed instead of the Map's default seed (useful for
   testing specific layouts).
5. WHERE debug mode is active (F12 toggled), THE Gui SHALL display the WFC generation seed and
   attempt count in the message log after level generation completes.

---

### Requirement 7: Default Hive City Tileset

**User Story:** As a player, I want the hive city to feel like a dense, industrial 40k
environment, so that the generated levels evoke the claustrophobic atmosphere of an underhive.

#### Acceptance Criteria

1. THE default WFC_Tileset in `Scripts/WfcTiles.lua` SHALL define the following minimum set of
   tile types: `hab-unit`, `corridor`, `wide-corridor`, `shaft`, `market`, `manufactorum`,
   `sump-pool`, `bulkhead-wall`, `chapel`, and `ventilation-duct`.
2. THE default WFC_Tileset SHALL encode the following adjacency constraints: `manufactorum` tiles
   connect only to `wide-corridor` or `corridor` tiles (never directly to `hab-unit`);
   `sump-pool` tiles connect only to `corridor` or `ventilation-duct` tiles; `chapel` tiles
   connect only to `corridor` or `wide-corridor` tiles; `hab-unit` tiles connect to `corridor`
   or other `hab-unit` tiles.
3. THE default WFC_Tileset SHALL assign `bulkhead-wall` tiles as non-walkable and non-transparent,
   serving as the primary structural barrier between regions.
4. THE default WFC_Tileset SHALL assign distinct glyph and colour combinations to each tile type
   so that the player can visually identify different hive areas (e.g., `corridor` uses '.' in
   grey, `hab-unit` uses '.' in brown, `sump-pool` uses '~' in green, `manufactorum` uses '=' in
   orange).
5. THE default WFC_Tileset SHALL assign higher weight values to `corridor` and `hab-unit` tiles
   (weight 3.0) and lower weight values to `chapel` and `sump-pool` tiles (weight 0.5) to
   produce levels dominated by connective tissue with occasional points of interest.
6. THE default WFC_Tileset SHALL ensure that `corridor` tiles permit adjacency with all walkable
   tile types in all directions, serving as the universal connector that guarantees the tileset
   can produce connected layouts.

---

### Requirement 8: Actor Spawning in WFC Levels

**User Story:** As a player, I want hive city levels to contain enemies and items appropriate to
the environment, so that gameplay in hive cities feels distinct from BSP dungeons.

#### Acceptance Criteria

1. WHEN `initWfc` is called with `withActors` set to true, THE Map SHALL spawn a configurable
   number of monsters (default range: `wfcMinMonsters` = 8, `wfcMaxMonsters` = 16, read from
   `Scripts/Config.lua`) on random walkable cells that are not occupied by the player or stairs.
2. WHEN `initWfc` is called with `withActors` set to true, THE Map SHALL spawn a configurable
   number of items (default range: `wfcMinItems` = 3, `wfcMaxItems` = 7, read from
   `Scripts/Config.lua`) on random walkable cells that are not occupied by actors or stairs.
3. THE Map SHALL place decorations on WFC levels using the existing `addDecorations` method,
   selecting decorations appropriate to the resolved tile type (e.g., cogitator consoles in
   manufactorum tiles, aquila shrines in chapel tiles).
4. WHEN placing actors on a WFC level, THE Map SHALL exclude cells within a 5-cell radius of the
   player's starting position from monster spawning, giving the player safe space on entry.
5. THE Map SHALL read actor count ranges (`wfcMinMonsters`, `wfcMaxMonsters`, `wfcMinItems`,
   `wfcMaxItems`) from `Scripts/Config.lua`, falling back to compiled defaults if values are
   absent.

---

### Requirement 9: WFC Level Persistence

**User Story:** As a developer, I want WFC levels to serialize and deserialize correctly, so that
the level cache and save/load system work with hive city levels.

#### Acceptance Criteria

1. WHEN serializing a WFC level, THE Map SHALL write the LevelType value (2 for WFC), the grid
   dimensions, the WFC seed used for generation, and the resolved tile type ID for each cell
   (as a string table index to minimize storage).
2. WHEN deserializing a WFC level, THE Map SHALL reconstruct the tile type array, reapply
   walkability and transparency to the TCODMap, and restore explored/scent state for each cell.
3. THE Map SHALL assign each tile type a stable integer index (based on load order from
   `Scripts/WfcTiles.lua`) and serialize tile assignments as integer indices rather than string
   IDs to minimize save file size.
4. IF the tileset definition has changed since the level was serialized (tile count mismatch or
   ID not found at expected index), THEN THE Map SHALL regenerate the WFC level from the stored
   seed rather than loading corrupted tile data, and log a warning via Gui.
5. THE LevelCache SHALL store and retrieve WFC levels using the same byte-buffer serialization
   interface as BSP and OUTDOOR levels, requiring no changes to the cache mechanism itself.

---

### Requirement 10: Fallback Behaviour

**User Story:** As a player, I want the game to remain playable even if WFC generation fails, so
that a bad tileset or unlucky seed does not soft-lock the game.

#### Acceptance Criteria

1. IF the WFC_Generator exhausts all restart attempts without producing a valid connected grid,
   THEN THE Map SHALL fall back to BSP generation for that level and log a warning via Gui
   indicating WFC generation failed.
2. IF `Scripts/WfcTiles.lua` fails to load (file missing, syntax error, or fewer than 5 valid
   tiles after validation), THEN THE Map SHALL fall back to BSP generation and log an error via
   Gui identifying the loading failure.
3. IF WFC fallback to BSP occurs, THE Map SHALL set `levelType` to `LevelType::BSP` so that
   rendering and serialization use the BSP code paths correctly.
4. WHEN falling back to BSP, THE Map SHALL use the same seed that was intended for WFC generation,
   ensuring the fallback level is still deterministic and reproducible.
5. THE fallback behaviour SHALL be transparent to the Engine — the `init` call returns a playable
   level regardless of which generator succeeded, requiring no error handling in calling code.
