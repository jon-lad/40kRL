# Rogue Trader Core Mechanics Reference
## Optimized for AI consumption — extracted from The Liber Imperium v1.6

---

## 1. Core Mechanic

- **System**: d100 roll-under
- **Test**: Roll d100 ≤ (Characteristic + Modifiers) = Success
- **Auto-success**: Roll of 1 always succeeds
- **Auto-fail**: Roll of 100 always fails
- **Degrees of Success (DoS)**: (Target Number - Roll) / 10, rounded down, minimum 1 on success
- **Degrees of Failure (DoF)**: (Roll - Target Number) / 10, rounded down, minimum 1 on failure

---

## 2. Characteristics

| Abbreviation | Name | Description |
|---|---|---|
| WS | Weapon Skill | Melee attack accuracy |
| BS | Ballistic Skill | Ranged attack accuracy |
| S / St | Strength | Physical power, melee damage bonus |
| T | Toughness | Damage resistance |
| Ag | Agility | Initiative, dodge, movement rate |
| Int | Intelligence | Knowledge, logic, tech-use |
| Per | Perception | Awareness, detection |
| WP | Willpower | Mental resilience, psychic resistance |
| Fel | Fellowship | Social interaction, leadership |
| Inf | Influence | Wealth, connections, acquisition |

### Characteristic Bonus
- **Bonus** = tens digit of the characteristic value
- Example: Strength 42 → Strength Bonus (SB) = 4
- Used for: Toughness Bonus (damage soak), Strength Bonus (melee damage), Agility Bonus (movement)

### Characteristic Ranges
- Human baseline: 20-40
- Exceptional human: 41-60
- Superhuman: 61-80
- Legendary: 81-100
- Generation: 2d10 + 20 (gives 22-40 range for starting characters)

---

## 3. Test Difficulty Modifiers

| Difficulty | Modifier |
|---|---|
| Trivial | +60 |
| Elementary | +50 |
| Simple | +40 |
| Easy | +30 |
| Routine | +20 |
| Ordinary | +10 |
| Challenging | +0 |
| Difficult | -10 |
| Hard | -20 |
| Very Hard | -30 |
| Arduous | -40 |
| Punishing | -50 |
| Hellish | -60 |

- Maximum bonus cap: +60
- Maximum penalty cap: -60
- Default difficulty if unspecified: Challenging (+0)
- Untrained skill penalty: -20

---

## 4. Combat Structure

### Turn Order
1. **Surprise**: Awareness test to avoid; surprised characters lose first turn and reactions
2. **Initiative**: 1d10 + Agility Bonus (ties broken by higher Ag, then re-roll)
3. **Turns**: Each character takes one turn per round in Initiative order
4. **Round duration**: ~5 seconds of game time

### Action Economy (per turn)
- **1 Full Action** OR **2 Half Actions**
- **1 Reaction** per round (refreshes at start of your turn)
- **Unlimited Free Actions** (GM discretion)

### Attack Actions
| Action | Type | Effect |
|---|---|---|
| Standard Attack | Half | +10 to WS/BS |
| Aim (Half) | Half | +10 to next attack |
| Aim (Full) | Full | +20 to next attack |
| All Out Attack | Full | +30 WS, +Felling(1d5), lose Reaction |
| Charge | Full | +20 WS, must move 4+ metres |
| Called Shot | Full | +0, choose hit location |
| Semi-Auto Burst | Half | -10 BS, extra hit per 2 DoS |
| Full Auto Burst | Half | -10 BS, extra hit per DoS |
| Swift Attack | Half | +0 WS, extra hit per 2 DoS (melee) |
| Lightning Attack | Half | -10 WS, extra hit per DoS (melee) |

### Surprise Bonuses
- Non-surprised attacker vs surprised target: +30 to WS/BS

---

## 5. Attack Resolution

### Step-by-Step
1. **Apply Modifiers** to WS (melee) or BS (ranged)
2. **Roll d100** — success if ≤ modified characteristic
3. **Determine Hit Location** — reverse the attack roll digits
4. **Determine Damage** — roll weapon damage dice + SB (melee only)
5. **Apply Damage** — subtract Toughness Bonus and Armour Points

### Hit Location Table (reverse d100 roll)

| Reversed Roll | Location |
|---|---|
| 01-10 | Head |
| 11-20 | Right Arm |
| 21-30 | Left Arm |
| 31-70 | Body |
| 71-85 | Right Leg |
| 86-100 | Left Leg |

### Multiple Hits (additional hits from bursts/swift attacks)
Starting from the first hit location, additional hits follow this pattern per row:

| First Hit | 2nd | 3rd | 4th | 5th | 6th |
|---|---|---|---|---|---|
| Head | Right Arm | Body | Left Arm | Body | Head |
| Right Arm | Body | Head | Left Arm | Body | Head |
| Left Arm | Body | Head | Right Arm | Body | Head |
| Body | Left Leg | Right Arm | Body | Right Leg | Left Arm |
| Right Leg | Body | Left Leg | Body | Right Leg | Body |
| Left Leg | Body | Right Leg | Body | Left Leg | Body |

### Damage Formula
```
Final Damage = Weapon Damage Roll + Strength Bonus (melee only)
                - Target's Toughness Bonus
                - Target's Armour Points (at hit location)
```
- If result ≤ 0: no damage dealt
- Attacker may replace ONE damage die with their DoS from the attack roll

---

## 6. Wounds and Critical Damage

### Wound Threshold
- Characters have a fixed Wounds value (typically 10-20 for humans)
- Damage accumulates; when total damage ≥ Wounds, excess becomes **Critical Damage**
- Critical Damage triggers effects from Critical Effects tables (by damage type + location)

### Righteous Fury
- **Trigger**: Natural 10 on any damage die
- **If damage gets through** (after TB + AP reduction): Roll 1d5 on Critical Effects table
- **If damage is fully absorbed**: Deal 1 point of damage (ignores armour/toughness)

### Damage Types
| Code | Type | Description |
|---|---|---|
| E | Energy | Las, plasma, flame, power weapons |
| I | Impact | Blunt force, bolts, explosives |
| R | Rending | Sharp/cutting, chainblades, claws |
| X | Explosive | Blast weapons, grenades |

---

## 7. Evasion

- **Dodge**: Agility test as a Reaction to negate one hit
- **Parry**: Weapon Skill test as a Reaction to negate one melee hit (requires melee weapon)
- Only ONE Evasion reaction per round (unless talents grant more)
- Each DoS beyond the first negates one additional hit

---

## 8. Combat Circumstance Modifiers

| Circumstance | Modifier |
|---|---|
| Darkness (melee) | -20 WS |
| Darkness (ranged) | -30 BS |
| Difficult terrain (melee/dodge) | -10 |
| Arduous terrain (melee/dodge) | -30 |
| Shooting into melee | -20 BS |
| Target is prone (ranged, 3+ metres) | -10 BS |
| Target is prone (melee) | +10 WS |
| Higher ground | +10 WS |
| Point blank (≤ 2m ranged) | +30 BS |
| Fog/smoke (light) | -10 BS |
| Fog/smoke (heavy) | -20 BS |
| Cover (light) | target gets AP bonus or concealment |
| Stunned target | +20 to hit |

---

## 9. Movement

### Movement Values (per Half Action Move)
- **Half Move**: AgB metres
- **Full Move**: AgB × 2 metres
- **Charge**: AgB × 3 metres (Full Action, +20 WS)
- **Run**: AgB × 6 metres (Full Action, no attack)

### Example
- Agility 40 → AgB 4 → Half Move 4m, Full Move 8m, Charge 12m, Run 24m

---

## 10. Ranged Weapon Properties

| Property | Meaning |
|---|---|
| Range | Maximum effective range in metres |
| RoF (S/B/A) | Single / Semi-Auto burst size / Full-Auto burst size |
| Clip | Ammunition capacity before reload |
| Reload | Half Actions to reload |
| Damage | Dice + modifier (e.g., 1d10+3) |
| Pen | Penetration — reduces target's effective AP |
| Type | E/I/R/X damage type |

### Range Brackets
| Range | Modifier |
|---|---|
| Point Blank (≤ 2m) | +30 |
| Short (≤ half range) | +10 |
| Normal (≤ full range) | +0 |
| Long (≤ 2× range) | -10 |
| Extreme (≤ 3× range) | -30 |

---

## 11. Weapon Qualities (Game-Relevant Subset)

| Quality | Effect |
|---|---|
| Accurate | +10 additional bonus when Aiming |
| Balanced | +10 to Parry tests |
| Blast (X) | Hits everyone within X metres of impact |
| Concussive (X) | Target must pass Toughness test or be stunned |
| Felling (X) | Reduces target's Unnatural Toughness by X for damage |
| Flame | Sets target on fire (Ag test or burn for 1d10 E per round) |
| Overheats | On 91+ BS roll, weapon overheats (wielder takes damage) |
| Pen (X) | Penetration — ignores X points of armour |
| Power Field | On Parry, 75% chance to destroy attacker's weapon (non-power/natural) |
| Primitive (X) | Maximum damage per die capped at X |
| Proven (X) | Minimum result on each damage die is X |
| Razor Sharp | On 2+ DoS, double penetration |
| Reliable | Only jams on roll of 100 |
| Scatter | +10 at Point Blank; -3 damage per 5m beyond Short range |
| Shocking | On damage dealt, target must pass Toughness or be stunned 1 round |
| Spray | Cone attack, hits all in 30° arc to weapon's range |
| Tearing | Roll one extra damage die, discard the lowest |
| Twin-Linked | +20 to hit, +1 hit on 2+ DoS |
| Unbalanced | -10 to Parry tests |
| Unreliable | Jams on 91+ (instead of 96+) |
| Unwieldy | Cannot Parry; cannot use Lightning Attack |

---

## 12. Armour

### Armour by Location
- Each body location has separate AP (Armour Points)
- AP stacks only if explicitly stated (e.g., natural armour + worn armour)
- Standard locations: Head, Body, Left Arm, Right Arm, Left Leg, Right Leg

### Common Armour Types

| Armour | AP (All/Varies) | Notes |
|---|---|---|
| Heavy Robes | 1 (Arms, Body, Legs) | - |
| Flak (Vest/Coat) | 3 (Body, Arms, Legs) | Common Guard issue |
| Flak (Full) | 3 (All) | Full coverage |
| Carapace | 5-6 (varies) | Heavy, military-grade |
| Power Armour | 7-8 (All) | Space Marine standard |
| Mesh | 4 (Body) | Lightweight, expensive |

---

## 13. Fatigue

- Gained from: exhaustion, starvation, stunning, concussive weapons
- Each level of Fatigue: -10 to all tests
- If Fatigue levels ≥ Toughness Bonus: character falls unconscious
- Removed by: rest (1 level per period of rest)
