# Crit Hit Location Display Bugfix Design

## Overview

The player reported that the right-hand sidebar's "Critical Injuries" section always shows
the hit location as "Body". The confirmed encounter was a **melee** attack resolved through
`Attacker::resolveCharacterAttack`, and the fatal blow was a **leg crit that killed the
character** — yet the sidebar showed only one line, "Body". This design tackles the bug in
two parts.

**Part A — Root-cause the "always Body" symptom for NON-FATAL recorded crits.** The one
pure, engine-free seam in the melee crit path is `HitLocationTable::resolve(int d100Roll)`
(`Source/HitLocation.cpp`), which maps the to-hit d100 roll to a `HitLocation` via digit
reversal. The exploration test drives `resolve` across the FULL `1..100` roll domain and
asserts the mapping matches the intended Rogue Trader digit-reversal table (Head 01-10,
Right Arm 11-20, Left Arm 21-30, Body 31-70, Right Leg 71-80, Left Leg 81-00), i.e. that
`resolve` is not collapsing or over-representing BODY. The design specifies two possible
outcomes and how the analysis adapts to each (see Hypothesized Root Cause).

**Part B — Record and display fatal critical hits (NEW behavior, now in scope).** By user
decision, the fatal blow's hit location must now be recorded so a future death screen can
display it. Currently, in `Attacker::resolveCharacterAttack` the "dead outright" branch
(`hp <= 0`) and the fatal-crit branch (`critEffect.fatal`) call `die()` WITHOUT recording an
`InjuryRecord`; the same pattern exists in `RangedCombat::resolve`. This design adds a
near-pure `InjuryTracker` seam to append a fatal crit record without applying debuffs to a
corpse, and wires it into the fatal branches of both combat paths.

**Out of scope:** how the death screen surfaces the player's high-score placement (Enter
behavior) is the separate `death-screen-highscore-jump` spec. This bugfix only ensures the
fatal crit's location is recorded/available for that future screen to read.

The strategy honours the project's test-isolation rules: all new logic lives in pure or
near-pure seams (`HitLocation.cpp`, `InjuryTracker`) that unit- and property-test cleanly
without an initialized `Engine`.

## Glossary

- **Bug_Condition (C)**: For Part A, the condition that `resolve(X)` returns a location that
  differs from the RT-table expected location for roll `X`. For Part B, the condition that a
  fatal critical hit is dealt (via the "dead outright" or `critEffect.fatal` branches).
- **Property (P)**: The desired behavior — for Part A, `resolve(X)` equals the RT-table
  expected location for every `X in [1,100]`; for Part B, the fatal blow's hit location is
  recorded as an `InjuryRecord` (or equivalent field) so it can be displayed.
- **Preservation**: Existing behavior that must remain unchanged — non-fatal crit recording,
  debuff application, magnitude display, save/load round-trip, and rolls that genuinely map
  to BODY still returning BODY.
- **`HitLocationTable::resolve(int d100Roll)`**: The pure function in `Source/HitLocation.cpp`
  that reverses the d100 roll's digits and maps the reversed value to a `HitLocation`.
- **`expectedLocation(X)`**: The reference RT-table mapping used by tests: reverse the digits
  of `X` (treating roll 100 as reversed 00), then bucket Head 01-10, Right Arm 11-20, Left Arm
  21-30, Body 31-70, Right Leg 71-80, Left Leg 81-00 (reversed 00 → Left Leg).
- **`InjuryTracker::applyCrit`**: Appends an `InjuryRecord`, updates cumulative magnitude, and
  applies debuffs; returns `false` when the new total reaches the fatal threshold (caller
  triggers death) WITHOUT recording a record.
- **`InjuryRecord`**: `{ HitLocation location; int magnitude; }` — one recorded crit event
  that the sidebar renders via `HitLocationTable::name(record.location)`.
- **Fatal branch**: In `Attacker::resolveCharacterAttack`, the `hp <= 0` branch and the
  `critEffect.fatal` branch (and, in `InjuryTracker::applyCrit`, the `!survived` overload).
  In `RangedCombat::resolve`, the mirrored `hp <= 0`, `critEffect.fatal`, and `!survived`
  branches.

## Bug Details

### Bug Condition

**Part A (pure seam).** The bug — if present — manifests when `HitLocationTable::resolve`
maps a to-hit d100 roll to a location that differs from the intended RT digit-reversal table,
in particular by collapsing or over-representing BODY. Because `resolve` is deterministic and
engine-free, the bug condition is decidable over the entire `1..100` roll domain.

**Formal Specification (Part A):**
```
FUNCTION isBugCondition(input)
  INPUT: input of type int  // a d100 roll in [1, 100]
  OUTPUT: boolean

  RETURN input IN [1..100]
         AND HitLocationTable::resolve(input) != expectedLocation(input)
END FUNCTION

FUNCTION expectedLocation(X)
  INPUT: X of type int in [1, 100]
  OUTPUT: HitLocation

  // Reverse digits; roll 100 is treated as reversed "00".
  IF X == 100 THEN reversed := 0
  ELSE reversed := (X MOD 10) * 10 + (X DIV 10)

  IF reversed == 0        THEN RETURN LEFT_LEG   // "00" bucket (81-00)
  ELSE IF reversed <= 10  THEN RETURN HEAD       // 01-10
  ELSE IF reversed <= 20  THEN RETURN RIGHT_ARM  // 11-20
  ELSE IF reversed <= 30  THEN RETURN LEFT_ARM   // 21-30
  ELSE IF reversed <= 70  THEN RETURN BODY       // 31-70
  ELSE IF reversed <= 80  THEN RETURN RIGHT_LEG  // 71-80
  ELSE                    RETURN LEFT_LEG        // 81-99
END FUNCTION
```

**Part B (fatal recording).** The bug manifests when a critical hit is fatal: the code calls
`target->destructible->die(target)` without ever writing an `InjuryRecord` for the fatal
blow, so the fatal location is unavailable to the sidebar / death screen.

**Formal Specification (Part B):**
```
FUNCTION isFatalUnrecorded(event)
  INPUT: event of type CritEvent { location, magnitude, killedTarget: boolean, recorded: boolean }
  OUTPUT: boolean

  RETURN event.killedTarget == true
         AND event.recorded == false   // no InjuryRecord exists for the fatal blow
END FUNCTION
```

### Examples

- **Roll 01** → reversed `10` → Head. Expected: Head. (Part A: matches.)
- **Roll 10** → reversed `01` → Head. Expected: Head. (Part A: matches.)
- **Roll 11** → reversed `11` → Right Arm. Expected: Right Arm. (Part A: matches.)
- **Roll 50** → reversed `05` → Head. Expected: Head. (Part A: matches.)
- **Roll 55** → reversed `55` → Body. Expected: Body. (Part A: genuine Body — preservation.)
- **Roll 100** → special-cased reversed `00` → Left Leg. Expected: Left Leg. (Part A: matches.)
- **Part B example:** A leg crit (`resolve` → Left Leg) drives HP into the fatal region;
  `critEffect.fatal` is true, `die()` is called, and NO `InjuryRecord` is written — so the
  sidebar cannot show "Left Leg". The single "Body" line the player saw came from an earlier
  NON-FATAL crit that genuinely landed on Body (40% of the table).

## Expected Behavior

### Preservation Requirements

**Unchanged Behaviors:**
- Rolls whose reversed value falls in 31-70 (genuine Body) must continue to return `BODY`.
- Non-fatal crit recording must continue: `applyCrit` still appends `InjuryRecord{loc, total}`
  and applies the correct `(location, magnitude)` debuffs (bugfix.md 3.4).
- The sidebar must continue to display each record's magnitude value unchanged (bugfix.md 3.2).
- Save/load must continue to round-trip each record's location and magnitude symmetrically
  (bugfix.md 3.3).
- Mouse/keyboard flows, hit/miss classification, DoS/damage math, and status-effect triggers
  must be unaffected.

**Scope:**
All inputs that do NOT satisfy a bug condition must be completely unaffected by this fix.
For Part A this means every roll whose current `resolve` result already equals
`expectedLocation` (which, per the determination below, is expected to be ALL rolls). For
Part B this means all NON-fatal crit resolutions — their recording path is untouched; only
the fatal branches gain a new recording call.

## Hypothesized Root Cause

### Part A — Determination from reading `Source/HitLocation.cpp`

`resolve` computes `reversed = (X % 10) * 10 + (X / 10)` (with roll 100 special-cased to
reversed `0`), then buckets reversed values as Head ≤10, Right Arm ≤20, Left Arm ≤30,
Body ≤70, Right Leg ≤80, Left Leg otherwise (and reversed `0` → Left Leg). Working through
the domain, this is a faithful bijective digit reversal that reproduces the intended RT
distribution exactly — Body occupies reversed 31-70 (40 of 100 rolls, 40%), and every other
location gets its correct share. **`resolve` appears CORRECT across the full `1..100`
domain; it does NOT collapse or over-represent BODY beyond the intended 40%.**

This points to **Outcome (i)** below.

**Two-outcome branch:**

- **Outcome (i) — `resolve` is CORRECT across `1..100` (expected).** The "always Body" is
  NOT in `resolve`. The analysis then pivots to verifying that the record path is faithful:
  1. `resolveCharacterAttack` computes `loc = HitLocationTable::resolve(roll)` once and
     passes that SAME `loc` to `applyCrit` — confirmed by inspection.
  2. `InjuryTracker::applyCrit` stores `InjuryRecord{ loc, newTotal }` and `getRecords()`
     returns it faithfully; `save`/`load` round-trip `location` — confirmed by inspection.
  3. `Gui.cpp` renders `HitLocationTable::name(record.location)` directly — confirmed.
  With the pure seam and the record path both faithful, the most likely explanation for the
  player's single "Body" line is a **sampling artifact**: the player had exactly one recorded
  NON-FATAL crit, and Body is 40% of the table, so a single sample landing on Body is
  unremarkable. The fatal leg crit was simply never recorded (Part B). An end-to-end
  record-fidelity property (Property 3) locks this in: given a known roll, the resulting
  `InjuryRecord.location` equals `HitLocationTable::resolve(roll)`.
- **Outcome (ii) — `resolve` is INCORRECT (over-produces BODY).** If the exploration test
  finds any roll `X` where `resolve(X) != expectedLocation(X)`, fix `resolve` so the digit
  reversal and bucketing match the RT table. Property 1 then guards the fix.

Based on the code reading, Outcome (i) is the expected result; the exploration test will
confirm or refute it before any fix is committed.

### Part B — Fatal crits are never recorded

By inspection of both combat paths, three fatal branches call `die()` without recording:

1. `Attacker::resolveCharacterAttack`, "dead outright" branch (`hp <= 0`).
2. `Attacker::resolveCharacterAttack`, `critEffect.fatal` branch (and the `!survived`
   overload where cumulative magnitude reaches the fatal threshold — note `applyCrit` returns
   `false` and does NOT push a record in that case).
3. `RangedCombat::resolve`, the mirrored `hp <= 0`, `critEffect.fatal`, and `!survived`
   branches.

Root cause: the fatal blow's location is discarded because there is no API to record a crit
without applying debuffs to a dying/dead actor (`applyCrit` both records AND applies debuffs,
and it refuses to record once the fatal threshold is crossed).

## Correctness Properties

Property 1: Bug Condition - `resolve` matches the RT hit-location table over `1..100`

_For any_ d100 roll `X` in `[1, 100]` where the bug condition holds (`isBugCondition(X)` is
true, i.e. `resolve(X) != expectedLocation(X)`), the fixed `HitLocationTable::resolve`
function SHALL return `expectedLocation(X)` — the intended RT digit-reversal location
(Head 01-10, Right Arm 11-20, Left Arm 21-30, Body 31-70, Right Leg 71-80, Left Leg 81-00).
Equivalently, for all `X` in `[1, 100]`, `resolve(X) == expectedLocation(X)`.

**Validates: Requirements 2.1, 2.2, 4.1**

Property 2: Preservation - Genuine Body rolls still return BODY

_For any_ roll `X` in `[1, 100]` where the bug condition does NOT hold — specifically rolls
whose reversed value lies in `31..70` (genuine Body) — the fixed `resolve` SHALL produce the
same result as the original function, namely `HitLocation::BODY`, preserving correct Body
resolution and all non-Body mappings that are already correct.

**Validates: Requirements 3.1**

Property 3: Fatal-Crit Recording Fidelity - recorded location equals resolved location

_For any_ critical hit that is fatal (Part B bug condition holds), the fatal blow SHALL be
recorded such that the recorded location equals the location resolved for that attack:
`recordedFatalLocation == HitLocationTable::resolve(roll)` (or, for the auto-hit ranged
destructible path, equals the location assigned to that attack). For a known roll driven
through the fatal path, the appended fatal `InjuryRecord.location` SHALL equal
`HitLocationTable::resolve(roll)`, making the fatal blow's location available for display.

**Validates: Requirements 2.1, 2.2**

## Fix Implementation

### Part A — `Source/HitLocation.cpp` (conditional on Outcome (ii))

**File:** `Source/HitLocation.cpp`
**Function:** `HitLocationTable::resolve(int d100Roll)`

- If the exploration test confirms **Outcome (i)** (`resolve` correct across `1..100`), make
  **no change** to `resolve`. The Bug Analysis is revised to record that "always Body" is a
  sampling artifact plus the unrecorded fatal crit (Part B), per bugfix.md 4.2.
- If the exploration test reveals **Outcome (ii)**, correct the digit-reversal and/or bucket
  boundaries so `resolve(X) == expectedLocation(X)` for all `X in [1,100]`. Keep roll 100
  special-cased to the reversed "00" → Left Leg bucket.

### Part B — `InjuryTracker` fatal-recording API

**File:** `Headers/InjuryTracker.hpp` / `Source/InjuryTracker.cpp`

Add a near-pure seam that appends a fatal crit record WITHOUT applying debuffs to a corpse
(debuffs on a dead actor are meaningless and would risk touching a partially torn-down actor
in tests). This keeps the method engine-free and unit-testable per `test-isolation.md`.

Proposed API:
```cpp
// Records a fatal critical hit's location for display purposes without applying
// characteristic debuffs (the actor is dying/dead). Sets cumulative magnitude to the
// fatal threshold so the sidebar reflects the killing blow. Engine-free / test-safe.
void recordFatalCrit(HitLocation loc, int magnitude);
```
Implementation notes:
- Push `InjuryRecord{ loc, clampedMagnitude }` onto `records_` (clamp magnitude to
  `[1, MAX_MAGNITUDE]` for save/load symmetry, or store `FATAL_MAGNITUDE` if we choose to
  surface the fatal value — decide during implementation; either way it must round-trip).
- Do NOT call `applyDebuff` (no living characteristics to modify).
- `save`/`load` already round-trip every record in `records_`, so the fatal record persists
  symmetrically with no format change (Property/Requirement 3.3 preserved).

### Part B — Call sites

**File:** `Source/Attacker.cpp`, function `resolveCharacterAttack`
1. "Dead outright" branch (`hp <= 0`): before `target->destructible->die(target)`, ensure
   `target->injuryTracker` exists and call `target->injuryTracker->recordFatalCrit(loc, critMagnitude)`.
2. `critEffect.fatal` branch: same pattern — record `(loc, critMagnitude)` before `die()`.
3. `!survived` overload (cumulative magnitude reached fatal threshold): record the fatal
   `(loc, critMagnitude)` before `die()` (since `applyCrit` returned `false` and did not push
   a record).

**File:** `Source/RangedCombat.cpp`, function `resolve`
4. Mirror the three fatal branches (`hp <= 0`, `critEffect.fatal`, `!survived`) to call
   `recordFatalCrit(result.location, critMagnitude)` before `die()`.

Notes:
- Guard `injuryTracker` creation exactly as the existing non-fatal branches do
  (`std::make_unique<InjuryTracker>()` when null) so a fatal-on-first-crit actor still gets a
  tracker to hold the record.
- No `engine.gui` calls are added inside `InjuryTracker`; all messaging stays in the combat
  path where the engine is guaranteed initialized (test-isolation.md).

## Testing Strategy

### Validation Approach

Two-phase: first surface counterexamples on the UNFIXED code (exploration), then verify the
fix works and preserves existing behavior. Property-based tests use RapidCheck with a minimum
of 100 iterations; range generators use INCLUSIVE bounds (`rc::gen::inRange(1, 100)` for the
full roll domain). All new/changed `.cpp` files are registered in BOTH `40kRL.vcxproj` and
`Tests/40kRL_Tests.vcxproj` (test-project.md), and every test avoids engine globals
(test-isolation.md).

### Exploratory Bug Condition Checking

**Goal:** Surface counterexamples that demonstrate the bug BEFORE implementing any fix.
Confirm or refute the Part A root-cause analysis. If refuted, re-hypothesize.

**Test Plan (Part A, the pure seam):** Drive `HitLocationTable::resolve` across the FULL
`1..100` roll domain and assert `resolve(X) == expectedLocation(X)` for every `X`, where
`expectedLocation` is an independent test-side implementation of the RT table. Run on the
UNFIXED code to observe whether any roll disagrees (i.e. whether resolve collapses/over-
represents BODY).

**Test Cases (Part A):**
1. **Full-domain sweep**: exhaustive `for X in 1..100` equality check `resolve(X) == expectedLocation(X)`.
2. **Body-share check**: exactly 40 of the 100 rolls resolve to BODY (reversed 31-70).
3. **Boundary rolls**: 01, 10, 11, 20, 21, 30, 31, 70, 71, 80, 81, 100 map to the expected buckets.
4. **Roll-100 special case**: `resolve(100) == LEFT_LEG`.

**Test Plan (Part B, the recording gap):** With a stubbed/near-pure `InjuryTracker`, assert
that after a fatal crit no record exists on the UNFIXED code (documents the current gap), and
after the fix a record exists whose location equals the resolved location.

**Expected Counterexamples:**
- Part A: expected to find NONE (Outcome (i)); resolve is anticipated correct across `1..100`.
  If any roll disagrees, that is the Outcome (ii) counterexample and drives a resolve fix.
- Part B: on unfixed code, a fatal crit leaves `records_` without the fatal blow (the
  observable gap).

### Fix Checking

**Goal:** Verify that for all inputs where a bug condition holds, the fixed code produces the
expected behavior.

**Pseudocode:**
```
// Part A
FOR ALL X IN [1..100] WHERE resolve_original(X) != expectedLocation(X) DO
  ASSERT resolve_fixed(X) == expectedLocation(X)
END FOR

// Part B
FOR ALL fatalCrit(loc, roll) DO
  recordFatalCrit(loc, magnitude)
  ASSERT lastRecord().location == HitLocationTable::resolve(roll)   // == loc
END FOR
```

### Preservation Checking

**Goal:** Verify that for all inputs where no bug condition holds, the fixed code produces the
same result as the original.

**Pseudocode:**
```
// Part A: genuine Body and all already-correct rolls unchanged
FOR ALL X IN [1..100] WHERE resolve_original(X) == expectedLocation(X) DO
  ASSERT resolve_fixed(X) == resolve_original(X)
END FOR

// Part B: non-fatal crit path unchanged
FOR ALL nonFatalCrit(loc, mag) DO
  ASSERT applyCrit_fixed(loc, mag) == applyCrit_original(loc, mag)
         AND recordsAfterFixed == recordsAfterOriginal
         AND debuffsAfterFixed == debuffsAfterOriginal
END FOR
```

**Testing Approach:** Property-based testing is recommended for preservation because it
generates many cases across the input domain, catches edge cases manual tests miss, and gives
strong guarantees that behavior is unchanged for all non-buggy inputs.

**Test Plan:** Observe behavior on UNFIXED code first (genuine-Body rolls; non-fatal crit
recording/debuffs; save/load round-trip), then write property-based tests capturing that
behavior so it is locked against regression.

**Test Cases:**
1. **Genuine-Body preservation**: for rolls with reversed value in 31-70, `resolve` returns BODY before and after.
2. **Non-fatal recording preservation**: `applyCrit` still appends `{loc, total}` and applies the same debuffs.
3. **Save/load preservation**: records (including any new fatal record) round-trip location and magnitude symmetrically.
4. **Magnitude display preservation**: sidebar magnitude value per record is unchanged.

### Unit Tests

- `resolve` boundary rolls and roll-100 special case (Part A).
- `recordFatalCrit` appends a record with the given location and does NOT alter owner
  characteristics (pass a null/characteristic-less owner to confirm engine-free safety).
- Fatal-branch call-site behavior via the near-pure tracker seam (record present after a
  simulated fatal crit).

### Property-Based Tests

- Part A: `rc::check` over `rc::gen::inRange(1, 100)` asserting `resolve(X) == expectedLocation(X)` (≥100 iterations).
- Part A preservation: for generated rolls whose reversed value is in 31-70, `resolve(X) == BODY`.
- Part B fidelity: for a generated roll `X`, driving the fatal path records a location equal
  to `HitLocationTable::resolve(X)` (≥100 iterations).
- Save/load round-trip: generate a vector of `InjuryRecord`s (including fatal-magnitude ones),
  save then load, assert per-record `location`/`magnitude` equality.

### Integration Tests

- Full melee crit flow: a non-fatal crit at a non-Body location shows that location in the
  records list (via `getRecords()`), confirming end-to-end fidelity (Outcome (i)).
- Fatal melee crit flow: after a fatal blow, a record exists whose location matches the
  resolved location, making it available to a future death screen.
- Fatal ranged crit flow: mirror of the melee case through `RangedCombat::resolve`.
- Context/round-trip: save after a fatal crit, reload, and confirm the fatal record's
  location and magnitude survive.
```

