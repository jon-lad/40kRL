# Requirements Document

## Introduction

Comprehensive UI overhaul for 40kRL to improve information density, readability, and navigation. The rework expands the game window, introduces dedicated sidebar panels for equipment/stats, fixes the overlapping message log, adds skill shortcut indicators, corrects actor render order, and introduces a tabbed menu system with pagination and improved character generation descriptions. The game currently uses an 80×50 console rendered via libtcod 2.2.2 and SDL3.

## Glossary

- **Root_Console**: The top-level libtcod TCOD_Console that represents the entire game window
- **Viewport**: The rectangular region of the Root_Console where the map is drawn, scrolled by the Camera
- **Right_Sidebar**: A vertical panel on the right edge of the Root_Console displaying equipment and ammo status
- **Left_Sidebar**: A vertical panel on the left edge of the Root_Console displaying character stats and passive skills
- **Message_Log**: A scrollable text panel showing recent game messages, positioned in a dedicated sidebar region
- **Skill_Bar**: A horizontal strip at the bottom of the Root_Console showing available skill key bindings
- **HUD_Panel**: The bottom-of-screen panel containing health bars, XP, dungeon level, and mouse-look text
- **Tabbed_Menu**: A full-screen overlay menu with selectable tabs for inventory, skills, and character info
- **Menu_Page**: A single page of menu items; each page displays up to a configurable maximum number of items
- **Actor**: Any entity on the map (player, monsters, items, doors, decorations)
- **AI_Actor**: An Actor that possesses an Ai component (player, enemies, NPCs)
- **Static_Actor**: An Actor without an Ai component (items, decorations, doors)
- **CharGen_Menu**: The character generation overlay presented when starting a new game
- **Renderer**: The Engine::render() function and its helper methods responsible for drawing all visual elements

## Requirements

### Requirement 1: Expanded Window Dimensions

**User Story:** As a player, I want a larger game window, so that sidebars and additional UI panels can be displayed without reducing map visibility.

#### Acceptance Criteria

1. THE Root_Console SHALL have a width of at least 120 columns
2. THE Root_Console SHALL have a height of at least 50 rows
3. WHEN the Root_Console is initialized, THE Viewport SHALL occupy the central region between the Left_Sidebar and Right_Sidebar, spanning from the top of the screen to above the HUD_Panel
4. THE Viewport SHALL maintain a minimum width of 80 columns and a minimum height of 43 rows

### Requirement 2: Right Sidebar — Equipment and Ammo

**User Story:** As a player, I want to see my equipped items and remaining ammo at a glance, so that I can make tactical decisions without opening menus.

#### Acceptance Criteria

1. THE Right_Sidebar SHALL display a list of all currently equipped items grouped by equipment slot
2. WHEN an equipped weapon has ranged capabilities, THE Right_Sidebar SHALL display the current ammo count and maximum clip size for that weapon
3. WHEN no item is equipped in a slot, THE Right_Sidebar SHALL display the slot name followed by "empty"
4. WHEN the player equips or unequips an item, THE Right_Sidebar SHALL update its display on the next render frame
5. THE Right_Sidebar SHALL have a fixed width of at least 20 columns

### Requirement 3: Left Sidebar — Stats and Passive Skills

**User Story:** As a player, I want my character stats and passive skills visible during gameplay, so that I understand my current capabilities without opening the character sheet.

#### Acceptance Criteria

1. THE Left_Sidebar SHALL display the player's core characteristics (WS, BS, S, T, Ag, Int, Per, WP, Fel)
2. THE Left_Sidebar SHALL display a list of the player's acquired passive skills with their current rank
3. WHEN a characteristic value changes, THE Left_Sidebar SHALL reflect the updated value on the next render frame
4. WHEN the player acquires or ranks up a passive skill, THE Left_Sidebar SHALL reflect the change on the next render frame
5. THE Left_Sidebar SHALL have a fixed width of at least 20 columns

### Requirement 4: Message Log Relocation and Fix

**User Story:** As a player, I want the message log displayed in a dedicated panel without overlapping other UI elements, so that I can read combat feedback clearly.

#### Acceptance Criteria

1. THE Message_Log SHALL render in a dedicated rectangular region that does not overlap the Viewport, Left_Sidebar, or Right_Sidebar
2. THE Message_Log SHALL display messages in chronological order with the most recent message at the bottom
3. WHEN the number of messages exceeds the visible line count, THE Message_Log SHALL discard the oldest messages to maintain the visible capacity
4. THE Message_Log SHALL display at least 5 lines of message history simultaneously
5. WHEN a new message is added, THE Message_Log SHALL render the new message on the next frame without visual artifacts from previous frame content

### Requirement 5: Skill Shortcut Bar

**User Story:** As a player, I want to see my available skill key bindings at the bottom of the screen, so that I can quickly activate abilities during combat.

#### Acceptance Criteria

1. THE Skill_Bar SHALL display each bound skill as a key label followed by the skill name
2. THE Skill_Bar SHALL be positioned at the bottom edge of the Root_Console, below the HUD_Panel
3. WHEN the player has no skills bound, THE Skill_Bar SHALL display "No skills bound"
4. WHEN a skill is on cooldown, THE Skill_Bar SHALL visually distinguish the cooldown skill from ready skills using a dimmed colour
5. THE Skill_Bar SHALL display a maximum of 10 skill bindings simultaneously

### Requirement 6: Actor Render Order

**User Story:** As a player, I want monsters and the player character to appear on top of items and decorations, so that important tactical information is never hidden.

#### Acceptance Criteria

1. WHEN multiple Actors occupy the same tile, THE Renderer SHALL draw AI_Actors after Static_Actors so that AI_Actors appear on top
2. THE Renderer SHALL draw the player Actor last among all AI_Actors sharing the same tile
3. WHEN a Static_Actor and an AI_Actor occupy the same tile, THE Renderer SHALL display the AI_Actor glyph and colour at that screen position
4. WHEN an AI_Actor dies and loses its Ai component, THE Renderer SHALL treat the corpse as a Static_Actor for render ordering purposes

### Requirement 7: Tabbed Menu System

**User Story:** As a player, I want a tabbed menu interface for inventory, skills, and character information, so that I can navigate between related panels without closing and reopening menus.

#### Acceptance Criteria

1. WHEN the player opens the menu, THE Tabbed_Menu SHALL display a row of tab labels at the top of the overlay
2. THE Tabbed_Menu SHALL include at minimum tabs for Inventory, Skills, and Character
3. WHEN the player presses a tab navigation key, THE Tabbed_Menu SHALL switch the visible content to the selected tab without closing the overlay
4. THE Tabbed_Menu SHALL highlight the currently active tab label to distinguish it from inactive tabs
5. WHEN the player presses the close key, THE Tabbed_Menu SHALL close the overlay and return to the IDLE game state

### Requirement 8: Menu Pagination

**User Story:** As a player, I want menus to paginate long lists, so that I can browse inventories with more than 25 items.

#### Acceptance Criteria

1. WHEN a menu list contains more items than fit on one Menu_Page, THE Tabbed_Menu SHALL split the list into multiple pages
2. THE Tabbed_Menu SHALL display a page indicator showing the current page number and total page count
3. WHEN the player presses the next-page key, THE Tabbed_Menu SHALL advance to the next Menu_Page if one exists
4. WHEN the player presses the previous-page key, THE Tabbed_Menu SHALL return to the previous Menu_Page if one exists
5. THE Tabbed_Menu SHALL display a maximum of 25 items per Menu_Page
6. WHEN the player is on the first page, THE Tabbed_Menu SHALL not respond to the previous-page key
7. WHEN the player is on the last page, THE Tabbed_Menu SHALL not respond to the next-page key

### Requirement 9: Character Generation Descriptions

**User Story:** As a player, I want descriptions of each character generation choice, so that I understand the gameplay implications beyond raw stat numbers.

#### Acceptance Criteria

1. WHEN a homeworld option is highlighted in the CharGen_Menu, THE CharGen_Menu SHALL display a text description explaining the homeworld's theme and gameplay effects
2. WHEN a career option is highlighted in the CharGen_Menu, THE CharGen_Menu SHALL display a text description explaining the career's role and playstyle
3. THE CharGen_Menu SHALL display descriptions in a dedicated area separate from the stat modifier list
4. WHEN the highlighted option changes, THE CharGen_Menu SHALL update the description text on the next render frame
5. THE CharGen_Menu SHALL source description text from the Lua data files (Homeworlds.lua, Careers.lua)
