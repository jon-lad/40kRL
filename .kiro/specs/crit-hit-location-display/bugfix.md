# Bugfix Requirements Document

## Introduction

The right-hand sidebar's "Critical Injuries" section always displays the hit location
as "Body" regardless of where the attack actually landed. The player reports that every
critical injury shown in the sidebar reads "Body", even though the underlying hit-location
system (`HitLocationTable::resolve`) can produce Head, Right Arm, Left Arm, Right Leg, and
Left Leg. This misreports critical injury information to the player, undermining the tactical
value of the hit-location system.

New reproduction evidence from the player narrows the path considerably. The confirmed
encounter was a **melee** attack that resolved through `Attacker::resolveCharacterAttack`
(`Source/Attacker.cpp`). The fatal blow was a **leg crit that killed the character**, yet
the sidebar showed **only one line — "Body"**, and the player is unsure whether they had
taken any crit beforehand. This evidence has two structural consequences:

1. **It rules in the melee character-attack path and rules out the auto-hit path.**
   `RangedCombat::resolveDestructibleAttack` hardcodes `HitLocation::BODY`, but the observed
   crit was a melee character attack. The auto-hit BODY hardcode is therefore NOT the cause
   of what the player saw.
2. **The fatal blow was never recorded.** In `resolveCharacterAttack`, both the "dead
   outright" branch (`hp <= 0`) and the fatal-crit branch (`critEffect.fatal`) call
   `target->destructible->die(target)` WITHOUT calling `injuryTracker->applyCrit(...)`. A
   fatal crit is therefore never written as an `InjuryRecord`. The single "Body" line the
   player saw must have come from an EARLIER, NON-FATAL crit that was recorded — consistent
   with the player being "not sure if I had taken a crit previously" (exactly one prior
   non-fatal crit, displayed as Body).

This reframes the investigation. The suspect behaviour is that the earlier **non-fatal
melee crit was recorded/resolved as Body** when the player expected location variety. The
prime suspects are now: (a) `HitLocationTable::resolve` mapping the melee to-hit d100 roll
to a location — whether it over-produces BODY or reverses the roll incorrectly; and
(b) whether the location stored via `applyCrit` matches what `resolve` returned. The sidebar
display code (`Gui.cpp` reading `record.location`) and `InjuryTracker` save/load were
inspected and appear correct.

Because the root cause is not yet confirmed, this bugfix treats root-cause confirmation as
part of the exploration phase (see design and tasks). The exploration test targets the one
pure, engine-free seam in the path — `HitLocationTable::resolve` — to determine whether it
collapses or over-represents BODY across the full `1..100` roll domain.

## Bug Analysis

### Current Behavior (Defect)

1.1 WHEN the player receives a non-fatal critical injury from a melee attack (resolved via `Attacker::resolveCharacterAttack`) at a non-body hit location (Head, Right Arm, Left Arm, Right Leg, or Left Leg) THEN the sidebar "Critical Injuries" section displays the location as "Body" instead of the actual location
1.2 WHEN multiple non-fatal critical injuries are recorded at distinct hit locations THEN the sidebar displays every record's location as "Body"
1.3 WHEN a critical injury is fatal (the "dead outright" branch where `hp <= 0`, or the `critEffect.fatal` branch) THEN the system calls `die()` without calling `injuryTracker->applyCrit(...)`, so the fatal blow is never recorded as an `InjuryRecord` and never appears in the sidebar (this is why the player who took a fatal leg crit saw only a single "Body" line from an earlier non-fatal crit)

### Expected Behavior (Correct)

2.1 WHEN the player receives a critical injury at a specific hit location THEN the system SHALL display the actual hit location that was resolved for that attack (Head, Right Arm, Left Arm, Body, Right Leg, or Left Leg)
2.2 WHEN multiple critical injuries are received at distinct hit locations THEN the system SHALL display each record with its own resolved hit location, preserving per-record location fidelity

### Unchanged Behavior (Regression Prevention)

3.1 WHEN a critical injury genuinely lands on the Body (resolved location is BODY) THEN the system SHALL CONTINUE TO display "Body"
3.2 WHEN a critical injury is recorded THEN the system SHALL CONTINUE TO display the associated magnitude value unchanged
3.3 WHEN critical injuries are saved and reloaded THEN the system SHALL CONTINUE TO round-trip each record's hit location and magnitude symmetrically
3.4 WHEN a critical injury is applied THEN the system SHALL CONTINUE TO apply the correct characteristic debuffs for the resolved (location, magnitude) pair
3.5 WHEN a critical hit is fatal THEN the exploration and any fix SHALL CONTINUE TO treat the current "fatal crits are not recorded" behaviour as the baseline — tests SHALL NOT assert that a fatal crit produces a sidebar `InjuryRecord` (whether fatal blows should be recorded/displayed is a separate scope decision flagged in 4.x)

### Notes and Scope Flags

4.1 The exploration test SHALL target the pure `HitLocationTable::resolve` mapping (engine-free, deterministic over the full `1..100` roll domain) to determine whether `resolve` collapses or over-represents BODY, or is correct. This is the single pure seam in the melee crit path.
4.2 IF `HitLocationTable::resolve` is proven correct across the full `1..100` roll domain THEN the exploration test documents that the "always Body" symptom is a display/recording-path issue or a sampling artifact, and this Bug Analysis MUST be revised before a fix is designed.
4.3 The fact that fatal critical hits are never recorded (clause 1.3) MAY or MAY NOT be intended behaviour. It is documented here as current behaviour. Whether the fatal blow's location should be recorded and shown in the sidebar is a separate scope decision to confirm with the user; it is out of scope for the "always Body" fix unless the user opts in.

## Exploration Outcome (Task 1 result — resolves clause 4.2)

The Task 1 bug-condition exploration test was run against the current (unfixed) code:

- **Part A — Outcome (i) confirmed.** `HitLocationTable::resolve` is CORRECT across the
  entire `1..100` roll domain (exhaustive sweep + a RapidCheck property of ≥100 iterations,
  147 assertions, all passing; exactly 40 of 100 rolls resolve to BODY as intended by the
  Rogue Trader digit-reversal table). No counterexample was found, so per clause 4.2 the
  "always Body" symptom is NOT a `resolve` defect and the conditional Part A fix (tasks.md
  Task 4) makes NO change to `Source/HitLocation.cpp`. The observed "one line — Body" was a
  sampling artifact: the player had a single recorded non-fatal crit, and Body is the most
  common location (40% of the table).

- **Part B — the real defect.** Fatal critical hits were never recorded (the fatal branches
  called `die()` without pushing an `InjuryRecord`), so the killing blow's location never
  appeared in the sidebar/death screen. This is fixed by `InjuryTracker::recordFatalCrit`
  wired into the fatal branches of `Attacker::resolveCharacterAttack` and
  `RangedCombat::resolve` (tasks.md Task 3).
