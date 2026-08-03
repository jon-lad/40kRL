# Requirements Document

## Introduction

The critical injury system extends the existing critical hit mechanics to apply actual gameplay debuffs when a non-fatal critical hit occurs. Currently, critical hits at low magnitudes only produce flavour text and allow the target to survive at 1 HP. This feature adds persistent mechanical penalties (stat reductions, movement restrictions) that match the thematic descriptions, along with injury tracking, escalation on duplicate injuries, and healing to clear them.

## Glossary

- **Critical_Injury_System**: The component responsible for tracking active critical injuries on an Actor, applying and removing their associated debuffs, and handling escalation and healing logic.
- **Critical_Injury**: A persistent debuff applied to an Actor as a result of a non-fatal critical hit. Each injury is associated with a specific HitLocation and magnitude level, and applies one or more Characteristic penalties.
- **Debuff**: A negative modifier applied to one or more Characteristics (WS, BS, S, T, Ag, Int, Per, WP, Fel) representing the mechanical effect of a Critical_Injury.
- **HitLocation**: One of six body zones (HEAD, RIGHT_ARM, LEFT_ARM, BODY, RIGHT_LEG, LEFT_LEG) that determines which area received the critical hit.
- **Magnitude**: An integer from 1 to 10 representing the severity of a critical hit. Magnitudes 1-4 are non-fatal; magnitudes 5-10 are fatal.
- **Escalation**: The process of increasing the magnitude of an existing Critical_Injury when the same HitLocation would receive a duplicate injury before healing.
- **Actor**: Any entity in the game world (player or enemy) that can have Characteristics, Attacker, and Destructible components.
- **Characteristics**: The nine Rogue Trader RPG stats (WS, BS, S, T, Ag, Int, Per, WP, Fel) clamped to [1, 99].
- **Healing_Event**: Any game event that clears critical injuries — resting, using healing items, or transitioning between levels.

## Requirements

### Requirement 1: Apply Debuffs on Non-Fatal Critical Hit

**User Story:** As a player, I want non-fatal critical hits to apply mechanical debuffs to the target, so that critical injuries have meaningful tactical consequences beyond flavour text.

#### Acceptance Criteria

1. WHEN a non-fatal critical hit is resolved at magnitude 1-4 on any HitLocation, THE Critical_Injury_System SHALL create a Critical_Injury record on the target Actor with the corresponding HitLocation and magnitude.
2. WHEN a Critical_Injury is created, THE Critical_Injury_System SHALL apply the associated Debuff modifiers to the target Actor's Characteristics immediately.
3. THE Critical_Injury_System SHALL apply debuffs to both player-controlled and enemy Actors without distinction.
4. WHEN a non-fatal critical hit is resolved, THE Critical_Injury_System SHALL retain the existing behaviour of setting the target's HP to 1 for magnitudes below 3.

### Requirement 2: Location-Based Debuff Mapping

**User Story:** As a player, I want critical injuries to debuff stats thematically appropriate to the hit location, so that the gameplay effects match the narrative descriptions.

#### Acceptance Criteria

1. WHEN a Critical_Injury occurs on HEAD at magnitude 1, THE Critical_Injury_System SHALL apply a -5 penalty to Per.
2. WHEN a Critical_Injury occurs on HEAD at magnitude 2, THE Critical_Injury_System SHALL apply a -10 penalty to Per and a -5 penalty to BS.
3. WHEN a Critical_Injury occurs on HEAD at magnitude 3, THE Critical_Injury_System SHALL apply a -10 penalty to WS and a -10 penalty to BS.
4. WHEN a Critical_Injury occurs on HEAD at magnitude 4, THE Critical_Injury_System SHALL apply a -20 penalty to WS, a -20 penalty to BS, and a -10 penalty to Int.
5. WHEN a Critical_Injury occurs on RIGHT_ARM or LEFT_ARM at magnitude 1, THE Critical_Injury_System SHALL apply a -5 penalty to WS.
6. WHEN a Critical_Injury occurs on RIGHT_ARM or LEFT_ARM at magnitude 2, THE Critical_Injury_System SHALL apply a -10 penalty to WS and a -5 penalty to BS.
7. WHEN a Critical_Injury occurs on RIGHT_ARM or LEFT_ARM at magnitude 3, THE Critical_Injury_System SHALL apply a -10 penalty to WS and a -10 penalty to BS.
8. WHEN a Critical_Injury occurs on RIGHT_ARM or LEFT_ARM at magnitude 4, THE Critical_Injury_System SHALL apply a -20 penalty to WS and a -20 penalty to BS.
9. WHEN a Critical_Injury occurs on BODY at magnitude 1, THE Critical_Injury_System SHALL apply a -5 penalty to T.
10. WHEN a Critical_Injury occurs on BODY at magnitude 2, THE Critical_Injury_System SHALL apply a -10 penalty to T.
11. WHEN a Critical_Injury occurs on BODY at magnitude 3, THE Critical_Injury_System SHALL apply a -10 penalty to T and a -5 penalty to S.
12. WHEN a Critical_Injury occurs on BODY at magnitude 4, THE Critical_Injury_System SHALL apply a -20 penalty to T and a -10 penalty to S.
13. WHEN a Critical_Injury occurs on RIGHT_LEG or LEFT_LEG at magnitude 1, THE Critical_Injury_System SHALL apply a -5 penalty to Ag.
14. WHEN a Critical_Injury occurs on RIGHT_LEG or LEFT_LEG at magnitude 2, THE Critical_Injury_System SHALL apply a -10 penalty to Ag.
15. WHEN a Critical_Injury occurs on RIGHT_LEG or LEFT_LEG at magnitude 3, THE Critical_Injury_System SHALL apply a -15 penalty to Ag and a -5 penalty to WS.
16. WHEN a Critical_Injury occurs on RIGHT_LEG or LEFT_LEG at magnitude 4, THE Critical_Injury_System SHALL apply a -20 penalty to Ag and a -10 penalty to WS.

### Requirement 3: Injury Escalation on Duplicate Location

**User Story:** As a player, I want repeated critical hits to the same location to escalate in severity, so that focusing attacks on an already-injured body part is tactically meaningful.

#### Acceptance Criteria

1. WHEN a non-fatal critical hit targets a HitLocation that already has an active Critical_Injury, THE Critical_Injury_System SHALL increase the existing injury's magnitude by 1 instead of creating a duplicate injury.
2. WHEN escalation increases a Critical_Injury's magnitude from 4 to 5, THE Critical_Injury_System SHALL trigger the fatal critical effect for that HitLocation at magnitude 5 and kill the target Actor.
3. WHEN escalation increases magnitude within the non-fatal range (1-3 to 2-4), THE Critical_Injury_System SHALL remove the old Debuff and apply the new Debuff corresponding to the escalated magnitude.
4. THE Critical_Injury_System SHALL permit a maximum of one active Critical_Injury per HitLocation per Actor at any time.

### Requirement 4: Healing Clears Critical Injuries

**User Story:** As a player, I want healing events to clear my critical injuries, so that I can recover from debilitating wounds through gameplay actions.

#### Acceptance Criteria

1. WHEN a Healing_Event occurs, THE Critical_Injury_System SHALL remove all active Critical_Injury records from the affected Actor.
2. WHEN a Critical_Injury is removed, THE Critical_Injury_System SHALL reverse all associated Debuff modifiers from the Actor's Characteristics.
3. WHEN a level transition occurs, THE Critical_Injury_System SHALL clear all Critical_Injury records from the player Actor.
4. WHEN a healing item is used on an Actor, THE Critical_Injury_System SHALL clear all Critical_Injury records from that Actor.

### Requirement 5: Serialization of Critical Injuries

**User Story:** As a player, I want my critical injuries to persist across save and load cycles, so that the tactical state is preserved when resuming a game.

#### Acceptance Criteria

1. WHEN the game state is saved, THE Critical_Injury_System SHALL serialize all active Critical_Injury records (HitLocation and magnitude) for each Actor to the save archive.
2. WHEN the game state is loaded, THE Critical_Injury_System SHALL deserialize all Critical_Injury records and reapply the corresponding Debuffs to each Actor.
3. FOR ALL valid save archives containing Critical_Injury data, loading then saving then loading SHALL produce an equivalent set of active Critical_Injury records on each Actor (round-trip property).
4. WHEN a save archive from a version without Critical_Injury data is loaded, THE Critical_Injury_System SHALL initialize with zero active injuries and continue normal operation.

### Requirement 6: Injury Status Display

**User Story:** As a player, I want to see my active critical injuries in the UI, so that I can make informed tactical decisions based on my current debuffs.

#### Acceptance Criteria

1. WHILE the player Actor has one or more active Critical_Injury records, THE Gui SHALL display each injury's HitLocation name and magnitude level in the character status panel.
2. WHEN the player examines an enemy with active Critical_Injury records, THE Gui SHALL display that enemy's injuries in the look panel.
3. WHEN a Critical_Injury is applied or removed, THE Gui SHALL display a message in the combat log describing the injury or recovery.
