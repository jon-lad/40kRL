# Requirements Document

## Introduction

NPC and player skills, talents, and traits are already **stored** on the `CareerProgression` component (`skills` map, `talents` set, `traits` vector) and parsed from Lua enemy definitions (`Scripts/Enemies.lua` via `populateStatBlockFromLua`). Today most of that data is **inert** — only two entries influence gameplay:

- **Dodge skill** → evasion reaction (`Source/ReactionResolver.cpp`: `computeDodgeTarget` / `dodgeSucceeds`)
- **Weapon Training talent** → melee proficiency penalty only (`Source/WeaponTypes.cpp`: `proficiencyModifier`, consumed in `Source/Attacker.cpp` `resolveCharacterAttack`)

This feature **wires the remaining stored-but-inert data into the combat and AI pipeline**, using the existing `Skill_Provider` pure-seam pattern (`Headers/StatBlock.hpp` / `Source/StatBlock.cpp`) so that all new mechanics remain engine-free, null-safe, and unit/property-testable per the test-isolation steering. Every mechanic defaults to the Rogue Trader RPG rules and cites the `Reference/` file and section per the rt-mechanics-fallback steering.

The scope covers five mechanics:

1. **Parry** — add the Parry skill-rank bonus and weapon Balanced / Unbalanced / Unwieldy qualities to the melee Parry reaction.
2. **Ranged Weapon Training** — apply the proficiency penalty to BS hit chance (currently only melee applies it — an existing gap).
3. **Awareness** — expose an Awareness-based surprise/initiative helper (flagging the missing surprise/initiative system as a dependency).
4. **Combat/AI traits** — Brutal Charge, Sturdy, Size(X), Mob Rule, Cowardly.
5. **Data & compatibility** — trait/quality vocabulary, Lua data, and backwards compatibility.

### Dependencies and Assumptions (called out explicitly)

- **A1 — Surprise/initiative system does not exist.** No current code path reads Awareness for surprise or initiative order. This spec introduces only a **pure helper** for the Awareness surprise test and its outcome; it does **not** introduce a turn-order/initiative scheduler. Wiring the helper into an actual surprise round is out of scope and flagged as a follow-on dependency (see Requirement 3).
- **A2 — Knockdown/prone-from-knockdown system is partial.** A `Prone` status exists (`StatusType::Prone`), but there is no dedicated "knockdown" event that Sturdy would suppress. This spec defines Sturdy as a pure predicate (`isImmuneToKnockdown`) and requires it to gate any knockdown application that exists or is later added; introducing a new knockdown source is out of scope and flagged (see Requirement 6).
- **A3 — Size wound-threshold effect.** RT-Bestiary lists Size as affecting both to-hit and wound threshold, but the concrete wound-threshold numbers are not tabulated in the Reference. Only the to-hit modifier is numerically specified (RT-Bestiary, Size Categories table). This spec wires the **to-hit modifier** (fully specified) and defines the wound-threshold effect as a flagged dependency requiring user input on exact values (see Requirement 7).
- **A4 — Trait string vocabulary.** Existing Lua data uses parenthesized, space-containing trait strings such as `"Size (Puny)"`, `"Brutal Charge"`, `"Mob Rule"`, `"Sturdy"`, `"Cowardly"`. New parsing/predicate helpers MUST accept the existing vocabulary without requiring a data migration.

## Glossary

- **Skill_Provider**: The set of free functions in `Headers/StatBlock.hpp` (`getSkillRank`, `hasSkill`, `hasTalent`) that read an actor's stat-block uniformly for player and NPC. All accessors are pure with respect to the engine and null-safe. New pure helpers introduced by this feature extend this seam.
- **Trait_Provider**: New pure, null-safe helpers (extending the Skill_Provider pattern) that answer questions about an actor's `CareerProgression::traits` vector — e.g. `hasTrait`, `getSizeCategory`, `brutalChargeBonus`, `isImmuneToKnockdown`.
- **Reaction_Resolver**: The evasion resolution logic in `Source/ReactionResolver.cpp` (`resolveReaction`) that offers Dodge/Parry as a Reaction.
- **Melee_Attack_Resolver**: `Attacker::resolveCharacterAttack` in `Source/Attacker.cpp`, which computes effective WS and applies melee damage.
- **Ranged_Attack_Resolver**: `RangedCombat::resolveCharacterAttack` in `Source/RangedCombat.cpp`, which computes effective BS.
- **Charge_Resolver**: `ChargeResolver::compute` in `Source/ChargeResolver.cpp` and the charge branches in `Source/Ai.cpp` (`PlayerAi::handleActionKey` case `'C'`, `MonsterAi::selectAndExecuteAction`).
- **Monster_AI**: `MonsterAi::selectAndExecuteAction` in `Source/Ai.cpp`, which chooses an NPC action each turn.
- **Skill Rank**: Integer stored per skill (0 = Trained, 1 = +10, 2 = +20), sentinel `SKILL_UNTRAINED = -1` when absent. Bonus in to-hit points = `rank * 10` when the skill is present.
- **Parry**: A Weapon Skill test taken as a Reaction to negate one melee hit; requires an equipped melee weapon (RT-CoreMechanics §7).
- **Balanced / Unbalanced / Unwieldy**: Melee weapon qualities affecting Parry: Balanced = +10 to Parry, Unbalanced = -10 to Parry, Unwieldy = cannot Parry (RT-CoreMechanics §11; RT-Weapons quality lists).
- **Proficiency Penalty**: The -20 modifier applied to a weapon attack when the attacker lacks the matching `Weapon Training (<group>)` talent (RT-CoreMechanics §3 "Untrained skill penalty: -20"; existing `proficiencyModifier`).
- **Brutal Charge**: A trait granting +3 damage on a Charge attack (RT-Bestiary, Creature Traits Reference).
- **Sturdy**: A trait that makes a creature immune to being knocked down (RT-Bestiary, Creature Traits Reference).
- **Size Category**: One of Puny, Scrawny, Average, Hulking, Enormous, Massive, with an associated to-hit modifier (RT-Bestiary, Size Categories table).
- **Mob Rule**: An Ork trait; in the tabletop rules a mob of Orks gains resilience/morale while numerous. For this roguelike adaptation it is modelled as an AI-behaviour trait (see Requirement 8) — flagged because the Reference lists the trait name but does not give a fully specified solo-combat rule.
- **Cowardly**: An AI-behaviour trait causing an actor to flee when threatened (roguelike adaptation; not a tabletop-specified numeric rule — flagged in Requirement 8).
- **Surprise Test**: An Awareness (Perception-based) test to avoid being surprised; a surprised character loses its first turn and reactions, and a non-surprised attacker gains +30 to hit a surprised target (RT-CoreMechanics §4).

## Requirements

### Requirement 1: Parry skill-rank bonus in the Parry reaction

**User Story:** As a player facing skilled melee opponents, I want the Parry reaction to account for the defender's Parry skill rank, so that trained fighters parry more reliably per Rogue Trader rules.

RT reference: RT-CoreMechanics §7 (Parry is a WS test as a Reaction to negate one melee hit); RT-CoreMechanics §3 (Trained/+10/+20 skill advances map to +0/+10/+20).

#### Acceptance Criteria

1. THE Reaction_Resolver SHALL compute the Parry target number as the sum of the defender's WeaponSkill value and a `parryBonus`, WHERE `parryBonus` is produced by a helper that returns a defined integer value for every input including a null or absent defender (returning `parryBonus = 0` when the defender reference is null).
2. WHERE the defender has the Parry skill at rank R (R in the integer set {0, 1, 2}), THE Reaction_Resolver SHALL set `parryBonus` to `R * 10`, yielding +0 at rank 0, +10 at rank 1, and +20 at rank 2.
3. IF the defender does not have the Parry skill, THEN THE Reaction_Resolver SHALL set `parryBonus` to -20 (untrained skill penalty, RT-CoreMechanics §3), and SHALL NOT also apply the rank-based bonus from criterion 2.
4. THE Reaction_Resolver SHALL clamp the final Parry target number to the inclusive range [0, 100], such that any computed value below 0 becomes exactly 0 and any computed value above 100 becomes exactly 100.
5. WHEN the defender rolls a d100 for a Parry test and the roll equals 1, THE Reaction_Resolver SHALL treat the Parry as a success regardless of the clamped Parry target number (auto-success, RT-CoreMechanics §1).
6. WHEN the defender rolls a d100 for a Parry test and the roll equals 100, THE Reaction_Resolver SHALL treat the Parry as a failure regardless of the clamped Parry target number (auto-fail, RT-CoreMechanics §1).
7. WHEN the d100 roll is an integer in the inclusive range [2, 99], THE Reaction_Resolver SHALL treat the Parry as a success IF the roll is less than or equal to the clamped Parry target number, and as a failure IF the roll is greater than the clamped Parry target number.
8. WHEN a Parry is treated as a success, THE Reaction_Resolver SHALL return `ReactionResult::NEGATED`.
9. WHEN a Parry is treated as a failure, THE Reaction_Resolver SHALL return `ReactionResult::FAILED`.

### Requirement 2: Weapon quality effects on Parry (Balanced / Unbalanced / Unwieldy)

**User Story:** As a player, I want weapon quality to matter when parrying, so that a balanced blade helps and an unwieldy weapon cannot parry, per Rogue Trader rules.

RT reference: RT-CoreMechanics §11 (Balanced = +10 to Parry, Unbalanced = -10 to Parry, Unwieldy = cannot Parry); RT-Weapons melee quality lists. Weapon qualities are stored in `MeleeStats::qualities` (`std::vector<std::string>`, `Headers/Equippable.hpp`).

#### Acceptance Criteria

1. THE Reaction_Resolver SHALL derive a Parry weapon-quality modifier from the equipped melee weapon's `qualities` list using a pure, null-safe helper function.
2. WHERE the equipped melee weapon has the "Balanced" quality, THE Reaction_Resolver SHALL add +10 to the Parry target number.
3. WHERE the equipped melee weapon has the "Unbalanced" quality, THE Reaction_Resolver SHALL add -10 to the Parry target number.
4. WHERE the equipped melee weapon has the "Unwieldy" quality, THE Reaction_Resolver SHALL make Parry unavailable so that the defender cannot choose Parry as a reaction, and this SHALL take precedence over the presence of any other quality (including "Balanced" or "Unbalanced").
5. WHERE the equipped melee weapon has both "Balanced" and "Unbalanced" qualities (and not "Unwieldy"), THE Reaction_Resolver SHALL apply a net weapon-quality modifier of 0 (the +10 and -10 cancel).
6. WHERE a defender has no equipped melee weapon, THE Reaction_Resolver SHALL make Parry unavailable (preserving existing `hasEquippedMeleeWeapon` behaviour).
7. WHERE Parry is available, THE Reaction_Resolver SHALL compute the Parry target number as `WeaponSkill + parryBonus + weaponQualityModifier` and then clamp the result to the inclusive range [0, 100], WHERE `parryBonus` is the skill-rank/untrained value from Requirement 1 and `weaponQualityModifier` is the sum of the applicable Balanced (+10) and Unbalanced (-10) contributions.
8. WHERE Parry is unavailable AND Dodge is available, THE Reaction_Resolver SHALL select Dodge for an AI defender.
9. WHERE both Parry and Dodge are unavailable, THE Reaction_Resolver SHALL return `ReactionResult::NO_REACTION`.

### Requirement 3: Awareness-based surprise test helper

**User Story:** As a developer, I want an Awareness-driven surprise test helper, so that a future surprise/initiative system can determine which combatants are surprised and apply the RT surprise bonus.

RT reference: RT-CoreMechanics §4 (Surprise: Awareness test to avoid; surprised characters lose first turn and reactions; non-surprised attacker vs surprised target: +30 to WS/BS). See dependency A1.

#### Acceptance Criteria

1. THE Skill_Provider SHALL provide a pure, null-safe helper that computes a surprise-avoidance target number for an actor as `Perception + awarenessModifier`, returning a defined value for a null actor or an actor with no `CareerProgression` (treated as untrained, awarenessModifier = -20).
2. WHERE the actor has the Awareness skill at rank R (R in {0, 1, 2}), THE surprise helper SHALL set `awarenessModifier` to `R * 10` (+0 / +10 / +20).
3. IF the actor does not have the Awareness skill, THEN THE surprise helper SHALL set `awarenessModifier` to -20 (untrained skill penalty, RT-CoreMechanics §3) and SHALL NOT also apply the rank-based bonus from criterion 2.
4. THE surprise helper SHALL clamp the surprise-avoidance target number to the inclusive range [0, 100], such that any value below 0 becomes 0 and any value above 100 becomes 100.
5. WHEN a d100 roll in the inclusive range [2, 99] is supplied to the surprise helper, THE surprise helper SHALL report the actor as "not surprised" IF the roll is less than or equal to the clamped target number, and "surprised" otherwise.
6. WHEN a d100 roll equal to 1 is supplied, THE surprise helper SHALL report "not surprised"; WHEN a d100 roll equal to 100 is supplied, THE surprise helper SHALL report "surprised" (auto outcomes, RT-CoreMechanics §1).
7. THE feature SHALL expose the surprise bonus value of +30 (RT-CoreMechanics §4) as a named constant so that a future surprise round can apply it to a non-surprised attacker's WS/BS against a surprised target.
8. IF no surprise/initiative turn-order system is present in the codebase, THEN THE feature SHALL document in the design that wiring the surprise helper into an actual surprise round is a follow-on dependency, AND THE feature SHALL NOT introduce a turn-order scheduler in this spec (dependency A1).

### Requirement 4: Ranged Weapon Training proficiency penalty on BS

**User Story:** As a player, I want untrained ranged weapon use to suffer the same proficiency penalty as melee, so that Weapon Training talents matter for shooting and the existing melee/ranged inconsistency is fixed.

RT reference: RT-CoreMechanics §3 (Untrained skill penalty: -20); existing `proficiencyModifier` / `hasProficiency` in `Source/WeaponTypes.cpp`. This closes an existing gap where only `Melee_Attack_Resolver` applies the penalty.

#### Acceptance Criteria

1. WHEN the Ranged_Attack_Resolver resolves an attack with an equipped ranged weapon that has a weapon-group classification, THE Ranged_Attack_Resolver SHALL derive a proficiency modifier for that weapon group via `proficiencyModifier(shooter, weaponGroup)` and include it in the effective BS computation.
2. WHERE the shooter lacks the matching `Weapon Training (<group>)` talent for the equipped ranged weapon's group, THE derived proficiency modifier SHALL be -20.
3. WHERE the shooter has the matching `Weapon Training (<group>)` talent for the equipped ranged weapon's group, THE derived proficiency modifier SHALL be 0.
4. WHERE the equipped ranged weapon has no weapon-group classification (or no weapon is equipped), THE Ranged_Attack_Resolver SHALL use a proficiency modifier of 0 (no penalty).
5. THE Ranged_Attack_Resolver SHALL compute effective BS as `baseBS + attackerModifierSum + aimBonus + proficiencyModifier` and then clamp the result to the inclusive range [1, 99].
6. WHEN the Melee_Attack_Resolver computes effective WS, THE Melee_Attack_Resolver SHALL include the same proficiency modifier (-20 when untrained, 0 when trained) for the equipped melee weapon's group, so that melee proficiency behaviour is identical to that produced before this feature.

### Requirement 5: Brutal Charge trait damage bonus

**User Story:** As a player fighting beasts and Orks with Brutal Charge, I want their charge attacks to hit harder, so that charging creatures are appropriately dangerous per Rogue Trader rules.

RT reference: RT-Bestiary, Creature Traits Reference ("Brutal Charge | +3 damage on Charge attacks").

#### Acceptance Criteria

1. THE Trait_Provider SHALL provide a pure, null-safe helper `brutalChargeBonus(actor)` that returns 3 WHERE the actor has the "Brutal Charge" trait and 0 in all other cases, including a null actor, an actor with no `CareerProgression`, and an actor whose `traits` do not contain "Brutal Charge".
2. A "charge attack" for the purposes of this requirement SHALL mean a melee attack resolved via the Charge_Resolver charge path (as opposed to a standard melee attack resolved directly by the Melee_Attack_Resolver).
3. WHEN the Charge_Resolver resolves a charge attack, THE Charge_Resolver SHALL add `brutalChargeBonus(attacker)` to the pre-floor final melee damage of that charge attack (adding 3 when the attacker has "Brutal Charge", and 0 otherwise).
4. WHEN the Melee_Attack_Resolver resolves a standard (non-charge) melee attack, THE Melee_Attack_Resolver SHALL NOT add the Brutal Charge damage bonus, even WHERE the attacker has the "Brutal Charge" trait.
5. THE Charge_Resolver SHALL enforce a minimum final damage of 0 after all additions and reductions, so that no charge attack deals negative damage (RT-CoreMechanics §5, "If result ≤ 0: no damage dealt").

### Requirement 6: Sturdy trait — immunity to knockdown

**User Story:** As a developer, I want the Sturdy trait to prevent affected creatures from being knocked down, so that Orks and beasts stay on their feet per Rogue Trader rules.

RT reference: RT-Bestiary, Creature Traits Reference ("Sturdy | Cannot be knocked down"). See dependency A2.

#### Acceptance Criteria

1. THE Trait_Provider SHALL provide a pure predicate `isImmuneToKnockdown(actor)` that returns true WHERE the actor has the "Sturdy" trait and returns false in all other cases, including a null actor, an actor with no `CareerProgression`, and an actor whose `traits` do not contain "Sturdy".
2. THE `isImmuneToKnockdown` predicate SHALL be side-effect-free: calling it SHALL NOT modify the actor or any other state, and repeated calls with the same input SHALL return the same result (idempotent).
3. IF a knockdown effect would be applied to an actor for which `isImmuneToKnockdown` returns true, THEN the applying code SHALL NOT apply the knockdown, so that the actor remains standing (no state transition).
4. WHERE an actor is targeted by a knockdown effect and `isImmuneToKnockdown` returns false, THE knockdown-applying code SHALL apply the knockdown, transitioning the actor to the knocked-down state.
5. IF the codebase contains no distinct knockdown source that Sturdy would gate, THEN THE design SHALL document that introducing a knockdown source is a follow-on dependency, AND THE feature SHALL still provide the `isImmuneToKnockdown` predicate ready for that future source to consult (dependency A2).

### Requirement 7: Size(X) trait — to-hit modifier and wound-threshold effect

**User Story:** As a player, I want a target's size to affect how easily it is hit, so that large creatures are easier to hit and tiny ones harder, per Rogue Trader rules.

RT reference: RT-Bestiary, Size Categories table (Puny -30, Scrawny -10, Average +0, Hulking +10, Enormous +20, Massive +30, as "Modifier to Hit" applied to the attacker against that target). See dependency A3 for the wound-threshold portion.

#### Acceptance Criteria

1. THE Trait_Provider SHALL provide a pure, null-safe helper `getSizeCategory(actor)` that parses an actor's Size trait string of the form `"Size (<Category>)"` (e.g. `"Size (Puny)"`, `"Size (Hulking)"`) into a size category, matching the `<Category>` token exactly against the known category names.
2. WHERE an actor has no Size trait, is null, or has no `CareerProgression`, THE `getSizeCategory` helper SHALL report the size category as Average.
3. THE Trait_Provider SHALL provide a pure helper `sizeToHitModifier(category)` that maps a size category to its to-hit modifier: Puny -30, Scrawny -10, Average +0, Hulking +10, Enormous +20, Massive +30 (RT-Bestiary, Size Categories table).
4. WHEN the Melee_Attack_Resolver computes effective WS against a target, THE Melee_Attack_Resolver SHALL add `sizeToHitModifier(getSizeCategory(target))` to the attacker's effective WS before the [1, 99] clamp.
5. WHEN the Ranged_Attack_Resolver computes effective BS against a target, THE Ranged_Attack_Resolver SHALL add `sizeToHitModifier(getSizeCategory(target))` to the shooter's effective BS before the [1, 99] clamp.
6. IF a Size trait string is present but its `<Category>` token does not match a known size category (case-sensitively), THEN THE `getSizeCategory` helper SHALL report the size category as Average, so that unrecognized data does not alter to-hit.
7. THE feature SHALL treat the Size wound-threshold effect as a flagged dependency (A3): because the Reference does not tabulate concrete wound-threshold numbers per size, THE design SHALL request explicit values from the user before implementing any wound-threshold change, AND THE feature SHALL NOT alter wound thresholds until those values are provided.

### Requirement 8: Mob Rule and Cowardly AI-behaviour traits

**User Story:** As a player, I want Orks to fight more aggressively in groups and cowardly enemies to flee, so that enemy behaviour reflects their traits.

RT reference: RT-Bestiary, Ork profiles list "Mob Rule" as an Ork trait; the Creature Traits Reference does not give a fully specified solo-combat numeric rule, so the roguelike adaptation below is flagged for user confirmation. "Cowardly" is a roguelike-adaptation behaviour trait not tabulated in the Reference.

#### Acceptance Criteria

1. THE Trait_Provider SHALL provide a general pure predicate `hasTrait(actor, name)` that returns true WHERE `CareerProgression::traits` contains an exact match for `name`, and returns false in all other cases, including a null actor, an actor with no `CareerProgression`, and an empty `traits` list (used for "Mob Rule" and "Cowardly").
2. WHEN the Monster_AI evaluates a Cowardly actor's turn AND the actor's current wounds are at or below the configured flee threshold AND the actor is not emboldened by Mob Rule, THE Monster_AI SHALL select a move action that increases the actor's distance from the player, using a pure decision helper that takes the actor's state and returns the intended action.
3. WHEN the Monster_AI evaluates a Cowardly actor's turn AND the actor's current wounds are above the configured flee threshold, THE Monster_AI SHALL select the same action it would select for an otherwise-identical actor without the "Cowardly" trait (approach and attack).
4. WHILE an actor with the "Mob Rule" trait has at least the configured nearby-allies count of allied actors within the configured radius, THE Monster_AI decision helper SHALL report the actor as emboldened, suppressing flee behaviour and preferring to attack.
5. THE Monster_AI trait-driven decision SHALL be computed by a pure, engine-free helper (extending the Skill_Provider/Trait_Provider seam) that is unit-testable without initializing the Engine, WHERE the `Source/Ai.cpp` layer is responsible only for reading actor state, calling the helper, and executing the chosen action.
6. THE feature SHALL flag the exact flee threshold and the nearby-allies count and radius as design decisions requiring user confirmation, because the Reference does not specify solo-combat numbers for these traits, AND THE design document SHALL record the chosen values with rationale before implementation.

### Requirement 9: Trait/skill vocabulary parsing and data compatibility

**User Story:** As a developer, I want the new helpers to read the existing Lua trait/skill/talent vocabulary unchanged, so that no data migration is required and existing enemies work immediately.

RT reference: N/A (data-format requirement). Existing data: `Scripts/Enemies.lua`, parsed by `populateStatBlockFromLua` (`Source/StatBlock.cpp`).

#### Acceptance Criteria

1. THE Trait_Provider SHALL compare a queried trait name against each string in `CareerProgression::traits` using exact byte-for-byte string equality (case- and whitespace-sensitive), reporting a match only on exact equality and no match otherwise, accepting the existing vocabulary including embedded spaces and parentheses (e.g. `"Brutal Charge"`, `"Mob Rule"`, `"Sturdy"`, `"Cowardly"`, `"Size (Puny)"`).
2. WHEN the Skill_Provider is queried for the "Dodge", "Parry", or "Awareness" skill, THE Skill_Provider SHALL return the stored rank (0, 1, or 2) from the `skills` map when present, and the sentinel `SKILL_UNTRAINED` when absent, unchanged from existing `getSkillRank` / `hasSkill` semantics.
3. WHEN `populateStatBlockFromLua` parses an enemy entry, THE parser SHALL store skills, talents, and traits into the same keys, values, and data structures it uses today, requiring zero changes to existing Lua files.
4. WHERE an enemy's Lua entry declares one of the wired traits ("Brutal Charge", "Sturdy", "Size (<Category>)", "Mob Rule", "Cowardly"), THE feature SHALL make that trait mechanically active without requiring any new Lua field.
5. WHEN `Scripts/Enemies.lua` is loaded at startup, THE feature SHALL load and spawn every enemy referencing a wired trait without a parse error.
6. IF a trait string stored on an actor does not match any wired trait name, THEN THE feature SHALL treat that trait as mechanically inactive (no effect) while still allowing the actor to load and spawn normally.

### Requirement 10: Engine-free, testable design and backwards compatibility (NFRs)

**User Story:** As a maintainer, I want all new logic to be pure and testable and to preserve existing behaviour and saves, so that the change is safe and follows project conventions.

RT reference: N/A (project conventions — test-isolation, tdd-workflow, versioning steering).

#### Acceptance Criteria

1. THE feature SHALL implement each of the following new numeric formulas and decisions as a pure, null-safe helper function that performs zero `engine.*` member access (no `engine.gui`, `engine.map`, `engine.player`, or any other engine member): Parry bonus, weapon-quality modifier, surprise target, Brutal Charge bonus, Size modifier, and AI decision, so that each helper is unit- and property-testable without initializing the Engine (test-isolation steering).
2. WHEN the test suite is executed, THE feature SHALL provide at least one Catch2 v3 test asserting a specific expected value for each of the six helpers named in criterion 1, and SHALL provide for each of the six helpers at least one RapidCheck property-based test that runs at least 100 iterations (tdd-workflow steering), WHERE any `rc::gen::inRange(a, b)` bound is treated as the inclusive interval [a, b].
3. THE feature SHALL preserve the existing serialized field set and field order of the `CareerProgression`, `Attacker`, and `Equippable` save/load format unchanged, so that any save file produced by the immediately preceding released version loads successfully with identical component state and without a MAJOR version bump (versioning steering).
4. WHEN the existing Dodge evasion reaction (`computeDodgeTarget` / `dodgeSucceeds`) and the melee Weapon Training proficiency penalty (`proficiencyModifier` consumed in `resolveCharacterAttack`) are exercised with any input, THE feature SHALL return values identical to those returned before this feature was introduced, for every input.
5. WHERE an actor has no `CareerProgression` component, THE new helpers SHALL treat the actor as untrained (no skills, no talents, no traits) and return these neutral values: no proficiency/skill bonus (numeric zero), no talent bonus (numeric zero), Average size, not immune to knockdown, and no AI decision change.
6. IF a numeric input passed to any helper named in criterion 1 falls outside its defined valid range, THEN THE helper SHALL clamp that input to the nearest valid bound before computing, and SHALL return a value within the helper's defined output range without throwing or accessing `engine.*`.
7. WHERE any new `.cpp` source file is added, THE feature SHALL register that file in both `40kRL.vcxproj` and `Tests/40kRL_Tests.vcxproj`, so that the test build links with zero unresolved external symbol errors (test-project steering).

## Round-Trip / Property Notes (for the design and test phase)

- **Parry target monotonicity (metamorphic):** For fixed WS and weapon quality, increasing the Parry skill rank never decreases the clamped Parry target number.
- **Weapon-quality symmetry (invariant):** Applying "Balanced" and "Unbalanced" together yields the same Parry target as applying neither.
- **Clamp invariant:** All computed target numbers (Parry, surprise) lie in [0, 100]; all effective WS/BS lie in [1, 99].
- **Size default invariant:** An actor with no Size trait, an empty trait list, or an unrecognized Size string always resolves to Average (+0), so unknown data never changes to-hit.
- **Proficiency parity (model-based):** For the same weapon group and talent state, the ranged proficiency modifier equals the melee proficiency modifier (both use `proficiencyModifier`).
- **Null-safety invariant:** Every new helper returns its neutral value for a null actor or an actor with no career.
