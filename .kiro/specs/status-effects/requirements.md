# Requirements Document

## Introduction

This feature adds a status effect system to 40kRL. Status effects are conditions applied to actors that modify their capabilities over time — some are temporary (burning, stunned, prone) and some are permanent (missing limbs, blinded). Statuses are triggered by critical injuries (based on hit location and magnitude, per RT-CriticalEffects) and by weapon qualities (Flame, Shocking, Toxic). The system integrates with the existing InjuryTracker and InjuryDebuffs infrastructure, extending the crit pipeline to produce named conditions with distinct mechanical effects rather than only flat characteristic penalties.

## Glossary

- **Status_Effect_System**: The component responsible for tracking, applying, ticking, and removing status effects on an Actor.
- **Status_Effect**: A named condition (e.g., Burning, Prone, Stunned) attached to an Actor that modifies behaviour or characteristics for a duration.
- **Duration**: The number of turns a temporary status effect persists before automatic removal. A duration of 0 indicates a permanent status.
- **Weapon_Quality**: A property defined in a weapon's Lua `qualities` table (e.g., "Flame", "Shocking", "Toxic") that can trigger a status effect on hit.
- **Critical_Trigger**: The combination of hit location, crit magnitude, and damage type that causes a specific status effect to be applied, per RT-CriticalEffects §Conditions.
- **Actor**: Any entity on the map that has components (player, monsters).
- **InjuryTracker**: The existing component that tracks cumulative crit magnitude and applies debuffs per hit location.
- **Tick**: Processing that occurs at the start of an Actor's turn, where active status effects decrement duration and apply per-turn effects.

## Requirements

### Requirement 1: Status Effect Data Model

**User Story:** As a developer, I want a well-defined data model for status effects, so that effects can be created, queried, and serialized uniformly.

#### Acceptance Criteria

1. THE Status_Effect_System SHALL support the following status types: Burning, Prone, Stunned, Bleeding, Missing_Right_Arm, Missing_Left_Arm, Missing_Right_Leg, Missing_Left_Leg, Blinded, Poisoned
2. WHEN a status effect is created, THE Status_Effect_System SHALL store the status type, remaining duration in turns, and the source description
3. THE Status_Effect_System SHALL represent permanent effects with a duration value of 0, indicating the effect does not expire through normal ticking
4. THE Status_Effect_System SHALL represent temporary effects with a positive integer duration that decrements each turn

### Requirement 2: Status Effect Application from Critical Injuries

**User Story:** As a player, I want critical injuries to cause appropriate status effects based on damage type, location, and severity, so that combat has meaningful consequences beyond characteristic penalties.

#### Acceptance Criteria

1. WHEN a critical hit of Energy damage type reaches magnitude 3 or higher on the Head location, THE Status_Effect_System SHALL apply the Blinded status to the target (per RT-CriticalEffects §Energy Head, critical 3+)
2. WHEN a critical hit of Energy damage type reaches magnitude 5 or higher on the Head location, THE Status_Effect_System SHALL apply the Burning status to the target (per RT-CriticalEffects §Energy Head, critical 5: "Head ablaze. On fire.")
3. WHEN a critical hit of Energy damage type reaches magnitude 5 on an Arm location, THE Status_Effect_System SHALL apply the corresponding Missing_Arm status to the target (per RT-CriticalEffects §Energy Arm, critical 5: "Arm destroyed")
4. WHEN a critical hit of Energy damage type reaches magnitude 5 on a Leg location, THE Status_Effect_System SHALL apply the corresponding Missing_Leg status to the target (per RT-CriticalEffects §Energy Leg, critical 5: "Leg destroyed")
5. WHEN a critical hit of Energy damage type reaches magnitude 3 on the Body location, THE Status_Effect_System SHALL apply the Burning status to the target (per RT-CriticalEffects §Energy Body, critical 3: "On fire.")
6. WHEN a critical hit of Impact damage type reaches magnitude 1 or higher on the Head location, THE Status_Effect_System SHALL apply the Stunned status to the target (per RT-CriticalEffects §Impact Head, critical 1: "Stunned 1 round")
7. WHEN a critical hit of Impact damage type reaches magnitude 5 on the Body location, THE Status_Effect_System SHALL apply the Prone status to the target (per RT-CriticalEffects §Impact Body, critical 5: "Prone")
8. WHEN a critical hit of Impact damage type reaches magnitude 6 on an Arm location, THE Status_Effect_System SHALL apply the corresponding Missing_Arm status to the target (per RT-CriticalEffects §Impact Arm, critical 6+: "Arm torn off")
9. WHEN a critical hit of Impact damage type reaches magnitude 6 on a Leg location, THE Status_Effect_System SHALL apply the corresponding Missing_Leg status to the target (per RT-CriticalEffects §Impact Leg, critical 6+: "Leg torn off")
10. WHEN a critical hit of Rending damage type reaches magnitude 1 or higher on the Head location, THE Status_Effect_System SHALL apply the Bleeding status to the target (per RT-CriticalEffects §Rending Head, critical 1: "Blood Loss")
11. WHEN a critical hit of Rending damage type reaches magnitude 5 on the Head location, THE Status_Effect_System SHALL apply the Blinded status to the target (per RT-CriticalEffects §Rending Head, critical 5: "blind 1d10 rounds")
12. WHEN a critical hit of Rending damage type reaches magnitude 6 on an Arm location, THE Status_Effect_System SHALL apply the corresponding Missing_Arm status to the target (per RT-CriticalEffects §Rending Arm, critical 6+: "Arm severed")
13. WHEN a critical hit of Rending damage type reaches magnitude 5 on a Leg location, THE Status_Effect_System SHALL apply the corresponding Missing_Leg status to the target (per RT-CriticalEffects §Rending Leg, critical 5: "Foot severed" / critical 6+: "Leg severed")
14. WHEN a critical hit of Explosive damage type reaches magnitude 1 or higher on the Head location, THE Status_Effect_System SHALL apply the Stunned status to the target (per RT-CriticalEffects §Explosive Head, critical 1: "Stunned 1 round")
15. WHEN a critical hit of Explosive damage type reaches magnitude 4 on an Arm location, THE Status_Effect_System SHALL apply the corresponding Missing_Arm status to the target (per RT-CriticalEffects §Explosive Arm, critical 4: "Arm shredded. Permanent loss.")
16. WHEN a critical hit of Explosive damage type reaches magnitude 4 on a Leg location, THE Status_Effect_System SHALL apply the corresponding Missing_Leg status to the target (per RT-CriticalEffects §Explosive Leg, critical 4: "Leg blown off")
17. WHEN a critical hit of Rending damage type reaches magnitude 4 on the Body location, THE Status_Effect_System SHALL apply the Bleeding status and Prone status to the target (per RT-CriticalEffects §Rending Body, critical 4: "Prone. Blood Loss.")
18. WHEN a critical hit of Explosive damage type reaches magnitude 4 on the Body location, THE Status_Effect_System SHALL apply the Bleeding status and Prone status to the target (per RT-CriticalEffects §Explosive Body, critical 4: "Prone. Blood Loss.")

### Requirement 3: Status Effect Application from Weapon Qualities

**User Story:** As a player, I want weapons with special qualities to apply status effects on hit, so that weapon choice has tactical meaning.

#### Acceptance Criteria

1. WHEN a weapon with the "Flame" quality deals damage to a target, THE Status_Effect_System SHALL apply the Burning status to the target
2. WHEN a weapon with the "Shocking" quality deals damage to a target, THE Status_Effect_System SHALL apply the Stunned status to the target with a duration of 1 turn
3. WHEN a weapon with the "Toxic" quality deals damage to a target, THE Status_Effect_System SHALL apply the Poisoned status to the target
4. THE Status_Effect_System SHALL read weapon qualities from the existing Lua `qualities` table in the weapon definition

### Requirement 4: Temporary Status Effect Duration and Ticking

**User Story:** As a player, I want temporary status effects to resolve over time, so that combat consequences are meaningful but not permanently crippling for minor injuries.

#### Acceptance Criteria

1. WHEN an Actor's turn begins, THE Status_Effect_System SHALL decrement the remaining duration of each temporary status effect on that Actor by 1
2. WHEN a temporary status effect's duration reaches 0, THE Status_Effect_System SHALL remove the status effect from the Actor and reverse any modifier-based mechanical effects
3. THE Status_Effect_System SHALL apply Stunned with a default duration of 1 turn when triggered by magnitude 1 critical hits
4. THE Status_Effect_System SHALL apply Burning with a default duration of 3 turns when triggered by weapon qualities or critical hits
5. THE Status_Effect_System SHALL apply Blinded with a duration determined by 1d5 turns when triggered by Energy Head magnitude 3 critical hits (per RT-CriticalEffects: "Blinded 1d5 rounds")
6. THE Status_Effect_System SHALL apply Bleeding with no automatic expiry, requiring explicit removal through healing or a future First Aid action
7. THE Status_Effect_System SHALL apply Poisoned with a default duration of 5 turns

### Requirement 5: Mechanical Effects of Statuses

**User Story:** As a player, I want each status effect to produce distinct mechanical consequences, so that I must adapt my tactics to my current condition.

#### Acceptance Criteria

1. WHILE an Actor has the Burning status, THE Status_Effect_System SHALL deal 1d10 Energy damage (ignoring armour) to the Actor at the start of each of the Actor's turns (per RT-CriticalEffects §Conditions: "On Fire")
2. WHILE an Actor has the Prone status, THE Status_Effect_System SHALL apply a -10 penalty to the Actor's Weapon Skill characteristic
3. WHILE an Actor has the Prone status, THE Status_Effect_System SHALL apply a -10 modifier to ranged attacks targeting the Actor (making the Actor harder to hit at range, per RT-CriticalEffects §Conditions)
4. WHILE an Actor has the Stunned status, THE Status_Effect_System SHALL prevent the Actor from taking any actions during the Actor's turn (skip turn) and apply -20 to the Actor's Evasion tests (per RT-CriticalEffects §Conditions)
5. WHILE an Actor has the Bleeding status, THE Status_Effect_System SHALL deal 1 wound of damage to the Actor at the end of each of the Actor's turns (per RT-CriticalEffects §Conditions: "Blood Loss — Lose 1 Wound per round")
6. WHILE an Actor has a Missing_Right_Arm or Missing_Left_Arm status, THE Status_Effect_System SHALL prevent the Actor from equipping two-handed weapons
7. WHILE an Actor has a Missing_Right_Arm or Missing_Left_Arm status, THE Status_Effect_System SHALL prevent the Actor from equipping any weapon in the corresponding arm slot
8. WHILE an Actor has a Missing_Right_Leg or Missing_Left_Leg status, THE Status_Effect_System SHALL reduce the Actor's movement to half (1 tile per 2 AP instead of 1 tile per 1 AP)
9. WHILE an Actor has the Blinded status, THE Status_Effect_System SHALL apply a -30 penalty to the Actor's Weapon Skill and Ballistic Skill characteristics (per RT-CriticalEffects §Conditions)
10. WHILE an Actor has the Blinded status, THE Status_Effect_System SHALL apply a +30 bonus to attack rolls made by opponents targeting the Blinded Actor (per RT-CriticalEffects §Conditions)
11. WHILE an Actor has the Poisoned status, THE Status_Effect_System SHALL apply a -10 penalty to the Actor's Strength, Toughness, and Agility characteristics
12. WHEN a Prone Actor spends a Half Action (1 AP) to stand, THE Status_Effect_System SHALL remove the Prone status from the Actor

### Requirement 6: Status Effect Stacking and Interaction Rules

**User Story:** As a developer, I want clear rules for how statuses interact when multiple are applied, so that edge cases are handled predictably.

#### Acceptance Criteria

1. WHEN a status effect is applied to an Actor that already has the same status type active, THE Status_Effect_System SHALL refresh the duration to the new value if the new duration is longer than the remaining duration
2. WHEN a status effect is applied to an Actor that already has the same status type active, THE Status_Effect_System SHALL keep the existing duration if the existing duration is longer than the new value
3. THE Status_Effect_System SHALL allow multiple different status types to be active on the same Actor simultaneously
4. WHEN a permanent status (duration 0) is already active and a temporary instance of the same type is applied, THE Status_Effect_System SHALL retain the permanent instance unchanged
5. THE Status_Effect_System SHALL apply Missing_Right_Arm and Missing_Left_Arm as distinct statuses that can coexist on the same Actor
6. THE Status_Effect_System SHALL apply Missing_Right_Leg and Missing_Left_Leg as distinct statuses that can coexist on the same Actor

### Requirement 7: Serialization of Status Effects

**User Story:** As a player, I want my status effects to persist across save and load, so that game state is preserved accurately.

#### Acceptance Criteria

1. WHEN the game is saved, THE Status_Effect_System SHALL serialize each active status effect including its type, remaining duration, and source description
2. WHEN the game is loaded, THE Status_Effect_System SHALL deserialize all saved status effects and reapply their mechanical modifiers to the Actor
3. THE Status_Effect_System SHALL use the existing Actor save/load sentinel pattern (presence flag followed by payload) for backward compatibility with saves that predate the status effect system
4. IF a save file does not contain status effect data, THEN THE Status_Effect_System SHALL initialize the Actor with no active status effects

### Requirement 8: UI Display of Status Effects

**User Story:** As a player, I want to see my active status effects in the interface, so that I can make informed tactical decisions.

#### Acceptance Criteria

1. THE Status_Effect_System SHALL display active status effects for the player in the sidebar panel using abbreviated text labels (e.g., "BRN", "PRN", "STN", "BLD", "PSN", "ARM-R", "LEG-L", "BLND")
2. WHEN a status effect is applied to any Actor, THE Status_Effect_System SHALL log a message to the message log describing the effect (e.g., "You are on fire!", "The Ork is stunned.")
3. WHEN a temporary status effect expires, THE Status_Effect_System SHALL log a message to the message log indicating expiry (e.g., "The flames die out.", "You are no longer stunned.")
4. THE Status_Effect_System SHALL display the remaining duration in turns next to each temporary status label in the sidebar
5. THE Status_Effect_System SHALL display permanent status effects without a duration number, using a distinct color to indicate permanence

### Requirement 9: AI Awareness of Status Effects

**User Story:** As a player, I want enemies to be affected by status effects in the same way the player is, so that combat feels fair and consistent.

#### Acceptance Criteria

1. WHILE an enemy Actor has the Stunned status, THE Ai_Component SHALL skip the enemy's turn
2. WHILE an enemy Actor has the Prone status, THE Ai_Component SHALL spend 1 AP standing up before taking other actions
3. WHILE an enemy Actor has the Burning status, THE Status_Effect_System SHALL deal damage to the enemy at the start of the enemy's turn using the same rules as for the player
4. WHILE an enemy Actor has the Bleeding status, THE Status_Effect_System SHALL deal damage to the enemy at the end of the enemy's turn using the same rules as for the player
5. WHILE an enemy Actor has a Missing_Leg status, THE Ai_Component SHALL use the reduced movement rate when pathfinding toward the player

### Requirement 10: Standing Up from Prone

**User Story:** As a player, I want to be able to stand up from prone, so that the penalty is temporary and I can resume normal combat.

#### Acceptance Criteria

1. WHEN the player has the Prone status and presses a designated "stand up" key, THE Status_Effect_System SHALL remove the Prone status and consume 1 AP from the Actor's action budget
2. IF the Actor does not have enough AP to stand (less than 1 AP remaining), THEN THE Status_Effect_System SHALL display a message indicating insufficient AP and keep the Prone status active
3. WHEN the Prone status is removed by standing, THE Status_Effect_System SHALL remove the associated WS penalty modifier from the Actor's characteristics

