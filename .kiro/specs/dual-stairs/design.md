# Design Document: Dual Stairs

## Overview

This feature replaces the single `Actor* stairs` pointer in Engine with two dedicated stair actors: `stairsUp` (glyph `<`) and `stairsDown` (glyph `>`). The change propagates through Engine initialization, level transitions, BSP room generation, outdoor placement, input handling, and save/load serialization.

## Architecture

The architecture follows the existing pattern where Engine owns raw-pointer aliases into the `actors` list. The key difference is that either stair pointer may be `nullptr` on boundary levels (surface has no up-stairs; starting level has no down-stairs).

All stair actors are created fresh on each level transition — old stair actors are destroyed with the previous level's actor list. The `nextLevel()` method accepts a `StairDirection` parameter to determine depth change and player arrival position.

## Components and Interfaces

### Engine (Header + Source)

**Changes to `Engine.h`:**
- Remove `Actor* stairs;`
- Add `Actor* stairsUp;` (may be `nullptr` on surface level)
- Add `Actor* stairsDown;` (may be `nullptr` on starting level)

**Changes to `Engine::init()`:**
- Create the stairsUp actor with glyph `<`, `blocks = false`, `fovOnly = false`
- Create the stairsDown actor — on the starting level (depth 20) stairsDown is NOT created, so the pointer remains `nullptr`
- Both actors are emplaced at the front of the actors list (drawn beneath other actors)

**Changes to `Engine::nextLevel()`:**
- Determine direction: if player used stairsDown → descending (depth + 1); if player used stairsUp → ascending (depth - 1)
- Accept a `direction` parameter (enum or bool) set by the caller in PlayerAi
- Remove all actors except player (stair actors for the old level are destroyed)
- Generate the new map
- Create new stair actors appropriate for the destination depth:
  - Depth 0: only stairsDown (outdoors)
  - Depth 20: only stairsUp
  - Depths 1–19: both stairsUp and stairsDown
- Place the player on the arrival stair:
  - Descended → player placed on stairsUp of new level
  - Ascended → player placed on stairsDown of new level

```cpp
enum class StairDirection { UP, DOWN };

void Engine::nextLevel(StairDirection direction)
{
    if (direction == StairDirection::DOWN) {
        dungeonLevel++;
    } else {
        dungeonLevel--;
    }

    gui->message(Colors::healing, "You take a moment to rest and recover your strength.");
    player->destructible->heal(player->destructible->maxHp / 2.0f);

    const bool isOutdoor = (dungeonLevel == 0);

    if (isOutdoor) {
        gui->message(Colors::surfaceMsg, "You emerge from the depths onto the planet surface.");
    } else if (direction == StairDirection::UP) {
        gui->message(Colors::damage, "You ascend closer to the surface.");
    } else {
        gui->message(Colors::damage, "You descend deeper underground.");
    }

    // Destroy old map and remove all actors except player.
    map.reset();
    for (auto i = actors.begin(); i != actors.end(); ) {
        i = (i->get() != player) ? actors.erase(i) : std::next(i);
    }
    stairsUp = nullptr;
    stairsDown = nullptr;

    // Generate new map.
    map = std::make_unique<Map>(MAP_WIDTH, MAP_HEIGHT);
    map->init(true, isOutdoor ? LevelType::OUTDOOR : LevelType::BSP);

    // Create stair actors for the new depth.
    const bool needsUp   = (dungeonLevel > 0);
    const bool needsDown = (dungeonLevel < 20);

    if (needsUp) {
        auto newUp = std::make_unique<Actor>(0, 0, '<', "stairs up", Colors::white);
        stairsUp = newUp.get();
        newUp->blocks = false;
        newUp->fovOnly = false;
        actors.emplace_front(std::move(newUp));
    }
    if (needsDown) {
        auto newDown = std::make_unique<Actor>(0, 0, '>', "stairs down", Colors::white);
        stairsDown = newDown.get();
        newDown->blocks = false;
        newDown->fovOnly = false;
        actors.emplace_front(std::move(newDown));
    }

    // Place player on arrival stair.
    if (direction == StairDirection::DOWN && stairsUp) {
        player->setX(stairsUp->getX());
        player->setY(stairsUp->getY());
    } else if (direction == StairDirection::UP && stairsDown) {
        player->setX(stairsDown->getX());
        player->setY(stairsDown->getY());
    }

    camera->mapWidth  = map->getWidth();
    camera->mapHeight = map->getHeight();
    camera->update(player, isOutdoor);
    gameStatus = STARTUP;
}
```

### Map (BSP Room Generation)

**Changes to `Map::createRoom()`:**

The method tracks room count via the BspListener. The first room places the player and stairsUp; subsequent rooms update the stairsDown position (last room wins).

```cpp
void Map::createRoom(bool isFirstRoom, int x1, int y1, int x2, int y2, bool withActors)
{
    dig(x1, y1, x2, y2);
    if (!withActors) { return; }

    const int centreX = (x1 + x2) / 2;
    const int centreY = (y1 + y2) / 2;

    if (isFirstRoom) {
        // Player and stairsUp co-locate in first room.
        engine.player->setX(centreX);
        engine.player->setY(centreY);
        if (engine.stairsUp) {
            engine.stairsUp->setX(centreX);
            engine.stairsUp->setY(centreY);
        }
    } else {
        // Every non-first room updates stairsDown position — last room wins.
        if (engine.stairsDown) {
            engine.stairsDown->setX(centreX);
            engine.stairsDown->setY(centreY);
        }

        // Spawn monsters and items.
        TCODRandom* rng = TCODRandom::getInstance();
        int monstersToPlace = rng->getInt(0, MAX_ROOM_MONSTERS);
        int itemsToPlace    = rng->getInt(0, MAX_ROOM_ITEMS);

        while (monstersToPlace-- > 0) {
            const int mx = rng->getInt(x1, x2);
            const int my = rng->getInt(y1, y2);
            if (canWalk(mx, my)) { addMonster(mx, my); }
        }
        while (itemsToPlace-- > 0) {
            const int ix = rng->getInt(x1, x2);
            const int iy = rng->getInt(y1, y2);
            if (canWalk(ix, iy)) { addItem(ix, iy); }
        }
    }
}
```

### Map (Outdoor Placement)

**Changes to `Map::placeOutdoorActors()`:**

On the surface (depth 0), only stairsDown is placed. It goes in the largest connected ground region, at maximum Euclidean distance from the player.

```cpp
void Map::placeOutdoorActors()
{
    if (outdoorRegion.empty()) { return; }

    // Place player on a random ground tile.
    int playerIdx = rng->getInt(0, static_cast<int>(outdoorRegion.size()) - 1);
    auto [playerX, playerY] = outdoorRegion[playerIdx];
    engine.player->setX(playerX);
    engine.player->setY(playerY);

    // Place stairsDown (dungeon entrance) at furthest ground tile from player.
    if (engine.stairsDown) {
        int bestIdx = 0;
        float bestDist = 0.0f;
        for (int i = 0; i < static_cast<int>(outdoorRegion.size()); ++i) {
            auto [tx, ty] = outdoorRegion[i];
            float dx = static_cast<float>(tx - playerX);
            float dy = static_cast<float>(ty - playerY);
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > bestDist) {
                bestDist = dist;
                bestIdx = i;
            }
        }
        auto [sx, sy] = outdoorRegion[bestIdx];
        engine.stairsDown->setX(sx);
        engine.stairsDown->setY(sy);
    }

    // stairsUp is nullptr on the surface — nothing to place.

    // ... (monster and item placement unchanged) ...
}
```

### PlayerAi (Input Handling)

**Changes to `PlayerAi::handleActionKey()`:**

The `<` and `>` key handlers check the corresponding stair pointer. If the pointer is `nullptr` (boundary level), the stair doesn't exist and the message is shown.

```cpp
case '<': // ascend stairs
    if (engine.stairsUp
        && engine.stairsUp->getX() == owner->getX()
        && engine.stairsUp->getY() == owner->getY())
    {
        engine.nextLevel(StairDirection::UP);
    } else {
        engine.gui->message(Colors::uiText, "There are no stairs here.");
    }
    break;

case '>': // descend stairs
    if (engine.stairsDown
        && engine.stairsDown->getX() == owner->getX()
        && engine.stairsDown->getY() == owner->getY())
    {
        engine.nextLevel(StairDirection::DOWN);
    } else {
        engine.gui->message(Colors::uiText, "There are no stairs here.");
    }
    break;
```

### Persistent (Save/Load)

**Changes to `Engine::save()`:**

Serialize stairsUp then stairsDown. Use a sentinel integer (`-1`) to mark absence on boundary levels.

```cpp
void Engine::save()
{
    // ... (existing player/map/camera saves) ...

    // Stairs: write presence flag then actor data.
    static constexpr int STAIR_PRESENT = 1;
    static constexpr int STAIR_ABSENT  = -1;

    if (stairsUp) {
        zip.putInt(STAIR_PRESENT);
        stairsUp->save(zip);
    } else {
        zip.putInt(STAIR_ABSENT);
    }

    if (stairsDown) {
        zip.putInt(STAIR_PRESENT);
        stairsDown->save(zip);
    } else {
        zip.putInt(STAIR_ABSENT);
    }

    // Save remaining actors (excluding player, stairsUp, stairsDown).
    int count = 0;
    for (const auto& a : actors) {
        if (a.get() != player && a.get() != stairsUp && a.get() != stairsDown) count++;
    }
    zip.putInt(count);
    for (const auto& a : actors) {
        if (a.get() != player && a.get() != stairsUp && a.get() != stairsDown) {
            a->save(zip);
        }
    }

    // ... (gui, dungeonLevel) ...
}
```

**Changes to `Engine::load()`:**

```cpp
void Engine::load()
{
    // ... (map, camera, player restored) ...

    // Restore stairsUp.
    int upFlag = zip.getInt();
    if (upFlag == STAIR_PRESENT) {
        auto newUp = std::make_unique<Actor>(0, 0, 0, "", Colors::white);
        stairsUp = newUp.get();
        actors.emplace_front(std::move(newUp));
        stairsUp->load(zip);
    } else {
        stairsUp = nullptr;
    }

    // Restore stairsDown.
    int downFlag = zip.getInt();
    if (downFlag == STAIR_PRESENT) {
        auto newDown = std::make_unique<Actor>(0, 0, 0, "", Colors::white);
        stairsDown = newDown.get();
        actors.emplace_front(std::move(newDown));
        stairsDown->load(zip);
    } else {
        stairsDown = nullptr;
    }

    // ... (remaining actors, gui, dungeonLevel) ...
}
```

## Data Models

### StairDirection Enum

```cpp
// Declared in Engine.h (or a shared header).
enum class StairDirection { UP, DOWN };
```

### Engine Member Changes

| Old Member | New Members | Notes |
|---|---|---|
| `Actor* stairs;` | `Actor* stairsUp;` | `nullptr` when depth == 0 |
| | `Actor* stairsDown;` | `nullptr` when depth == 20 |

### Save Format Change

The save format replaces the single `stairs->save(zip)` call with:

| Field | Type | Description |
|---|---|---|
| stairsUp presence | `int` | 1 = present, -1 = absent |
| stairsUp data | Actor save | only if present |
| stairsDown presence | `int` | 1 = present, -1 = absent |
| stairsDown data | Actor save | only if present |

This is a **breaking save format change** — old saves are incompatible.

## Error Handling

- **Null stair pointers**: All code that dereferences stairsUp or stairsDown must first check for `nullptr`. This naturally occurs at boundary levels.
- **Level bounds**: `nextLevel()` should not be callable when descending from depth 20 or ascending from depth 0, enforced by the null-pointer check in PlayerAi input handling.
- **Outdoor region empty**: If the outdoor region is empty (degenerate case), stairsDown placement is skipped gracefully (existing guard clause).

## Interfaces

### Modified Function Signatures

```cpp
// Engine.h
void nextLevel(StairDirection direction);

// No change to Map::createRoom signature — it uses engine.stairsUp / engine.stairsDown directly.
```

### Actor List Iteration Changes

The `nextLevel()` cleanup loop removes ALL actors except `player`. Stair actors for the old level are destroyed. New stair actors are created fresh for the destination level after map generation.

## Testing Strategy

- **Unit tests**: Verify specific examples (stair creation properties like fovOnly/blocks, input handling at exact positions, boundary level behavior)
- **Property tests**: Verify universal invariants across randomized map seeds and depth values (stair presence rules, BSP placement separation, arrival positioning, save/load round trips)
- **Integration tests**: Verify full level-transition sequences through multiple depths

Property tests use Catch2's `GENERATE` with random seeds to cover the input space. Each property test runs a minimum of 100 iterations.

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system — essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Boundary-level stair presence invariant

*For any* dungeon depth d in [0, 20], the set of non-null stair pointers satisfies: stairsUp is present if and only if d > 0, and stairsDown is present if and only if d < 20.

**Validates: Requirements 2.1, 2.2, 2.3, 10.2**

### Property 2: BSP stair separation

*For any* BSP-generated map at a depth where both stairs exist (d in [1, 19]), stairsUp is placed in the first room (co-located with the player spawn) and stairsDown is placed in the last room, such that stairsUp position ≠ stairsDown position.

**Validates: Requirements 3.1, 3.2, 3.3**

### Property 3: Level transition arrival placement

*For any* level transition, if the player descended (used stairsDown), the player's position on the new level equals stairsUp's position; if the player ascended (used stairsUp), the player's position on the new level equals stairsDown's position.

**Validates: Requirements 4.1, 4.2**

### Property 4: Invalid stair-use rejection

*For any* player position that does not match the corresponding stair's position (or when the stair is absent), attempting to use that stair produces the message "There are no stairs here." and does not change depth.

**Validates: Requirements 5.2, 5.3, 6.2, 6.3**

### Property 5: Depth change direction

*For any* valid descent from depth d (where d < 20), the resulting depth is d + 1. *For any* valid ascent from depth d (where d > 0), the resulting depth is d - 1.

**Validates: Requirements 9.1, 9.2**

### Property 6: Save/load round trip preserves stair state

*For any* game state with stairs (present or absent due to boundary level), saving then loading produces equivalent stairsUp and stairsDown state: same positions, same glyphs, same presence/absence.

**Validates: Requirements 8.1, 8.2, 8.3**

### Property 7: Outdoor stairsDown placement in ground region

*For any* outdoor map generated at depth 0 with a non-empty ground region, stairsDown's position is a member of the largest connected ground region.

**Validates: Requirements 10.1**
