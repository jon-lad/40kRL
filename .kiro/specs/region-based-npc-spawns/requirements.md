# Requirements Document

## Introduction

This feature introduces a region data layer over the map that determines which NPCs (enemies) spawn on a given level. This is the basic first iteration of a larger spawn-control system.

In this iteration a **region** is defined at whole-level granularity and is derived from the existing `BiomeType` classification. Each enemy definition in `Enemies.lua` declares its spawn chance **per region** rather than as a single flat value, and the C++ spawn path (`Map::addMonster` → `spawnEnemy`) passes the active level's region name into Lua so the correct per-region chance column is selected.

To keep this iteration small, region names are initially the same as the race names that spawn in them (for example, an `Ork` region contains Orks). The data structure and the C++/Lua boundary are deliberately shaped so that later iterations can decouple region names from race names, support weighted multi-race regions, add per-tile granularity, and add neighbour-region fallback without a breaking redesign.

### In Scope

- Assigning a single region to an entire level, derived from `BiomeType`.
- A configurable default region for the initial BSP level that is generated without a world-map context.
- BSP dungeon levels inheriting the biome of the world-map tile they were entered from.
- Changing the `Enemies.lua` `chance` field from a flat integer to a per-region table keyed by region name.
- Passing the active region name from C++ into the Lua `spawnEnemy` function.
- Persisting the assigned region for cached/persisted levels.
- Preserving the existing Ork enemy set so enemies still spawn after the change.

### Out of Scope (Future Expansion)

- Region names that are distinct from race names.
- Regions defining weighted chances across multiple races or specific named NPCs.
- Per-tile or per-area region granularity within a single level.
- Neighbour-region fallback when a region has no matching enemy definitions.

## Glossary

- **Region**: A named classification applied to an entire level that determines which enemies may spawn on that level. In this iteration a Region name is identical to the race name that spawns in it.
- **Region_Name**: The string identifier of a Region. In this iteration it equals a race name (for example `"Ork"`).
- **BiomeType**: The existing world-map biome enumeration defined in `Headers/WorldMap.hpp` with values `TOXIC_SWAMP`, `DEAD_FOREST`, `ASH_DESERT`, `WASTELAND`, and `HIVE_CITY`.
- **Default_Region**: A configurable Region_Name applied to a level when no world-map biome context is available (for example the initial BSP level generated at game start).
- **Spawn_System**: The C++ code path responsible for spawning enemies, specifically `Map::addMonster` and its call into the Lua `spawnEnemy` function.
- **Enemy_Script**: The Lua script `Scripts/Enemies.lua`, which defines enemy templates and the `spawnEnemy(roll, x, y, region)` function.
- **Enemy_Entry**: A single enemy definition table within `Enemy_Script`.
- **Region_Chance_Table**: The per-region table on an `Enemy_Entry` that maps a `Region_Name` to a cumulative spawn chance integer, replacing the previous flat `chance` integer.
- **Level**: A single generated map, identified by its dungeon depth, with an associated `LevelType` (BSP, WFC, or OUTDOOR).
- **Persistent_Level_Store**: The existing system that caches and persists generated levels so they can be revisited.
- **Cumulative_Chance_Selection**: The existing selection algorithm in `spawnEnemy` that iterates enemy entries in order and picks the first entry whose cumulative chance threshold exceeds the roll.

## Requirements

### Requirement 1: Region Definition from Biome

**User Story:** As a game designer, I want each level to carry a single region derived from the map biome, so that enemy spawning can be controlled by the character of the environment.

#### Acceptance Criteria

1. WHEN a Level is generated, THE Spawn_System SHALL assign exactly one Region_Name to that Level, and THE Spawn_System SHALL retain that Region_Name unchanged for the lifetime of the Level.
2. WHERE a Level is an OUTDOOR level, THE Spawn_System SHALL derive the Level Region_Name from the BiomeType of that Level.
3. WHERE a Level is a BSP dungeon level entered from a world-map tile, THE Spawn_System SHALL set the Level Region_Name to the Region_Name derived from the BiomeType of that world-map tile.
4. THE Spawn_System SHALL derive the Region_Name from a BiomeType using a total mapping in which each of the five BiomeType values (TOXIC_SWAMP, DEAD_FOREST, ASH_DESERT, WASTELAND, HIVE_CITY) maps to exactly one Region_Name, and each resulting Region_Name SHALL equal the name of the race associated with that BiomeType.
5. THE Spawn_System SHALL apply the Region_Name at whole-level granularity, such that every spawnable tile within a single Level resolves to the same Region_Name.
6. IF a Level provides neither a BiomeType nor a source world-map tile from which a Region_Name can be derived, THEN THE Spawn_System SHALL assign the Default_Region to that Level consistent with Requirement 2.

### Requirement 2: Default Region for Context-Free Levels

**User Story:** As a developer, I want a configurable default region for levels generated without world-map context, so that the very first BSP level still spawns enemies deterministically.

#### Acceptance Criteria

1. IF a Level is generated without a world-map biome context, THEN THE Spawn_System SHALL assign the Default_Region as that Level Region_Name.
2. THE Spawn_System SHALL read the Default_Region value from configuration.
3. IF the configured Default_Region value is absent, THEN THE Spawn_System SHALL use a compiled fallback Region_Name so that every Level has a defined Region_Name.

### Requirement 3: Whole-Level Region Granularity

**User Story:** As a game designer, I want a region to apply to an entire level in this iteration, so that the first version stays simple and predictable.

#### Acceptance Criteria

1. THE Spawn_System SHALL apply a single Region_Name uniformly to every spawn location on a Level.
2. WHEN the Spawn_System spawns any enemy on a Level, THE Spawn_System SHALL use that Level single Region_Name for the spawn selection.

### Requirement 4: Per-Region Enemy Chance Table

**User Story:** As a game designer, I want each enemy to declare its spawn chance per region, so that different regions can contain different enemies.

#### Acceptance Criteria

1. THE Enemy_Script SHALL define each Enemy_Entry with a Region_Chance_Table that maps a Region_Name to a cumulative chance integer in the range 0 to 100 inclusive, replacing the previous flat chance integer.
2. WHEN the Spawn_System requests a spawn for a given Region_Name, THE Enemy_Script SHALL select an Enemy_Entry using Cumulative_Chance_Selection with an integer roll in the range 0 to 100 inclusive scoped to the chance values under that Region_Name.
3. IF an Enemy_Entry has no entry for the requested Region_Name, THEN THE Enemy_Script SHALL treat that Enemy_Entry chance as 0 for that Region_Name and SHALL never select that Enemy_Entry for that Region_Name.
4. THE Enemy_Script SHALL preserve the ordering behaviour of Cumulative_Chance_Selection within the requested Region_Name column.
5. IF the requested Region_Name column contains no Enemy_Entry with a chance greater than 0, THEN THE Enemy_Script SHALL select no Enemy_Entry for that Region_Name, consistent with the empty-region fallback in Requirement 6.

### Requirement 5: Passing the Region into the Spawn Function

**User Story:** As a developer, I want the C++ spawn path to pass the active region name into Lua, so that the script can select the correct per-region chance column.

#### Acceptance Criteria

1. WHEN Map::addMonster spawns an enemy, THE Spawn_System SHALL pass the active Level Region_Name into the Enemy_Script spawnEnemy function.
2. THE Enemy_Script SHALL accept the Region_Name as a parameter of the spawnEnemy function.
3. WHEN the Enemy_Script selects an Enemy_Entry for the passed Region_Name, THE Enemy_Script SHALL call the injected addActor function with the selected Enemy_Entry.

### Requirement 6: Empty-Region Fallback

**User Story:** As a game designer, I want spawning to do nothing when a region has no matching enemies, so that empty regions behave predictably in this iteration.

#### Acceptance Criteria

1. IF the requested Region_Name has no Enemy_Entry with a non-zero chance, THEN THE Enemy_Script SHALL spawn no enemy for that spawn request.
2. IF the Enemy_Script spawns no enemy for a spawn request, THEN THE Spawn_System SHALL leave the spawn location unoccupied by any new enemy.

### Requirement 7: Backwards Compatibility for the Ork Enemy Set

**User Story:** As a player, I want enemies to keep spawning after the region change, so that the game remains playable.

#### Acceptance Criteria

1. THE Enemy_Script SHALL retain the existing Ork enemy set (Gretchin, Ork, Shoota Boy, Nob) with their existing template fields.
2. THE Enemy_Script SHALL declare a Region_Chance_Table for the existing Ork enemy set under an `Ork` Region_Name whose cumulative values reproduce the previous flat chance distribution (Gretchin 50, Ork 75, Shoota Boy 90, Nob 100).
3. WHEN the Spawn_System spawns an enemy in a Level whose Region_Name resolves to `Ork`, THE Spawn_System SHALL spawn one of the existing Ork enemy set according to Cumulative_Chance_Selection.
4. IF the Enemy_Script fails to load or execute, THEN THE Spawn_System SHALL fall back to the existing hard-coded Ork spawn behaviour.

### Requirement 8: Region Persistence Across Save and Load

**User Story:** As a player, I want a level's region to survive save and load, so that revisiting a cached level spawns consistent enemies.

#### Acceptance Criteria

1. WHEN a Level is persisted by the Persistent_Level_Store, THE Persistent_Level_Store SHALL store that Level Region_Name in the TCODZip binary archive used by the Persistent_Level_Store.
2. WHEN a persisted Level is restored by the Persistent_Level_Store, THE Persistent_Level_Store SHALL restore that Level Region_Name byte-for-byte identical to the value stored at persist time.
3. FOR ALL Levels, persisting a Level and then restoring that Level SHALL yield a string-equal Region_Name to the value the Level had before persistence (round-trip property).
4. IF a persisted Level record predates region support and contains no stored Region_Name, THEN THE Persistent_Level_Store SHALL assign the Default_Region to that restored Level.
5. IF a stored Region_Name is absent or malformed during restore, THEN THE Persistent_Level_Store SHALL assign the Default_Region and SHALL continue the load without aborting.

### Requirement 9: Extensibility of the Region Data Structure

**User Story:** As a developer, I want the initial data structure and C++/Lua boundary to avoid painting future iterations into a corner, so that later features can be added without a breaking redesign.

#### Acceptance Criteria

1. THE Enemy_Script SHALL represent per-region chance as a keyed table so that additional Region_Name keys can be added without changing the spawn function signature.
2. THE Spawn_System SHALL pass the region to the Enemy_Script as a Region_Name string so that a Region_Name can later differ from a race name without changing the C++/Lua call boundary.
3. THE Spawn_System SHALL treat Region_Name derivation from BiomeType as a single mapping point so that later per-tile granularity can supply a Region_Name per spawn location without changing the spawn selection interface.
