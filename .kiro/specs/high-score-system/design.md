# Design Document

## Overview

The high-score system records the outcome of each completed run (primarily player death) as a `ScoreEntry`, ranks entries by a combined metric (total experience earned as the primary key, deepest dungeon level reached as the tiebreaker), and maintains a `Leaderboard` capped at 100 entries sorted descending. The leaderboard persists to a dedicated file `highscores.dat`, wholly independent of the main save file `game.sav`, so scores survive new games, save deletion, and lost runs. Players can review the leaderboard from a new "High Scores" main-menu entry, and the death screen highlights the entry the just-finished run earned.

The central design constraint (from the `test-isolation.md` steering rule and Requirements 2.5, 3.7, 6.7) is that **all scoring, ranking, insertion, truncation, and serialization logic must be engine-independent pure functions/types** that never touch `engine.gui`, `engine.map`, or `engine.player`. This makes the entire behavioral core unit- and property-testable without initializing the `Engine`. The Engine layer is only responsible for (a) gathering values from live game objects at death, (b) invoking the pure core, (c) file I/O, and (d) rendering.

This separation mirrors the existing pattern in the codebase: `Paginator` is a pure struct with no engine access that `TabbedMenuState` composes and `Engine` renders. The high-score core follows the same shape.

### Grounding in the existing codebase

- **Death trigger**: `PlayerDestructible::die()` (`Source/Destructible.cpp`) sets `engine.gameStatus = Engine::DEFEAT`. This is the single hook point for recording a run outcome.
- **XP source**: `CareerProgression::xpPool` (`Headers/CareerProgression.hpp`) — total XP earned. Accessed via `player->career->xpPool`.
- **Depth source**: `Engine::dungeonLevel` (`Headers/Engine.hpp`).
- **Character identity**: `CareerProgression::homeworldName`, `CareerProgression::careerName`, `CareerProgression::currentRank`; the rank title comes from the matching `CareerTemplate::ranks[].rankTitle`. The character's display name is `player->name` (an `Actor` field).
- **Menu**: `Menu` / `Menu::MenuItemCode` in `Headers/Gui.hpp`; the menu loop lives in `Engine::load()` (`Source/Persistent.cpp`).
- **Pagination**: `Paginator` (`Headers/Paginator.hpp`) drives PageUp/PageDown scrolling and the "Page X/Y" indicator, exactly as `Engine::renderTabbedMenu()` uses it (`Source/Engine.cpp`).
- **Persistence style**: existing save/load uses `TCODZip`. The high-score file uses the same `TCODZip` archive mechanism for consistency and to avoid introducing a new serialization dependency, but writes to `highscores.dat` and is completely decoupled from the `game.sav` stream.

## Architecture

```mermaid
flowchart TD
    subgraph Engine Layer (engine-dependent)
        Die["PlayerDestructible::die()<br/>sets DEFEAT"]
        Rec["Engine::recordRunOutcome()<br/>gather name/career/homeworld/rank/xp/depth/cause"]
        Store["HighScoreStore<br/>(file I/O for highscores.dat)"]
        MenuLoop["Engine::load() menu loop<br/>HIGH_SCORES item"]
        ViewUI["Engine::beginHighScores / updateHighScores / renderHighScores"]
        DeathUI["Engine::renderDefeat()<br/>death screen w/ highlight"]
    end

    subgraph Pure Core (engine-independent, testable)
        Entry["ScoreEntry (struct)"]
        Board["Leaderboard<br/>insert / sort / truncate / earnedPlace"]
        Cmp["scoreCompare() / scoreLess()<br/>pure ranking functions"]
        Ser["serializeLeaderboard() / deserializeLeaderboard()<br/>TCODZip round-trip"]
    end

    Die --> Rec
    Rec -->|build ScoreEntry| Entry
    Rec -->|insert| Board
    Board --> Cmp
    Rec -->|persist| Store
    Store --> Ser
    MenuLoop --> ViewUI
    ViewUI --> Store
    DeathUI --> Board
    Store -->|load on startup| Board
```

### Layering rules

| Concern | Layer | Engine access allowed? |
|---|---|---|
| `ScoreEntry` data model | Pure core | No |
| Score comparison / ranking | Pure core | No |
| Leaderboard insert / sort / truncate | Pure core | No |
| Serialize / deserialize leaderboard | Pure core | No (operates on `TCODZip`, not `engine.*`) |
| Read `highscores.dat` from disk | Engine layer (`HighScoreStore`) | Filesystem only, no `engine.*` |
| Gather live values at death | Engine layer | Yes (`player`, `dungeonLevel`) |
| Render leaderboard / death screen | Engine layer | Yes (`gui`, consoles) |

The pure core is compiled into `HighScore.cpp` and added to **both** `40kRL.vcxproj` and `Tests/40kRL_Tests.vcxproj` (per `test-project.md`). Because it never dereferences the global `engine`, tests can construct `ScoreEntry` and `Leaderboard` directly without initializing SDL/libtcod/Gui/Map (per `test-isolation.md`).

## Components and Interfaces

### New files

- `Headers/HighScore.hpp` — `ScoreEntry`, `Leaderboard`, ranking + serialization free functions (pure core).
- `Source/HighScore.cpp` — implementation of the pure core.
- `Headers/HighScoreStore.hpp` / `Source/HighScoreStore.cpp` — thin engine-layer wrapper for file I/O against `highscores.dat`.
- Engine methods added to `Engine` (in `Headers/Engine.hpp`, implemented in `Source/Engine.cpp`): `recordRunOutcome(...)`, `beginHighScores()`, `updateHighScores()`, `renderHighScores()`, `renderDefeat()`.
- A new `GameStatus` value `HIGH_SCORES` and a new `Menu::MenuItemCode::HIGH_SCORES`.

### Pure core: `HighScore.hpp`

```cpp
#pragma once
#include <string>
#include <vector>

class TCODZip; // forward declaration; serialization uses TCODZip but not engine.*

// A single recorded run outcome. All fields are plain data (Requirement 1, 5.1).
struct ScoreEntry {
    std::string characterName;   // player->name
    std::string careerName;      // CareerProgression::careerName
    std::string homeworldName;   // CareerProgression::homeworldName
    std::string rankTitle;       // resolved from CareerTemplate ranks (final rank)
    int         totalXp = 0;     // CareerProgression::xpPool  (>= 0)   (Req 1.2)
    int         deepestLevel = 1;// max Engine::dungeonLevel reached (>= 1) (Req 1.3)
    std::string outcome;         // "Slain by <name>" or "Slain" (Req 1.5-1.7, 5.1)
    std::string date;            // fixed human-readable format (Req 1.4)
};

// Ranking (Requirement 2). Pure functions — no engine access (Req 2.5).
// Returns negative if a ranks below b, 0 if equal rank, positive if a ranks above b.
int scoreCompare(const ScoreEntry& a, const ScoreEntry& b);

// Strict-weak "a ranks strictly below b" ordering used for descending sort.
bool scoreRanksHigher(const ScoreEntry& a, const ScoreEntry& b);

// The leaderboard: ordered, capacity-bounded collection (Requirement 3).
class Leaderboard {
public:
    static constexpr int DEFAULT_CAPACITY = 100; // Req 3.6

    explicit Leaderboard(int capacity = DEFAULT_CAPACITY);

    // Inserts entry at the correct sorted (descending) position, then truncates
    // to capacity. Returns true iff the entry earned and retained a place. (Req 3.1-3.5)
    bool insert(const ScoreEntry& entry);

    const std::vector<ScoreEntry>& entries() const { return entries_; }
    int  size()     const { return static_cast<int>(entries_.size()); }
    int  capacity() const { return capacity_; }
    bool empty()    const { return entries_.empty(); }

private:
    std::vector<ScoreEntry> entries_; // invariant: sorted descending, size <= capacity_
    int capacity_;
};

// Serialization (Requirement 6.7, 8). Pure — operates on a TCODZip archive only.
void serializeLeaderboard(const Leaderboard& board, TCODZip& zip);
Leaderboard deserializeLeaderboard(TCODZip& zip);
```

**Deterministic tiebreak (Req 2.4).** When two entries have equal `totalXp` and equal `deepestLevel`, `scoreCompare` falls back to a lexicographic comparison of the remaining fields in a fixed order (`characterName`, then `date`, then `outcome`, etc.). This guarantees a total, deterministic order so that sorting and round-tripping are stable. `scoreRanksHigher(a,b)` is defined as `scoreCompare(a,b) > 0`, giving a strict weak ordering suitable for `std::stable_sort` / `std::upper_bound`.

**Insertion (Req 3).** `insert` finds the position via the descending order, inserts, and if `size() > capacity_` erases the last (lowest-ranked) element. It returns `true` iff the inserted entry survives truncation (i.e., it is still present after truncation). This directly answers "did this run earn a place?" (Req 3.5) and covers the "board already full and entry ranks below all" case by returning `false` and leaving the board unchanged (Req 3.4).

### Engine layer: `HighScoreStore`

```cpp
#pragma once
#include "HighScore.hpp"
#include <string>

// Engine-layer file wrapper. Touches the filesystem only — never engine.gui/map/player.
class HighScoreStore {
public:
    static constexpr const char* FILE_NAME = "highscores.dat"; // Req 6.1

    // Loads the leaderboard from FILE_NAME. On missing/empty/unreadable/corrupt
    // file, returns an empty Leaderboard (Requirement 7.1-7.3). Never throws.
    static Leaderboard load();

    // Writes the leaderboard to FILE_NAME (Requirement 6.4).
    static void save(const Leaderboard& board);
};
```

`load()` checks `std::filesystem::exists` / file size, wraps `TCODZip::loadFromFile` + `deserializeLeaderboard` in a try/catch, and on any failure returns `Leaderboard{}`. This isolates all "graceful degradation" behavior (Requirement 7) at the I/O boundary while keeping `deserializeLeaderboard` itself pure and total.

### Engine integration points

1. **Startup load (Req 6.5).** In `Engine::init()` (and once at program start), call `highScores_ = HighScoreStore::load();` storing the board in a new `Engine` member `Leaderboard highScores_;` plus `std::optional<int> lastEntryIndex_;` (the index of the entry the most recent run earned, for death-screen highlighting).

2. **Recording on death (Req 4).** `PlayerDestructible::die()` continues to set `DEFEAT`. Immediately after setting `DEFEAT`, it calls `engine.recordRunOutcome(cause)` where `cause` is the killing actor's name when known (see below). `recordRunOutcome`:
   - builds a `ScoreEntry` from `player->name`, `player->career->careerName`, `player->career->homeworldName`, resolved final `rankTitle`, `player->career->xpPool`, `engine.dungeonLevel`, formatted `outcome`, and the current date;
   - calls `highScores_.insert(entry)`, storing whether it earned a place and, if so, its index;
   - calls `HighScoreStore::save(highScores_)` (Req 4.3, 6.4).
   A `bool runRecorded_` guard on the Engine ensures **at most one entry per run** (Req 4.4): `recordRunOutcome` early-returns if `runRecorded_` is already set, and it is reset when a new game starts.

3. **Cause of death (Req 1.6, 1.7).** The codebase has no existing killer tracking. Design adds a minimal hook: `PlayerDestructible::die(Actor* owner)` already receives the dying actor; the attacker context is available at the call site in `Source/Attacker.cpp`. We thread the attacker's name to death recording via a new `Engine` field `std::string pendingCauseOfDeath_` that the melee/ranged pipeline sets to the attacker's `name` just before it calls `target->destructible->die(target)` on the player. `recordRunOutcome` formats `outcome` as `"Slain by " + pendingCauseOfDeath_` when non-empty, else `"Slain"`. Non-combat deaths (e.g., Burning tick) leave it empty, yielding `"Slain"`.

4. **Menu entry (Req 9.1).** In `Engine::load()` (`Source/Persistent.cpp`), add `menu.addItem(Menu::MenuItemCode::HIGH_SCORES, "High Scores")` between "Continue" and "Help". Handle the choice by calling `engine.beginHighScores()` and returning (mirroring the existing `HELP` handling).

5. **Viewing (Req 9).** `beginHighScores()` reloads the board via `HighScoreStore::load()` (Req 6.6), initializes a `Paginator`, and sets `gameStatus = HIGH_SCORES`. `updateHighScores()` handles PageUp/PageDown (via `Paginator::nextPage/prevPage`, which are already clamped — Req 9.7) and ESC/Enter to return to the menu (Req 9.9). `renderHighScores()` draws each visible entry's character name, homeworld, total XP, deepest level, date (Req 9.3), plus a combined career/rank/cause description line (Req 9.4), the `Paginator::indicator()` when more than one page (Req 9.6), and an empty-state message when the board is empty (Req 9.8).

6. **Death screen (Req 10).** A `renderDefeat()` method renders the leaderboard while `gameStatus == DEFEAT`. When `lastEntryIndex_` is set (the run earned a place), it visually distinguishes that row (e.g., highlighted color) — Req 10.2 — and sets the `Paginator`'s current page so that index is visible (Req 10.5). When `lastEntryIndex_` is unset, no row is highlighted (Req 10.3). PageUp/PageDown scroll the list on the death screen too (Req 10.4).

### Menu / GameStatus additions

- `Menu::MenuItemCode` gains `HIGH_SCORES`. Because `MenuItemCode` is serialized nowhere (it is a transient UI selection), adding a value is non-breaking.
- `Engine::GameStatus` gains `HIGH_SCORES`. `update()` and `render()` get early-return branches for it, following the exact pattern used by `HELP`, `TABBED_MENU`, etc.

## Data Models

### ScoreEntry

| Field | Type | Source | Constraints |
|---|---|---|---|
| `characterName` | `std::string` | `player->name` | may be empty |
| `careerName` | `std::string` | `CareerProgression::careerName` | — |
| `homeworldName` | `std::string` | `CareerProgression::homeworldName` | — |
| `rankTitle` | `std::string` | `CareerTemplate::ranks[currentRank].rankTitle` | fallback to `"Rank N"` if not found |
| `totalXp` | `int` | `CareerProgression::xpPool` | `>= 0` (Req 1.2) |
| `deepestLevel` | `int` | `Engine::dungeonLevel` | `>= 1` (Req 1.3) |
| `outcome` | `std::string` | formatted at death | `"Slain by X"` / `"Slain"` |
| `date` | `std::string` | system clock at death | fixed format `YYYY-MM-DD HH:MM` (Req 1.4) |

### Leaderboard

- `entries_`: `std::vector<ScoreEntry>` maintained sorted by `scoreCompare` descending.
- `capacity_`: default `100` (Req 3.6).
- Invariants: `entries_.size() <= capacity_`; for all `i < j`, `scoreCompare(entries_[i], entries_[j]) >= 0` (descending).

### Serialization format (`highscores.dat`)

Written via `TCODZip` for parity with the existing save system, but as its own file and stream:

```
[int]    HIGHSCORE_SENTINEL (0x48534452 "HSDR") — format marker
[int]    formatVersion (1)
[int]    entryCount
repeat entryCount times:
    [string] characterName
    [string] careerName
    [string] homeworldName
    [string] rankTitle
    [int]    totalXp
    [int]    deepestLevel
    [string] outcome
    [string] date
```

`deserializeLeaderboard` reads the sentinel first; if it does not match (empty/foreign/corrupt archive), it returns an empty `Leaderboard` (the `HighScoreStore::load` try/catch is the outer safety net for unreadable files). Every `ScoreEntry` field is serialized, so no field is lost across a round trip (Req 8.2).

## Correctness Properties

_A property is a characteristic or behavior that should hold true across all valid executions of a system — essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees._

The pure core (`ScoreEntry`, `Leaderboard`, ranking, serialization) is where universal properties apply. The acceptance criteria collapse into four distinct properties: a ranking total-order property, a leaderboard insertion-invariant property, a serialization round-trip property, and a deserialization robustness property. All UI, file-wiring, and architectural criteria are covered by example/integration/smoke tests (see Testing Strategy).

### Property 1: Ranking is a deterministic total order with XP primary and depth tiebreak

_For any_ two `ScoreEntry` values `a` and `b`, `scoreCompare` ranks the entry with greater `totalXp` higher; when `totalXp` is equal, it ranks the entry with greater `deepestLevel` higher; when both `totalXp` and `deepestLevel` are equal, it produces a deterministic, antisymmetric ordering that does not depend on any other single field being "more important" than the ranking keys. The comparison is a consistent total order (for all `a`, `b`: exactly one of `a` ranks above `b`, `b` ranks above `a`, or they compare equal), so sorting any collection of entries is stable and repeatable.

**Validates: Requirements 2.1, 2.2, 2.3, 2.4, 5.2**

### Property 2: Leaderboard insertion preserves the sorted, capacity-bounded invariant and reports placement correctly

_For any_ `Leaderboard` (whose entries are already sorted descending) and any `ScoreEntry`, after `insert`: (a) the entries remain sorted descending by `scoreCompare`; (b) the size never exceeds `capacity`; (c) if the pre-insert size was below capacity, the entry is retained and no prior entry is lost; (d) if inserting overflows capacity, exactly the lowest-ranked entry is discarded; and (e) `insert` returns `true` if and only if the inserted entry is present in the board after truncation (and `false`, leaving the board otherwise unchanged, when the entry ranks below all retained entries of a full board).

**Validates: Requirements 3.1, 3.2, 3.3, 3.5**

### Property 3: Serialization round-trip preserves the leaderboard exactly

_For any_ `Leaderboard`, serializing it to a `TCODZip` archive and then deserializing produces a `Leaderboard` equivalent to the original — identical entry count, identical field values for every `ScoreEntry` (character name, career, homeworld, rank title, total XP, deepest level, outcome, date), and identical descending ordering.

**Validates: Requirements 6.7, 8.1, 8.2, 8.3**

### Property 4: Deserialization of invalid data yields an empty leaderboard without failing

_For any_ byte buffer that is not a valid serialized leaderboard (arbitrary/corrupt/foreign/empty content), loading the leaderboard returns an empty `Leaderboard` and never throws or aborts, allowing the game to continue.

**Validates: Requirements 7.3**

## Error Handling

- **Missing / empty / unreadable / corrupt file (Req 7.1-7.3).** `HighScoreStore::load()` returns an empty `Leaderboard` for all failure modes: file absent, zero-length, `TCODZip` read failure, or sentinel mismatch. It never throws and never blocks startup. The game continues with an empty in-memory board.
- **First write after empty init (Req 7.4).** Because the in-memory board is a valid (empty) `Leaderboard`, inserting the first entry and calling `HighScoreStore::save` writes a valid, well-formed `highscores.dat` regardless of the prior file state.
- **Independence from `game.sav` (Req 6.2, 6.3).** The high-score code path never reads or writes `game.sav`, and `Engine::save()`'s "dead player clears save" logic (`std::remove("game.sav")`) does not touch `highscores.dat`. Starting a new game (`term()` + `init()`) reloads but never rewrites the high-score file except through `recordRunOutcome`.
- **Missing player components.** `recordRunOutcome` null-guards `player`, `player->career`, and template lookups; if career data is missing it records conservative defaults (empty strings, `totalXp = 0`, `deepestLevel = max(1, dungeonLevel)`) rather than crashing.
- **Test isolation (steering).** Since the pure core never dereferences `engine`, and file I/O is confined to `HighScoreStore` (invoked only from the Engine layer), unit/property tests exercise `ScoreEntry`, `Leaderboard`, and serialization with no `engine.gui`/`engine.map`/`engine.player` access. Any test that needs a file uses a temp path and cleans it up, mirroring the `_level_cache_temp.sav` pattern in `Persistent.cpp`.

## Testing Strategy

This feature has a substantial pure-logic core (ranking, insertion, truncation, serialization) with well-defined universal properties over a large input space, so **property-based testing applies** to that core. The UI/rendering and file-wiring pieces are covered by example-based unit tests, consistent with the project's existing practice (`test_paginator.cpp` for pure logic; example tests for rendering/integration).

### Frameworks & conventions

- **Catch2 v3** (amalgamated, `Tests/lib/`) for the harness.
- **RapidCheck** for property tests, invoked with `rc::check`, **minimum 100 iterations** (per `tdd-workflow.md`).
- New test file `Tests/test_high_score.cpp` added to `Tests/40kRL_Tests.vcxproj`.
- New source `Source/HighScore.cpp` (and `Source/HighScoreStore.cpp`) added to **both** `40kRL.vcxproj` and `Tests/40kRL_Tests.vcxproj` (per `test-project.md`).
- Per `test-isolation.md`: RapidCheck generators use inclusive `inRange` bounds; index generators use `inRange(0, N-1)`.
- Each property test is tagged with a comment: **Feature: high-score-system, Property N: <text>** and references the design property it validates.

### Property tests (pure core)

Implement one property-based test per correctness property below, each running ≥100 iterations, generating random `ScoreEntry` values (random names, `totalXp` in a wide non-negative range, `deepestLevel` positive) and random `Leaderboard` states.

### Unit / example tests

- Ranking tiebreak examples: equal XP → deeper level wins; equal XP and level → deterministic order (Req 2.2-2.4).
- Insertion boundary examples: inserting into a below-capacity board retains all; inserting into a full board evicts the lowest; inserting a below-all entry into a full board is a no-op returning `false` (Req 3.2-3.5).
- Capacity default is 100 (Req 3.6).
- Serialization edge cases: empty board round-trips to empty; unicode/empty strings in fields survive; sentinel-mismatch archive deserializes to empty board (Req 7.3, 8).
- `HighScoreStore` integration (temp file): save-then-load equivalence on disk (Req 6.1, 8.1); missing file → empty board (Req 7.1); truncated/garbage file → empty board (Req 7.3); first save after empty init produces a loadable file (Req 7.4).
- Independence: after `HighScoreStore::save`, removing `game.sav` leaves `highscores.dat` intact and loadable (Req 6.3).
- Outcome formatting: known cause → `"Slain by X"`; unknown cause → `"Slain"` (Req 1.6, 1.7).

### UI (example tests, no PBT)

- Menu contains a "High Scores" item (Req 9.1).
- Empty-board view shows the "no runs recorded" message (Req 9.8).
- Paginator wiring for the view/death screens reuses the already-property-tested `Paginator`; only example tests confirm the indicator appears when `totalPages() > 1` and that the earned-entry page is selected on the death screen (Req 9.6, 10.5). Rendering pixels themselves are not asserted.
