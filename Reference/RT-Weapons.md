# Rogue Trader Weapons Reference
## Optimized for AI consumption — game-relevant weapon profiles

---

## Weapon Profile Format

```
Name | Class | Range | RoF (S/B/A) | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source
```

- **Class**: Pistol, Basic, Heavy, Melee, Thrown, Mounted
- **Range**: Metres (melee = "-")
- **RoF**: Single/Semi-Auto burst/Full-Auto burst (0 = cannot fire in that mode)
- **Damage**: Dice + Bonus + Type (E/I/R/X)
- **Pen**: Penetration (armour points ignored)
- **Clip**: Shots before reload
- **Reload**: Half Actions required
- **Weight (kg)** / **Availability**: included where a source supplies them (`—` when absent)
- **Source**: citation for the row — `Existing` (prior RT-Weapons.md), `RT Core Table 5-N` (Source B), `Liber Imperium §6` (Source A); combined form (`Existing + RT Core Table 5-N`) marks a reconciled row, with the winning source noted parenthetically for conflict-resolved fields

> Precedence Order for reconciliation: **Liber Imperium §6 (Source A) > RT Core Table 5-N (Source B) > Existing**. A value present in only one source is recorded as-is (absence is not a conflict).

---

## Las Weapons

> Source: rows reconciled from Existing Target and RT Core Table 5-4 (weight/availability added from Table 5-4 where present). NEW rows cite RT Core Table 5-4.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Laspistol | Pistol | 30m | S/2/- | 1d10+2 E | 0 | 30 | Half | Reliable | 1.5 | Common | Existing + RT Core Table 5-4 |
| Lasgun | Basic | 100m | S/3/- | 1d10+3 E | 0 | 60 | Half | Reliable | 4 | Common | Existing + RT Core Table 5-4 |
| Long-Las | Basic | 150m | S/-/- | 1d10+3 E | 1 | 40 | Full | Accurate, Reliable, Felling(4) | 4.5 | Scarce | Existing + RT Core Table 5-4 |
| Hot-Shot Laspistol | Pistol | 20m | S/2/- | 1d10+4 E | 7 | 40 | 2 Full | — | — | — | Existing |
| Hot-Shot Lasgun | Basic | 60m | S/3/- | 1d10+4 E | 7 | 30 | 2 Full | — | — | — | Existing |
| Lascannon | Heavy | 300m | S/-/- | 5d10+10 E | 10 | 5 | 2 Full | Proven(3), Reliable | 55 | Very Rare | Existing + RT Core Table 5-4 |
| Multi-laser | Heavy | 150m | -/-/5 | 2d10+10 E | 2 | 100 | 2 Full | Reliable | — | — | Existing |
| Archeotech Laspistol | Pistol | 90m | S/3/- | 1d10+3 E | 2 | 70 | Full | Accurate, Reliable | 4 | Near Unique | RT Core Table 5-4 |
| Belasco Dueling Pistol | Pistol | 45m | S/-/- | 1d10+5 E | 4 | 1 | Full | Accurate | 1.5 | Very Rare | RT Core Table 5-4 |
| Hellpistol (Lucius) | Pistol | 35m | S/2/- | 1d10+4 E | 7 | 40 | 2 Full | — | 4 | Rare | RT Core Table 5-4 |
| Hellgun (Lucius) | Basic | 110m | S/3/- | 1d10+4 E | 7 | 30 | 2 Full | — | 6 | Rare | RT Core Table 5-4 |
| Las Gauntlets | Pistol | 50m | S/4/- | 1d10+4 E | 1 | 20 | Full | Reliable | 3 | Very Rare | RT Core Table 5-4 |
| Lascarbine (Locke) | Basic | 60m | S/2/- | 1d10+3 E | 0 | 40 | 2 Full | Reliable | 2.5 | Scarce | RT Core Table 5-4 |

---

## Solid Projectile (SP) Weapons

> Source: rows reconciled from Existing Target and RT Core Table 5-4. Existing curated RoF/Range kept where they differ from Source B (single-source curated values). NEW rows cite RT Core Table 5-4.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Stub Automatic | Pistol | 30m | S/3/- | 1d10+3 I | 0 | 9 | Half | — | 1.5 | Plentiful | Existing + RT Core Table 5-4 |
| Stub Revolver | Pistol | 30m | S/-/- | 1d10+3 I | 0 | 6 | 2 Half | Reliable | 1 | Plentiful | Existing + RT Core Table 5-4 |
| Autopistol | Pistol | 30m | S/-/6 | 1d10+2 I | 0 | 18 | Half | — | 2.5 | Common | Existing + RT Core Table 5-4 |
| Autogun | Basic | 100m | S/3/- | 1d10+3 I | 0 | 30 | Half | — | 3.5 | Average | Existing + RT Core Table 5-4 |
| Combat Shotgun | Basic | 30m | S/3/- | 1d10+4 I | 0 | 18 | Half | Scatter | — | — | Existing |
| Pump Shotgun | Basic | 30m | S/-/- | 1d10+4 I | 0 | 8 | 2 Half | Scatter, Reliable | 5 | Average | Existing + RT Core Table 5-4 |
| Heavy Stubber | Heavy | 100m | -/-/8 | 1d10+4 I | 3 | 200 | 2 Full | — | 35 | Average | Existing + RT Core Table 5-4 |
| Autocannon | Heavy | 300m | S/3/- | 3d10+8 I | 6 | 20 | 2 Full | Reliable | — | — | Existing |
| Hand Cannon | Pistol | 35m | S/-/- | 1d10+4 I | 2 | 5 | 2 Full | — | 3 | Average | RT Core Table 5-4 |
| Heavy Stubber (Orthlack) | Heavy | 120m | -/-/10 | 1d10+5 I | 3 | 200 | 2 Full | — | 35 | Average | RT Core Table 5-4 |
| Heavy Stubber (Ursid) | Heavy | 120m | -/-/10 | 1d10+5 I | 3 | 40 | Full | — | 35 | Scarce | RT Core Table 5-4 |
| Naval Pistol (Mars) | Pistol | 20m | S/3/- | 1d10+4 I | 0 | 6 | Full | Tearing | 3 | Scarce | RT Core Table 5-4 |
| Naval Shotcannon | Heavy | 40m | S/3/- | 2d10+4 I | 0 | 24 | 2 Full | Scatter, Unreliable | 7 | Scarce | RT Core Table 5-4 |
| Shotgun | Basic | 30m | S/-/- | 1d10+4 I | 0 | 2 | 2 Full | Scatter | 5 | Common | RT Core Table 5-4 |
| Shotgun Pistol | Pistol | 10m | S/-/- | 1d10+4 I | 0 | 1 | Full | Reliable, Scatter | 1 | Average | RT Core Table 5-4 |

---

## Bolt Weapons

> Source: rows reconciled from Existing Target and RT Core Table 5-4. Heavy Bolter Damage is a named conflict resolved to Source A `1d10+8 X` (Liber Imperium §6). Existing curated Range/RoF/Clip kept where they differ from Source B.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Bolt Pistol | Pistol | 30m | S/2/- | 1d10+5 X | 4 | 8 | Full | Tearing | 3.5 | Rare | Existing + RT Core Table 5-4 |
| Boltgun | Basic | 100m | S/2/- | 1d10+5 X | 4 | 24 | Full | Tearing | 7 | Very Rare | Existing + RT Core Table 5-4 |
| Heavy Bolter | Heavy | 150m | -/-/6 | 1d10+8 X | 5 | 60 | 2 Full | Tearing | 40 | Very Rare | Existing + RT Core Table 5-4 (Damage: Liber Imperium §6) |
| Storm Bolter | Basic | 90m | S/2/4 | 1d10+5 X | 4 | 60 | 2 Full | Tearing, Storm | 9 | Extremely Rare | Existing + RT Core Table 5-4 |

---

## Plasma Weapons

> Source: rows reconciled from Existing Target and RT Core Table 5-4. Existing curated Clip/Reload and Qualities (Concussive/Overheats spellings) kept where they differ from Source B (single-source curated values); weight/availability added from Table 5-4.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Plasma Pistol | Pistol | 30m | S/2/- | 1d10+6 E | 6 | 10 | 3 Full | Overheats | 4 | Very Rare | Existing + RT Core Table 5-4 |
| Plasma Gun | Basic | 90m | S/2/- | 1d10+7 E | 6 | 20 | 4 Full | Overheats | 18 | Very Rare | Existing + RT Core Table 5-4 |
| Plasma Cannon | Heavy | 120m | S/-/- | 2d10+10 E | 8 | 16 | 5 Full | Blast(1), Overheats | 40 | Very Rare | Existing + RT Core Table 5-4 |

---

## Melta Weapons

> Source: rows reconciled from Existing Target and RT Core Table 5-4. Inferno Pistol Pen (12) and Multi-Melta Damage (2d10+16 E) are named conflicts resolved to Source A (Liber Imperium §6). Inferno Pistol Damage is also overwritten to the Source A value (2d10+10 E) as a precedence consequence (A > Existing). Meltagun (not a named conflict) takes Source B Damage/Pen per Precedence (B > Existing). Existing curated Melta quality retained where Source B omitted it. NEW rows cite RT Core Table 5-4.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Inferno Pistol | Pistol | 10m | S/-/- | 2d10+10 E | 12 | 3 | Full | Melta | 2.5 | Very Rare | Existing + RT Core Table 5-4 (Damage/Pen: Liber Imperium §6) |
| Meltagun | Basic | 20m | S/-/- | 2d10+8 E | 13 | 5 | 2 Full | Melta | 40 | Rare | RT Core Table 5-4 + Existing |
| Multi-Melta | Heavy | 60m | S/-/- | 2d10+16 E | 12 | 12 | 2 Full | Blast(1), Melta | 40 | Very Rare | Existing + RT Core Table 5-4 (Damage: Liber Imperium §6) |
| Meltagun (Mezoa) | Basic | 20m | S/-/- | 2d10+8 E | 13 | 10 | 3 Full | — | 46 | Rare | RT Core Table 5-4 |
| Thermal Lance (Mars) | Heavy | 10m | S/-/- | 2d10+10 E | 12 | 2 | 2 Full | Accurate | 40 | Rare | RT Core Table 5-4 |

*Melta: At half range or less, roll extra d10 for damage (take best two)*

---

## Flame Weapons

> Source: rows reconciled from Existing Target and RT Core Table 5-4. Heavy Flamer (not a named conflict) takes Source B Damage `2d10+4 E` per Precedence (B > Existing `1d10+5 E`). Existing curated Spray quality retained where Source B omitted it; weight/availability added from Table 5-4.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Hand Flamer | Pistol | 10m | S/-/- | 1d10+4 E | 2 | 2 | 2 Full | Flame, Spray | 3.5 | Rare | Existing + RT Core Table 5-4 |
| Flamer | Basic | 20m | S/-/- | 1d10+4 E | 2 | 6 | 2 Full | Flame, Spray | 6 | Scarce | Existing + RT Core Table 5-4 |
| Heavy Flamer | Heavy | 30m | S/-/- | 2d10+4 E | 4 | 10 | 2 Full | Flame, Spray | 20 | Rare | RT Core Table 5-4 + Existing |

---

## Primitive Ranged Weapons

> Source: RT Core Table 5-4 (all rows).

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Bolas | Thrown | 10m | S/-/- | — | 0 | 1 | — | Primitive, Snare, Inaccurate | 1.5 | Average | RT Core Table 5-4 |
| Bow | Basic | 30m | S/-/- | 1d10 R | 0 | 1 | Half | Primitive, Reliable | 2 | Common | RT Core Table 5-4 |
| Crossbow | Basic | 30m | S/-/- | 1d10 R | 0 | 1 | 2 Full | Primitive | 3 | Common | RT Core Table 5-4 |
| Hand Bow | Pistol | 15m | S/-/- | 1d10 R | 0 | 1 | Full | Primitive | 1 | Rare | RT Core Table 5-4 |
| Flintlock Pistol | Pistol | 15m | S/-/- | 1d10+2 I | 0 | 1 | 3 Full | Primitive, Unreliable, Inaccurate | 4 | Common | RT Core Table 5-4 |
| Musket | Basic | 30m | S/-/- | 1d10+2 I | 0 | 1 | 5 Full | Primitive, Unreliable, Inaccurate | 7 | Common | RT Core Table 5-4 |
| Sling | Basic | 15m | S/-/- | 1d10-2 I | 0 | 1 | Full | Primitive | 0.5 | Plentiful | RT Core Table 5-4 |

---

## Launchers

> Source: RT Core Table 5-4 (all rows).
> `†` Grenade/Missile Launcher Damage, Pen, and Special vary with the ammunition loaded.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Grenade Launcher (Mezoa) | Basic | 80m | S/-/- | † | † | 1 | Half | † | 10 | Scarce | RT Core Table 5-4 |
| Grenade Launcher (Voss) | Basic | 60m | S/-/- | † | † | 6 | Full | †, Inaccurate | 12.5 | Scarce | RT Core Table 5-4 |
| Missile Launcher (Locke) | Heavy | 250m | S/-/- | † | † | 1 | Full | † | 12 | Scarce | RT Core Table 5-4 |
| Missile Launcher (Retobi) | Heavy | 200m | S/-/- | † | † | 5 | 2 Full | † | 35 | Rare | RT Core Table 5-4 |

---

## Exotic Ranged Weapons

> Source: RT Core Table 5-4 (all rows).
> `†` Ork weapons: Inaccurate, Unreliable.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Crux Beam Gun | Basic | 80m | S/3/- | 2d10+5 E | 6 | 25 | 4 Full | Scatter | 4 | Near Unique | RT Core Table 5-4 |
| Dartcaster | Pistol | 30m | S/-/- | 1d10 R | 0 | 1 | Full | Toxic | 2.5 | Rare | RT Core Table 5-4 |
| Digi-laser | Pistol | 3m | S/-/- | 1d10+3 E | 7 | 1 | Full | Reliable | 0.5 | Extremely Rare | RT Core Table 5-4 |
| Digi-melta | Pistol | 3m | S/-/- | 2d10+4 E | 12 | 1 | Full | — | 0.5 | Extremely Rare | RT Core Table 5-4 |
| Digi-needler | Pistol | 3m | S/-/- | 1d10 R | 0 | 1 | Full | Toxic | 0.5 | Extremely Rare | RT Core Table 5-4 |
| Digi-flamer | Pistol | 3m | S/-/- | 1d10+4 E | 2 | 1 | Full | Flame | 0.5 | Extremely Rare | RT Core Table 5-4 |
| Graviton Gun | Basic | 30m | S/-/- | Special | — | 3 | 2 Full | Blast(5) | 5 | Near Unique | RT Core Table 5-4 |
| Kroot Rifle | Basic | 110m | S/2/- | 1d10+5 E | 1 | 6 | 2 Full | — | 6 | Extremely Rare | RT Core Table 5-4 |
| Kroot Rifle (Melee) | Melee | — | — | 1d10 R | 0 | — | — | Balanced | — | — | RT Core Table 5-4 |
| Needle Pistol | Pistol | 30m | S/-/- | 1d10 R | 0 | 6 | Full | Accurate, Toxic | 1.5 | Very Rare | RT Core Table 5-4 |
| Needle Rifle | Basic | 180m | S/-/- | 1d10 R | 0 | 6 | 2 Full | Accurate, Toxic | 2 | Very Rare | RT Core Table 5-4 |
| Ork Shoota | Basic | 60m | S/3/10 | 1d10+4 I | 0 | 30 | Full | Inaccurate, Unreliable † | 4 | Scarce | RT Core Table 5-4 |
| Ork Slugga | Pistol | 20m | S/3/- | 1d10+4 I | 0 | 18 | Full | Inaccurate, Unreliable † | 2 | Scarce | RT Core Table 5-4 |
| Shuriken Catapult | Basic | 60m | S/3/10 | 1d10+4 R | 6 | 100 | 2 Full | Reliable | 2.5 | Very Rare | RT Core Table 5-4 |
| Shuriken Pistol | Pistol | 30m | S/3/5 | 1d10+2 R | 4 | 40 | 2 Full | Reliable | 1.2 | Very Rare | RT Core Table 5-4 |

---

## Grenades & Explosives

> Source: rows reconciled from Existing Target and RT Core Table 5-4 (Grenades & Missiles). Overlapping items (Frag, Krak, Photon Flash, Smoke, Hallucinogen) take Source B differing stat values per Precedence (B > Existing); Existing-only qualities (Ogryn-Proof, Concussive(0), Blind(4)) are retained as single-source values. Fire Bomb is Existing-only. Weight/availability added from Table 5-4. Footnotes preserved verbatim.
> `†` Anti-Plant rounds only affect flora and have no other effect.
> `††` Blind grenades: unlike smoke, this only interferes with visual sight.
> `†††` Virus: if the grenade causes any Damage, then each round after the first, check for another target at random (friend or foe) within d5 metres to see if any Damage is caused to them (rolling new Damage to represent the mutated virus). Continue until Damage is not taken or after d10 rounds have passed.

| Name | Class | Range | Damage | Pen | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|---|
| Anti-Plant | Thrown | SBx3 | 3d10 E | 0 | Blast(3) † | 0.5 | Very Rare | RT Core Table 5-4 |
| Blind | Thrown | SBx3 | — | 0 | Smoke | 0.5 | Rare | RT Core Table 5-4 |
| Filament | Thrown | SBx3 | 4d10+4 R | 6 | Blast(1), Tearing | 0.5 | Extremely Rare | RT Core Table 5-4 |
| Frag Grenade | Thrown | SBx3 | 2d10 X | 0 | Blast(4), Ogryn-Proof | 0.5 | Common | Existing + RT Core Table 5-4 |
| Frag Missile | — | — | 2d10 X | 4 | Blast(6) | 1 | Average | RT Core Table 5-4 |
| Geode | Thrown | SBx3 | 2d10+3 R | 4 | Blast(3) | 0.5 | Extremely Rare | RT Core Table 5-4 |
| Hallucinogen | Thrown | SBx3 | — | 0 | Blast(5), Hallucinogenic(2) | 0.5 | Rare | Existing + RT Core Table 5-4 |
| Krak Grenade | Thrown | SBx3 | 2d10+4 X | 6 | Concussive(0) | 0.5 | Rare | Existing + RT Core Table 5-4 |
| Krak Missile | — | — | 3d10+10 X | 10 | Blast(1) | 1 | Scarce | RT Core Table 5-4 |
| Photon Flash | Thrown | SBx3 | — | 0 | Blind(4), Blast(10) | 0.5 | Rare | Existing + RT Core Table 5-4 |
| Plasma | Thrown | SBx3 | 1d10+6 | 6 | Blast(1) | 4 | Very Rare | RT Core Table 5-4 |
| Smoke Grenade | Thrown | SBx3 | — | 0 | Smoke(6) †† | 0.5 | Common | Existing + RT Core Table 5-4 |
| Stun | Thrown | SBx3 | — | 0 | Blast(3) | 0.2 | Scarce | RT Core Table 5-4 |
| Virus | Thrown | SBx3 | 3d10 I | 0 | Toxic ††† | 0.5 | Extremely Rare | RT Core Table 5-4 |
| Fire Bomb | Thrown | SBx3 | 1d10+3 E | 0 | Blast(3), Flame | — | — | Existing |

---

## Melee Weapons — Primitive / Low-Tech

> Source: rows reconciled from Existing Target and RT Core Table 5-8. Existing curated `+SB` Damage formulas and Existing-only qualities (Primitive(7)/(8) magnitudes, Concussive(0), Defensive) retained where Source B omitted or differed. Weight/availability added from Table 5-8. NEW rows (Groxwhip, Improvised, Kraken Tooth Dagger, Truncheon) cite RT Core Table 5-8.
> `†††` Shield provides Armour 2 to the Body and the Arm wielding the Shield.

| Name | Range | Damage | Pen | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|---|
| Knife | 5m | 1d5+SB R | 0 | Primitive | 1 | Plentiful | Existing + RT Core Table 5-8 |
| Sword | — | 1d10+SB R | 0 | Balanced, Primitive | 3 | Common | Existing + RT Core Table 5-8 |
| Great Weapon | — | 2d10+SB R | 0 | Unbalanced, Primitive | 7 | Scarce | Existing + RT Core Table 5-8 |
| Axe | — | 1d10+2+SB R | 0 | Unbalanced | — | — | Existing |
| Shield | — | 1d5+SB I | 0 | Defensive, Primitive(7) ††† | 3 | Common | Existing + RT Core Table 5-8 |
| Staff | — | 1d10+SB I | 0 | Balanced, Primitive(7) | 3 | Plentiful | Existing + RT Core Table 5-8 |
| Spear | — | 1d10+SB R | 0 | Primitive(8) | 3 | Common | Existing + RT Core Table 5-8 |
| Warhammer | — | 1d10+3+SB I | 1 | Concussive(0), Unbalanced, Primitive | 4.5 | Scarce | Existing + RT Core Table 5-8 |
| Groxwhip | 3m | 1d10+3 R | 0 | Flexible, Tearing, Primitive | 4 | Scarce | RT Core Table 5-8 |
| Improvised | — | 1d10-2 I | 0 | Primitive, Unbalanced | — | — | RT Core Table 5-8 |
| Kraken Tooth Dagger | 5m | 1d5+1 R | 1 | Primitive | 0.4 | Extremely Rare | RT Core Table 5-8 |
| Truncheon | — | 1d10 I | 0 | Primitive | 2 | Plentiful | RT Core Table 5-8 |

---

## Melee Weapons — Shock

> Source: RT Core Table 5-7 (Shock Weapons). All rows NEW.

| Name | Damage | Pen | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|
| Officer's Cutlass | 1d10 R | 0 | Shocking | 3 | Scarce | RT Core Table 5-7 |
| Shock Glove | 1d10 I | 0 | Shocking | 1.5 | Rare | RT Core Table 5-7 |
| Shock-Staff | 1d5+3 I | 0 | Shocking | 2 | Rare | RT Core Table 5-7 |

---

## Melee Weapons — Chain

> Source: rows reconciled from Existing Target and RT Core Table 5-5. Existing curated `+SB` Damage formulas retained (Source B lists base Damage without SB; SB is added in play). Existing-only qualities (Chain Axe Unbalanced) retained. Weight/availability added from Table 5-5. Eviscerator is Existing-only.

| Name | Damage | Pen | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|
| Chainsword | 1d10+2+SB R | 2 | Balanced, Tearing | 6 | Average | Existing + RT Core Table 5-5 |
| Chain Axe | 1d10+4+SB R | 2 | Tearing, Unbalanced | 13 | Average | Existing + RT Core Table 5-5 |
| Eviscerator | 1d10+9+SB R | 5 | Tearing, Razor Sharp, Unwieldy | — | — | Existing |

---

## Melee Weapons — Power

> Source: rows reconciled from Existing Target and RT Core Table 5-6. Existing curated `+SB` Damage formulas and Existing-only qualities (Power Maul Concussive(1); Thunder Hammer/Lightning Claw whole rows) retained. Weight/availability added from Table 5-6. NEW rows cite RT Core Table 5-6.
> `†` Power Fists add the user's SB×2 to the Damage.

| Name | Damage | Pen | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|
| Power Sword | 1d10+5+SB E | 5 | Balanced, Power Field | 3 | Very Rare | Existing + RT Core Table 5-6 |
| Power Axe | 1d10+7+SB E | 7 | Power Field, Unbalanced | 6 | Very Rare | Existing + RT Core Table 5-6 |
| Power Maul | 1d10+5+SB E | 4 | Power Field, Concussive(1), Shocking | 3.5 | Very Rare | Existing + RT Core Table 5-6 |
| Power Fist | 2d10 † +SB E | 9 | Power Field, Unwieldy | 13 | Very Rare | Existing + RT Core Table 5-6 |
| Thunder Hammer | 2d10+5+SB E | 10 | Power Field, Concussive(3), Unwieldy | — | — | Existing |
| Lightning Claw | 1d10+6+SB E | 8 | Power Field, Razor Sharp | — | — | Existing |
| Omnissian Axe (Sollex) | 2d10+4 E | 6 | Power Field, Unbalanced | 8 | Extremely Rare | RT Core Table 5-6 |
| Power Maul (Low) | 1d10+1 E | 2 | Shocking | 3.5 | Very Rare | RT Core Table 5-6 |

---

## Melee Weapons — Exotic/Xenos

> Source: rows reconciled from Existing Target and RT Core Table 5-7. Choppa (Ork) ↔ Source B "Ork Choppa": Existing curated `+SB` Damage retained; differing Pen resolved to Source B (Pen 2) per Precedence (B > Existing); weight/availability added. Power Klaw, Big Choppa, 'Uge Choppa are Existing-only. NEW rows cite RT Core Table 5-7.
> `††` Unlike other melee weapons, do not add the wielder's Strength bonus to the damage inflicted by a Harlequin's Kiss.

| Name | Damage | Pen | Qualities | Weight (kg) | Availability | Source |
|---|---|---|---|---|---|---|
| Power Klaw (Ork) | 2d10+SB I | 7 | Power Field, Unwieldy | — | — | Existing |
| Choppa (Ork) | 1d10+2+SB R | 2 | Unbalanced | 8 | Scarce | Existing + RT Core Table 5-7 |
| Big Choppa (Ork) | 2d5+4+SB R | 2 | Unbalanced | — | — | Existing |
| 'Uge Choppa (Ork) | 2d10+SB R | 0 | Unbalanced, Unwieldy | — | — | Existing |
| Fractal Blade | 1d10+1 R | 7 | Power Field, Balanced | 1 | Extremely Rare | RT Core Table 5-7 |
| Ghost Sword | 1d10+3 E | 6 | Power Field, Balanced | 1 | Extremely Rare | RT Core Table 5-7 |
| Harlequin's Kiss | 1d10+8 R †† | 10 | Tearing | 1 | Extremely Rare | RT Core Table 5-7 |

---

## Weapon Size Classes (for game implementation)

| Size Class | Description | One-Hand? | Example |
|---|---|---|---|
| Pistol | Small ranged, usable in melee | Yes | Laspistol, Bolt Pistol |
| Basic | Standard ranged | Two-hand | Lasgun, Autogun, Boltgun |
| Heavy | Crew-served / braced | Two-hand + brace | Heavy Bolter, Lascannon |
| Melee | Close combat | Varies | Chainsword (1h), Eviscerator (2h) |
| Thrown | Lobbed/hurled | One-hand | Grenades, Knives |

---

## Weapon Groups (for Talent requirements)

- **Las**: Laspistol, Lasgun, Long-Las, Hot-Shot, Lascannon
- **SP (Solid Projectile)**: Autopistol, Autogun, Shotgun, Stub, Heavy Stubber
- **Bolt**: Bolt Pistol, Boltgun, Heavy Bolter, Storm Bolter
- **Plasma**: Plasma Pistol, Plasma Gun, Plasma Cannon
- **Melta**: Inferno Pistol, Meltagun, Multi-Melta
- **Flame**: Hand Flamer, Flamer, Heavy Flamer
- **Launcher**: Grenade Launcher, Missile Launcher
- **Exotic**: Xenos weapons, unique archaeotech
- **Chain**: Chainsword, Chain Axe, Eviscerator
- **Power**: Power Sword, Power Axe, Power Fist, etc.
- **Primitive**: Sword, Axe, Knife, Spear, etc.
- **Shock**: Shock Maul, Neural Whip
