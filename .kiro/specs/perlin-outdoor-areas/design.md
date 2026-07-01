# Design Document — Perlin Outdoor Areas

## Overview

This design adds Perlin-noise-based outdoor map generation to 40kRL. When `dungeonLevel` reaches
20, `Engine::nextLevel` delegates to a new generation path inside `Map` that uses libtcod's
`TCODNoise` to produce open terrain with ground, trees, and water instead of BSP rooms and
corridors. The change is contained within `Map` (new `initOutdoor` method + rendering branch),
`Engine::nextLevel` (level-type routing), `Config.lua` (threshold tuning), and `Persistent.cpp`
(save/load level-type indicator). No new classes are introduced — the `Map` class gains an
`isOutdoor` flag and a second generation path.

Design priorities:

1. **Minimal surface area** — no new class hierarchy; extend `Map` with a flag and a private
   generation method.
2. **Determinism** — same seed always produces the same outdoor terrain (required for save/load
   regeneration).
3. **Compatibility** — FOV, scent, camera, rendering pipeline, and `canWalk`/`isWall` queries
   continue to work without modification because `TCODMap` properties are set correctly during
   generation.
4. **Configurability** — thresholds and noise parameters are read from Lua at generation time.

---

## Architecture

### Level-Type Routing

```
Engine::nextLevel()  (renamed conceptually — now handles both ascend and descend)
│
├─ Ascending (current depth > 0, player used '<' stairs):
│    depth--
│    if depth == 0:
│      gui->message("You emerge onto the planet surface.")
│      map->init(true, LevelType::OUTDOOR)
│    else:
│      gui->message("You ascend closer to the surface.")
│      map->init(true, LevelType::BSP)
│
├─ Descending (current depth == 0, player used '>' dungeon entrance):
│    depth = 1
│    gui->message("You descend into the depths below...")
│    map->init(true, LevelType::BSP)
│
└─ camera->update(player), gameStatus = STARTUP
```

The `dungeonLevel` field is renamed to `depth` conceptually (though the code variable name may
remain for now). Depth 0 = surface (outdoor). Depth 1–20 = underground (BSP). The player starts
at depth 20 and ascends. The stairs glyph flips: `<` underground (ascend), `>` on surface
(descend into dungeon entrance).

Future: from the surface, multiple dungeon entrances at different locations can each lead to
independent dungeon chains with their own depth counters. For now, there's one linear chain.

### Map Generation Dispatch

```
Map::init(bool withActors, LevelType type = LevelType::BSP)
│
├─ rng = make_unique<TCODRandom>(seed, TCOD_RNG_CMWC)
├─ tiles = vector<Tile>(width * height)
├─ map = make_unique<TCODMap>(width, height)
├─ levelType = type
│
├─ if type == LevelType::BSP:
│    [existing BSP path — unchanged]
│
└─ if type == LevelType::OUTDOOR:
     initOutdoor(withActors)
```

### Outdoor Generation Pipeline

```
Map::initOutdoor(bool withActors)
│
├─ 1. Load config thresholds from Lua
│     groundThreshold  (default -0.1)
│     waterThreshold   (default -0.5)
│     octaves          (default 4)
│     lacunarity       (default 2.0)
│
├─ 2. Create TCODNoise(2, lacunarity, 1.0/lacunarity, rng.get())
│
├─ 3. Sample noise at each (x, y):
│     float coords[2] = { x * NOISE_SCALE, y * NOISE_SCALE }
│     float value = noise->get(coords, TCOD_NOISE_PERLIN)
│     ├─ value > groundThreshold  → ground (walkable=true, transparent=true)
│     ├─ value > waterThreshold   → tree   (walkable=false, transparent=false)
│     └─ value <= waterThreshold  → water  (walkable=false, transparent=true)
│     Store terrain type in terrainTypes[x + y * width]
│
├─ 4. Connectivity guarantee:
│     Flood-fill from a random ground tile to find largest connected region.
│     If region size < MIN_PLAYABLE_AREA (e.g., 200 tiles):
│       Increment seed, regenerate (retry up to MAX_RETRIES=10)
│
├─ 5. Actor placement (if withActors):
│     ├─ Player: random ground tile in the largest connected region
│     ├─ Stairs: ground tile in same region, Euclidean distance ≥ 40 from player
│     │          (fallback: furthest ground tile)
│     ├─ Enemies: scatter across ground tiles using addMonster()
│     │          (count: rng->getInt(MIN_OUTDOOR_MONSTERS, MAX_OUTDOOR_MONSTERS))
│     └─ Items: scatter across ground tiles using addItem()
│              (count: rng->getInt(MIN_OUTDOOR_ITEMS, MAX_OUTDOOR_ITEMS))
│
└─ 6. Set TCODMap properties for each tile via map->setProperties(x, y, transparent, walkable)
```

### Rendering Branch

```
Map::render()
│
├─ if levelType == LevelType::OUTDOOR:
│    renderOutdoor()
│
└─ else:
     [existing dungeon render — unchanged]

Map::renderOutdoor()
│
└─ for each (x, y):
     auto [screenX, screenY] = camera->apply(x, y)
     TerrainType terrain = terrainTypes[x + y * width]
     │
     ├─ if isInFOV(x, y):
     │    switch (terrain):
     │      GROUND → setChar('.'), setForeground(LIGHT_OUTDOOR_GROUND)
     │      TREE   → setChar(TCOD_CHAR_SPADE), setForeground(LIGHT_TREE)
     │      WATER  → setChar('~'), setForeground(LIGHT_WATER)
     │
     └─ elif isExplored(x, y):
          switch (terrain):
            GROUND → setChar('.'), setForeground(DARK_OUTDOOR_GROUND)
            TREE   → setChar(TCOD_CHAR_SPADE), setForeground(DARK_TREE)
            WATER  → setChar('~'), setForeground(DARK_WATER)
```

---

## Components and Interfaces

### New Types and Fields on Map

```cpp
// In Map.h

enum class LevelType : int {
    BSP     = 0,
    OUTDOOR = 1
};

enum class TerrainType : uint8_t {
    GROUND = 0,
    TREE   = 1,
    WATER  = 2
};

class Map : public Persistent {
    // ... existing fields ...

    LevelType levelType = LevelType::BSP;
    std::vector<TerrainType> terrainTypes; // only populated for OUTDOOR levels

    // New private methods:
    void initOutdoor(bool withActors);
    void renderOutdoor() const;
    void placeOutdoorActors();
};
```

### Engine Changes

```cpp
// Engine.h — no new fields; only nextLevel() logic changes.
// Engine::nextLevel checks dungeonLevel >= 20 to route to outdoor generation.
// Engine::save/load write/read dungeonLevel (already saved in the existing format — 
//   actually dungeonLevel is NOT currently saved; it must be added to the save stream).
```

**Important discovery**: `dungeonLevel` is not currently serialised. The save/load must be extended
to persist `dungeonLevel` so that on reload the engine knows whether to regenerate as BSP or
outdoor.

### Config.lua Extension

```lua
config = {
    -- ... existing fields ...

    -- Outdoor generation
    outdoorGroundThreshold = -0.1,
    outdoorWaterThreshold  = -0.5,
    outdoorOctaves         = 4,
    outdoorLacunarity      = 2.0,
    outdoorNoiseScale      = 0.05,  -- controls terrain feature size
    outdoorMinMonsters     = 6,
    outdoorMaxMonsters     = 12,
    outdoorMinItems        = 2,
    outdoorMaxItems        = 5,
    outdoorTransitionLevel = 20,
}
```

---

## Data Models

### TerrainType Storage

For outdoor levels, `terrainTypes` stores the terrain classification for each tile in a parallel
vector to `tiles`. This is needed for rendering (to pick glyph/colour) but is NOT saved — it is
regenerated deterministically from the seed on load.

| Field            | Type                       | Notes                                     |
|------------------|----------------------------|-------------------------------------------|
| `terrainTypes`   | `std::vector<TerrainType>` | Size = width × height, only for OUTDOOR   |
| `levelType`      | `LevelType`                | BSP (0) or OUTDOOR (1), saved to archive  |

### Noise Configuration (loaded at generation time)

| Parameter          | Type    | Default | Range            |
|--------------------|---------|---------|------------------|
| groundThreshold    | float   | -0.1    | [-1.0, 1.0]      |
| waterThreshold     | float   | -0.5    | [-1.0, groundTh] |
| octaves            | int     | 4       | [1, 8]           |
| lacunarity         | float   | 2.0     | [1.0, 4.0]       |
| noiseScale         | float   | 0.05    | (0.0, 1.0)       |

### Outdoor Colour Palette

| Constant              | Value (R, G, B)  | Usage                              |
|-----------------------|------------------|------------------------------------|
| `LIGHT_OUTDOOR_GROUND`| (100, 160, 60)   | Ground in FOV                      |
| `DARK_OUTDOOR_GROUND` | (40, 80, 25)     | Ground explored, not in FOV        |
| `LIGHT_TREE`          | (0, 180, 0)      | Tree in FOV                        |
| `DARK_TREE`           | (0, 90, 0)       | Tree explored, not in FOV          |
| `LIGHT_WATER`         | (60, 100, 200)   | Water in FOV                       |
| `DARK_WATER`          | (25, 50, 100)    | Water explored, not in FOV         |

### Outdoor Glyphs

| Terrain | Glyph             | Notes                                  |
|---------|--------------------|----------------------------------------|
| Ground  | `'.'` (0x2E)       | Same character as dungeon floor        |
| Tree    | `TCOD_CHAR_SPADE` (6) | Spade suit symbol — resembles a tree |
| Water   | `'~'` (0x7E)       | Tilde — common roguelike water glyph   |

---

## Serialisation Changes

### Save Stream Layout (extended)

The save format is extended by inserting a **level-type indicator** and **dungeonLevel** into the
existing stream. Backward compatibility is achieved by exploiting the fact that old saves do not
contain the level-type indicator — the loader checks for a sentinel value.

New save order (changes marked with `★`):

```
 1. int       map.width
 2. int       map.height
 3. Map::save
      ★ int   SAVE_VERSION_SENTINEL (0x4F444F52 = "ODOR" as int)  ← new
      ★ int   levelType (0 = BSP, 1 = OUTDOOR)                    ← new
      int     seed
      foreach tile (width*height):
        int   tile.explored
        int   tile.scent
      int     currentScentValue
 4. Camera::save  [unchanged]
 5. Actor::save  [player, unchanged]
 6. Actor::save  [stairs, unchanged]
 7. int       actorCount
 8. foreach remaining actor: Actor::save
 9. Gui::save [unchanged]
★10. int       dungeonLevel                                         ← new (appended at end)
```

### Backward Compatibility (Load)

```cpp
void Map::load(TCODZip& zip)
{
    int firstInt = zip.getInt();
    if (firstInt == SAVE_VERSION_SENTINEL) {
        // New format: read level type, then seed
        levelType = static_cast<LevelType>(zip.getInt());
        seed = zip.getInt();
    } else {
        // Old format: firstInt IS the seed
        levelType = LevelType::BSP;
        seed = firstInt;
    }

    // Regenerate geometry from seed
    if (levelType == LevelType::OUTDOOR) {
        initOutdoor(false);  // regenerate terrain, no actors
    } else {
        // existing BSP init path
        initBsp(false);
    }

    // Restore tile state from archive (same as before)
    for (auto& tile : tiles) {
        tile.explored = static_cast<bool>(zip.getInt());
        tile.scent    = zip.getInt();
    }
    currentScentValue = zip.getInt();
}
```

For `dungeonLevel` (appended at end of save stream):
```cpp
// Engine::load — after gui->load(zip):
if (zip.getInt() != 0) {  // check if more data exists
    dungeonLevel = zip.getInt();
} else {
    dungeonLevel = 1; // old save, assume level 1
}
```

**Alternative (simpler)**: Since `TCODZip` does not have a reliable "has more data" check, append
`dungeonLevel` at the very end of `Engine::save` and wrap the read in the existing data count.
Actually, `TCODZip::getInt` returns 0 when no data remains, so reading past the end returns 0
which is a valid signal for "old save format" (dungeonLevel=0 means default to 1).

---

## Connectivity Guarantee Algorithm

To ensure the player, stairs, and enemies can always be placed on reachable ground:

```
1. After noise sampling, collect all ground tiles into a set.
2. Pick the first ground tile as flood-fill seed.
3. BFS/DFS from that tile, marking all reachable ground tiles.
4. Record the connected component. Repeat for remaining unvisited ground tiles.
5. Select the largest connected component.
6. If largest_component.size() < MIN_PLAYABLE_AREA:
     seed++; goto step 1 (re-sample noise with new seed)
     Repeat up to MAX_RETRIES times.
7. All actor placement is restricted to tiles in the largest component.
```

`MIN_PLAYABLE_AREA` = 200 tiles (enough for player + stairs + 12 monsters + 5 items with spacing).

---

## FOV, Scent, and Camera Compatibility

No changes are needed to `computeFOV()`, scent stamping, or `Camera`. The key invariant is that
`TCODMap::setProperties` is called correctly for each tile during outdoor generation:

| Terrain | walkable | transparent |
|---------|----------|-------------|
| Ground  | true     | true        |
| Tree    | false    | false       |
| Water   | false    | true        |

This ensures:
- `isWall(x,y)` returns `true` for trees and water (not walkable).
- `isInFOV(x,y)` correctly blocks sight through trees but not through water.
- `canWalk(x,y)` correctly prevents movement into trees and water.
- Scent propagates through transparent tiles (water) but FOV does not propagate through opaque
  tiles (trees).

---

## Testing Strategy

### PBT Applicability Assessment

The outdoor generation system has strong property-based testing opportunities:

- **Determinism** — pure function of seed → terrain, easily verified.
- **Threshold ordering** — noise classification logic is a stateless mapping.
- **Connectivity** — structural property verifiable on the output grid.

UI rendering (colour selection, glyph output) and Lua config loading are not amenable to PBT.

### Property-Based Tests

**Property 1: Outdoor generation determinism**
*For any* seed value, generating two outdoor maps with the same seed, width, height, and thresholds
SHALL produce identical `terrainTypes` vectors element-wise.

**Property 2: Terrain classification exhaustiveness**
*For any* noise value `v` in [-1.0, 1.0] and thresholds `groundTh > waterTh`:
- `v > groundTh` → GROUND
- `waterTh < v <= groundTh` → TREE
- `v <= waterTh` → WATER

No noise value shall be unclassified.

**Property 3: TCODMap properties match terrain type**
*For any* outdoor tile at `(x, y)`, the `TCODMap` walkability and transparency flags SHALL match
the terrain type table (ground=walk+trans, tree=!walk+!trans, water=!walk+trans).

**Property 4: Connectivity guarantee**
*For any* successfully generated outdoor map, there SHALL exist a connected component of ground
tiles containing the player position, stairs position, and all monster positions.

**Property 5: Stairs distance constraint**
*For any* generated outdoor map with actors, the Euclidean distance between the player and stairs
SHALL be ≥ 40, OR the stairs SHALL be on the ground tile furthest from the player if no tile at
distance ≥ 40 exists in the connected region.

### Unit Tests

- `initOutdoor` with known seed → verify specific tiles match expected terrain.
- `renderOutdoor` → verify correct glyphs/colours for each terrain type (mock console).
- Config loading with missing values → verify defaults are used.
- Config loading with out-of-range values → verify clamping and warning.
- Save/load round-trip for outdoor level → verify `levelType` persists and terrain regenerates.
- Backward-compatible load of old save → verify `levelType` defaults to BSP.

---

## File Changes Summary

| File                  | Change Type | Description                                      |
|-----------------------|-------------|--------------------------------------------------|
| `Headers/Map.h`       | Modify      | Add `LevelType`, `TerrainType`, `terrainTypes`, `initOutdoor`, `renderOutdoor` |
| `Source/Map.cpp`      | Modify      | Implement `initOutdoor`, `renderOutdoor`, modify `init` to accept `LevelType` |
| `Source/Engine.cpp`   | Modify      | Route `nextLevel` by `dungeonLevel`, add outdoor message |
| `Source/Persistent.cpp`| Modify     | Extend `Map::save/load` with level-type indicator, save `dungeonLevel` |
| `Headers/Engine.h`    | No change   | `dungeonLevel` already exists as a field         |
| `Headers/Gui.h` / Camera | Modify  | Add `scrollMargin` field, modify `update()` for edge-triggered scrolling |
| `Scripts/Config.lua`  | Modify      | Add outdoor threshold/noise parameters + `outdoorScrollMargin` |

---

## Camera Scrolling (Outdoor Only)

### Overview

On outdoor levels, the camera uses edge-triggered scrolling instead of always staying centred on
the player. The camera only moves when the player gets within a configurable margin (default 20
tiles) of the viewport edge — and it moves by exactly one tile per player step, matching the
player's movement. This gives a smooth scroll feel rather than a jarring snap-to-centre.

On BSP levels, the existing always-centred behaviour remains unchanged.

### Logic Change in Camera::update

```cpp
void Camera::update(Actor* player, bool isOutdoor) {
    if (!isOutdoor) {
        // BSP: always centre on player (existing behaviour)
        x = -(player->getX()) + width / 2;
        y = -(player->getY()) + height / 2;
    } else {
        // Outdoor: scroll by 1 tile when player enters margin zone
        int screenX = player->getX() + x;
        int screenY = player->getY() + y;

        if (screenX < scrollMargin)
            x += 1;  // push viewport left (player near left edge)
        else if (screenX >= width - scrollMargin)
            x -= 1;  // push viewport right (player near right edge)

        if (screenY < scrollMargin)
            y += 1;  // push viewport up (player near top edge)
        else if (screenY >= height - scrollMargin)
            y -= 1;  // push viewport down (player near bottom edge)
    }
    clamp();
}
```

This means the camera scrolls one tile at a time, keeping pace with the player. The viewport
shifts smoothly as the player walks toward an edge, rather than snapping to centre.

### Camera Fields Addition

```cpp
class Camera {
    // ... existing fields ...
    int scrollMargin = 20;  // NEW: tiles from edge before camera scrolls (outdoor only)
};
```

`scrollMargin` is read from `Config.lua` as `outdoorScrollMargin` (default 20). It is NOT
serialised — it's a display preference loaded at startup.

### Config.lua Addition

```lua
config = {
    -- ... existing fields ...
    outdoorScrollMargin = 20,  -- tiles from viewport edge that triggers camera scroll
}
```

### Integration Points

- `Camera::update` gains an `isOutdoor` parameter (or reads it from the map's level type).
- `Engine::update` and `Map::computeFOV` already call `camera->update(player)` — the call site
  passes the outdoor flag.
- No new input handling or key bindings required.

---

## Assumptions and Constraints

1. `TCODNoise` is available in the project's libtcod build (it is a core libtcod class, always
   present).
2. The `TCOD_CHAR_SPADE` constant (value 6) is available in the libtcod character set being used.
3. `NOISE_SCALE` of 0.05 produces terrain features roughly 20 tiles wide — appropriate for 160×86
   maps. This is tunable via `Config.lua`.
4. The existing `addMonster` and `addItem` functions work on any walkable tile — they do not assume
   BSP room bounds.
5. `TCODZip::getInt()` returns 0 when the archive is exhausted, providing a natural sentinel for
   backward-compatible save format extension.
