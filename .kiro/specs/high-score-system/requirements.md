# Requirements Document

## Introduction

This feature adds a high score system to 40kRL. When a run ends — primarily by player death (the `DEFEAT` game state set by `PlayerDestructible::die()`) — the system records the outcome of that run as a score entry and persists a leaderboard of the best runs across game sessions. The player can review the leaderboard from the main menu, and the death screen highlights the newly recorded entry when it earns a place on the board.

Scores are ranked by a combined metric: total experience earned during the run is the primary score, with the deepest dungeon level reached serving as a tiebreaker. This ranking metric is a roguelike-specific mechanic with no Warhammer 40,000 Rogue Trader tabletop equivalent, so it is defined here as a project convention rather than cited from the `Reference/` rules. The formula is intentionally simple so it can evolve.

The leaderboard persists to a separate file (`highscores.dat`) that is independent of the main save file (`game.sav`). High scores survive starting a new game, deleting a save, and losing a run. The core scoring, ranking, and insertion logic is engine-independent so it can be unit- and property-tested without initializing the Engine, and the leaderboard serialization is likewise testable in isolation.

## Glossary

- **High_Score_System**: The component responsible for recording run outcomes, ranking entries, maintaining the leaderboard, and coordinating persistence.
- **Score_Entry**: A single recorded run outcome, containing the character name, career name, homeworld name, final rank, total experience earned, deepest dungeon level reached, outcome/cause description, and a date.
- **Leaderboard**: The ordered collection of Score_Entry records, sorted by score descending and truncated to the configured maximum size.
- **Score_Value**: The numeric ranking key derived from a Score_Entry: total experience earned as the primary component, deepest dungeon level reached as the tiebreaker.
- **Total_Experience**: The `xpPool` value from the player's `CareerProgression`, representing all experience earned during the run.
- **Deepest_Level**: The greatest `Engine::dungeonLevel` value the player reached during the run.
- **Outcome**: A human-readable phrase describing the manner in which the run ended and how the character died (e.g., "Slain by <enemy name>", or "Slain" when the cause is unknown), including the cause-of-death actor name when available.
- **Leaderboard_Capacity**: The maximum number of Score_Entry records retained on the Leaderboard. Default value is 100.
- **High_Score_File**: The persistence file `highscores.dat`, stored separately from the main save file `game.sav`.
- **DEFEAT**: The `Engine::GameStatus` value set by `PlayerDestructible::die()` when the player character is killed.
- **Menu**: The existing main menu system (`Source/Menu.*`, `Source/Persistent.cpp`) presenting items such as New Game and Continue.

## Requirements

### Requirement 1: Score Entry Data Model

**User Story:** As a developer, I want a well-defined data model for a recorded run, so that entries can be created, ranked, and serialized uniformly.

#### Acceptance Criteria

1. THE High_Score_System SHALL represent a Score_Entry containing the character name, career name, homeworld name, final rank, total experience earned, deepest dungeon level reached, outcome description, and a run date
2. THE High_Score_System SHALL store the total experience earned as a non-negative integer sourced from the player's `CareerProgression` `xpPool`
3. THE High_Score_System SHALL store the deepest dungeon level reached as a positive integer sourced from `Engine::dungeonLevel`
4. THE High_Score_System SHALL store the run date as a text value in a fixed, human-readable format
5. THE High_Score_System SHALL store the outcome description as a human-readable phrase describing the manner of death
6. WHERE the cause-of-death actor name is available at run end, THE High_Score_System SHALL format the outcome description as "Slain by " followed by the cause-of-death actor name
7. WHERE the cause-of-death actor name is unavailable at run end, THE High_Score_System SHALL record the outcome description as "Slain"

### Requirement 2: Score Ranking

**User Story:** As a player, I want runs ranked by a consistent metric, so that the leaderboard reflects my best performances.

#### Acceptance Criteria

1. THE High_Score_System SHALL compute a Score_Value for each Score_Entry using total experience earned as the primary component and deepest dungeon level reached as the tiebreaker
2. WHEN two Score_Entry records are compared, THE High_Score_System SHALL rank the entry with greater total experience earned higher
3. WHEN two Score_Entry records have equal total experience earned, THE High_Score_System SHALL rank the entry with the greater deepest dungeon level reached higher
4. WHEN two Score_Entry records have equal total experience earned and equal deepest dungeon level reached, THE High_Score_System SHALL preserve a deterministic ordering between the two records
5. THE High_Score_System SHALL implement the Score_Value computation and comparison as engine-independent pure functions that do not access `engine.gui`, `engine.map`, or `engine.player`

### Requirement 3: Leaderboard Insertion and Capacity

**User Story:** As a player, I want only the best runs kept on the leaderboard, so that the board stays meaningful and bounded.

#### Acceptance Criteria

1. WHEN a Score_Entry is inserted into the Leaderboard, THE High_Score_System SHALL position the entry so that the Leaderboard remains sorted by Score_Value descending
2. WHILE the Leaderboard contains fewer than Leaderboard_Capacity entries, THE High_Score_System SHALL retain every inserted Score_Entry
3. WHEN inserting a Score_Entry would cause the Leaderboard to exceed Leaderboard_Capacity, THE High_Score_System SHALL discard the lowest-ranked Score_Entry so that the Leaderboard size equals Leaderboard_Capacity
4. IF a Score_Entry ranks below every retained entry and the Leaderboard is already at Leaderboard_Capacity, THEN THE High_Score_System SHALL leave the Leaderboard unchanged
5. THE High_Score_System SHALL report whether a newly inserted Score_Entry earned a place on the Leaderboard
6. THE High_Score_System SHALL use a default Leaderboard_Capacity of 100
7. THE High_Score_System SHALL implement Leaderboard insertion, sorting, and truncation as engine-independent pure functions that do not access `engine.gui`, `engine.map`, or `engine.player`

### Requirement 4: Recording a Run Outcome on Player Death

**User Story:** As a player, I want my run recorded when my character dies, so that my performance is preserved on the leaderboard.

#### Acceptance Criteria

1. WHEN the game transitions into the DEFEAT state via player death, THE High_Score_System SHALL construct a Score_Entry from the player's current character name, career name, homeworld name, final rank, total experience earned, and deepest dungeon level reached
2. WHEN a Score_Entry is constructed on player death, THE High_Score_System SHALL insert the Score_Entry into the Leaderboard
3. WHEN a Score_Entry is inserted on player death, THE High_Score_System SHALL persist the updated Leaderboard to the High_Score_File
4. THE High_Score_System SHALL record at most one Score_Entry per completed run

### Requirement 5: Extensibility for Future Outcomes

**User Story:** As a developer, I want the recording mechanism to accommodate future run outcomes, so that a victory outcome can be added later without redesigning the system.

#### Acceptance Criteria

1. THE High_Score_System SHALL represent the Outcome as data on the Score_Entry rather than assuming a single fixed cause
2. WHERE a non-death run outcome is provided, THE High_Score_System SHALL record the Score_Entry using the same ranking and insertion rules applied to death outcomes

### Requirement 6: Persistence to a Separate File

**User Story:** As a player, I want the leaderboard to survive new games and save deletion, so that my history of runs is not lost.

#### Acceptance Criteria

1. THE High_Score_System SHALL persist the Leaderboard to the High_Score_File `highscores.dat`, separate from the main save file `game.sav`
2. WHEN a new game is started via the Menu New Game flow, THE High_Score_System SHALL retain the existing High_Score_File contents unchanged
3. WHEN the main save file is deleted, THE High_Score_System SHALL retain the existing High_Score_File contents unchanged
4. WHEN a Score_Entry is added to the Leaderboard, THE High_Score_System SHALL write the updated Leaderboard to the High_Score_File
5. WHEN the game starts, THE High_Score_System SHALL load the Leaderboard from the High_Score_File
6. WHEN the high scores screen is opened, THE High_Score_System SHALL present the Leaderboard loaded from the High_Score_File
7. THE High_Score_System SHALL implement Leaderboard serialization and deserialization as logic that is testable in isolation from the Engine

### Requirement 7: Graceful Handling of Missing or Invalid Files

**User Story:** As a player, I want the game to behave correctly on a first run or after file corruption, so that a bad high score file never blocks play.

#### Acceptance Criteria

1. IF the High_Score_File does not exist when the Leaderboard is loaded, THEN THE High_Score_System SHALL initialize an empty Leaderboard
2. IF the High_Score_File is empty when the Leaderboard is loaded, THEN THE High_Score_System SHALL initialize an empty Leaderboard
3. IF the High_Score_File cannot be read or parsed when the Leaderboard is loaded, THEN THE High_Score_System SHALL initialize an empty Leaderboard and continue running
4. WHEN a Score_Entry is added after the Leaderboard was initialized empty due to a missing or invalid file, THE High_Score_System SHALL write a valid High_Score_File containing the new Leaderboard

### Requirement 8: Round-Trip Persistence Integrity

**User Story:** As a developer, I want the leaderboard to reload exactly as it was saved, so that persisted scores are trustworthy.

#### Acceptance Criteria

1. WHEN a Leaderboard is written to the High_Score_File and then loaded, THE High_Score_System SHALL produce a Leaderboard equivalent to the one written, including entry count, entry field values, and ordering
2. THE High_Score_System SHALL serialize every field of each Score_Entry so that no field is lost across a write-then-load cycle
3. THE High_Score_System SHALL preserve the descending Score_Value ordering of the Leaderboard across a write-then-load cycle

### Requirement 9: Viewing the Leaderboard from the Main Menu

**User Story:** As a player, I want a High Scores option in the main menu, so that I can review past runs at any time.

#### Acceptance Criteria

1. THE High_Score_System SHALL present a "High Scores" entry in the Menu
2. WHEN the player selects the "High Scores" menu entry, THE High_Score_System SHALL display the Leaderboard entries in ranked order
3. WHEN the Leaderboard is displayed, THE High_Score_System SHALL show, for each entry, the character name, homeworld name, total experience earned, deepest dungeon level reached, and run date
4. WHEN the Leaderboard is displayed, THE High_Score_System SHALL show, for each entry, a human-readable description combining the career name, final rank, and cause of death
5. WHILE the number of Leaderboard entries exceeds the visible area, THE High_Score_System SHALL allow the player to scroll through the full list using PageUp and PageDown, consistent with the paginator used by `TabbedMenuState` in `Source/Engine.cpp`
6. WHILE the number of Leaderboard entries exceeds the visible area, THE High_Score_System SHALL display a page indicator showing that more entries exist beyond the visible area, consistent with the existing paginator page indicator
7. WHEN the player scrolls the Leaderboard, THE High_Score_System SHALL bound scrolling so that the view cannot move before the first entry or past the last entry
8. WHILE the Leaderboard is empty, THE High_Score_System SHALL display a message indicating that no runs have been recorded
9. WHEN the player dismisses the high scores screen, THE High_Score_System SHALL return to the Menu

### Requirement 10: Highlighting the New Entry on the Death Screen

**User Story:** As a player, I want the death screen to show where my run placed, so that I get immediate feedback on my performance.

#### Acceptance Criteria

1. WHILE the game is in the DEFEAT state after recording the run, THE High_Score_System SHALL display the Leaderboard on the death screen
2. IF the newly recorded Score_Entry earned a place on the Leaderboard, THEN THE High_Score_System SHALL visually distinguish the newly recorded Score_Entry from the other entries
3. IF the newly recorded Score_Entry did not earn a place on the Leaderboard, THEN THE High_Score_System SHALL display the Leaderboard without highlighting any entry
4. WHILE the number of Leaderboard entries exceeds the visible area of the death screen, THE High_Score_System SHALL allow the player to scroll through the full list using PageUp and PageDown, consistent with the paginator used by `TabbedMenuState` in `Source/Engine.cpp`
5. WHEN the death screen is opened and the newly recorded Score_Entry earned a place on the Leaderboard, THE High_Score_System SHALL scroll the newly recorded Score_Entry into the visible area
