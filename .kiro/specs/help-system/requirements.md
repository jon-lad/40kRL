# Requirements Document

## Introduction

The game has grown complex enough to require a dedicated help system. This feature provides two complementary access points for control/keybinding reference: an in-game help overlay accessible during gameplay, and a standalone manual file included in the release package. The system is structured for future expansion with game mechanics documentation (action points, reactions, combat resolution, etc.) without requiring architectural changes.

## Glossary

- **Help_Overlay**: The in-game modal overlay screen that displays help content using the existing modal overlay rendering pattern (same as inventory, character sheet, look mode, etc.)
- **Help_Content_Registry**: The internal data structure that stores help sections and entries, serving as the single source of truth for both the in-game overlay and the generated manual file
- **Manual_File**: A plain-text file (MANUAL.txt) included in the release package containing the same help content formatted for offline reading in any text editor
- **Help_Section**: A named grouping of related help entries within the Help_Content_Registry (e.g., "Movement", "Actions", "Overlays")
- **Help_Entry**: A single key-to-description mapping within a Help_Section (e.g., "g" → "Pick up item")
- **Engine**: The global game state machine that owns the actor list, map, camera, and GUI
- **Menu**: The main menu class with MenuItemCode enum used for the title screen and pause screen

## Requirements

### Requirement 1: In-Game Help Overlay Access

**User Story:** As a player, I want to open a help screen during gameplay, so that I can look up controls without leaving the game or consulting an external file.

#### Acceptance Criteria

1. WHEN the player presses the '?' key during the IDLE or PLAYER_TURN game state, THE Engine SHALL transition to a HELP game state and display the Help_Overlay
2. WHEN the player presses ESC while the Help_Overlay is displayed, THE Engine SHALL close the Help_Overlay and return to the previous game state without consuming action points
3. WHEN the player presses '?' while the Help_Overlay is displayed, THE Engine SHALL close the Help_Overlay and return to the previous game state without consuming action points

### Requirement 2: Main Menu Help Access

**User Story:** As a player, I want to access the help screen from the main menu, so that I can learn the controls before starting a game.

#### Acceptance Criteria

1. THE Menu SHALL include a "Help" item in both MAIN and PAUSE display modes
2. WHEN the player selects the "Help" menu item, THE Engine SHALL display the Help_Overlay
3. WHEN the player presses ESC while viewing Help_Overlay from the menu, THE Engine SHALL return to the Menu display

### Requirement 3: Help Content Registry

**User Story:** As a developer, I want help content stored in a structured registry, so that new sections can be added without modifying rendering logic.

#### Acceptance Criteria

1. THE Help_Content_Registry SHALL store help content as an ordered list of Help_Sections
2. THE Help_Content_Registry SHALL allow each Help_Section to contain an ordered list of Help_Entries
3. THE Help_Content_Registry SHALL support adding new Help_Sections without modifying existing rendering or export code
4. WHEN the Help_Content_Registry is queried, THE Help_Content_Registry SHALL return sections in their defined insertion order

### Requirement 4: Controls Help Section Content

**User Story:** As a player, I want to see all available keybindings grouped by function, so that I can quickly find the control I need.

#### Acceptance Criteria

1. THE Help_Content_Registry SHALL contain a "Movement" section listing arrow keys, numpad keys (KP_1 through KP_9), and vi-keys (h, j, k, l, y, u, b, n) with their directional descriptions
2. THE Help_Content_Registry SHALL contain an "Actions" section listing action keys: g (pick up), s (shoot), r (reload), o (open door), a (aim), A (all-out attack), R (run), C (charge), KP_5 (end turn)
3. THE Help_Content_Registry SHALL contain an "Overlays" section listing overlay keys: i (inventory), c (character/equipment), l (look mode), x (advances), m (world map), e (equipment), ? (help)
4. THE Help_Content_Registry SHALL contain a "Navigation" section listing navigation keys: < (ascend stairs), > (descend stairs), ESC (save and return to menu)

### Requirement 5: Help Overlay Rendering

**User Story:** As a player, I want the help screen to be readable and navigable, so that I can find information without difficulty.

#### Acceptance Criteria

1. THE Help_Overlay SHALL render help content as a full-screen overlay using the existing modal overlay pattern
2. THE Help_Overlay SHALL display section headers visually distinct from entry text
3. THE Help_Overlay SHALL display each Help_Entry as a key name followed by its description on a single line
4. WHEN help content exceeds the visible area, THE Help_Overlay SHALL support vertical scrolling via arrow keys or Page Up/Page Down
5. THE Help_Overlay SHALL display navigation instructions at the bottom of the screen (e.g., "ESC: close, Up/Down: scroll")

### Requirement 6: Manual File Generation

**User Story:** As a player, I want a manual file in the release package, so that I can reference controls without running the game.

#### Acceptance Criteria

1. THE Manual_File SHALL be generated from the same Help_Content_Registry used by the in-game overlay
2. THE Manual_File SHALL be written in plain text format as MANUAL.txt with ASCII formatting (no Markdown syntax) so it is readable in any text editor without a renderer
3. THE Manual_File SHALL include a title header and version identifier
4. THE Manual_File SHALL format each Help_Section as a section title line followed by a dashed underline, with entries listed below as indented key-description pairs
5. THE Manual_File SHALL be included in the release package staging step of the release workflow

### Requirement 7: Release Packaging

**User Story:** As a developer, I want the manual automatically included in releases, so that distribution always contains up-to-date documentation.

#### Acceptance Criteria

1. WHEN the release workflow stages files, THE release workflow SHALL copy MANUAL.txt from the repository root into the release directory
2. IF MANUAL.txt does not exist in the repository, THEN THE release workflow SHALL continue without error and log a warning

