# Requirements Document

## Introduction

Refactor the tile-picking and targeting system from a blocking sub-loop (`pickAtTile`) into a state-machine-based approach that runs within the main game loop. The current implementation calls `TCODConsole::flush()` from a nested rendering loop, which does not present console contents correctly under libtcod 2.2.0 + SDL3. The deprecated libtcod API requires all rendering to happen in the single main loop.

The solution adds a TARGETING game state to the Engine. When the player uses an item requiring tile selection (SELECTED_MONSTER, SELECTED_RANGE), the game transitions to TARGETING state. The main update/render loop then handles tile highlights, cursor rendering, input reading, and target confirmation or cancellation — eliminating the need for any secondary rendering loop.

Additionally, the inventory menu (`chooseFromInventory`) and `Menu::pick` use the deprecated `TCODSystem::waitForEvent` API which is incompatible with SDL3. These must also be refactored to operate within the main loop's event system.

## Glossary

- **Engine**: The global game state machine that owns the actor list, map, camera, GUI, and drives the main update/render loop.
- **GameStatus**: An enum on Engine controlling which systems run each frame (STARTUP, IDLE, NEW_TURN, VICTORY, DEFEAT, and the new TARGETING).
- **TARGETING**: A new GameStatus value indicating the Engine is in tile-selection mode awaiting player confirmation or cancellation.
- **TargetingContext**: A struct storing all information needed during TARGETING state: the item being used, the owning actor, max range, selector type, and effect reference.
- **TargetSelector**: A class that determines which actors are affected when a Pickable item is used. Selector types include SELF, CLOSEST_MONSTER, SELECTED_MONSTER, WEARER_RANGE, and SELECTED_RANGE.
- **InputState**: A per-frame struct containing KeyState (SDL keycode, character, pressed flag) and MouseState (cellX, cellY, pixelX, pixelY, button flags).
- **Camera**: Converts between world tile coordinates and screen cell coordinates via offset transforms.
- **FOV**: Field of view — the set of tiles currently visible to the player.
- **Main_Loop**: The single update/render cycle in the game's main function that calls `Engine::update()` then `Engine::render()` then `TCODConsole::flush()` each frame.
- **Inventory_Menu**: The modal UI displayed by `chooseFromInventory` that lists inventory items and accepts a selection keypress.
- **pollInput**: The function that drains the SDL3 event queue and populates InputState once per frame.

## Requirements

### Requirement 1: TARGETING Game State

**User Story:** As a developer, I want a TARGETING value in the GameStatus enum, so that the main loop can branch into targeting-specific update and render logic without a secondary loop.

#### Acceptance Criteria

1. THE Engine SHALL include a TARGETING value in the GameStatus enum.
2. WHEN GameStatus is TARGETING, THE Engine SHALL skip normal player movement and monster AI updates during `update()`.
3. WHEN GameStatus is TARGETING, THE Engine SHALL process targeting-specific input (mouse movement, left-click, right-click, ESC) during `update()`.

### Requirement 2: Entering Targeting State

**User Story:** As a player, I want using a targeted scroll to transition the game into targeting mode, so that I can visually select my target on the map.

#### Acceptance Criteria

1. WHEN the player uses an item with SelectorType SELECTED_MONSTER, THE Engine SHALL transition GameStatus to TARGETING.
2. WHEN the player uses an item with SelectorType SELECTED_RANGE, THE Engine SHALL transition GameStatus to TARGETING.
3. WHEN transitioning to TARGETING, THE Engine SHALL store a TargetingContext containing the item pointer, the owning actor pointer, the max range, and the selector type.
4. WHEN transitioning to TARGETING, THE Engine SHALL display a guidance message indicating left-click to confirm and right-click to cancel.

### Requirement 3: Targeting State Rendering

**User Story:** As a player, I want to see which tiles are valid targets while in targeting mode, so that I can make an informed selection.

#### Acceptance Criteria

1. WHILE GameStatus is TARGETING, THE Engine SHALL highlight all tiles that are within the player FOV and within the stored max range (or all FOV tiles if max range is zero).
2. WHILE GameStatus is TARGETING, THE Engine SHALL render a distinct cursor highlight on the tile under the mouse pointer.
3. WHILE GameStatus is TARGETING, THE Engine SHALL update the cursor highlight position each frame based on InputState mouse cellX and cellY converted to world coordinates via Camera.
4. WHILE GameStatus is TARGETING, THE Engine SHALL render the cursor highlight only when the hovered tile is within FOV and within range.

### Requirement 4: Target Confirmation

**User Story:** As a player, I want to left-click a valid tile to confirm my target, so that the item effect is applied and the game advances.

#### Acceptance Criteria

1. WHEN the player left-clicks a tile that is within FOV and within max range while in TARGETING state, THE Engine SHALL apply the stored item effect to the confirmed target location.
2. WHEN SelectorType is SELECTED_MONSTER and the confirmed tile contains a living actor, THE Engine SHALL apply the effect to that actor.
3. WHEN SelectorType is SELECTED_MONSTER and the confirmed tile contains no living actor, THE Engine SHALL not confirm the target and SHALL remain in TARGETING state.
4. WHEN SelectorType is SELECTED_RANGE, THE Engine SHALL apply the effect to all living actors within the TargetSelector range of the confirmed tile.
5. WHEN the effect is successfully applied, THE Engine SHALL remove the consumed item from the player inventory.
6. WHEN targeting is successfully confirmed, THE Engine SHALL transition GameStatus to NEW_TURN.

### Requirement 5: Target Cancellation

**User Story:** As a player, I want to cancel targeting with right-click or ESC, so that I can abort without wasting the item.

#### Acceptance Criteria

1. WHEN the player presses ESC while in TARGETING state, THE Engine SHALL cancel targeting.
2. WHEN the player right-clicks while in TARGETING state, THE Engine SHALL cancel targeting.
3. WHEN targeting is cancelled, THE Engine SHALL transition GameStatus to IDLE.
4. WHEN targeting is cancelled, THE Engine SHALL not consume the item.
5. WHEN targeting is cancelled, THE Engine SHALL clear the stored TargetingContext.

### Requirement 6: Invalid Click Handling

**User Story:** As a player, I want clicking an out-of-range or out-of-FOV tile to do nothing, so that I am not penalized for misclicks.

#### Acceptance Criteria

1. WHEN the player left-clicks a tile that is outside the player FOV while in TARGETING state, THE Engine SHALL ignore the click and remain in TARGETING state.
2. WHEN the player left-clicks a tile that is beyond max range while in TARGETING state, THE Engine SHALL ignore the click and remain in TARGETING state.

### Requirement 7: Remove Blocking pickAtTile

**User Story:** As a developer, I want the blocking `pickAtTile` function removed, so that all rendering goes through the single main loop as required by libtcod 2.2.0 + SDL3.

#### Acceptance Criteria

1. THE Engine SHALL not contain a blocking `pickAtTile` method that calls `TCODConsole::flush()` or `SDL_WaitEvent` in a secondary loop.
2. THE TargetSelector SHALL initiate targeting by setting GameStatus to TARGETING and storing TargetingContext, instead of calling `pickAtTile`.

### Requirement 8: Inventory Menu Refactor

**User Story:** As a developer, I want the inventory menu to use the SDL3 event system via pollInput instead of the deprecated `TCODSystem::waitForEvent`, so that it works correctly with libtcod 2.2.0.

#### Acceptance Criteria

1. THE Inventory_Menu SHALL not call `TCODSystem::waitForEvent`.
2. THE Inventory_Menu SHALL use `pollInput` to read keyboard input for item selection.
3. WHEN the player presses a valid item shortcut key (a-z) while the Inventory_Menu is displayed, THE Inventory_Menu SHALL return the corresponding item.
4. WHEN the player presses ESC or an invalid key while the Inventory_Menu is displayed, THE Inventory_Menu SHALL close without selecting an item.

### Requirement 9: Menu::pick Refactor

**User Story:** As a developer, I want `Menu::pick` to use `pollInput` for its input loop instead of `TCODSystem::waitForEvent`, so that it is compatible with SDL3.

#### Acceptance Criteria

1. THE Menu SHALL not call `TCODSystem::waitForEvent`.
2. THE Menu SHALL use `pollInput` to read keyboard input for menu navigation.
3. WHEN the player presses Up/Down arrows while a Menu is displayed, THE Menu SHALL change the highlighted selection.
4. WHEN the player presses Enter while a Menu is displayed, THE Menu SHALL return the highlighted item code.

### Requirement 10: Mouse Position Continuity

**User Story:** As a player, I want the mouse tile position tracked continuously in the main input handler, so that targeting mode immediately knows the cursor location without additional polling.

#### Acceptance Criteria

1. THE Main_Loop SHALL call `pollInput` exactly once per frame before any update logic.
2. WHILE the game is running, THE InputState SHALL contain the current mouse cell position (cellX, cellY) updated from SDL mouse motion events.
3. WHEN GameStatus transitions to TARGETING, THE Engine SHALL use the existing InputState mouse position for the initial cursor highlight without requiring additional input polling.

### Requirement 11: TargetingContext Lifecycle

**User Story:** As a developer, I want the TargetingContext to be well-defined and cleared after use, so that stale targeting data does not affect subsequent item uses.

#### Acceptance Criteria

1. WHEN TARGETING state completes (confirmed or cancelled), THE Engine SHALL clear the TargetingContext.
2. WHILE GameStatus is not TARGETING, THE Engine SHALL not reference TargetingContext data for any game logic.
3. THE TargetingContext SHALL store the selector type, max range, item pointer, and owning actor pointer.
