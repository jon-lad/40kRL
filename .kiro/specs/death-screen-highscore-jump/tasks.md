# Implementation Plan: Death Screen High-Score Jump

## Overview

This plan implements the two-phase death screen (prompt → high-score view) described in the design. Following the project's TDD workflow, the pure engine-free seam is specified by tests first: unit and RapidCheck property tests for `computeDeathScoreView()` and `placementNumber()` are written before the seam exists (so the test build fails to link until the seam is added — the expected TDD state). The pure helper is then implemented in the existing `Headers/HighScore.hpp` / `Source/HighScore.cpp` pair (already linked into both the game and test projects, so no new source file is needed and `test-project.md` does not apply here).

Engine wiring follows: a new `DeathScreenPhase` enum plus a `deathScreenPhase_` member (reset to `PROMPT` in the run-init path), a modified `DEFEAT` branch of `update()` driving phase transitions and scrolling, a new `renderDeathPrompt()` method, and a modified `renderDefeat()` that branches on phase and uses `placementNumber()` in the title. Per `test-isolation.md`, the engine `update()`/`render()` flow cannot run headless, so it is validated by a manual QA checklist at the final checkpoint. All code is C++17, built with the full-path MSBuild from `msbuild.md`, Debug/x64.

## Tasks

- [ ] 1. Write tests for the pure view-decision seam (TDD — before implementation)
  - [ ]* 1.1 Create `Tests/test_death_score_view.cpp` with unit (example) tests for `placementNumber` and `computeDeathScoreView`
    - Add the new test file to `Tests/40kRL_Tests.vcxproj` (test project only — the seam lives in the already-linked `HighScore.cpp`, so no game-project change)
    - Include `Headers/HighScore.hpp`; use Catch2 v3 test cases tagged `[death-screen-highscore-jump]`
    - `placementNumber`: assert `placementNumber(0) == 1`, `placementNumber(9) == 10`, `placementNumber(99) == 100`
    - `computeDeathScoreView` ranked on page 0: `computeDeathScoreView(3, 50, 10)` → `initialPage == 0`, `highlight == true`
    - `computeDeathScoreView` ranked on later page: `computeDeathScoreView(42, 100, 10)` → `initialPage == 4`, `highlight == true`
    - `computeDeathScoreView` unranked: `computeDeathScoreView(std::nullopt, 50, 10)` → `initialPage == 0`, `highlight == false`
    - Empty board unranked: `computeDeathScoreView(std::nullopt, 0, 10)` → `initialPage == 0`, `highlight == false`
    - Small board (1–9 entries) unranked stays on page 0
    - Degenerate `pageSize == 0` clamps to 1 without crashing (division-by-zero guard)
    - Out-of-range `lastEntryIndex` (e.g. index 50 with `totalItems == 10`) treated as unranked: `initialPage == 0`, `highlight == false`
    - NOTE: expected to FAIL to build/link until task 2.1 adds the seam — this is the intended TDD state
    - _Requirements: 2.1, 2.2, 2.3, 3.1, 3.2, 3.3, 3.4_

  - [ ]* 1.2 Write property test for ranked opens on the containing page
    - **Property 1: Ranked opens on the containing page**
    - Generate `pageSize` via `rc::gen::inRange(1, 50)`, `totalItems` via `inRange(1, 100)`, `v` via `inRange(0, totalItems - 1)` (inclusive bounds per `test-isolation.md`)
    - Assert `initialPage == v / pageSize` AND `initialPage*pageSize <= v < initialPage*pageSize + pageSize`
    - Use `rc::check` (≥100 iterations)
    - **Validates: Requirements 2.1**

  - [ ]* 1.3 Write property test for highlight iff ranked in-range
    - **Property 2: Ranked always highlights; unranked never does**
    - For in-range `v`: `computeDeathScoreView(v, totalItems, pageSize).highlight == true`
    - For `std::nullopt`: `highlight == false`
    - `rc::gen::inRange` inclusive bounds; `rc::check` (≥100 iterations)
    - **Validates: Requirements 2.2, 3.1**

  - [ ]* 1.4 Write property test for unranked opens at the first page
    - **Property 3: Unranked opens at the first page**
    - For all `totalItems` in `inRange(0, 100)` and `pageSize` in `inRange(1, 50)`: `computeDeathScoreView(std::nullopt, totalItems, pageSize).initialPage == 0`
    - `rc::check` (≥100 iterations)
    - **Validates: Requirements 3.1, 3.2, 3.3**

  - [ ]* 1.5 Write property test for initial page always a valid page index
    - **Property 4: Initial page is always a valid page index**
    - For `totalItems` in `inRange(1, 100)`, `pageSize` in `inRange(1, 50)`, optional in-range `v`: assert `0 <= initialPage < ceil(totalItems / pageSize)` (compute totalPages the same way `Paginator::totalPages()` does)
    - `rc::check` (≥100 iterations)
    - **Validates: Requirements 2.1, 4.1, 4.2**

  - [ ]* 1.6 Write property test for placement number is 1-based
    - **Property 5: Placement is 1-based and matches the rendered rank**
    - For `entryIndex` in `inRange(0, 99)`: `placementNumber(entryIndex) == entryIndex + 1`
    - `rc::check` (≥100 iterations)
    - **Validates: Requirements 2.3**

  - [ ]* 1.7 Write property test for determinism / no side effects
    - **Property 6: Determinism and no side effects**
    - For arbitrary in-range inputs, two successive calls to `computeDeathScoreView` with identical arguments return equal `initialPage` and `highlight`
    - `rc::check` (≥100 iterations)
    - **Validates: Requirements 2.1, 3.1**

- [ ] 2. Implement the pure view-decision seam (make tests pass)
  - [ ] 2.1 Add `DeathScoreView`, `computeDeathScoreView`, and `placementNumber` to the high-score module
    - Declare `struct DeathScoreView { int initialPage = 0; bool highlight = false; };` in `Headers/HighScore.hpp`
    - Declare `DeathScoreView computeDeathScoreView(std::optional<int> lastEntryIndex, int totalItems, int pageSize);` and `int placementNumber(int entryIndex);` in `Headers/HighScore.hpp` (include `<optional>`)
    - Implement both in `Source/HighScore.cpp` with NO `engine.*` access (test-isolation): clamp `pageSize` to `std::max(1, pageSize)`; treat `lastEntryIndex` unset OR out of `[0, totalItems)` as unranked (`initialPage = 0`, `highlight = false`); otherwise `initialPage = *lastEntryIndex / pageSize`, `highlight = true`; `placementNumber(i)` returns `i + 1`
    - `HighScore.cpp` is already in both `40kRL.vcxproj` and `Tests/40kRL_Tests.vcxproj` — no project edits needed
    - _Requirements: 2.1, 2.2, 2.3, 3.1, 3.2, 3.3_

- [ ] 3. Checkpoint - build test project and confirm the seam tests pass
  - Build `Tests/40kRL_Tests.vcxproj` (Debug/x64) with the full-path MSBuild, run `.\x64\Debug\40kRL_Tests.exe "[death-screen-highscore-jump]"`
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 4. Add death-screen phase data model to Engine
  - [ ] 4.1 Add `DeathScreenPhase` enum and `deathScreenPhase_` member, reset in the run-init path
    - In `Headers/Engine.hpp`, declare `enum class DeathScreenPhase { PROMPT, SCORES };` and add member `DeathScreenPhase deathScreenPhase_ = DeathScreenPhase::PROMPT;` near `defeatScreenInitialized_`
    - In `Source/Engine.cpp`, set `deathScreenPhase_ = DeathScreenPhase::PROMPT;` in the run-init path alongside the existing `defeatScreenInitialized_` / `lastEntryIndex_` reset so every new run starts at the prompt
    - _Requirements: 1.1_

- [ ] 5. Implement death-screen rendering (prompt + phase-aware leaderboard)
  - [ ] 5.1 Add `renderDeathPrompt()` and branch `renderDefeat()` on phase
    - Declare `void renderDeathPrompt();` (private) in `Headers/Engine.hpp`; implement in `Source/Engine.cpp` drawing a framed "You Died" message stating the player died, that Enter shows the high-score placement, and that ESC returns to the menu
    - In `renderDefeat()`: keep the lazy paginator init (`pageSize`/`totalItems`); if phase is `PROMPT` call `renderDeathPrompt()` and return; in `SCORES` phase set `totalItems = highScores_.size()`, build the title (`"-- You placed #" + std::to_string(placementNumber(*lastEntryIndex_)) + " --"` when ranked, else `"-- High Scores --"`), and call `renderLeaderboard(highScoresPaginator_, lastEntryIndex_, title, "PgUp/PgDn: scroll   ESC: menu")` so highlight only occurs when ranked
    - Remove the old inline `*lastEntryIndex_ / pageSize` page computation from the lazy-init block (the page is now set at transition time in task 6.1)
    - _Requirements: 1.1, 1.2, 1.3, 2.2, 2.3, 3.2, 3.3, 3.4_

- [ ] 6. Wire death-screen input transitions into the update loop
  - [ ] 6.1 Modify the DEFEAT branch of `Engine::update()` for phase transitions and scrolling
    - In `Source/Engine.cpp` DEFEAT branch: in `PROMPT` phase, on `SDLK_RETURN` set `highScoresPaginator_.totalItems = highScores_.size()`, call `computeDeathScoreView(lastEntryIndex_, highScoresPaginator_.totalItems, highScoresPaginator_.pageSize)`, set `currentPage = v.initialPage`, and switch phase to `SCORES`; on `SDLK_ESCAPE` call `load()` and return
    - In `SCORES` phase: `SDLK_PAGEDOWN` → `highScoresPaginator_.nextPage()`; `SDLK_PAGEUP` → `highScoresPaginator_.prevPage()`; `SDLK_ESCAPE` → `load()` and return; ignore other keys
    - _Requirements: 2.1, 3.1, 4.1, 4.2, 5.1_

- [ ] 7. Checkpoint - build both projects, re-run tests, and manual QA
  - Build `40kRL.vcxproj` and `Tests/40kRL_Tests.vcxproj` (Debug/x64) with the full-path MSBuild
  - Re-run `.\x64\Debug\40kRL_Tests.exe "[death-screen-highscore-jump]"` (seam tests still pass) plus the broader suite `.\x64\Debug\40kRL_Tests.exe`
  - Manual QA (the Engine update/render flow cannot run headless per `test-isolation.md`):
    - Ranked run: die → prompt shows "You died" + Enter/ESC hints → Enter → leaderboard opens on the earned entry's page, that entry highlighted, title shows `#N` → PageUp/PageDown scroll (bounded) → ESC → main menu
    - Unranked run: die → prompt → Enter → leaderboard opens at Top 10 (page 0), no highlight → ESC → main menu
    - Empty board (fresh install): die → prompt → Enter → "No runs recorded yet." → ESC → main menu
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional test sub-tasks and can be skipped for a faster MVP, but per the project's TDD workflow they are written first and drive the seam implementation.
- Task 1 tests are expected to fail to build/link until task 2.1 adds the pure seam — this is the intended TDD "red" state.
- The pure seam lives in `Headers/HighScore.hpp` / `Source/HighScore.cpp`, which are already registered in both `40kRL.vcxproj` and `Tests/40kRL_Tests.vcxproj`; no new source file is added, so `test-project.md`'s dual-registration rule is satisfied trivially. Only the new test file `Tests/test_death_score_view.cpp` is added, to the test project only.
- Property tests use RapidCheck `rc::check` with ≥100 iterations and inclusive `rc::gen::inRange` bounds (`inRange(0, N - 1)` for indices) per `test-isolation.md`.
- The Engine `update()`/`render()` flow is validated by the manual QA checklist because it depends on SDL/libtcod/Gui that are absent in the headless test binary.
- All builds use the full MSBuild path from `msbuild.md`, Debug/x64.

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1", "1.2", "1.3", "1.4", "1.5", "1.6", "1.7"] },
    { "id": 1, "tasks": ["2.1"] },
    { "id": 2, "tasks": ["4.1"] },
    { "id": 3, "tasks": ["5.1"] },
    { "id": 4, "tasks": ["6.1"] }
  ]
}
```
