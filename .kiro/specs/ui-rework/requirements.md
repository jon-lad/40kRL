# Requirements Document

## Introduction

Comprehensive UI overhaul for 40kRL transforming the current 80×50 single-panel layout into a modern roguelike interface inspired by Caves of Qud and Cogmind. The rework introduces persistent sidebars for character information, a dedicated non-overlapping message log, skill shortcut bar, z-ordered actor rendering, tabbed modal menus with pagination, improved character generation menus with descriptive lore text, and tileset remapping for the new CP437-16×16 tileset.

## Glossary

- **Root_Console**: The top-level libtcod console created by `TCODConsole::initRoot()` that defines the total character-cell window dimensions
- **Map_Viewport**: The central rectangular region of the Root_Console where the game map is rendered through the Camera
- **Right_Sidebar**: A persistent vertical panel on the right side of the Root_Console displaying equipped items, ammo, characteristics, and passive skills
- **Left_Sidebar**: An optional persistent vertical panel on the left side of the Root_Console reserved for additional character information
- **Message_Log**: A dedicated panel for scrollable combat and event messages that does not overlap the Map_Viewport
- **Skill_Bar**: A horizontal bar at the bottom of the screen displaying available skill keybindings
- **Render_Layer**: A numeric priority value controlling the draw order of actors on the map (higher values render on top)
- **Tabbed_Menu**: A modal overlay with multiple switchable content pages (inventory, skills, equipment) accessible via keybindings
- **Menu_Pagination**: A scrollable list within a menu that supports navigating beyond the visible page size
- **CP437**: Code Page 437, the standard IBM PC character encoding used by the 16×16 tileset
- **Glyph_Index**: The integer codepoint used to reference a specific character in the loaded tileset
- **Character_Generation**: The new-game flow where the player selects homeworld and career before entering gameplay
- **Characteristics**: The nine Rogue Trader stats (WS, BS, S, T, Ag, Int, Per, WP, Fel)
- **HUD_Panel**: The bottom section of the Root_Console containing the HP bar, message log, and skill bar

## Requirements

### Requirement 1: Enlarged Root Console

**User Story:** As a player, I want a larger game window, so that there is room for persistent sidebars alongside the map.

#### Acceptance Criteria

1. THE Root_Console SHALL have a width of at least 120 character cells
2. THE Root_Console SHALL have a height of at least 50 character cells
3. WHEN the Root_Console is initialized, THE Engine SHALL load the CP437-16×16 tileset from terminal.png
4. THE Map_Viewport SHALL occupy the central region of the Root_Console between the Left_Sidebar and Right_Sidebar

### Requirement 2: Right Sidebar — Equipment and Stats

**User Story:** As a player, I want a persistent right sidebar showing my equipped items, ammo, and character stats, so that I can see my character status at a glance without opening menus.

#### Acceptance Criteria

1. THE Right_Sidebar SHALL display the name of each currently equipped item grouped by equipment slot (weapon, armour, accessory)
2. WHEN a ranged weapon is equipped, THE Right_Sidebar SHALL display the current ammo count and maximum ammo capacity for that weapon
3. THE Right_Sidebar SHALL display all nine Characteristics with their current numeric values
4. THE Right_Sidebar SHALL display the names of all passive skills the player character possesses
5. WHEN an equipment slot is empty, THE Right_Sidebar SHALL display a placeholder label indicating the slot is unoccupied
6. THE Right_Sidebar SHALL have a fixed width of at least 20 character cells

### Requirement 3: Left Sidebar (Optional Panel)

**User Story:** As a player, I want an optional left sidebar, so that additional character information can be displayed without cluttering the right sidebar.

#### Acceptance Criteria

1. WHERE the Left_Sidebar is enabled, THE Left_Sidebar SHALL display supplementary character information (talents, traits, or combat modifiers)
2. WHERE the Left_Sidebar is enabled, THE Left_Sidebar SHALL have a fixed width of at least 20 character cells
3. WHERE the Left_Sidebar is disabled, THE Map_Viewport SHALL expand leftward to fill the available space

### Requirement 4: Message Log Bug Fix

**User Story:** As a player, I want the message log to display messages without overlaying previous entries, so that I can read all combat and exploration messages clearly.

#### Acceptance Criteria

1. THE Message_Log SHALL render within a sidebar panel without overlapping the Map_Viewport
2. WHEN a shooting or look-mode message is displayed, THE Message_Log SHALL NOT overwrite or overlay previously displayed messages such as item pickup notifications
3. THE Message_Log SHALL display messages in chronological order from oldest at the top to newest at the bottom
4. WHEN a new message is added and the Message_Log is full, THE Message_Log SHALL scroll older messages upward and remove the oldest to make room
5. WHEN multiple messages arrive in a single turn, THE Message_Log SHALL display each message on a separate line without overlapping

### Requirement 5: Skill Shortcut Bar

**User Story:** As a player, I want a bar at the bottom of the screen showing my available skills and their keybindings, so that I know which keys activate which abilities.

#### Acceptance Criteria

1. THE Skill_Bar SHALL display each available combat skill alongside its assigned keybinding
2. THE Skill_Bar SHALL render in a horizontal row within the HUD_Panel below the Map_Viewport
3. WHEN a skill is on cooldown or unavailable, THE Skill_Bar SHALL render that skill entry in a dimmed colour
4. WHEN the player gains or loses a skill, THE Skill_Bar SHALL update to reflect the current skill set

### Requirement 6: Actor Render Order (Z-Ordering)

**User Story:** As a player, I want living actors (monsters, NPCs, player) to render on top of items, doors, and decorations, so that important game entities are always visible.

#### Acceptance Criteria

1. THE Engine SHALL assign a Render_Layer value to each actor based on its category (decoration, item, door, living creature)
2. WHEN rendering the actor list, THE Engine SHALL draw actors in ascending Render_Layer order so that higher-layer actors appear on top of lower-layer actors
3. THE Engine SHALL assign living actors with AI components (player, monsters, NPCs) a higher Render_Layer than items, doors, and decorations
4. WHEN two actors share the same Render_Layer and occupy the same tile, THE Engine SHALL render the actor added most recently on top

### Requirement 7: Tabbed Menu System

**User Story:** As a player, I want a tabbed menu interface for inventory, skills, and equipment, so that I can switch between related screens without closing and reopening menus.

#### Acceptance Criteria

1. WHEN the player opens the inventory menu, THE Tabbed_Menu SHALL display tab labels for Inventory, Skills, and Equipment
2. WHEN the player presses the designated tab-switch key, THE Tabbed_Menu SHALL cycle to the next tab and display its content
3. THE Tabbed_Menu SHALL visually highlight the currently active tab label
4. WHEN switching tabs, THE Tabbed_Menu SHALL preserve the scroll position of each tab independently
5. WHEN the player presses the close key, THE Tabbed_Menu SHALL close the entire overlay and return to gameplay

### Requirement 8: Menu Pagination

**User Story:** As a player, I want menus to support pagination, so that I can hold and browse more than 25 items without being limited by screen space.

#### Acceptance Criteria

1. WHEN a menu list contains more entries than fit on a single page, THE Menu_Pagination SHALL display only the entries that fit within the visible area
2. WHEN the player scrolls past the last visible entry, THE Menu_Pagination SHALL advance to the next page of entries
3. WHEN the player scrolls before the first visible entry, THE Menu_Pagination SHALL return to the previous page of entries
4. THE Menu_Pagination SHALL display a page indicator showing the current page number and total page count
5. THE Menu_Pagination SHALL support lists of arbitrary length without a hardcoded item cap

### Requirement 9: Character Generation Descriptions

**User Story:** As a player, I want the homeworld and career selection menus to show lore descriptions and flavour text, so that I can make informed character choices based on narrative context.

#### Acceptance Criteria

1. WHEN a homeworld option is highlighted, THE Character_Generation menu SHALL display a lore description for that homeworld alongside its stat modifiers
2. WHEN a career option is highlighted, THE Character_Generation menu SHALL display a lore description for that career alongside its stat modifiers
3. THE Character_Generation menu SHALL display homeworld descriptions sourced from the Homeworlds.lua data file
4. THE Character_Generation menu SHALL display career descriptions sourced from the Careers.lua data file
5. WHEN the player navigates between options, THE Character_Generation menu SHALL update the displayed description to match the currently highlighted option

### Requirement 10: CP437 Tileset Remapping

**User Story:** As a developer, I want all glyph references updated to match CP437 codepoints, so that the new 16×16 tileset renders the correct characters.

#### Acceptance Criteria

1. THE Engine SHALL reference all actor glyphs using standard CP437 codepoint values
2. WHEN the tileset is loaded, THE Engine SHALL map CP437 codepoints to the correct tile positions in the 16×16 grid
3. IF a glyph reference uses a non-standard index that does not match CP437, THEN THE Engine SHALL remap that index to the corresponding CP437 codepoint
4. THE Engine SHALL render the '@' symbol (CP437 codepoint 64) for the player actor
5. THE Engine SHALL render wall, floor, door, and stair glyphs using their standard CP437 codepoints
