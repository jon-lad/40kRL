# Requirements Document

## Introduction

This feature replaces the current 1-action-per-turn system with the Rogue Trader RPG action economy (per RT-CoreMechanics §4). Each actor receives an Action Point budget per turn, enabling richer tactical decisions: a character can spend 2 Half Actions (1 AP each), or 1 Full Action (2 AP), plus a Reaction per round and unlimited Free Actions.

## Glossary

- **Action_System**: The component that tracks and enforces action point budgets, reaction availability, and free action eligibility for all actors.
- **Actor**: Any entity on the map that takes turns (player character or NPC/monster).
- **Action_Points (AP)**: A per-turn resource. Each actor receives 2 AP at the start of their turn.
- **Half_Action**: An action costing 1 AP.
- **Full_Action**: An action costing 2 AP (requires the full budget).
- **Free_Action**: An action costing 0 AP. Can be performed without limit.
- **Reaction**: A defensive response triggered by an external event (e.g., being attacked). Each actor gets 1 Reaction per round; it refreshes at the start of that actor's next turn.
- **Round**: One full cycle of all actors taking their turns.
- **Turn**: A single actor's opportunity to spend their AP budget.
- **Aim_Bonus**: A stacking attack modifier granted by spending AP on the Aim action (+10 per AP spent, maximum +20 for spending both AP aiming).
- **Charge**: A Full Action combining extended movement (up to Agility Bonus × 3 tiles) with a melee attack at +20 WS.
- **All_Out_Attack**: A Full Action granting +30 WS to a melee attack, but forfeiting the actor's Reaction for the current round.
- **AgB**: Agility Bonus — the tens digit of an actor's Agility characteristic.
- **Engine**: The game's main loop controller that manages game states and turn transitions.
- **PlayerAi**: The AI component that processes player input during the player's turn.
- **MonsterAi**: The AI component that drives NPC/monster behaviour during their turn.

## Requirements

### Requirement 1: Action Point Budget

**User Story:** As a player, I want each actor to have an action point budget per turn, so that turns allow multiple actions and tactical depth.

#### Acceptance Criteria

1. WHEN an actor's turn begins, THE Action_System SHALL set that actor's remaining AP to 2.
2. WHEN an actor performs a Half_Action, THE Action_System SHALL deduct 1 AP from that actor's remaining AP.
3. WHEN an actor performs a Full_Action, THE Action_System SHALL deduct 2 AP from that actor's remaining AP.
4. WHEN an actor performs a Free_Action, THE Action_System SHALL deduct 0 AP from that actor's remaining AP.
5. IF an actor attempts a Half_Action with fewer than 1 AP remaining, THEN THE Action_System SHALL reject the action and leave AP unchanged.
6. IF an actor attempts a Full_Action with fewer than 2 AP remaining, THEN THE Action_System SHALL reject the action and leave AP unchanged.
7. WHEN an actor's remaining AP reaches 0, THE Action_System SHALL end that actor's turn.

### Requirement 2: Player Turn Flow

**User Story:** As a player, I want my turn to persist until I have spent my AP budget or explicitly ended my turn, so that I can combine multiple half actions.

#### Acceptance Criteria

1. WHILE the player has AP remaining, THE Engine SHALL keep the game in the player's turn state and accept further input.
2. WHEN the player's AP reaches 0, THE Engine SHALL transition to the enemy turn phase.
3. WHEN the player performs the End_Turn free action, THE Engine SHALL set the player's remaining AP to 0 and transition to the enemy turn phase.
4. WHILE the player has 1 AP remaining, THE PlayerAi SHALL prevent selection of Full_Action commands and indicate they are unavailable.

### Requirement 3: Enemy Turn Flow

**User Story:** As a player, I want enemies to spend their full AP budget each turn using AI logic, so that combat feels fair and tactical.

#### Acceptance Criteria

1. WHEN an enemy's turn begins, THE MonsterAi SHALL spend its AP budget by selecting valid actions until AP reaches 0.
2. WHEN an enemy has 2 AP remaining and a valid Full_Action is tactically preferred, THE MonsterAi SHALL select that Full_Action.
3. WHEN an enemy has AP remaining and no valid Full_Action is preferred, THE MonsterAi SHALL select Half_Actions until AP is exhausted.
4. IF an enemy cannot perform any action with its remaining AP, THEN THE MonsterAi SHALL end that enemy's turn immediately.

### Requirement 4: Reaction System

**User Story:** As a player, I want a reaction system so that actors can dodge or parry incoming attacks, adding defensive depth to combat.

#### Acceptance Criteria

1. WHEN an actor's turn begins, THE Action_System SHALL refresh that actor's Reaction availability to 1.
2. WHEN an actor uses a Reaction (Dodge or Parry), THE Action_System SHALL set that actor's remaining Reactions to 0 for the current round.
3. IF an actor has 0 Reactions remaining and a Reaction-triggering event occurs, THEN THE Action_System SHALL skip the Reaction opportunity for that actor.
4. WHEN an actor performs All_Out_Attack, THE Action_System SHALL set that actor's remaining Reactions to 0 for the current round.
5. WHEN a melee attack hits an actor with a Reaction available, THE Action_System SHALL offer a Dodge or Parry opportunity before applying damage.
6. WHEN a ranged attack hits an actor with a Reaction available, THE Action_System SHALL offer a Dodge opportunity before applying damage.

### Requirement 5: Dodge Reaction

**User Story:** As a player, I want to dodge incoming attacks by testing Agility, so that nimble characters can avoid damage.

#### Acceptance Criteria

1. WHEN an actor attempts a Dodge reaction, THE Action_System SHALL perform an Agility test for that actor.
2. WHEN the Agility test succeeds, THE Action_System SHALL negate the triggering hit entirely.
3. WHEN the Agility test fails, THE Action_System SHALL apply the hit normally.
4. THE Dodge reaction SHALL cost 0 AP.

### Requirement 6: Parry Reaction

**User Story:** As a player, I want to parry melee attacks by testing Weapon Skill, so that skilled fighters can deflect blows.

#### Acceptance Criteria

1. WHEN an actor attempts a Parry reaction, THE Action_System SHALL perform a Weapon_Skill test for that actor.
2. WHEN the Weapon_Skill test succeeds, THE Action_System SHALL negate the triggering melee hit entirely.
3. WHEN the Weapon_Skill test fails, THE Action_System SHALL apply the melee hit normally.
4. IF an actor attempts a Parry without a melee weapon equipped, THEN THE Action_System SHALL deny the Parry and offer Dodge instead.
5. THE Parry reaction SHALL only be available against melee attacks.
6. THE Parry reaction SHALL cost 0 AP.

### Requirement 7: Half Actions

**User Story:** As a player, I want a set of half actions that each cost 1 AP, so that I can combine two of them in a single turn.

#### Acceptance Criteria

1. WHEN an actor performs a Move action, THE Action_System SHALL deduct 1 AP and move the actor 1 tile in the chosen direction.
2. WHEN an actor performs a Standard Attack (melee), THE Action_System SHALL deduct 1 AP and resolve a melee attack with a +10 WS modifier.
3. WHEN an actor performs a Standard Attack (ranged), THE Action_System SHALL deduct 1 AP and resolve a ranged attack with a +10 BS modifier.
4. WHEN an actor performs an Aim action, THE Action_System SHALL deduct 1 AP and grant a +10 Aim_Bonus to the actor's next attack this turn.
5. WHEN an actor has already aimed once this turn and performs a second Aim action, THE Action_System SHALL deduct 1 AP and increase the Aim_Bonus to +20.
6. WHEN an actor performs a Reload action, THE Action_System SHALL deduct 1 AP and refill the equipped ranged weapon's ammunition to its clip capacity.
7. WHEN an actor performs an Open_Door action, THE Action_System SHALL deduct 1 AP and open the targeted adjacent door.
8. WHEN an actor performs a Pick_Up_Item action, THE Action_System SHALL deduct 1 AP and transfer the targeted item from the ground to the actor's inventory.
9. WHEN an actor performs a Use_Item action, THE Action_System SHALL deduct 1 AP and activate the targeted consumable item's effect.

### Requirement 8: Full Actions

**User Story:** As a player, I want full actions that cost 2 AP for powerful combat manoeuvres that use my entire turn.

#### Acceptance Criteria

1. WHEN an actor performs a Charge action, THE Action_System SHALL deduct 2 AP, move the actor up to AgB × 3 tiles toward the target, and resolve a melee attack with a +20 WS modifier.
2. IF a Charge path is blocked before reaching an adjacent tile to the target, THEN THE Action_System SHALL reject the Charge and leave AP unchanged.
3. WHEN an actor performs an All_Out_Attack, THE Action_System SHALL deduct 2 AP, resolve a melee attack with a +30 WS modifier, and set the actor's remaining Reactions to 0.
4. WHEN an actor performs a Run action, THE Action_System SHALL deduct 2 AP and move the actor up to AgB × 6 tiles along a valid path with no attack.

### Requirement 9: Free Actions

**User Story:** As a player, I want certain trivial actions to cost no AP, so that bookkeeping actions do not consume tactical resources.

#### Acceptance Criteria

1. WHEN the player performs the End_Turn action, THE Action_System SHALL deduct 0 AP and immediately end the player's turn.
2. THE Action_System SHALL allow Free_Actions to be performed regardless of remaining AP.

### Requirement 10: AP Display

**User Story:** As a player, I want to see my remaining action points in the HUD, so that I can plan my actions within my budget.

#### Acceptance Criteria

1. WHILE the player's turn is active, THE Gui SHALL display the player's current remaining AP and maximum AP.
2. WHEN the player's AP changes, THE Gui SHALL update the displayed AP value immediately.
3. WHEN the player selects an action, THE Gui SHALL indicate whether the action is affordable given current AP.

### Requirement 13: Numpad Movement Controls

**User Story:** As a player, I want to use the numpad for 8-directional movement and Numpad 5 to end my turn, so that I have ergonomic controls for tactical play.

#### Acceptance Criteria

1. THE InputHandler SHALL accept numpad keys for 8-directional movement: Numpad 7 (up-left), Numpad 8 (up), Numpad 9 (up-right), Numpad 4 (left), Numpad 6 (right), Numpad 1 (down-left), Numpad 2 (down), Numpad 3 (down-right).
2. WHEN the player presses Numpad 5, THE PlayerAi SHALL perform the End_Turn free action.
3. THE InputHandler SHALL continue to accept arrow keys and vi-keys (hjklyubn) as movement alternatives.
4. WHEN Num Lock is off, THE InputHandler SHALL still interpret numpad keys as directional input.

### Requirement 14: Action Log Messages

**User Story:** As a player, I want the message log to report actions taken by all actors, so that I can follow the flow of combat and understand what happened each round.

#### Acceptance Criteria

1. WHEN the player performs any action (Half, Full, or Reaction), THE Gui SHALL post a message describing the action to the message log.
2. WHEN an enemy performs an action that is visible to the player (enemy is in FOV), THE Gui SHALL post a message describing that action to the message log.
3. WHEN an enemy performs an action outside the player's FOV, THE Gui SHALL NOT post a message for that action.
4. WHEN a Reaction is triggered (Dodge or Parry), THE Gui SHALL post a message indicating the attempt and whether it succeeded or failed.
5. THE message log SHALL distinguish between player actions, enemy actions, and reaction outcomes using different text colours.

### Requirement 11: Action Registration

**User Story:** As a developer, I want each action to be registered with its AP cost and type, so that the system can validate and enforce budgets without hard-coding per-action logic.

#### Acceptance Criteria

1. THE Action_System SHALL maintain a registry of all actions with their AP cost and action type (Half, Full, Free, Reaction).
2. WHEN a new action is registered, THE Action_System SHALL validate that the cost matches the declared type (Half = 1, Full = 2, Free = 0, Reaction = 0).
3. WHEN an actor requests an action by identifier, THE Action_System SHALL look up the cost from the registry and apply budget validation.

### Requirement 12: Aim Bonus Lifecycle

**User Story:** As a player, I want aim bonuses to apply only to the next attack in the same turn, so that aiming is a deliberate tactical choice with immediate payoff.

#### Acceptance Criteria

1. WHEN an actor performs an Aim action, THE Action_System SHALL store the cumulative Aim_Bonus on that actor.
2. WHEN an actor performs an attack action after aiming, THE Action_System SHALL apply the stored Aim_Bonus as a modifier to the attack roll.
3. WHEN an actor's turn ends, THE Action_System SHALL clear any unused Aim_Bonus from that actor.
4. WHEN an actor performs an attack that consumes the Aim_Bonus, THE Action_System SHALL reset the Aim_Bonus to 0.
