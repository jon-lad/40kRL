# Decoration Render Priority Bugfix Design

## Overview

Decorations (crates, barrels, consoles, pillars) are drawn over actors (player, enemies) when they share the same tile because `Engine::render()` iterates `actors` in list-insertion order — decorations appended via `push_back` end up later in the list and overwrite the glyph of actors inserted earlier. The fix ensures decorations are always rendered before gameplay actors by inserting them at the front of the actors list (similar to the existing `sendToBack` pattern used for corpses).

## Glossary

- **Bug_Condition (C)**: A decoration and a gameplay actor (one that has `ai`, `destructible`, or `attacker`) share the same tile coordinates, and the decoration appears later in the `actors` list, causing its glyph to overwrite the actor's glyph during rendering
- **Property (P)**: Actors with gameplay components (ai, destructible, or attacker) are always rendered on top of decorations sharing the same tile
- **Preservation**: All existing behaviors — mouse clicks, corpse sendToBack, decoration rendering on empty tiles, and actor movement — remain unchanged
- **Decoration**: An Actor that has no `ai`, no `destructible`, and no `attacker` component (purely visual/environmental)
- **Gameplay Actor**: An Actor that has at least one of `ai`, `destructible`, or `attacker` (player, enemies, destructible objects)
- **`sendToBack(Actor*)`**: Existing function in `Engine` that splices an actor to the front of the `actors` list so it renders beneath everything else
- **`actors`**: `std::list<std::unique_ptr<Actor>>` in `Engine` — render order is determined by iteration order (front = drawn first = underneath)

## Bug Details

### Bug Condition

The bug manifests when a decoration and a gameplay actor occupy the same tile. Because `addDecorations()` and `addOutdoorDecorations()` call `engine.actors.push_back()`, decorations are appended after gameplay actors in the list. When `Engine::render()` iterates front-to-back, decorations at the back overwrite the glyph of actors at the front.

**Formal Specification:**
```
FUNCTION isBugCondition(input)
  INPUT: input of type {actorsList: List<Actor>, tileX: int, tileY: int}
  OUTPUT: boolean
  
  LET actorsOnTile = actorsList.filter(a => a.x == tileX AND a.y == tileY)
  LET decorations = actorsOnTile.filter(a => a.ai == NULL AND a.destructible == NULL AND a.attacker == NULL)
  LET gameplayActors = actorsOnTile.filter(a => a.ai != NULL OR a.destructible != NULL OR a.attacker != NULL)
  
  RETURN decorations.notEmpty()
         AND gameplayActors.notEmpty()
         AND EXISTS d IN decorations, g IN gameplayActors:
             listIndex(actorsList, d) > listIndex(actorsList, g)
END FUNCTION
```

### Examples

- Player (index 0) stands on an ammo crate (index 15): crate glyph 'X' overwrites player glyph '@' — player is invisible
- An Ork enemy (index 5) occupies same tile as a rockcrete pillar (index 20): pillar 'O' overwrites Ork 'o' — enemy is invisible
- Player walks onto a barricade (index 18): barricade '=' overwrites player '@' until player moves off
- Two decorations on same tile with no actor present: no bug — both are decorations, no gameplay actor is hidden

## Expected Behavior

### Preservation Requirements

**Unchanged Behaviors:**
- Decorations on tiles with no gameplay actor render normally at their position
- Corpses continue to render beneath living actors (existing `sendToBack` behavior preserved)
- Multiple decorations on the same tile render without specific ordering between them
- Decorations render normally after a player moves off their tile
- Decorations loaded from save/cache continue to render correctly beneath actors
- `canWalk()` behavior is not affected — blocking decorations still prevent movement

**Scope:**
All inputs that do NOT involve a decoration and a gameplay actor sharing the same tile should be completely unaffected by this fix. This includes:
- Tiles containing only gameplay actors
- Tiles containing only decorations
- Empty tiles
- Actor rendering order between non-decoration actors (player always renders, enemies render normally)

## Hypothesized Root Cause

Based on the code analysis, the root cause is confirmed:

1. **List Insertion Order**: Both `addDecorations()` (Map.cpp:1003) and `addOutdoorDecorations()` (Map.cpp:385) call `engine.actors.push_back(std::move(decoration))` which places decorations at the end of the list, after all previously inserted gameplay actors (monsters, player, stairs).

2. **Linear Render Loop Without Priority**: `Engine::render()` iterates `actors` front-to-back and each actor calls `actor->render()` which writes its glyph to the console at its screen position. Later actors overwrite earlier actors on the same tile — there is no z-ordering or priority check.

3. **No Classification at Insertion Time**: Decorations have no distinguishing component or flag that the render loop could use to skip them or defer them. They lack `ai`, `destructible`, and `attacker`, but this isn't leveraged.

4. **Contrast With Corpses**: The existing `sendToBack()` function already solves this exact problem for corpses — when an actor dies, it is spliced to the front of the list so living actors render on top. Decorations need the same treatment at spawn time.

## Correctness Properties

Property 1: Bug Condition - Decorations Render Beneath Gameplay Actors

_For any_ tile where a decoration and a gameplay actor (one with ai, destructible, or attacker) share the same coordinates, the decoration SHALL appear earlier in the actors list than the gameplay actor, ensuring the gameplay actor's glyph is rendered on top and remains visible.

**Validates: Requirements 2.1, 2.2**

Property 2: Preservation - Non-Overlapping Tile Rendering Unchanged

_For any_ tile where no decoration shares coordinates with a gameplay actor, the rendering behavior SHALL produce the same result as the original code, preserving decoration visibility on empty tiles, corpse ordering, and all actor interactions.

**Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.5**

## Fix Implementation

### Changes Required

Assuming our root cause analysis is correct:

**File**: `Source/Map.cpp`

**Functions**: `addDecorations()` and `addOutdoorDecorations()`

**Specific Changes**:

1. **Replace `push_back` with `push_front` in `addDecorations()`**: Change `engine.actors.push_back(std::move(decoration))` to `engine.actors.push_front(std::move(decoration))` at line ~1003. This places decorations at the front of the list (drawn first, underneath everything else), mirroring how `sendToBack()` works for corpses.

2. **Replace `push_back` with `push_front` in `addOutdoorDecorations()`**: Change `engine.actors.push_back(std::move(decoration))` to `engine.actors.push_front(std::move(decoration))` at line ~385. Same rationale — outdoor decorations must render beneath actors.

3. **Deserialization path**: Verify that `deserializeLevel()` places decorations at the front when restoring from cache. If it currently uses `push_back` for all actors, decorations loaded from cache must also be placed at the front to satisfy requirement 3.5.

4. **No render loop changes needed**: Because the fix operates at insertion time, the render loop in `Engine::render()` remains unchanged — it still iterates front-to-back, but now decorations are guaranteed to be at the front.

5. **No Actor class changes needed**: No new component, flag, or render priority field is required. The existing component presence (`ai == nullptr && destructible == nullptr && attacker == nullptr`) is the implicit marker for "decoration" used only at insertion time.

## Testing Strategy

### Validation Approach

The testing strategy follows a two-phase approach: first, surface counterexamples that demonstrate the bug on unfixed code, then verify the fix works correctly and preserves existing behavior.

### Exploratory Bug Condition Checking

**Goal**: Surface counterexamples that demonstrate the bug BEFORE implementing the fix. Confirm that decorations currently appear after gameplay actors in the list when sharing a tile.

**Test Plan**: Write tests that spawn a gameplay actor and a decoration on the same tile, then assert on the relative list positions. Run on UNFIXED code to observe that decorations always appear later in the list.

**Test Cases**:
1. **BSP Room Decoration Test**: Create a room, spawn a monster and a decoration on the same tile — verify decoration appears later in actors list (will fail assertion that decoration is before actor on unfixed code)
2. **Outdoor Decoration Test**: Generate outdoor map with player on a decoration tile — verify decoration is after player in list (will fail on unfixed code)
3. **Multiple Actors Same Tile**: Spawn player, enemy, and decoration on same tile — verify decoration is last (will fail on unfixed code)
4. **Post-Deserialize Test**: Save and load a level with overlapping decoration/actor — verify order after load (may fail on unfixed code)

**Expected Counterexamples**:
- Decorations have higher list indices than gameplay actors sharing their tile
- Confirmed cause: `push_back` places them at end; no reordering occurs

### Fix Checking

**Goal**: Verify that for all inputs where the bug condition holds, the fixed function produces the expected behavior.

**Pseudocode:**
```
FOR ALL input WHERE isBugCondition(input) DO
  result := renderOrder(actors_fixed)
  ASSERT FOR ALL tiles WITH overlapping decoration AND gameplay actor:
    listIndex(decoration) < listIndex(gameplayActor)
END FOR
```

### Preservation Checking

**Goal**: Verify that for all inputs where the bug condition does NOT hold, the fixed function produces the same result as the original function.

**Pseudocode:**
```
FOR ALL input WHERE NOT isBugCondition(input) DO
  ASSERT renderOutput_original(input) = renderOutput_fixed(input)
END FOR
```

**Testing Approach**: Property-based testing is recommended for preservation checking because:
- It generates many random map configurations with varying decoration/actor placements
- It catches edge cases like empty rooms, rooms with only decorations, or rooms with only actors
- It provides strong guarantees that behavior is unchanged for all non-overlapping tiles

**Test Plan**: Observe rendering behavior on UNFIXED code for tiles without overlap, then write property-based tests capturing that behavior continues after the fix.

**Test Cases**:
1. **Decoration-Only Tile Preservation**: Verify decorations on tiles with no actors render identically before and after fix
2. **Corpse Ordering Preservation**: Verify that `sendToBack` for corpses still places them at the front, rendering beneath living actors
3. **Actor-Only Tile Preservation**: Verify tiles with only gameplay actors (no decorations) render identically
4. **Empty Tile Preservation**: Verify empty tiles are unaffected

### Unit Tests

- Test that `addDecorations()` inserts decorations at the front of the actors list
- Test that `addOutdoorDecorations()` inserts decorations at the front of the actors list
- Test that decorations on a tile with a gameplay actor have a lower list index than the actor
- Test that existing `sendToBack()` behavior for corpses is unaffected
- Test that `deserializeLevel()` places decorations at the front

### Property-Based Tests

- Generate random room bounds and decoration counts, verify all spawned decorations appear before gameplay actors in the list
- Generate random actor/decoration placements on tiles, verify rendering order invariant holds (decorations always before actors)
- Generate random game states with mixed tile occupancy, verify non-overlapping tiles produce identical render output

### Integration Tests

- Test full level generation (BSP) with decorations — verify player is always visible when standing on a decoration
- Test full outdoor level generation — verify enemies and player render on top of decorations
- Test level transition and cache restore — verify decoration order is preserved across save/load
