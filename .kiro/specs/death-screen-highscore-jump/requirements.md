# Requirements Document

## Introduction

This feature defines a simple, one-way death-screen flow that lets a dead player see where they placed on the high-score table. When the player dies, the Death_Screen shows a brief "You died" indication and a prompt to press Enter. Pressing Enter opens the High_Score_View: if the run earned a place on the leaderboard, the view is scrolled so the player's own entry is visible and highlighted with its position number; if the run did not earn a place, the view shows the Top 10 (or fewer if the board is small, or an empty-state message if the board is empty). PageUp/PageDown scroll the leaderboard, and ESC returns to the main menu.

**Dependency:** This feature depends on the `death-screen-not-showing` bugfix being in place. Today the turn loop overwrites the DEFEAT game status back to IDLE, so `Engine::renderDefeat()` never runs and the death screen does not appear. This spec assumes that bug is fixed — that dying puts the game into a persistent DEFEAT state and the death screen renders. All requirements below are written on that assumption.

This builds on the existing high-score system already on `main`: the death screen is `Engine::renderDefeat()`, shown while `gameStatus == DEFEAT`, rendering a paginated leaderboard through the shared `renderLeaderboard(paginator, highlightIndex, title, footer)` helper. The board is `Engine::highScores_` (a `Leaderboard`), the index of the run's earned entry is `Engine::lastEntryIndex_` (a `std::optional<int>`, unset when the run did not place), and scrolling state lives in `Engine::highScoresPaginator_`.

## Glossary

- **Death_Screen**: The screen active while `Engine::gameStatus == DEFEAT`, rendered by `Engine::renderDefeat()`.
- **Death_Prompt**: The initial content shown on death — a brief "You died" indication and a prompt to press Enter.
- **High_Score_View**: The leaderboard presentation shown after the player presses Enter at the Death_Prompt, rendered via `renderLeaderboard`.
- **Leaderboard**: The in-memory `Engine::highScores_` object; exposes `entries()` (sorted descending), `size()`, and `empty()`.
- **Earned_Entry**: The Leaderboard entry produced by the most recent run, identified by `Engine::lastEntryIndex_`; set only when the run earned and retained a place.
- **Placement_Number**: The 1-based rank/position of an entry in the Leaderboard (entry index + 1).
- **Top_10**: The 10 highest-ranked entries of the Leaderboard.
- **Paginator**: The scrolling state `Engine::highScoresPaginator_` that determines which page of entries is visible.
- **Enter_Key**: The keyboard Return/Enter key.
- **Main_Menu**: The game's main menu, reached via the existing post-death return flow.

## Requirements

### Requirement 1: Death prompt on death

**User Story:** As a player who has just died, I want a clear indication that I died and how to continue, so that I know what to do next.

#### Acceptance Criteria

1. WHEN the game enters the DEFEAT state, THE Death_Screen SHALL display the Death_Prompt.
2. THE Death_Prompt SHALL indicate that the player died.
3. THE Death_Prompt SHALL indicate that pressing the Enter_Key shows the high-score placement.

### Requirement 2: Show ranked placement with Enter

**User Story:** As a player whose run earned a place, I want to press Enter to see my own entry highlighted with its position number, so that I can immediately see my ranking.

#### Acceptance Criteria

1. WHILE the Death_Prompt is shown AND the Earned_Entry is set, WHEN the Enter_Key is pressed, THE Death_Screen SHALL display the High_Score_View scrolled to the page containing the Earned_Entry.
2. WHILE the High_Score_View is shown AND the Earned_Entry is set, THE Death_Screen SHALL visually highlight the Earned_Entry distinctly from other entries.
3. WHILE the High_Score_View is shown AND the Earned_Entry is set, THE Death_Screen SHALL display the Placement_Number of the Earned_Entry.

### Requirement 3: Show Top 10 with Enter when unranked

**User Story:** As a player whose run did not earn a place, I want to press Enter to see the top of the high-score table, so that I can see the best runs even though mine did not qualify.

#### Acceptance Criteria

1. WHILE the Death_Prompt is shown AND the Earned_Entry is unset, WHEN the Enter_Key is pressed, THE Death_Screen SHALL display the High_Score_View positioned at the first page of the Leaderboard.
2. WHILE the High_Score_View is shown AND the Earned_Entry is unset AND the Leaderboard contains 10 or more entries, THE Death_Screen SHALL display the Top_10 as the 10 highest-ranked entries.
3. WHILE the High_Score_View is shown AND the Earned_Entry is unset AND the Leaderboard contains between 1 and 9 entries, THE Death_Screen SHALL display all entries present in the Leaderboard.
4. WHILE the High_Score_View is shown AND the Earned_Entry is unset AND the Leaderboard is empty, THE Death_Screen SHALL display an empty-state message indicating that no runs have been recorded.

### Requirement 4: Scrolling within the high-score view

**User Story:** As a player viewing the high-score table, I want to scroll the table, so that I can browse other entries.

#### Acceptance Criteria

1. WHILE the High_Score_View is shown, WHEN PageDown is pressed, THE Death_Screen SHALL advance the Paginator to the next page, bounded by the last page.
2. WHILE the High_Score_View is shown, WHEN PageUp is pressed, THE Death_Screen SHALL move the Paginator to the previous page, bounded by the first page.

### Requirement 5: Return to the main menu

**User Story:** As a player who has finished reviewing the death screen, I want to press ESC to return to the main menu, so that I can start a new run.

#### Acceptance Criteria

1. WHILE the Death_Screen is shown, WHEN the ESC key is pressed, THE Death_Screen SHALL return control to the Main_Menu, consistent with the existing post-death return flow.
