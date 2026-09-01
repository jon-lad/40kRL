# Requirements Document

## Introduction

This feature makes NPCs full combatants that share the same stat/skill/talent representation
as the player, so that combat resolution treats NPCs and the player identically based on
characteristics, skills, talents, and equipment. The single mechanical exception is that NPCs
never earn experience or purchase advances — an NPC's profile is fixed at spawn, mirroring how
the Rogue Trader RPG presents bestiary creature profiles (per RT-Bestiary "Stat Block Format").

Today NPCs carry only a standalone `Characteristics` component (nine stats parsed from
`Enemies.lua`). They hold no skills and no talents. Two consequences follow:

- The proficiency check (`hasProficiency` / `proficiencyModifier` in `WeaponTypes.cpp`) reads
  `actor->career` (a `CareerProgression`, which NPCs lack), so every NPC always eats the -20
  unproficient weapon penalty regardless of the creature's intended training. This is a
  fairness gap.
- Evasion (`ReactionResolver.cpp`) currently rolls a raw Agility test with no skill or training
  distinction, for both player and NPC.

This spec delivers three outcomes:

1. Implement the **Dodge skill** mechanic now, for player and NPCs equally, resolved through one
   shared code path (per RT-CoreMechanics §7 Evasion and §3 skill test rules).
2. Give NPCs a way to **store skills, talents, and traits** (parsed from optional `skills`,
   `talents`, and `traits` sections in `Enemies.lua`) using a representation shared with the
   player. Wire the **Weapon Training (<Group>)** talent — in the code's exact string format —
   into the existing proficiency check so trained NPCs no longer eat the -20 penalty. All other
   stored skills, talents, and traits remain mechanically inert this spec (tracked as a roadmap
   note); traits are recorded but inert, mirroring the player's inert traits vector.
3. Preserve the invariant that **NPCs never gain XP or advances** — their profile is fixed at
   spawn.

### Reference Basis

- **Dodge as a skill test**: RT-CoreMechanics §7 (Evasion) — "Dodge: Agility test as a Reaction
  to negate one hit ... Each DoS beyond the first negates one additional hit." Dodge is treated
  as an Agility-based skill. The effective target for the roll-under test is Agility plus the
  Dodge skill-rank bonus.
- **Untrained skill penalty**: RT-CoreMechanics §6.1 — "Untrained skill penalty: -20 to the
  governing characteristic." The Reference defines the untrained rule as a **-20 modifier to the
  characteristic**, NOT a "half characteristic" rule. An Actor with no Dodge skill therefore
  tests against Agility - 20. (Decision locked: untrained Dodge = Agility - 20; the
  half-Agility variant is not adopted.)
- **Creature skills/talents**: RT-Bestiary "Stat Block Format" and Ork profiles (e.g., Ork Boy
  Talents include "Basic Weapon Training (SP)", "Melee Weapon Training (Primitive)") confirm
  NPCs carry Skills and Talents lists in their profiles.
- **Talent naming**: `Enemies.lua` talent strings SHALL use the code's exact format
  `Weapon Training (<Group>)`, where `<Group>` is a `weaponGroupName` value from
  `WeaponTypes.cpp` (one of: Las, SP, Bolt, Melta, Plasma, Flame, Primitive, Launcher, Exotic).
  Examples: `Weapon Training (Primitive)`, `Weapon Training (SP)`. This matches the literal
  string that `hasProficiency` constructs, so no name translation layer is required.

### Resolved Decisions

The following previously-open questions are now decided and are reflected in the requirements
above:

1. **Dodge mechanic (in scope):** Dodge is a d100 roll-under test against an effective target of
   Agility plus the Dodge skill-rank bonus (Rank 0 = +0, Rank 1 = +10, Rank 2 = +20), or
   Agility - 20 when untrained (RT-CoreMechanics §7 and §6.1). The same shared code path applies
   to Player and NPCs. Parry retains its current WS-based behaviour; a Parry skill bonus is
   deferred (Requirement 8).
2. **Traits (stored but inert):** NPC stat blocks MAY store a list of trait names parsed from an
   optional Lua `traits` section, recorded but mechanically inert (mirroring the player's inert
   traits vector), for future use. Omitting the section yields an empty trait list (backward
   compatible).
3. **Talent naming (standardized):** `Enemies.lua` weapon-training talent strings use the code's
   exact `Weapon Training (<Group>)` format, where `<Group>` is a `weaponGroupName` value
   (Las, SP, Bolt, Melta, Plasma, Flame, Primitive, Launcher, Exotic).

## Glossary

- **Actor**: Any entity on the map. Combat capability is defined by optional components
  (`attacker`, `characteristics`, etc.).
- **NPC**: A non-player Actor spawned from an `Enemies.lua` entry (has an `ai` component and is
  not `Engine.player`).
- **Player**: The single player-controlled Actor (`Engine.player`).
- **Characteristics**: The existing component holding the nine Rogue Trader characteristics
  (WS, BS, S, T, Ag, Int, Per, WP, Fel), each in [1, 99]. Bonus = value / 10.
- **Skill_Rank**: Integer proficiency level for a named skill. 0 = Trained, 1 = +10, 2 = +20,
  following the existing `CareerProgression::skills` convention. Absence of a skill entry means
  Untrained.
- **Talent**: A named capability held in a set (e.g., "Weapon Training (Primitive)"). Presence
  is boolean.
- **Trait**: A named creature attribute held in a list (e.g., "Sturdy", "Brutal Charge") parsed
  from an optional `traits` section. Traits are recorded but mechanically inert this feature,
  mirroring the player's inert traits vector, and are reserved for future work.
- **Skill_Provider**: The shared accessor through which combat code reads an Actor's Skill_Rank
  for a named skill, used identically for the Player and NPCs. The Dodge_Resolver reads the
  Dodge Skill_Rank through the Skill_Provider.
- **Stat_Block**: The combined characteristics + skills + talents profile of an Actor. This spec
  requires a representation shared by player and NPC; the exact component is a design decision
  (see Requirement 6).
- **Dodge_Resolver**: The single code path that resolves a Dodge reaction for any Actor.
- **Enemy_Loader**: The `addActor` Lua callback in `Map.cpp` that constructs an NPC Actor from an
  `Enemies.lua` entry.
- **Proficiency_Check**: The `hasProficiency` / `proficiencyModifier` utility in `WeaponTypes.cpp`
  that determines the weapon-group training penalty.
- **Weapon_Training_Talent**: A talent named `Weapon Training (<Group>)` where `<Group>` is a
  `weaponGroupName` value (one of Las, SP, Bolt, Melta, Plasma, Flame, Primitive, Launcher,
  Exotic), e.g., "Weapon Training (Bolt)". This matches the literal string constructed by
  `hasProficiency` in `WeaponTypes.cpp`.
- **DoS**: Degrees of Success = (Target Number - Roll) / 10, rounded down, minimum 1 on success
  (RT-CoreMechanics §1).

## Requirements

### Requirement 1: Evasion and Dodge Skill Resolution (Player and NPC Parity)

**User Story:** As a player, I want Dodge resolved as an Agility-based skill test that accounts
for Dodge training, applied identically to my character and to NPCs, so that both evade attacks
using the same concrete formula and fair rules.

The Dodge mechanic is **in scope** this feature (not merely parity): the Dodge_Resolver
implements the concrete d100 roll-under test defined below, per RT-CoreMechanics §7 (Evasion)
and RT-CoreMechanics §6.1 (untrained skill penalty -20).

#### Acceptance Criteria

1. WHEN an Actor with a `Characteristics` component performs a Dodge reaction, THE Dodge_Resolver
   SHALL resolve it as a d100 roll-under test against an effective target number, treating the
   reaction as successful when the roll is less than or equal to that target number (per
   RT-CoreMechanics §7).
2. WHERE an Actor has the Dodge skill, THE Dodge_Resolver SHALL compute the effective target
   number as the Actor's Agility value plus the Dodge skill-rank bonus, where the bonus is
   Rank 0 (Trained) = +0, Rank 1 = +10, and Rank 2 = +20.
3. WHERE an Actor does not have the Dodge skill at any rank (untrained), THE Dodge_Resolver SHALL
   compute the effective target number as the Actor's Agility value minus 20 (per
   RT-CoreMechanics §6.1).
4. THE Dodge_Resolver SHALL read the Dodge Skill_Rank through the shared Skill_Provider for both
   the Player and NPCs.
5. THE Dodge_Resolver SHALL resolve Dodge for the Player and for NPCs through one shared code
   path with no branch that varies the effective-target-number formula by Actor type, so that
   identical Agility and Dodge rank yield identical thresholds for Player and NPC.
6. WHEN a d100 roll is 1, THE Dodge_Resolver SHALL treat the Dodge as successful regardless of
   the effective target number (auto-success, RT-CoreMechanics §1).
7. WHEN a d100 roll is 100, THE Dodge_Resolver SHALL treat the Dodge as failed regardless of the
   effective target number (auto-fail, RT-CoreMechanics §1).
8. THE Dodge_Resolver SHALL clamp the effective target number to the range [0, 100] before
   comparing against the roll.
9. WHERE an Actor performs a Parry reaction, THE Dodge_Resolver's sibling Parry path SHALL retain
   its current Weapon-Skill-based roll-under behaviour unchanged; the Parry path SHALL resolve
   identically for the Player and NPCs. (Only Dodge receives the new skill-bonus mechanic this
   feature; a Parry skill-rank bonus is deferred to future work — see Requirement 8.)

### Requirement 2: NPC Skill and Talent Storage

**User Story:** As a designer, I want NPCs to carry skills and talents in the same form the
player uses, so that combat code can read an NPC's profile identically to the player's.

#### Acceptance Criteria

1. THE Stat_Block SHALL provide a mapping from skill name to Skill_Rank for any Actor that has a
   Stat_Block.
2. THE Stat_Block SHALL provide a set of talent names for any Actor that has a Stat_Block.
3. WHEN combat code reads an Actor's skills or talents, THE system SHALL expose them for the
   Player and for NPCs through the same accessors.
4. WHERE an Actor has no entry for a given skill name, THE system SHALL treat that skill as
   Untrained.
5. WHERE an Actor does not have a given talent name in the talent set, THE system SHALL treat
   that talent as absent.
6. THE Stat_Block MAY store a list of Trait names for any Actor that has a Stat_Block, recording
   them as mechanically inert data (mirroring the Player's inert traits vector) reserved for
   future use.
7. WHERE an Actor has no stored traits, THE system SHALL treat the Actor's trait list as empty.

### Requirement 3: Parse Skills and Talents from Enemies.lua

**User Story:** As a designer, I want to declare `skills` and `talents` on enemy entries in
`Enemies.lua`, so that spawned NPCs carry the intended profile.

#### Acceptance Criteria

1. WHERE an `Enemies.lua` entry contains a `skills` table mapping skill names to integer ranks,
   THE Enemy_Loader SHALL populate the spawned NPC's skill mapping with each name and rank.
2. WHERE an `Enemies.lua` entry contains a `talents` table listing talent name strings, THE
   Enemy_Loader SHALL add each talent name to the spawned NPC's talent set.
3. WHERE an `Enemies.lua` entry omits the `skills` section, THE Enemy_Loader SHALL spawn the NPC
   with an empty skill mapping.
4. WHERE an `Enemies.lua` entry omits the `talents` section, THE Enemy_Loader SHALL spawn the NPC
   with an empty talent set.
5. IF a `skills` entry has a rank value less than 0, THEN THE Enemy_Loader SHALL clamp the stored
   Skill_Rank to 0.
6. WHERE an `Enemies.lua` entry contains a `traits` table listing trait name strings, THE
   Enemy_Loader SHALL record each trait name in the spawned NPC's trait list as mechanically
   inert data (no combat effect this feature).
7. WHERE an `Enemies.lua` entry omits the `traits` section, THE Enemy_Loader SHALL spawn the NPC
   with an empty trait list.
8. WHERE an `Enemies.lua` entry supplies a `talents` table, THE Enemy_Loader SHALL treat each
   weapon-training talent string in the code's exact `Weapon Training (<Group>)` format, where
   `<Group>` is a `weaponGroupName` value (Las, SP, Bolt, Melta, Plasma, Flame, Primitive,
   Launcher, Exotic), so that stored talent strings match the string `hasProficiency`
   constructs.
9. THE Enemy_Loader SHALL spawn every existing `Enemies.lua` entry that lacks `skills`,
   `talents`, and `traits` sections without error and with combat behaviour unchanged except for
   the proficiency correction defined in Requirement 4.

### Requirement 4: Weapon Training Proficiency for NPCs

**User Story:** As a player, I want NPCs that are trained with their weapons to hit without the
untrained penalty, so that trained NPC combatants fight at their intended effectiveness.

#### Acceptance Criteria

1. WHEN the Proficiency_Check evaluates an Actor for a given WeaponGroup, THE Proficiency_Check
   SHALL report proficiency when the Actor's talent set contains the Weapon_Training_Talent
   whose group name matches that WeaponGroup's display name.
2. WHEN the Proficiency_Check evaluates an Actor whose talent set does not contain the matching
   Weapon_Training_Talent, THE Proficiency_Check SHALL apply the -20 unproficient penalty.
3. THE Proficiency_Check SHALL read talents from the shared Stat_Block for both the Player and
   NPCs through the same accessor.
4. WHERE an NPC has the matching Weapon_Training_Talent for its equipped weapon's group, THE
   Proficiency_Check SHALL return a proficiency modifier of 0 for that NPC's attack.

### Requirement 5: NPC Profiles Are Fixed at Spawn

**User Story:** As a designer, I want NPC profiles fixed at spawn, so that NPCs never advance
like the player and remain balanced to their bestiary profile.

#### Acceptance Criteria

1. THE system SHALL NOT grant experience points to any NPC. (Error-handling exception to the
   positive-statement rule: this is an explicit invariant.)
2. THE system SHALL NOT apply characteristic, skill, or talent advances to any NPC after spawn.
3. WHILE combat is resolved, THE system SHALL leave every NPC's characteristics, skill mapping,
   and talent set unchanged except for temporary modifiers already defined by other systems
   (for example status-effect or injury modifiers).

### Requirement 6: Shared Stat-Block Representation

**User Story:** As a developer, I want the player and NPCs to share one stat-block
representation, so that combat code paths do not diverge by Actor type.

#### Acceptance Criteria

1. THE system SHALL represent an Actor's characteristics, skill mapping, talent set, and trait
   list in a form that combat code accesses identically for the Player and for NPCs.
2. WHERE an NPC uses the shared representation, THE system SHALL NOT expose XP-earning or advance
   -purchasing behaviour on that NPC (consistent with Requirement 5).
3. THE design phase SHALL decide whether NPCs reuse the existing `CareerProgression` structure
   (skills map + talents set, never advanced) or a new lighter-weight component, and SHALL
   document that decision.

### Requirement 7: Persistence of NPC Skills and Talents

**User Story:** As a player, I want NPC profiles to survive save and load, so that a reloaded
game keeps each NPC's fixed skills and talents.

#### Acceptance Criteria

1. WHEN an NPC with skills, talents, or traits is saved, THE system SHALL write the NPC's skill
   mapping, talent set, and trait list to the save data.
2. WHEN a saved NPC is loaded, THE system SHALL restore the NPC's skill mapping, talent set, and
   trait list to the values present at save time.
3. FOR ALL NPC skill mappings, talent sets, and trait lists, saving then loading SHALL produce an
   equivalent skill mapping, talent set, and trait list (round-trip property).
4. WHERE a save predates this feature and contains NPCs without stored skills, talents, or
   traits, THE system SHALL load those NPCs with an empty skill mapping, an empty talent set, and
   an empty trait list.

### Requirement 8: Deferred Skill and Talent Mechanics Roadmap

**User Story:** As a designer, I want stored-but-inactive skills and talents recorded on a
roadmap, so that later work can wire them into combat.

#### Acceptance Criteria

1. THE system SHALL store skills, talents, and traits other than the Dodge skill and the
   Weapon_Training_Talent without applying any combat effect from them in this feature.
2. THE system SHALL treat all stored Traits as mechanically inert this feature, recording them
   for future use without applying any combat effect.
3. THE design document SHALL record a roadmap note listing which stored skills, talents, and
   traits remain mechanically inert and are deferred to future work, and SHALL include the
   deferred Parry skill-rank bonus (Parry retains its current WS-based behaviour this feature).

## Correctness Properties (for later test design)

These properties inform property-based tests in the Tests project and use the `[npc-stat-blocks]`
tag. RapidCheck `inRange(a, b)` is INCLUSIVE; index generators must use `inRange(0, N - 1)`.
Dodge_Resolver and the Stat_Block accessors must be exercised without initializing the global
Engine (no `engine.gui`, no `engine.map`), per test-isolation rules.

1. **Dodge target formula (invariant):** For any Agility A in [1, 99] and any Dodge Skill_Rank R
   in {0, 1, 2}, the effective Dodge target number equals clamp(A + R * 10, 0, 100) when the
   Actor has the Dodge skill, and clamp(A - 20, 0, 100) when the Actor is untrained — computed
   identically for a Player Actor and an NPC Actor. (Requirement 1)
2. **Dodge monotonicity (metamorphic):** For fixed Agility and fixed roll, increasing Dodge
   Skill_Rank never decreases the set of rolls that succeed. (Requirement 1)
3. **Untrained penalty invariant:** For any Agility A in [1, 99], the untrained Dodge target
   number equals clamp(A - 20, 0, 100). (Requirement 1)
4. **Auto-success / auto-fail invariant:** A roll of 1 always succeeds and a roll of 100 always
   fails, independent of Agility and Skill_Rank. (Requirement 1)
5. **Player/NPC equivalence (model-based):** For identical Agility and Dodge rank, the
   Dodge_Resolver produces the same success outcome for a Player Actor and an NPC Actor given the
   same roll. (Requirements 1, 6)
6. **Skills/talents round-trip:** For any generated skill mapping and talent set, save then load
   yields an equivalent mapping and set. (Requirement 7)
7. **Traits round-trip and inertness:** For any generated trait list, save then load yields an
   equivalent trait list, and resolving combat produces the same outcomes whether or not traits
   are present (traits are mechanically inert this feature). (Requirements 2, 3, 8)
8. **Lua parse round-trip (representative):** For an entry declaring `skills`, `talents`, and
   `traits`, the loaded NPC's mapping, set, and trait list equal the declared data (negative
   ranks clamped to 0). (Requirement 3)
9. **Backward-compatible spawn:** For every existing enemy entry lacking `skills`, `talents`, and
   `traits`, spawning yields an empty skill mapping, empty talent set, and empty trait list and
   does not error. (Requirement 3)
10. **Proficiency equivalence:** For a given WeaponGroup, `proficiencyModifier` returns 0 iff the
    Actor's talent set contains the matching Weapon_Training_Talent (in the exact
    `Weapon Training (<Group>)` format), for both Player and NPC. (Requirement 4)
11. **Fixed-profile invariant:** Resolving any sequence of attacks against or by an NPC leaves the
    NPC's base characteristics, skill mapping, talent set, and trait list unchanged (ignoring
    temporary status/injury modifiers). (Requirement 5)
