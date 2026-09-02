# Character Name Entry Bugfix Design

## Overview

Character generation never lets the player name their character, so `player->name`
remains the hardcoded `"Player"` set in `Engine::init()`. The fix introduces a
name-entry step to the character-generation state machine, a name buffer on
`CharGenState`, a rendering pass for the prompt, and finalization logic that assigns
a sanitized name to `player->name`.

To keep the fix testable in the engine-free test binary (per `test-isolation.md`), the
core naming logic is extracted into a pure free function `sanitizeCharacterName(input)`
that applies the empty-fallback, max-length, and printable-only rules with no engine,
SDL, GUI, or libtcod dependencies. The character-generation code calls this helper; the
exploration and preservation property tests target the helper directly (RapidCheck,
>=100 iterations, inclusive `inRange` bounds).

The UI text-input mechanism reuses the existing precedent: `SDL_StartTextInput` is
already active (Engine construction), so `inputState.key.c` carries printable characters.
The debug seed entry in `updateWorldMap` demonstrates the accumulate-on-keypress /
commit-on-Enter pattern; the NAME step follows the same shape but also handles
`SDLK_BACKSPACE` and accepts printable name characters instead of digits only.

## Glossary

- **Bug_Condition (C)**: The condition that triggers the bug — a raw name input that is
  empty/whitespace-only, exceeds the maximum length, or contains non-printable characters,
  such that assigning it directly to `player->name` would produce an invalid or default name.
- **Property (P)**: The desired behavior — `sanitizeCharacterName` returns a non-empty,
  printable, length-bounded name (falling back to `"Rogue Trader"` when the input reduces
  to empty), and character generation assigns that result to `player->name`.
- **Preservation**: For already-valid names (non-empty, within max length, printable only)
  `sanitizeCharacterName` is the identity; and all existing character-generation steps
  (HOMEWORLD/CAREER/ADVANCES/DONE) and the high-score copy of `player->name` are unchanged.
- **`sanitizeCharacterName(input)`**: New pure helper (proposed `Headers/CharacterName.hpp`
  + `Source/CharacterName.cpp`) that normalizes a raw name string. Engine-free and testable.
- **`sanitizeCharacterName`**: applies rules in order — strip non-printable chars, truncate
  to `MAX_NAME_LENGTH` (24), trim surrounding whitespace, and if the result is empty return
  the default `"Rogue Trader"`.
- **`CharGenState::enteredName`**: New `std::string` field holding the in-progress name buffer.
- **NAME step**: New `Step` enum value; the first step of character generation, before HOMEWORLD.
- **`updateCharGen` / `renderCharGen` / `charGenGoBack`**: The char-gen state machine functions
  in `Source/Engine.cpp`.
- **`player->name`**: The player `Actor`'s name field; source of `ScoreEntry.characterName`.

## Bug Details

### Bug Condition

The bug manifests in two layers. At the workflow layer, character generation offers no NAME
step, so `player->name` is never assigned and stays `"Player"`. At the value layer, even once a
name is captured, a raw input string may be empty, over-length, or contain non-printable
characters; assigning such a raw string directly to `player->name` would be invalid. The pure
helper `sanitizeCharacterName` is the testable seam: the bug condition is any raw input that the
helper must correct (i.e., where the raw input differs from a valid name).

**Formal Specification:**
```
FUNCTION isBugCondition(input)
  INPUT: input of type string (raw name as typed)
  OUTPUT: boolean

  // The raw input is "buggy" if assigning it directly to player->name would
  // produce an invalid name: empty/whitespace-only, too long, or containing
  // non-printable characters.
  RETURN isEmptyOrWhitespace(input)
         OR length(input) > MAX_NAME_LENGTH        // MAX_NAME_LENGTH = 24
         OR containsNonPrintable(input)
END FUNCTION
```

### Examples

- Empty input `""` — currently no step captures a name at all; expected: sanitizes to `"Rogue Trader"`.
- Whitespace-only input `"   "` — expected: sanitizes to `"Rogue Trader"`.
- Over-length input `"AveryVeryLongCharacterNameExceedingLimit"` (40 chars) — expected: truncated to 24 chars.
- Non-printable input `"Ro\x01gue"` — expected: control char stripped, yields `"Rogue"`.
- Edge case: already-valid input `"Cordelia"` — expected: returned unchanged (identity).
- End-to-end: after completing char-gen with a valid name, `player->name` equals the sanitized
  name and, on death, `ScoreEntry.characterName` equals that name (not `"Player"`).

## Expected Behavior

### Preservation Requirements

**Unchanged Behaviors:**
- HOMEWORLD, CAREER, and ADVANCES navigation, selection, grants, and XP purchases must work
  exactly as before.
- Escape back-navigation (`charGenGoBack`) between existing steps must remain unchanged.
- The DONE finalization (CharacterSheet build, career combat stats, characteristics, skills,
  talents, traits, starting equipment, FOV/camera transition) must remain unchanged apart from
  the single added `player->name` assignment.
- The high-score copy `entry.characterName = player->name` must remain unchanged.
- For an already-valid name, `sanitizeCharacterName` must return the input unchanged (identity).

**Scope:**
All inputs that are already valid names (non-empty after trim, within `MAX_NAME_LENGTH`,
printable characters only) should be completely unaffected by the fix. This includes:
- Typical short names ("Boo", "Cordelia")
- Names exactly at the maximum length (24 characters, all printable)
- Names containing internal spaces or common punctuation that are printable

## Hypothesized Root Cause

Based on the investigation, the causes are:

1. **Missing workflow step**: The `Step` enum `{ HOMEWORLD, CAREER, ADVANCES, DONE }` has no
   NAME step, so there is nowhere in `updateCharGen` to capture a typed name.

2. **Missing state field**: `CharGenState` has no `enteredName` (or equivalent) buffer to
   accumulate keystrokes into.

3. **Missing assignment at finalization**: The DONE branch of `updateCharGen` finalizes every
   other aspect of the character but never sets `player->name`, so it retains the `"Player"`
   literal from `Engine::init()`.

4. **No validation seam**: Even if a name were captured inline, the validation/fallback logic
   would live inside `updateCharGen`, which requires the Engine/GUI and cannot be exercised by
   the engine-free test binary — leaving the behavior untestable.

## Correctness Properties

Property 1: Bug Condition - Name Sanitization Produces a Valid Name

_For any_ raw input where the bug condition holds (isBugCondition returns true — empty/whitespace,
over-length, or containing non-printable characters), the fixed function `sanitizeCharacterName`
SHALL return a name that is non-empty, no longer than `MAX_NAME_LENGTH` (24), and composed of
printable characters only; and when the input reduces to empty it SHALL return the default
`"Rogue Trader"`.

**Validates: Requirements 2.2, 2.3, 2.4, 2.5**

Property 2: Preservation - Valid Names Are Unchanged

_For any_ input where the bug condition does NOT hold (isBugCondition returns false — a name that
is already non-empty, within `MAX_NAME_LENGTH`, and printable only), the fixed function
`sanitizeCharacterName` SHALL return the input unchanged, preserving the exact string; existing
character-generation steps and the high-score copy of `player->name` remain unchanged.

**Validates: Requirements 3.1, 3.2, 3.3, 3.4**

## Fix Implementation

### Changes Required

Assuming the root-cause analysis is correct:

**New File**: `Headers/CharacterName.hpp` and `Source/CharacterName.cpp`

**Function**: `std::string sanitizeCharacterName(const std::string& input)`

1. **Add the pure sanitization helper**:
   - Define `constexpr int MAX_NAME_LENGTH = 24;` and `const std::string DEFAULT_NAME = "Rogue Trader";`.
   - Strip non-printable characters (keep bytes where a printable test passes; the exact
     printable test to be finalized in implementation, but must be deterministic and engine-free).
   - Truncate to `MAX_NAME_LENGTH`.
   - Trim leading/trailing whitespace.
   - If the result is empty, return `DEFAULT_NAME`.
   - No engine/SDL/GUI/libtcod access (test-isolation compliant).
   - Register `Source/CharacterName.cpp` in BOTH `40kRL.vcxproj` and
     `Tests/40kRL_Tests.vcxproj` (Game source files ItemGroup) per `test-project.md`.

**File**: `Headers/CharacterGenerator.hpp`

2. **Add NAME step and name buffer**:
   - Change the `Step` enum to `{ NAME, HOMEWORLD, CAREER, ADVANCES, DONE }` with NAME first.
   - Add `std::string enteredName;` to `CharGenState`.

**File**: `Source/Engine.cpp`

3. **`beginCharGen`**: set `charGenState->currentStep = CharGenState::Step::NAME;`.

4. **`updateCharGen` — new NAME case**:
   - On printable keypress (`inputState.key.c` printable), append to `enteredName` up to
     `MAX_NAME_LENGTH`.
   - On `SDLK_BACKSPACE`, remove the last character if any.
   - On `SDLK_RETURN`, advance to `Step::HOMEWORLD`.
   - Escape at NAME does nothing (or exits per existing convention — first step cannot go back).

5. **`updateCharGen` — DONE case**: after existing finalization, set
   `player->name = sanitizeCharacterName(charGenState->enteredName);`.

6. **`renderCharGen` — new NAME case**: draw a prompt showing the current `enteredName` buffer
   plus a cursor and an instruction line ("[Type name] [Backspace] Delete [Enter] Confirm").

7. **`charGenGoBack`**: handle NAME (no-op, first step) and update the CAREER-back / HOMEWORLD-back
   transitions so that going back from HOMEWORLD returns to NAME (preserving `enteredName`).

## Testing Strategy

### Validation Approach

Two-phase approach: first surface counterexamples that demonstrate the bug on the UNFIXED code
via the pure `sanitizeCharacterName` seam (which does not yet exist / does not yet apply the
rules), then verify the fix corrects buggy inputs and preserves already-valid names. All property
tests use RapidCheck with >=100 iterations and inclusive `inRange` bounds.

### Exploratory Bug Condition Checking

**Goal**: Surface counterexamples that demonstrate the bug BEFORE implementing the fix. Confirm
that raw inputs satisfying `isBugCondition` are not corrected today (the helper does not exist or
does not enforce the rules).

**Test Plan**: Add a property test targeting `sanitizeCharacterName`. Because the helper does not
exist on UNFIXED code, the test file will initially fail to compile / link (a legitimate "fails on
unfixed code" state under TDD). To make the failure a meaningful assertion rather than a build
error, the test may reference a minimal declared-but-unimplemented signature, or assert the
observable engine-level fact that char-gen has no NAME step. The preferred approach: declare the
`sanitizeCharacterName` signature and write the property; on UNFIXED code it fails (unimplemented /
returns raw input), confirming the bug.

**Test Cases**:
1. **Empty/whitespace input** — sanitize should return `"Rogue Trader"` (fails on unfixed code).
2. **Over-length input** — sanitize should truncate to 24 chars (fails on unfixed code).
3. **Non-printable input** — sanitize should strip control characters (fails on unfixed code).
4. **Edge: exactly 24 printable chars** — should be returned unchanged (may fail on unfixed code).

**Expected Counterexamples**:
- Raw over-length or empty inputs are not corrected (no NAME step exists; helper absent).
- Possible causes: missing NAME step, missing `enteredName` field, missing `player->name` assignment.

### Fix Checking

**Goal**: Verify that for all inputs where the bug condition holds, the fixed function produces
the expected behavior.

**Pseudocode:**
```
FOR ALL input WHERE isBugCondition(input) DO
  result := sanitizeCharacterName(input)
  ASSERT length(result) >= 1
     AND length(result) <= MAX_NAME_LENGTH
     AND allPrintable(result)
     AND (reducesToEmpty(input) IMPLIES result = "Rogue Trader")
END FOR
```

### Preservation Checking

**Goal**: Verify that for all inputs where the bug condition does NOT hold (already-valid names),
the fixed function returns the input unchanged.

**Pseudocode:**
```
FOR ALL input WHERE NOT isBugCondition(input) DO
  ASSERT sanitizeCharacterName(input) = input
END FOR
```

**Testing Approach**: Property-based testing (RapidCheck, >=100 iterations) is recommended for
preservation because it generates many valid names across the domain and catches edge cases such
as names exactly at the length boundary. Generators must use inclusive `inRange` bounds (e.g.
`inRange(1, MAX_NAME_LENGTH)` for valid lengths).

**Test Plan**: Generate already-valid names (length in `[1, 24]`, printable characters, no leading/
trailing whitespace) and assert `sanitizeCharacterName` returns them unchanged.

**Test Cases**:
1. **Valid name identity** — generate valid names, assert output equals input.
2. **Boundary length identity** — names of exactly 24 printable chars returned unchanged.
3. **Common punctuation/spaces** — internal spaces and printable punctuation preserved.

### Unit Tests

- `sanitizeCharacterName("")` returns `"Rogue Trader"`.
- `sanitizeCharacterName("   ")` returns `"Rogue Trader"`.
- `sanitizeCharacterName` truncates a 40-char input to 24 chars.
- `sanitizeCharacterName` strips a control character from the middle of a name.
- `sanitizeCharacterName("Cordelia")` returns `"Cordelia"` unchanged.

### Property-Based Tests

- Property 1 (fix): for all buggy inputs, output is non-empty, <=24, printable; empty-reducing
  inputs yield `"Rogue Trader"`.
- Property 2 (preservation): for all already-valid inputs, output equals input.

### Integration Tests

- (Engine-dependent, not run in the engine-free binary) Manual/integration verification that the
  NAME step prompts, accepts typed characters, supports Backspace, commits on Enter, and that the
  finalized `player->name` equals the sanitized entered name and flows into `ScoreEntry.characterName`
  on death.
