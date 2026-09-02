# Death Screen Not Showing Bugfix Design

## Overview

When the player dies, `PlayerDestructible::die()` (Source/Destructible.cpp) sets
`engine.gameStatus = Engine::DEFEAT` and records the run outcome. `Engine::render()`
correctly routes `DEFEAT` to `renderDefeat()` (the leaderboard overlay). The defect is
that the turn loop in `Engine::update()` unconditionally reassigns `gameStatus = IDLE`
at several end-of-turn sites within the same frame the player dies. Because those
assignments run *after* `die()` has already set `DEFEAT` (death happens inside
`runEnemyTurns()` or a start-of-turn status tick), the `DEFEAT` state is clobbered back
to `IDLE` before `render()` is ever reached. The death screen therefore never appears.

This is a state-management bug in the turn loop, not a rendering bug. The fix is minimal
and targeted: stop resetting `gameStatus` to `IDLE` once the player is dead, while
leaving every living-player transition exactly as it is today.

The core challenge is testability. Per `.kiro/steering/test-isolation.md`, the test binary
does not initialize the global `Engine` (no SDL window, no libtcod context, no `Gui`, no
`Map`), so `Engine::update()` cannot be exercised directly in a unit test. To make the fix
verifiable, the design introduces a small **pure predicate seam** — a free function that
encodes the "should this end-of-turn site reset to IDLE?" decision with no engine access —
that the turn loop consults instead of assigning `IDLE` unconditionally. The predicate is
unit-testable and property-testable in isolation.

## Glossary

- **Bug_Condition (C)**: The condition that triggers the bug — the turn loop is about to
  assign `gameStatus = IDLE` at an end-of-turn site while the player is already dead
  (`player->destructible->isDead() == true`, `die()` having just set `DEFEAT`).
- **Property (P)**: The desired behavior when the player is dead — the end-of-turn decision
  must be "do not reset to IDLE", so `DEFEAT` persists to `Engine::render()`.
- **Preservation**: Existing turn-flow behavior for a **living** player (all IDLE
  transitions after `player->update()` / `runEnemyTurns()`, the legacy `NEW_TURN` path, the
  stunned-skip path) and all DEFEAT-screen interactions and modal-overlay ESC→IDLE
  transitions, which must remain unchanged by the fix.
- **`shouldEndTurnToIdle(bool playerDead, Engine::GameStatus current)`**: The new pure free
  predicate (the seam) declared in `Headers/TurnFlow.hpp` and defined in
  `Source/TurnFlow.cpp`. Returns `false` when the turn loop must NOT reset to `IDLE`
  (player dead, or already in `DEFEAT`); returns `true` otherwise. It takes only value
  parameters and touches no engine globals, making it safe to call from the test binary.
- **`Engine::update()`**: The per-frame turn loop in Source/Engine.cpp that dispatches on
  `gameStatus` and, at several end-of-turn sites, assigns `gameStatus = IDLE`.
- **`PlayerDestructible::die()`**: Sets `engine.gameStatus = Engine::DEFEAT` when the player's
  HP reaches zero. The state the bug clobbers.
- **`Engine::GameStatus`**: The game-state enum (Headers/Engine.hpp) with values including
  `IDLE`, `PLAYER_TURN`, `NEW_TURN`, and `DEFEAT`.

## Bug Details

### Bug Condition

The bug manifests whenever `Engine::update()` reaches an end-of-turn `gameStatus = IDLE`
assignment during a frame in which the player has died. The player can die inside
`runEnemyTurns()` (enemy melee/ranged attack) or inside the start-of-turn status tick
(Burning/Bleeding). In every such case `PlayerDestructible::die()` has already set
`gameStatus = DEFEAT`, but the turn loop then overwrites it with `IDLE`, so `DEFEAT` never
survives to the render call.

**Formal Specification:**
```
FUNCTION isBugCondition(playerDead, gameStatus)
  INPUT: playerDead of type boolean      // player->destructible->isDead()
         gameStatus of type GameStatus    // engine.gameStatus at an IDLE-assignment site
  OUTPUT: boolean

  // The bug is triggered whenever the turn loop is about to reset to IDLE
  // even though the player has just died (die() already set DEFEAT).
  RETURN playerDead = true
END FUNCTION
```

The fixed decision function the turn loop consults instead of an unconditional assignment:

```
FUNCTION shouldEndTurnToIdle(playerDead, current)
  INPUT: playerDead of type boolean
         current of type GameStatus
  OUTPUT: boolean          // true  => turn loop may set gameStatus = IDLE
                           // false => turn loop must NOT reset (DEFEAT stands)

  IF playerDead = true THEN RETURN false        // never reset a dead player
  IF current = DEFEAT   THEN RETURN false        // never leave DEFEAT
  RETURN true                                    // living player: reset as before
END FUNCTION
```

### Examples

- **Melee crit kills player during `runEnemyTurns()`** (Req 1.1 / 2.1): `die()` sets
  `DEFEAT`; on return the trailing `gameStatus = IDLE` (after AP exhausted) overwrites it.
  Expected: end-of-turn decision is `false`, `DEFEAT` persists, death screen renders.
- **Ranged crit kills player during `runEnemyTurns()`** (Req 1.2 / 2.2): same trailing
  `gameStatus = IDLE` overwrites `DEFEAT`. Expected: `DEFEAT` persists.
- **Burning/Bleeding start-of-turn tick kills player** (Req 1.3 / 2.3): the IDLE-branch
  tick-death handler runs `gameStatus = IDLE; return;`, overwriting `DEFEAT`. Expected: the
  handler leaves `DEFEAT` and returns without resetting.
- **Player dies during legacy `NEW_TURN` enemy turns** (Req 1.4 / 2.4):
  `runEnemyTurns(); gameStatus = IDLE; return;` overwrites `DEFEAT`. Expected: `DEFEAT`
  persists.
- **Player dies during enemy turns on a stunned-skip turn** (Req 1.5 / 2.5): the
  stunned-skip path `runEnemyTurns(); gameStatus = IDLE; return;` overwrites `DEFEAT`.
  Expected: `DEFEAT` persists.
- **Edge — living player, AP exhausted** (Req 3.1): `shouldEndTurnToIdle(false, PLAYER_TURN)`
  returns `true`; the turn loop transitions to `IDLE` exactly as before.

## Expected Behavior

### Preservation Requirements

**Unchanged Behaviors:**
- A living player whose AP is exhausted after `player->update()` / `runEnemyTurns()` still
  transitions `gameStatus` to `IDLE` exactly as before (Req 3.1, 3.2).
- A living, stunned player still skips their turn, runs enemy turns, and transitions to
  `IDLE` (Req 3.3).
- Start-of-turn status ticks on a living player (that do not kill) still apply and proceed
  with the normal turn flow (Req 3.4); ticks on any living actor process as before (Req 3.5).
- DEFEAT-screen PageUp/PageDown scrolling continues to work (Req 3.6).
- Modal-overlay ESC/completion → IDLE transitions (inventory, look, character sheet, world
  map, help, pickup, tabbed menu, targeting) remain unchanged (Req 3.7). These are NOT
  end-of-turn death sites and are NOT touched by this fix.

**Scope:**
All inputs where the player is NOT dead (`playerDead == false`) must be completely
unaffected by this fix. This includes:
- Living-player end-of-turn transitions (PLAYER_TURN, IDLE, legacy NEW_TURN, stunned-skip)
- Enemy status-tick processing
- DEFEAT-screen scrolling and all modal-overlay ESC→IDLE transitions

The expected *correct* behavior for the dead-player case is defined in Property 1 below.

## Hypothesized Root Cause

Confirmed by reading Source/Engine.cpp and Source/Destructible.cpp. The root cause is that
`PlayerDestructible::die()` sets `engine.gameStatus = Engine::DEFEAT`, but `Engine::update()`
then unconditionally overwrites `gameStatus` back to `IDLE` in the same frame at four
end-of-turn sites. Categorized:

1. **Tick-death handler resets to IDLE (IDLE branch, ~line 183)**: after the player's
   start-of-turn status tick, the handler runs
   `if (player->destructible->isDead()) { gameStatus = IDLE; return; }`. This explicitly
   detects death and *then resets to IDLE* — directly clobbering the `DEFEAT` that `die()`
   set via the tick's `takeDamage()`.

2. **Trailing end-of-turn resets after AP exhaustion**: both the PLAYER_TURN path (~line 153)
   and the IDLE path (~line 201) run
   `if (AP <= 0) { runEnemyTurns(); gameStatus = IDLE; }`. If the player dies inside
   `runEnemyTurns()` (enemy attack), the following `gameStatus = IDLE` clobbers `DEFEAT`.

3. **Legacy NEW_TURN branch (~line 161)**: `runEnemyTurns(); gameStatus = IDLE; return;`
   clobbers `DEFEAT` if the player dies during those enemy turns.

4. **Stunned-skip path (~line 191)**: `runEnemyTurns(); gameStatus = IDLE; return;` clobbers
   `DEFEAT` if the player dies during the enemy turns run on the skipped turn.

The rendering path (`renderDefeat()`), DEFEAT scrolling, `recordRunOutcome()`, and modal
ESC→IDLE transitions are all correct and are not causes.

## Correctness Properties

Property 1: Bug Condition - Dead player is never reset to IDLE (DEFEAT preserved)

_For any_ input where the bug condition holds (`isBugCondition(playerDead, gameStatus)`
returns true, i.e. `playerDead == true`), the fixed decision function
`shouldEndTurnToIdle(playerDead, gameStatus)` SHALL return `false`, so the turn loop does
NOT assign `IDLE` and the `DEFEAT` state set by `die()` persists through to
`Engine::render()`, causing the death screen to be shown. Additionally, for any
`gameStatus == DEFEAT` (even with `playerDead == false`) the function SHALL return `false`.

**Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5, 2.6**

Property 2: Preservation - Living player still transitions to IDLE

_For any_ input where the bug condition does NOT hold (`isBugCondition` returns false, i.e.
`playerDead == false`) and `gameStatus != DEFEAT`, the fixed decision function SHALL produce
the same result as the original behavior — namely `shouldEndTurnToIdle(false, gameStatus)`
returns `true`, so the turn loop transitions to `IDLE` at those end-of-turn sites exactly as
before, preserving all living-player turn flow.

**Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.5**

## Fix Implementation

### The Seam (new pure, engine-free predicate)

Introduce a new header/source pair so the decision is unit-testable without the Engine.

**File**: `Headers/TurnFlow.hpp` (new)
```cpp
#pragma once

#include "Engine.hpp"  // for Engine::GameStatus

// Pure decision predicate for the end-of-turn "reset to IDLE" sites in
// Engine::update(). Engine-free and side-effect-free so it is safe to unit
// test without initializing the global Engine (see test-isolation.md).
//
// Returns true  => the turn loop MAY set gameStatus = IDLE.
// Returns false => the turn loop MUST NOT reset (a dead player / DEFEAT stands).
bool shouldEndTurnToIdle(bool playerDead, Engine::GameStatus current);
```

**File**: `Source/TurnFlow.cpp` (new)
```cpp
#include "TurnFlow.hpp"

bool shouldEndTurnToIdle(bool playerDead, Engine::GameStatus current)
{
    if (playerDead) return false;                 // never reset a dead player (Property 1)
    if (current == Engine::DEFEAT) return false;   // never leave DEFEAT (Property 1)
    return true;                                   // living player: reset as before (Property 2)
}
```

**Project registration** (per `.kiro/steering/test-project.md`): the new source file MUST be
added to BOTH projects.
- `40kRL.vcxproj` — in the source `ItemGroup`, add `<ClCompile Include="Source\TurnFlow.cpp" />`.
- `Tests/40kRL_Tests.vcxproj` — in the "Game source files" ItemGroup (all except main.cpp),
  add `<ClCompile Include="..\Source\TurnFlow.cpp" />`.

### Guarding the four overwrite sites

Include `"TurnFlow.hpp"` in Source/Engine.cpp, then guard each end-of-turn `IDLE`
assignment. In all cases the player-dead argument is `player->destructible->isDead()`.

**File**: `Source/Engine.cpp`  **Function**: `Engine::update()`

**Site 1 — tick-death handler (IDLE branch, ~line 183).** Do NOT reset to IDLE when the
player is dead; let `DEFEAT` stand and return.
```cpp
// BEFORE
if (player->destructible && player->destructible->isDead()) {
    gameStatus = IDLE;
    return;
}

// AFTER
if (player->destructible && player->destructible->isDead()) {
    // die() has already set DEFEAT; do not reset to IDLE (Property 1).
    return;
}
```

**Site 2 — trailing reset after AP exhaustion (PLAYER_TURN path, ~line 153).**
```cpp
// BEFORE
if (player->actionBudget && player->actionBudget->getAP() <= 0) {
    runEnemyTurns();
    gameStatus = IDLE;
}

// AFTER
if (player->actionBudget && player->actionBudget->getAP() <= 0) {
    runEnemyTurns();
    if (shouldEndTurnToIdle(player->destructible && player->destructible->isDead(), gameStatus))
        gameStatus = IDLE;
}
```

**Site 2 (cont.) — trailing reset after AP exhaustion (IDLE path, ~line 201).** Identical
guard applied to the second occurrence:
```cpp
// AFTER
if (player->actionBudget && player->actionBudget->getAP() <= 0) {
    runEnemyTurns();
    if (shouldEndTurnToIdle(player->destructible && player->destructible->isDead(), gameStatus))
        gameStatus = IDLE;
}
```

**Site 3 — legacy NEW_TURN branch (~line 161).**
```cpp
// BEFORE
if (gameStatus == NEW_TURN) {
    runEnemyTurns();
    gameStatus = IDLE;
    return;
}

// AFTER
if (gameStatus == NEW_TURN) {
    runEnemyTurns();
    if (shouldEndTurnToIdle(player->destructible && player->destructible->isDead(), gameStatus))
        gameStatus = IDLE;
    return;
}
```

**Site 4 — stunned-skip path (~line 191).**
```cpp
// BEFORE
if (!canAct) {
    if (player->actionBudget) player->actionBudget->setAP(0);
    engine.gui->message(Colors::damage, "You are stunned and cannot act!");
    runEnemyTurns();
    gameStatus = IDLE;
    return;
}

// AFTER
if (!canAct) {
    if (player->actionBudget) player->actionBudget->setAP(0);
    engine.gui->message(Colors::damage, "You are stunned and cannot act!");
    runEnemyTurns();
    if (shouldEndTurnToIdle(player->destructible && player->destructible->isDead(), gameStatus))
        gameStatus = IDLE;
    return;
}
```

**Note on the TARGETING sub-paths (~lines 61, 66):** the `NEW_TURN`/`PLAYER_TURN` resets
inside the `gameStatus == TARGETING` block also call `runEnemyTurns()`. For completeness and
consistency they may be guarded with the same predicate, since the player could in principle
die during those enemy turns. This is a safe extension of the same pattern and does not
change living-player behavior. The four sites enumerated in the requirements are the
mandatory fixes; the targeting sub-paths are guarded identically.

**Out of scope (must remain unchanged):** modal-overlay ESC→IDLE transitions,
`renderDefeat()`, DEFEAT PageUp/PageDown scrolling, and `recordRunOutcome()`.

## Testing Strategy

### Validation Approach

Two-phase approach: first surface a counterexample that demonstrates the bug on unfixed
code, then verify the fix works and preserves living-player behavior. Because the turn loop
requires a fully initialized Engine/GUI that the test binary does not create
(`test-isolation.md`), all assertions target the pure `shouldEndTurnToIdle` seam rather than
`Engine::update()` directly. Tests live in `Tests/` using Catch2 v3 and RapidCheck, with new
test `.cpp` files added to `Tests/40kRL_Tests.vcxproj`.

### Exploratory Bug Condition Checking

**Goal**: Surface the counterexample that demonstrates the bug BEFORE implementing the fix,
and confirm the root cause (an unconditional `gameStatus = IDLE` reset).

**Test Plan**: Write a test asserting `shouldEndTurnToIdle(true, Engine::IDLE) == false` and
`shouldEndTurnToIdle(true, Engine::DEFEAT) == false`. On the UNFIXED code the seam does not
exist, so the test **fails to build/link** (unresolved external symbol) — this is the
documented "fails on unfixed code" state for this bugfix. Once the seam is introduced, the
property holds. This mirrors the real defect: before the fix the turn loop resets a dead
player to IDLE; after the fix the decision is `false`.

**Test Cases**:
1. **Dead player, IDLE site**: `shouldEndTurnToIdle(true, Engine::IDLE)` must be `false`
   (fails to link on unfixed code).
2. **Dead player, PLAYER_TURN site**: `shouldEndTurnToIdle(true, Engine::PLAYER_TURN)` must
   be `false` (fails to link on unfixed code).
3. **Already DEFEAT**: `shouldEndTurnToIdle(false, Engine::DEFEAT)` must be `false`
   (fails to link on unfixed code).

**Expected Counterexamples**:
- On unfixed code: the symbol `shouldEndTurnToIdle` does not exist → link failure (LNK2001),
  the documented pre-fix failure. Conceptually, the pre-fix turn loop maps the dead-player
  input to a reset-to-IDLE, i.e. the wrong decision.
- Possible causes: unconditional `gameStatus = IDLE` at the four end-of-turn sites; the
  tick-death handler explicitly resetting to IDLE.

### Fix Checking

**Goal**: Verify that for all inputs where the bug condition holds, the fixed function
produces the expected behavior (never reset to IDLE; DEFEAT preserved).

**Pseudocode:**
```
FOR ALL (playerDead, gameStatus) WHERE isBugCondition(playerDead, gameStatus) DO
  result := shouldEndTurnToIdle(playerDead, gameStatus)   // playerDead == true
  ASSERT result = false
END FOR
// plus: for any status, DEFEAT is a fixed point
ASSERT shouldEndTurnToIdle(false, Engine::DEFEAT) = false
```

### Preservation Checking

**Goal**: Verify that for all inputs where the bug condition does NOT hold, the fixed
function produces the same result as the original behavior (living player → transition to
IDLE at these sites).

**Pseudocode:**
```
FOR ALL (playerDead, gameStatus) WHERE NOT isBugCondition(playerDead, gameStatus) DO
  // playerDead == false, and gameStatus != DEFEAT at these end-of-turn sites
  ASSERT shouldEndTurnToIdle(false, gameStatus) = true
END FOR
```

**Testing Approach**: Property-based testing is recommended for preservation because it
generates many `GameStatus` values automatically across the enum domain, catches edge cases
manual tests miss, and gives a strong guarantee that living-player behavior is unchanged.

**Test Plan**: Observe on the (fixed) seam that every non-DEFEAT status with a living player
maps to `true`, matching the original unconditional-IDLE behavior.

**Test Cases**:
1. **Living-player transition preserved**: for a living player at PLAYER_TURN / IDLE /
   NEW_TURN, the decision is `true` (reset to IDLE) — matches original behavior.
2. **DEFEAT fixed point**: `shouldEndTurnToIdle(false, Engine::DEFEAT) == false` — the loop
   never leaves DEFEAT.
3. **Dead player never resets**: `shouldEndTurnToIdle(true, anyStatus) == false`.

### Unit Tests

- `shouldEndTurnToIdle(true, Engine::DEFEAT) == false` (dead + DEFEAT — the exact death case).
- `shouldEndTurnToIdle(true, Engine::IDLE) == false` (tick-death handler case).
- `shouldEndTurnToIdle(true, Engine::PLAYER_TURN) == false` (enemy-turn death case).
- `shouldEndTurnToIdle(false, Engine::PLAYER_TURN) == true` (living player, AP exhausted —
  preservation).
- `shouldEndTurnToIdle(false, Engine::NEW_TURN) == true` (legacy path preservation).
- `shouldEndTurnToIdle(false, Engine::DEFEAT) == false` (never leave DEFEAT).

### Property-Based Tests

Use RapidCheck with a minimum of 100 iterations. Generate `Engine::GameStatus` values via
`rc::gen::inRange(0, static_cast<int>(Engine::HIGH_SCORES))` cast back to `GameStatus`
(inclusive bounds per `test-isolation.md`), plus a boolean dead/alive flag.

- **Property 1 (fix)**: for all generated `status`, `shouldEndTurnToIdle(true, status)` is
  `false` (a dead player is never reset to IDLE).
- **Property 1 (DEFEAT fixed point)**: for all generated `dead`,
  `shouldEndTurnToIdle(dead, Engine::DEFEAT)` is `false`.
- **Property 2 (preservation)**: for all generated `status != DEFEAT`,
  `shouldEndTurnToIdle(false, status)` is `true` (living-player behavior identical to the
  original unconditional reset).

### Integration Tests

Full `Engine::update()` flow integration is not viable in the headless test binary (no
initialized Engine/GUI/Map). Integration coverage is therefore documented as manual QA:
- Die to an enemy melee crit during enemy turns → death screen (leaderboard) appears.
- Die to an enemy ranged crit → death screen appears.
- Die to a Burning/Bleeding start-of-turn tick → death screen appears.
- On the death screen, PageUp/PageDown still scroll the leaderboard (preservation).
- A living player's turn still ends and control returns to IDLE normally (preservation).
