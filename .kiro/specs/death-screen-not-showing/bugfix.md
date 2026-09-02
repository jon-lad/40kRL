# Bugfix Requirements Document

## Introduction

When the player dies, the death screen — the high-score leaderboard overlay rendered by `Engine::renderDefeat()` — fails to appear. Instead the normal game view and HUD stay on screen (the sidebar with the injury line remains visible). This was reported by a player who died to a melee crit and never saw the leaderboard.

The rendering path itself is correctly wired: `PlayerDestructible::die()` (Source/Destructible.cpp) sets `engine.gameStatus = Engine::DEFEAT` and calls `engine.recordRunOutcome(...)`; `Engine::render()` routes `if (gameStatus == DEFEAT) { renderDefeat(); return; }`; and `Engine::update()` has a DEFEAT branch that handles PageUp/PageDown scrolling. The defect is that the turn loop in `Engine::update()` unconditionally reassigns `gameStatus` back to `IDLE` within the same frame the player dies, so the DEFEAT state set by `die()` is clobbered before `render()` is ever reached. Because DEFEAT never survives, the death screen never renders.

This is a state-management bug in the turn loop, not a rendering bug. The impact is severe: the end-of-run leaderboard is a core feature that is currently unreachable for the most common death cause (being killed on an enemy's turn).

## Bug Analysis

### Current Behavior (Defect)

When the player dies, `PlayerDestructible::die()` sets `gameStatus = DEFEAT`, but `Engine::update()` immediately overwrites it back to `IDLE` in the same frame, so `Engine::render()` never sees DEFEAT and the leaderboard overlay is not shown.

1.1 WHEN the player is killed by an enemy melee attack (e.g. a melee crit) during `runEnemyTurns()` THEN the turn loop reaches the trailing `gameStatus = IDLE` assignment (after `player->update()` / `runEnemyTurns()` when the player's AP is exhausted) and overwrites the DEFEAT state that `die()` just set, so the death screen does not appear.

1.2 WHEN the player is killed by an enemy ranged attack (e.g. a ranged crit) during `runEnemyTurns()` THEN the same trailing `gameStatus = IDLE` assignment overwrites DEFEAT and the death screen does not appear.

1.3 WHEN the player dies from a start-of-turn status-effect tick (e.g. Burning or Bleeding) THEN the IDLE-branch tick-death handler detects `player->destructible->isDead()` and executes `gameStatus = IDLE; return;`, overwriting the DEFEAT state that `die()` set, so the death screen does not appear.

1.4 WHEN the player dies during the enemy turns triggered by the legacy `NEW_TURN` branch THEN that branch runs `runEnemyTurns(); gameStatus = IDLE; return;` and overwrites DEFEAT, so the death screen does not appear.

1.5 WHEN the player is stunned and their turn is skipped, and the player dies during the enemy turns run on that skipped turn THEN the stunned-skip path runs `runEnemyTurns(); gameStatus = IDLE; return;` and overwrites DEFEAT, so the death screen does not appear.

1.6 WHEN the death screen fails to appear THEN the system continues displaying the normal game view and HUD (the sidebar with the injury line), leaving the player in a stuck state with no leaderboard and no run-end feedback.

### Expected Behavior (Correct)

Once the player is dead, the game must enter and remain in the DEFEAT state so that `Engine::render()` shows the death screen (leaderboard overlay), regardless of what killed the player.

2.1 WHEN the player is killed by an enemy melee attack during `runEnemyTurns()` THEN the system SHALL remain in the DEFEAT state after the turn loop completes so that `Engine::render()` shows the death screen.

2.2 WHEN the player is killed by an enemy ranged attack during `runEnemyTurns()` THEN the system SHALL remain in the DEFEAT state after the turn loop completes so that `Engine::render()` shows the death screen.

2.3 WHEN the player dies from a start-of-turn status-effect tick (e.g. Burning or Bleeding) THEN the system SHALL remain in the DEFEAT state rather than resetting to IDLE, so that `Engine::render()` shows the death screen.

2.4 WHEN the player dies during the enemy turns triggered by the legacy `NEW_TURN` branch THEN the system SHALL remain in the DEFEAT state rather than resetting to IDLE.

2.5 WHEN the player dies during enemy turns run on a stunned-skip turn THEN the system SHALL remain in the DEFEAT state rather than resetting to IDLE.

2.6 WHEN the player is dead THEN the turn loop in `Engine::update()` SHALL NOT assign `gameStatus` to IDLE (or any non-DEFEAT state) at any point after the player could have died (status ticks, `player->update()`, or `runEnemyTurns()`), so that the DEFEAT state set by `die()` persists through to the render call.

### Unchanged Behavior (Regression Prevention)

The fix must only affect the case where the player is already dead. All normal turn-flow behavior for a living player, and existing DEFEAT-screen interactions, must be preserved.

3.1 WHEN the player is alive and their AP is exhausted after `player->update()` / `runEnemyTurns()` THEN the system SHALL CONTINUE TO transition `gameStatus` to IDLE exactly as before.

3.2 WHEN the player is alive and the turn loop is in the PLAYER_TURN or legacy NEW_TURN paths THEN the system SHALL CONTINUE TO run enemy turns via `runEnemyTurns()` and then transition to IDLE as before.

3.3 WHEN the player is alive and stunned THEN the system SHALL CONTINUE TO skip the player's turn, run enemy turns, and transition to IDLE as before.

3.4 WHEN a start-of-turn status-effect tick occurs on a living player (does not reduce them to dead) THEN the system SHALL CONTINUE TO apply the tick and proceed with the normal turn flow unchanged.

3.5 WHEN status-effect ticks occur on any living actor (player or enemy) THEN the system SHALL CONTINUE TO process them exactly as before.

3.6 WHEN the game is already in the DEFEAT state and the player presses PageUp or PageDown THEN the system SHALL CONTINUE TO scroll the leaderboard as before.

3.7 WHEN a modal overlay (inventory, look, character sheet, world map, help, pickup, tabbed menu, targeting, etc.) is dismissed with ESC or completed THEN the system SHALL CONTINUE TO transition to IDLE exactly as before (these IDLE assignments are unrelated to the player-death path and must remain unchanged).

## Deriving the Bug Condition

**Bug Condition Function** — identifies inputs that trigger the bug:

```pascal
FUNCTION isBugCondition(playerDead, gameStatus)
  INPUT: playerDead of type boolean   // player->destructible->isDead()
         gameStatus of type Status    // engine.gameStatus at an IDLE-assignment site
  OUTPUT: boolean

  // The bug is triggered whenever the turn loop is about to reset to IDLE
  // even though the player has just died (die() already set DEFEAT).
  RETURN playerDead = true
END FUNCTION
```

**Property — Fix Checking** (desired behavior for buggy inputs):

```pascal
// Property: A dead player must never be transitioned back to IDLE.
FOR ALL (playerDead, gameStatus) WHERE isBugCondition(playerDead, gameStatus) DO
  result ← shouldEndTurnToIdle'(playerDead, gameStatus)
  ASSERT result = false          // never reset to IDLE when dead
  ASSERT finalGameStatus = DEFEAT // DEFEAT persists to render()
END FOR
```

**Property — Preservation Checking** (behavior for non-buggy inputs is unchanged):

```pascal
// Property: For a living player, turn-end behavior is identical before and after the fix.
FOR ALL (playerDead, gameStatus) WHERE NOT isBugCondition(playerDead, gameStatus) DO
  ASSERT shouldEndTurnToIdle(playerDead, gameStatus)   // F  (original)
       = shouldEndTurnToIdle'(playerDead, gameStatus)  // F' (fixed)
END FOR
```

- **F**: the turn loop as it exists today (unconditional `gameStatus = IDLE` at the end-of-turn sites).
- **F'**: the fixed turn loop that guards those assignments so a dead player is never reset to IDLE.

## Notes & Scope (for the Design phase)

- **Overwrite sites to address** (Source/Engine.cpp, `Engine::update()` and its branches):
  1. IDLE-branch start-of-turn tick-death handler: `if (player->destructible->isDead()) { gameStatus = IDLE; return; }` (~line 183).
  2. Trailing end-of-turn resets after `player->update()` / `runEnemyTurns()` when AP is exhausted (~lines 153, 201) and the PLAYER_TURN path (~line 66).
  3. Legacy `NEW_TURN` branch: `runEnemyTurns(); gameStatus = IDLE; return;` (~lines 61, 161).
  4. Stunned-skip path: `runEnemyTurns(); gameStatus = IDLE; return;` (~line 191).
  - The general fix pattern is to guard each end-of-turn `gameStatus = IDLE` assignment with a check that the player is not dead / `gameStatus` is not already DEFEAT, or to bail out early once the player is dead. The tick-death handler at (1) should stop resetting to IDLE entirely.

- **Testability challenge** (see `.kiro/steering/test-isolation.md`): the turn loop needs a fully initialized Engine and GUI, which the test binary does not create, so exercising `Engine::update()` directly in a unit test is not viable. The design phase should introduce a small, pure testable seam so the fix can be verified without the engine — for example:
  - a free/static predicate like `shouldEndTurnToIdle(bool playerDead, Status gameStatus)` that returns `false` when the player is dead, and which the turn loop consults instead of assigning IDLE unconditionally; or
  - an `isPlayerDead()` helper used to guard the assignments,
  so a unit test can assert that a dead player never transitions to IDLE and that DEFEAT is preserved, while a living player still transitions to IDLE exactly as before. The choice of seam is deferred to the design phase.

- **Out of scope for this bugfix**: the rendering of the leaderboard (`renderDefeat()`), the DEFEAT PageUp/PageDown scrolling, `recordRunOutcome(...)`, and the modal-overlay ESC → IDLE transitions. These are already correct and must remain unchanged.
