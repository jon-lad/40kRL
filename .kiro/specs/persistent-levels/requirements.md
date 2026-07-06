# Requirements Document

## Introduction

This feature replaces the current level-regeneration behaviour on stair transitions with a persistent level cache. When the player ascends or descends stairs, the level they leave (tile state, actors, items, decorations, explored map) is serialized and stored. When the player returns to a previously visited level, the cached state is deserialized and restored exactly as it was left — enemies stay dead, dropped items remain on the ground, and explored tiles stay revealed. The cache is also written to the save file so that quitting and reloading preserves all visited levels across play sessions.

## Glossary

- **Engine**: The global game state machine that owns the actor list, map, camera, GUI, level cache, and processes frames.
- **Level_Cache**: An in-memory data structure that maps dungeon depth numbers to serialized level snapshots, enabling preservation and restoration of previously visited levels.
- **Level_Snapshot**: A binary blob containing the complete serialized state of a single dungeon level: the Map (tiles, explored flags, terrain, seed, scent), and all non-player Actor instances that existed on that level at the time the player departed.
- **Map**: The 160×86 tile grid owning walkability, FOV, terrain type, explored flags, and scent values for the current dungeon floor.
- **Actor**: Any entity on the map identified by glyph, name, colour, position, and optional components (Ai, Attacker, Destructible, Pickable, Container, Equippable).
- **Player**: The single Actor that travels between levels and is excluded from per-level serialization.
- **Stair_Actor**: A non-blocking Actor (glyph '<' or '>') that connects adjacent dungeon levels and determines player placement on arrival.
- **Dungeon_Level**: An integer identifying a floor depth (0 = outdoor surface, 1+ = underground).
- **TCODZip**: The libtcod binary archive class used for serialization and deserialization of game state.
- **Save_File**: The game.sav binary file that persists game state between play sessions.

## Requirements

### Requirement 1: Serialize Level State on Departure

**User Story:** As a player, I want the level I leave via stairs to be saved in memory, so that I can return later and find everything as I left it.

#### Acceptance Criteria

1. WHEN the player uses a Stair_Actor to change Dungeon_Level, THE Engine SHALL serialize the current Map and all non-player Actor instances on that level into a Level_Snapshot before destroying the map and actors.
2. THE Level_Snapshot SHALL contain the complete Map state: tile walkability flags, tile transparency flags, tile explored flags, tile scent values, terrain types (for outdoor levels), the map seed, the level type (BSP or OUTDOOR), and the current scent counter value.
3. THE Level_Snapshot SHALL contain every Actor on the level except the Player, including dead actors (corpses), items on the ground, decorations, and Stair_Actors, with all component state preserved (Ai, Attacker, Destructible, Pickable, Container, Equippable, Equipment).
4. THE Engine SHALL store the Level_Snapshot in the Level_Cache keyed by the Dungeon_Level number that the player is departing.
5. WHEN the Level_Cache already contains a snapshot for the departing Dungeon_Level, THE Engine SHALL overwrite the existing snapshot with the new one.

### Requirement 2: Restore Level State on Return

**User Story:** As a player, I want to return to a previously visited level and find it exactly as I left it, so that my actions have lasting consequences.

#### Acceptance Criteria

1. WHEN the player uses a Stair_Actor and the destination Dungeon_Level exists in the Level_Cache, THE Engine SHALL deserialize the Level_Snapshot for that level instead of generating a new map.
2. WHEN the level is restored from the Level_Cache, THE Engine SHALL restore the Map with the serialized map seed, level type (BSP or OUTDOOR), all tile explored flags, scent values, terrain types, walkability state, and the currentScentValue counter matching the values stored in the Level_Snapshot.
3. WHEN the level is restored from the Level_Cache, THE Engine SHALL restore all Actor instances from the Level_Snapshot, including dead actors (corpses), ground items, decorations, and Stair_Actors, at their serialized positions with dead actors ordered behind living actors in the render list.
4. WHEN the level is restored, THE Engine SHALL place the Player at the position of the arrival Stair_Actor (stairs-up when descending, stairs-down when ascending).
5. WHEN the level is restored, THE Engine SHALL recompute the FOV from the Player position without modifying any stored explored flags beyond marking newly visible tiles as explored.
6. IF the level is restored and the expected arrival Stair_Actor is not present in the deserialized Level_Snapshot, THEN THE Engine SHALL place the Player at the first Stair_Actor found in the snapshot, or at tile position (1, 1) if no Stair_Actor exists, and log a warning message to the GUI.

### Requirement 3: Fresh Generation for Unvisited Levels

**User Story:** As a player, I want new levels to be procedurally generated the first time I visit them, so that exploration remains unpredictable.

#### Acceptance Criteria

1. WHEN the player uses a Stair_Actor and the destination Dungeon_Level does not exist in the Level_Cache, THE Engine SHALL generate a fresh level using the existing map generation algorithm (BSP for underground depths 1–20, OUTDOOR for depth 0).
2. WHEN a fresh level is generated, THE Engine SHALL populate it with monsters, items, decorations, and Stair_Actors using the existing spawn logic.
3. WHEN a fresh level is generated and the player descended, THE Engine SHALL place the Player at the position of the stairs-up Stair_Actor; WHEN the player ascended, THE Engine SHALL place the Player at the position of the stairs-down Stair_Actor.
4. WHEN a fresh level is generated, THE Engine SHALL create a stairs-up Stair_Actor if the Dungeon_Level is greater than 0, and a stairs-down Stair_Actor if the Dungeon_Level is less than 20, and assign the stairsUp and stairsDown pointer aliases accordingly.

### Requirement 4: Persist Level Cache to Save File

**User Story:** As a player, I want all visited levels to be preserved when I save and quit, so that loading my save restores the full dungeon state.

#### Acceptance Criteria

1. WHEN Engine::save() executes, THE Engine SHALL serialize the current active level's state (Map, stairs, and non-player actors) into the Save_File using the existing format, followed by the Level_Cache section: first the entry count as an integer, then for each cached level the Dungeon_Level number as an integer followed by the Level_Snapshot binary data.
2. WHEN Engine::load() restores a save, THE Engine SHALL restore the active level from the Save_File using the existing format, then read the Level_Cache entry count and, for each entry, read the Dungeon_Level number and Level_Snapshot data to populate the Level_Cache so that previously visited levels are available for restoration.
3. IF the Level_Cache is empty when Engine::save() executes, THEN THE Engine SHALL write an entry count of zero for the Level_Cache section and no snapshot data.
4. IF the player is dead when Engine::save() executes, THEN THE Engine SHALL delete the Save_File and discard all Level_Cache entries from memory.

### Requirement 5: Memory Management for Cached Levels

**User Story:** As a developer, I want the level cache to be bounded so that deep dungeon runs do not consume unbounded memory.

#### Acceptance Criteria

1. THE Level_Cache SHALL store Level_Snapshots as serialized binary blobs (produced via TCODZip) rather than retaining live Map and Actor objects in memory.
2. THE Level_Cache SHALL support storing snapshots for a configurable maximum number of levels, sourced from Scripts/Config.lua (field: maxCachedLevels, default: 30, minimum: 2, maximum: 200).
3. IF the maxCachedLevels value in Scripts/Config.lua is less than 2 or greater than 200 or is not an integer, THEN THE Engine SHALL clamp the value to the nearest valid bound and log a warning via the GUI message log.
4. WHEN the player arrives on a level that exists in the Level_Cache, THE Engine SHALL mark that Level_Snapshot as the most-recently-visited entry for eviction-ordering purposes.
5. WHEN the Level_Cache contains the configured maximum number of snapshots and a new snapshot must be stored, THE Engine SHALL evict the snapshot whose most-recent visit timestamp is oldest (least-recently-visited) to make room for the new entry.
6. THE Level_Cache SHALL NOT count the currently active level (the level the player is on) toward the configured maximum capacity; only levels stored as snapshots while the player is elsewhere count.
7. WHEN a Level_Snapshot is evicted from the Level_Cache, THE Engine SHALL treat that level as unvisited: if the player returns, a fresh level is generated using the standard generation algorithm.

### Requirement 6: Actor Persistence Fidelity

**User Story:** As a player, I want killed enemies to stay dead, dropped items to remain on the ground, and decorations to persist when I revisit a level, so that the world feels consistent.

#### Acceptance Criteria

1. WHEN a level is restored from the Level_Cache, THE Engine SHALL restore dead Actor instances (corpses) at their death position with their death state: name set to corpseName, glyph set to '%', colour set to dark red, blocks set to false, Ai set to null, and Destructible with hp <= 0.
2. WHEN a level is restored from the Level_Cache, THE Engine SHALL restore items that the player dropped on that level at the position where they were dropped, with all component state (Pickable, Equippable, Container) intact and identical to the values at departure time.
3. WHEN a level is restored from the Level_Cache, THE Engine SHALL restore Decoration actors at their original positions with their original properties (glyph, colour, description, blocks, coverValue, fovOnly).
4. WHEN a level is restored from the Level_Cache, THE Engine SHALL restore living enemies with their complete serialized state: position, glyph, colour, name, Attacker power, Destructible hp and maxHp and defense, AI type and state (including ConfusedMonsterAi turnsRemaining and saved oldAi), Container contents, and Equipment with all equipped items.
5. THE Engine SHALL NOT include the Player Actor in any Level_Snapshot; the Player is managed separately and placed on the arrival stair during restoration.
6. WHEN a level is restored from the Level_Cache, THE Engine SHALL place dead actors (corpses) behind living actors in the rendering order so that living actors on the same tile are drawn on top.

### Requirement 7: Stair Connectivity Preservation

**User Story:** As a player, I want stairs to connect the same positions between levels regardless of how many times I traverse them, so that navigation is predictable.

#### Acceptance Criteria

1. WHEN a level is restored from the Level_Cache, THE Engine SHALL restore Stair_Actors at the exact tile coordinates stored in the Level_Snapshot so that ascending and descending always lands the player at the same tile.
2. WHEN a level is restored from the Level_Cache, THE Engine SHALL identify the stairs-up Actor (glyph '<') and stairs-down Actor (glyph '>') from the restored actors and assign the stairsUp and stairsDown pointer aliases to them before placing the Player on the arrival stair.
3. WHEN a restored level has no stairs-up (depth 0 outdoor surface), THE Engine SHALL set the stairsUp pointer to nullptr.
4. WHEN a restored level has no stairs-down (depth equals the configured maximum dungeon depth of 20), THE Engine SHALL set the stairsDown pointer to nullptr.
5. IF a Level_Snapshot contains no Actor with glyph '<' or '>' where one is expected (depth > 0 expects stairs-up, depth < 20 expects stairs-down), THEN THE Engine SHALL treat the level as corrupted and regenerate it as a fresh level.

### Requirement 8: Save Format Compatibility

**User Story:** As a developer, I want the new save format to be distinguishable from the old format, so that old saves without level cache data do not crash the game.

#### Acceptance Criteria

1. WHEN Engine::save() writes the Level_Cache section, THE Engine SHALL write a 4-byte integer sentinel value immediately after the dungeonLevel field and before the Level_Cache entry count, so that Engine::load() can detect whether Level_Cache data is present.
2. WHEN Engine::load() reads the save file and the next value after the dungeonLevel field does not match the Level_Cache sentinel, THE Engine SHALL load the save using the existing format, initialize an empty Level_Cache, and set the current dungeonLevel as the only visited level with no cached snapshots.
3. IF the Level_Cache sentinel is present but a Level_Snapshot entry fails to deserialize (read returns fewer bytes than expected or an entry count is negative), THEN THE Engine SHALL log a warning message via the GUI indicating which Dungeon_Level snapshot was discarded, skip the corrupted entry, and continue loading any remaining Level_Cache entries.
4. IF one or more Level_Snapshots are discarded during load due to deserialization failure, THEN THE Engine SHALL leave the corresponding Dungeon_Level entries absent from the Level_Cache so that those levels are treated as unvisited and freshly generated if the player returns.
