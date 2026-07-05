# Requirements Document

## Introduction

This feature improves the inventory and item pickup user experience in two ways: (1) when multiple pickable items exist on the player's tile, the player is presented with a selection menu to choose which item to pick up rather than auto-grabbing the first one found, and (2) the inventory menu filters out equipped items so that only unequipped items are displayed, reducing clutter and preventing confusion between equipped gear and available inventory items.

## Glossary

- **Pickup_Menu**: A selection overlay displayed when the player presses the pickup key ('g') and multiple pickable items exist on the player's tile. Presents each item with a letter shortcut for selection.
- **Inventory_Menu**: The overlay displayed when the player presses 'i' or 'd', showing items available for use or dropping.
- **Engine**: The central game state machine that owns actors, map, camera, GUI, and manages game status transitions.
- **PlayerAi**: The AI component that processes player input and dispatches action keys.
- **Container**: The component that gives an actor an inventory — a list of owned item actors.
- **Equipment**: The component that tracks which items are equipped in the player's four slots (weapon, offhand, head, body).
- **Pickable_Item**: An actor on the map that has a pickable component, making it eligible for pickup.

## Requirements

### Requirement 1: Single Item Auto-Pickup

**User Story:** As a player, I want to automatically pick up an item when only one pickable item is on my tile, so that I do not need an extra selection step for the common case.

#### Acceptance Criteria

1. WHEN the player presses the pickup key and exactly one Pickable_Item exists on the player's tile, THE PlayerAi SHALL pick up that item without displaying the Pickup_Menu.
2. WHEN the player presses the pickup key and exactly one Pickable_Item exists on the player's tile, THE Engine SHALL display a message identifying the picked-up item.
3. IF the Container is full when a single-item pickup is attempted, THEN THE Engine SHALL display a message indicating the inventory is full.

### Requirement 2: Multi-Item Pickup Selection

**User Story:** As a player, I want to choose which item to pick up when multiple items are on my tile, so that I can prioritize what enters my limited inventory.

#### Acceptance Criteria

1. WHEN the player presses the pickup key and more than one Pickable_Item exists on the player's tile, THE Engine SHALL display the Pickup_Menu listing all pickable items on the tile.
2. THE Pickup_Menu SHALL assign a letter shortcut ('a' through 'z') to each listed item for selection.
3. WHEN the player presses a valid letter shortcut in the Pickup_Menu, THE PlayerAi SHALL pick up the corresponding item and close the Pickup_Menu.
4. WHEN the player presses a valid letter shortcut in the Pickup_Menu, THE Engine SHALL display a message identifying the picked-up item.
5. WHEN the player presses Escape while the Pickup_Menu is open, THE Engine SHALL close the Pickup_Menu without picking up any item.
6. IF the Container is full when a multi-item pickup is attempted via the Pickup_Menu, THEN THE Engine SHALL display a message indicating the inventory is full and close the Pickup_Menu.

### Requirement 3: Pickup Menu Rendering

**User Story:** As a player, I want the pickup selection menu to be visually clear and consistent with other game menus, so that I can quickly identify items on the ground.

#### Acceptance Criteria

1. THE Pickup_Menu SHALL display each item's name alongside its assigned letter shortcut.
2. THE Pickup_Menu SHALL be rendered as a framed overlay centered on the screen, consistent with the Inventory_Menu rendering style.
3. WHILE the Pickup_Menu is open, THE Engine SHALL not process movement or other action key inputs.

### Requirement 4: Inventory Menu Hides Equipped Items

**User Story:** As a player, I want my inventory menu to show only unequipped items, so that I can clearly see what is available to use, equip, or drop without confusion from already-equipped gear.

#### Acceptance Criteria

1. WHILE the Inventory_Menu is open, THE Engine SHALL display only items from the Container that are not currently equipped according to the Equipment component.
2. THE Inventory_Menu SHALL assign letter shortcuts sequentially starting from 'a' based on the filtered (unequipped-only) list.
3. WHEN the player selects an item by letter shortcut in the Inventory_Menu, THE Engine SHALL map the shortcut to the correct item in the filtered list.
4. WHILE the Inventory_Menu is open and all Container items are equipped, THE Engine SHALL display an empty inventory list.

### Requirement 5: No Pickup Key When Tile Is Empty

**User Story:** As a player, I want clear feedback when there is nothing to pick up, so that I know the key press was registered but no action was possible.

#### Acceptance Criteria

1. WHEN the player presses the pickup key and no Pickable_Item exists on the player's tile, THE Engine SHALL display a message indicating there is nothing to pick up.
2. WHEN the player presses the pickup key and no Pickable_Item exists on the player's tile, THE Engine SHALL not open the Pickup_Menu.
