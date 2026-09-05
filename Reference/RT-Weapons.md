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

> Source: rows reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-4 (B) > Existing**. Source A §6 supplies a full Las Weapon Profiles table, so its values win field-by-field where they differ (notably the **Variable** quality on nearly every las weapon, plus weight/availability and several RoF/Reload/Class values). Existing-only curated qualities that Source A does not contradict (e.g. Long-Las Reliable) are retained as single-source values. Source B-only patterned rows (Hellpistol/Hellgun (Lucius), Archeotech Laspistol, Belasco Dueling Pistol, Las Gauntlets, Lascarbine (Locke)) are kept distinct and cite Table 5-4.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Laspistol | Pistol | 30m | S/2/- | 1d10+2 E | 0 | 30 | Half | Reliable, Variable | 2 | Average | Liber Imperium §6 (Qualities/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Lasgun | Basic | 100m | S/2/3 | 1d10+3 E | 0 | 60 | Full | Reliable, Variable | 4 | Common | Liber Imperium §6 (RoF/Reload/Qualities) + Existing + RT Core Table 5-4 | ImperialHuman |
| Long-Las | Heavy | 150m | S/-/- | 1d10+3 E | 1 | 40 | Full | Accurate, Deadly, Felling(2), Reliable, Variable | 5 | Scarce | Liber Imperium §6 (Class/Pen/Felling/Deadly/Variable/Weight) + Existing (Reliable) + RT Core Table 5-4 | ImperialHuman |
| Hot-Shot Laspistol | Pistol | 20m | S/2/- | 1d10+4 E | 7 | 40 | 2 Full | Variable | 4 | Rare | Liber Imperium §6 (Qualities/Weight/Availability) + Existing | ImperialHuman |
| Hot-Shot Lasgun | Basic | 60m | S/3/- | 1d10+4 E | 7 | 30 | 2 Full | Variable | 6 | Rare | Liber Imperium §6 (Qualities/Weight/Availability) + Existing | ImperialHuman |
| Lascannon | Heavy | 300m | S/-/- | 5d10+10 E | 10 | 5 | 2 Full | Proven(3), Reliable | 55 | Very Rare | Liber Imperium §6 (Proven(3)/Weight) + Existing (Reliable) + RT Core Table 5-4 | ImperialHuman |
| Multi-Laser | Heavy | 150m | -/-/5 | 2d10+10 E | 2 | 100 | 2 Full | Reliable | 35 | Very Rare | Liber Imperium §6 (Weight/Availability) + Existing | ImperialHuman |
| Archeotech Laspistol | Pistol | 90m | S/3/- | 1d10+3 E | 2 | 70 | Full | Accurate, Reliable | 4 | Near Unique | RT Core Table 5-4 | ImperialHuman |
| Belasco Dueling Pistol | Pistol | 45m | S/-/- | 1d10+5 E | 4 | 1 | Full | Accurate | 1.5 | Very Rare | RT Core Table 5-4 | ImperialHuman |
| Hellpistol (Lucius) | Pistol | 35m | S/2/- | 1d10+4 E | 7 | 40 | 2 Full | — | 4 | Rare | RT Core Table 5-4 | ImperialHuman |
| Hellgun (Lucius) | Basic | 110m | S/3/- | 1d10+4 E | 7 | 30 | 2 Full | — | 6 | Rare | RT Core Table 5-4 | ImperialHuman |
| Las Gauntlets | Pistol | 50m | S/4/- | 1d10+4 E | 1 | 20 | Full | Reliable | 3 | Very Rare | RT Core Table 5-4 | ImperialHuman |
| Lascarbine (Locke) | Basic | 60m | S/2/- | 1d10+3 E | 0 | 40 | 2 Full | Reliable | 2.5 | Scarce | RT Core Table 5-4 | ImperialHuman |

> `Variable`: a las weapon with the Variable Quality can adjust its power output; see Liber Imperium §6.1 weapon-quality glossary.

---

## Solid Projectile (SP) Weapons

> Source: rows reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-4 (B) > Existing**. Source A §6 supplies a full Solid Projectile Weapon Profiles table; its values win field-by-field where they differ (RoF, Reload, Clip, added Reliable qualities, weight, availability). Source A "Automatic Shotgun" reconciles to Existing "Combat Shotgun"; Source A "Shotpistol" reconciles to Existing "Shotgun Pistol". Source B-only patterned rows (Heavy Stubber (Orthlack)/(Ursid), Naval Pistol, Naval Shotcannon, and the distinct Clip-2 Shotgun) are kept and cite Table 5-4.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Stub Automatic | Pistol | 30m | S/2/- | 1d10+3 I | 0 | 9 | Full | Reliable | 2 | Common | Liber Imperium §6 (Reload/Qualities/Weight/Availability) + Existing + RT Core Table 5-4 | Universal |
| Stub Revolver | Pistol | 30m | S/-/- | 1d10+3 I | 0 | 6 | 2 Full | Reliable | 2 | Plentiful | Liber Imperium §6 (Reload/Weight) + Existing + RT Core Table 5-4 | Universal |
| Autopistol | Pistol | 30m | S/3/6 | 1d10+2 I | 0 | 18 | Full | — | 2 | Average | Liber Imperium §6 (RoF/Reload/Weight/Availability) + Existing + RT Core Table 5-4 | Universal |
| Autogun | Basic | 100m | S/3/10 | 1d10+3 I | 0 | 30 | Full | — | 5 | Average | Liber Imperium §6 (RoF/Reload/Weight) + Existing + RT Core Table 5-4 | Universal |
| Combat Shotgun | Basic | 30m | S/3/- | 1d10+4 I | 0 | 18 | Full | Reliable, Scatter | 5 | Scarce | Liber Imperium §6 (Reload/Qualities/Weight/Availability, as "Automatic Shotgun") + Existing | Universal |
| Pump Shotgun | Basic | 30m | S/-/- | 1d10+4 I | 0 | 8 | 2 Full | Scatter, Reliable | 5 | Average | Liber Imperium §6 (Reload/Weight, as "Shotgun") + Existing + RT Core Table 5-4 | Universal |
| Heavy Stubber | Heavy | 100m | -/-/8 | 1d10+4 I | 3 | 80 | 2 Full | Reliable | 30 | Rare | Liber Imperium §6 (Clip/Qualities/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Autocannon | Heavy | 300m | S/3/- | 3d10+8 I | 6 | 15 | Full | Reliable | 40 | Very Rare | Liber Imperium §6 (Clip/Reload/Weight/Availability) + Existing | ImperialHuman |
| Hand Cannon | Pistol | 35m | S/-/- | 1d10+4 I | 2 | 5 | 2 Full | — | 3 | Scarce | Liber Imperium §6 (Availability) + RT Core Table 5-4 | Universal |
| Heavy Stubber (Orthlack) | Heavy | 120m | -/-/10 | 1d10+5 I | 3 | 200 | 2 Full | — | 35 | Average | RT Core Table 5-4 | ImperialHuman |
| Heavy Stubber (Ursid) | Heavy | 120m | -/-/10 | 1d10+5 I | 3 | 40 | Full | — | 35 | Scarce | RT Core Table 5-4 | ImperialHuman |
| Naval Pistol (Mars) | Pistol | 20m | S/3/- | 1d10+4 I | 0 | 6 | Full | Tearing | 3 | Scarce | RT Core Table 5-4 | ImperialHuman |
| Naval Shotcannon | Heavy | 40m | S/3/- | 2d10+4 I | 0 | 24 | 2 Full | Scatter, Unreliable | 7 | Scarce | RT Core Table 5-4 | ImperialHuman |
| Shotgun | Basic | 30m | S/-/- | 1d10+4 I | 0 | 2 | 2 Full | Scatter | 5 | Common | RT Core Table 5-4 | Universal |
| Shotgun Pistol | Pistol | 15m | S/-/- | 1d10+3 I | 0 | 4 | 2 Full | Reliable, Scatter | 3 | Average | Liber Imperium §6 (Range/Damage/Clip/Reload/Weight/Availability, as "Shotpistol") + Existing | Universal |

---

## Bolt Weapons

> Source: rows reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-4 (B) > Existing**. Source A §6 supplies a full Bolt Weapon Profiles table; its values win field-by-field where they differ. Heavy Bolter Damage `1d10+8 X` is the named conflict (Source A wins) and Source A additionally sets Heavy Bolter Reload to **Full**. Boltgun RoF becomes **S/2/3** (A); Bolt Pistol weight/availability become **4 kg / Very Rare** (A); Storm Bolter Damage/Pen become **1d10+4 X / Pen 2** with 10 kg (A).

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Bolt Pistol | Pistol | 30m | S/2/- | 1d10+5 X | 4 | 8 | Full | Tearing | 4 | Very Rare | Liber Imperium §6 (Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Boltgun | Basic | 100m | S/2/3 | 1d10+5 X | 4 | 24 | Full | Tearing | 7 | Very Rare | Liber Imperium §6 (RoF/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Heavy Bolter | Heavy | 150m | -/-/6 | 1d10+8 X | 5 | 60 | Full | Tearing | 40 | Very Rare | Liber Imperium §6 (Damage/Reload/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Storm Bolter | Basic | 90m | S/2/4 | 1d10+4 X | 2 | 60 | 2 Full | Tearing, Storm | 10 | Extremely Rare | Liber Imperium §6 (Damage/Pen/Weight) + Existing + RT Core Table 5-4 | ImperialHuman |

---

## Plasma Weapons

> Source: rows reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-4 (B) > Existing**. Source A §6 supplies a full Plasma Weapon Profiles table; its values win field-by-field where they differ. Source A sets Penetration to **8** across the family (Existing had 6), Plasma Gun to **Clip 40 / 5 Full** (Existing 20 / 4 Full), and the authoritative Qualities to **Maximal, Overheat** (Source A spelling; the Existing "Overheats" is superseded).

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Plasma Pistol | Pistol | 30m | S/2/- | 1d10+6 E | 8 | 10 | 3 Full | Maximal, Overheat | 4 | Very Rare | Liber Imperium §6 (Pen/Qualities/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Plasma Gun | Basic | 90m | S/2/- | 1d10+7 E | 8 | 40 | 5 Full | Maximal, Overheat | 18 | Very Rare | Liber Imperium §6 (Pen/Clip/Reload/Qualities/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Plasma Cannon | Heavy | 120m | S/-/- | 2d10+10 E | 8 | 16 | 5 Full | Blast(1), Maximal, Overheat | 40 | Very Rare | Liber Imperium §6 (Qualities/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |

---

## Melta Weapons

> Source: rows reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-4 (B) > Existing**. Source A §6 supplies a full Melta Weapon Profiles table; its values win field-by-field where they differ. Named conflicts (Inferno Pistol Pen 12, Multi-Melta Damage 2d10+16 E) resolve to Source A, and Source A further sets: Inferno Pistol **3 kg / Near Unique**; Meltagun **2d10+10 E, Pen 12, Clip 5, Full, 15 kg, Very Rare** (Source B "Meltagun (Mars)" 2d10+8 E / Pen 13 / 2 Full / 40 kg / Rare discarded); Multi-Melta Reload **Full** and Availability **Extremely Rare**. Source B-only patterned rows (Meltagun (Mezoa), Thermal Lance (Mars)) are kept and cite Table 5-4.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Inferno Pistol | Pistol | 10m | S/-/- | 2d10+10 E | 12 | 3 | Full | Melta | 3 | Near Unique | Liber Imperium §6 (Damage/Pen/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Meltagun | Basic | 20m | S/-/- | 2d10+10 E | 12 | 5 | Full | Melta | 15 | Very Rare | Liber Imperium §6 (Damage/Pen/Reload/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Multi-Melta | Heavy | 60m | S/-/- | 2d10+16 E | 12 | 12 | Full | Blast(1), Melta | 40 | Extremely Rare | Liber Imperium §6 (Damage/Pen/Reload/Weight/Availability) + Existing + RT Core Table 5-4 | ImperialHuman |
| Meltagun (Mezoa) | Basic | 20m | S/-/- | 2d10+8 E | 13 | 10 | 3 Full | — | 46 | Rare | RT Core Table 5-4 | ImperialHuman |
| Thermal Lance (Mars) | Heavy | 10m | S/-/- | 2d10+10 E | 12 | 2 | 2 Full | Accurate | 40 | Rare | RT Core Table 5-4 | ImperialHuman |

*Melta: At half range or less, roll extra d10 for damage (take best two)*

---

## Flame Weapons

> Source: rows reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-4 (B) > Existing**. Source A §6 supplies a full Flame Weapon Profiles table; its values win field-by-field. Hand Flamer weight becomes **4 kg** (A; Existing 3.5). Heavy Flamer resolves to Source A `1d10+5 E, Pen 4, Clip 10, 2 Full, 45 kg, Rare` (Source B "Heavy Flamer (Locke)" 2d10+4 E / 20 kg discarded). Flamer matches across A/B/Existing (1d10+4 E, Pen 2, Clip 6, 2 Full, 6 kg, Scarce).

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Hand Flamer | Pistol | 10m | S/-/- | 1d10+4 E | 2 | 2 | 2 Full | Flame, Spray | 4 | Rare | Liber Imperium §6 (Weight) + Existing + RT Core Table 5-4 | ImperialHuman |
| Flamer | Basic | 20m | S/-/- | 1d10+4 E | 2 | 6 | 2 Full | Flame, Spray | 6 | Scarce | Liber Imperium §6 + Existing + RT Core Table 5-4 | ImperialHuman |
| Heavy Flamer | Heavy | 30m | S/-/- | 1d10+5 E | 4 | 10 | 2 Full | Flame, Spray | 45 | Rare | Liber Imperium §6 (Damage/Weight) + Existing + RT Core Table 5-4 | ImperialHuman |

---

## Primitive Ranged Weapons

> Source: reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-4 (B) > Existing**. Source A §6 Low-Tech Ranged Weapon Profiles supply Bow and Crossbow values, which win over the Source B rows (Bow → StBx5m, S/2/-, 1d10+1 R, Primitive(6); Crossbow → 1d10+2 R, Clip 8, Primitive(7)). Items with no Source A equivalent (Bolas, Hand Bow, Flintlock Pistol, Musket, Sling) remain Source B.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Bolas | Thrown | 10m | S/-/- | — | 0 | 1 | — | Primitive, Snare, Inaccurate | 1.5 | Average | RT Core Table 5-4 | Universal |
| Bow | Basic | SBx5m | S/2/- | 1d10+1 R | 0 | 1 | Half | Primitive(6), Reliable | 2 | Common | Liber Imperium §6 (Range/RoF/Damage/Qualities) + RT Core Table 5-4 | Universal |
| Crossbow | Basic | 30m | S/-/- | 1d10+2 R | 0 | 8 | 2 Full | Primitive(7) | 3 | Common | Liber Imperium §6 (Damage/Clip/Qualities) + RT Core Table 5-4 | Universal |
| Hand Bow | Pistol | 15m | S/-/- | 1d10 R | 0 | 1 | Full | Primitive | 1 | Rare | RT Core Table 5-4 | Universal |
| Flintlock Pistol | Pistol | 15m | S/-/- | 1d10+2 I | 0 | 1 | 3 Full | Primitive, Unreliable, Inaccurate | 4 | Common | RT Core Table 5-4 | Universal |
| Musket | Basic | 30m | S/-/- | 1d10+2 I | 0 | 1 | 5 Full | Primitive, Unreliable, Inaccurate | 7 | Common | RT Core Table 5-4 | Universal |
| Sling | Basic | 15m | S/-/- | 1d10-2 I | 0 | 1 | Full | Primitive | 0.5 | Plentiful | RT Core Table 5-4 | Universal |

---

## Launchers

> Source: RT Core Table 5-4 (all rows). Source A §6's Launcher Weapon Profiles list generic Grenade/Missile Launcher entries whose Damage/Pen also vary with the loaded round (recorded as `-`/`†`), so Source A supplies no overriding numeric values for these patterned rows.
> `†` Grenade/Missile Launcher Damage, Pen, and Special vary with the ammunition loaded.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Grenade Launcher (Mezoa) | Basic | 80m | S/-/- | † | † | 1 | Half | † | 10 | Scarce | RT Core Table 5-4 | ImperialHuman |
| Grenade Launcher (Voss) | Basic | 60m | S/-/- | † | † | 6 | Full | †, Inaccurate | 12.5 | Scarce | RT Core Table 5-4 | ImperialHuman |
| Missile Launcher (Locke) | Heavy | 250m | S/-/- | † | † | 1 | Full | † | 12 | Scarce | RT Core Table 5-4 | ImperialHuman |
| Missile Launcher (Retobi) | Heavy | 200m | S/-/- | † | † | 5 | 2 Full | † | 35 | Rare | RT Core Table 5-4 | ImperialHuman |

---

## Exotic Ranged Weapons

> Source: RT Core Table 5-4 (all rows). Source A §6's Exotic Ranged Weapon Profiles cover a distinct Adeptus Mechanicus / archeotech set (Galvanic, Conversion, Radium, Phosphor, Graviton, Web, etc.) with **no overlap** with the xenos weapons listed here, so no Source A override applies to this family.
> `†` Ork weapons: Inaccurate, Unreliable.

| Name | Class | Range | RoF | Damage | Pen | Clip | Reload | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Crux Beam Gun | Basic | 80m | S/3/- | 2d10+5 E | 6 | 25 | 4 Full | Scatter | 4 | Near Unique | RT Core Table 5-4 | ImperialHuman |
| Dartcaster | Pistol | 30m | S/-/- | 1d10 R | 0 | 1 | Full | Toxic | 2.5 | Rare | RT Core Table 5-4 | ImperialHuman |
| Digi-laser | Pistol | 3m | S/-/- | 1d10+3 E | 7 | 1 | Full | Reliable | 0.5 | Extremely Rare | RT Core Table 5-4 | ImperialHuman |
| Digi-melta | Pistol | 3m | S/-/- | 2d10+4 E | 12 | 1 | Full | — | 0.5 | Extremely Rare | RT Core Table 5-4 | ImperialHuman |
| Digi-needler | Pistol | 3m | S/-/- | 1d10 R | 0 | 1 | Full | Toxic | 0.5 | Extremely Rare | RT Core Table 5-4 | ImperialHuman |
| Digi-flamer | Pistol | 3m | S/-/- | 1d10+4 E | 2 | 1 | Full | Flame | 0.5 | Extremely Rare | RT Core Table 5-4 | ImperialHuman |
| Graviton Gun | Basic | 30m | S/-/- | Special | — | 3 | 2 Full | Blast(5) | 5 | Near Unique | RT Core Table 5-4 | ImperialHuman |
| Kroot Rifle | Basic | 110m | S/2/- | 1d10+5 E | 1 | 6 | 2 Full | — | 6 | Extremely Rare | RT Core Table 5-4 | Kroot |
| Kroot Rifle (Melee) | Melee | — | — | 1d10 R | 0 | — | — | Balanced | — | — | RT Core Table 5-4 | Kroot |
| Needle Pistol | Pistol | 30m | S/-/- | 1d10 R | 0 | 6 | Full | Accurate, Toxic | 1.5 | Very Rare | RT Core Table 5-4 | ImperialHuman |
| Needle Rifle | Basic | 180m | S/-/- | 1d10 R | 0 | 6 | 2 Full | Accurate, Toxic | 2 | Very Rare | RT Core Table 5-4 | ImperialHuman |
| Ork Shoota | Basic | 60m | S/3/10 | 1d10+4 I | 0 | 30 | Full | Inaccurate, Unreliable † | 4 | Scarce | RT Core Table 5-4 | Ork |
| Ork Slugga | Pistol | 20m | S/3/- | 1d10+4 I | 0 | 18 | Full | Inaccurate, Unreliable † | 2 | Scarce | RT Core Table 5-4 | Ork |
| Shuriken Catapult | Basic | 60m | S/3/10 | 1d10+4 R | 6 | 100 | 2 Full | Reliable | 2.5 | Very Rare | RT Core Table 5-4 | Eldar |
| Shuriken Pistol | Pistol | 30m | S/3/5 | 1d10+2 R | 4 | 40 | 2 Full | Reliable | 1.2 | Very Rare | RT Core Table 5-4 | Eldar |

---

## Grenades & Explosives

> Source: rows reconciled from Existing Target and RT Core Table 5-4 (Grenades & Missiles). Overlapping items (Frag, Krak, Photon Flash, Smoke, Hallucinogen) take Source B differing stat values per Precedence (B > Existing); Existing-only qualities (Ogryn-Proof, Concussive(0), Blind(4)) are retained as single-source values. Fire Bomb is Existing-only. Weight/availability added from Table 5-4. Footnotes preserved verbatim.
> `†` Anti-Plant rounds only affect flora and have no other effect.
> `††` Blind grenades: unlike smoke, this only interferes with visual sight.
> `†††` Virus: if the grenade causes any Damage, then each round after the first, check for another target at random (friend or foe) within d5 metres to see if any Damage is caused to them (rolling new Damage to represent the mutated virus). Continue until Damage is not taken or after d10 rounds have passed.

| Name | Class | Range | Damage | Pen | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|---|
| Anti-Plant | Thrown | SBx3 | 3d10 E | 0 | Blast(3) † | 0.5 | Very Rare | RT Core Table 5-4 | ImperialHuman |
| Blind | Thrown | SBx3 | — | 0 | Smoke | 0.5 | Rare | RT Core Table 5-4 | ImperialHuman |
| Filament | Thrown | SBx3 | 4d10+4 R | 6 | Blast(1), Tearing | 0.5 | Extremely Rare | RT Core Table 5-4 | ImperialHuman |
| Frag Grenade | Thrown | SBx3 | 2d10 X | 0 | Blast(4), Ogryn-Proof | 0.5 | Common | Existing + RT Core Table 5-4 | ImperialHuman |
| Frag Missile | — | — | 2d10 X | 4 | Blast(6) | 1 | Average | RT Core Table 5-4 | ImperialHuman |
| Geode | Thrown | SBx3 | 2d10+3 R | 4 | Blast(3) | 0.5 | Extremely Rare | RT Core Table 5-4 | ImperialHuman |
| Hallucinogen | Thrown | SBx3 | — | 0 | Blast(5), Hallucinogenic(2) | 0.5 | Rare | Existing + RT Core Table 5-4 | ImperialHuman |
| Krak Grenade | Thrown | SBx3 | 2d10+4 X | 6 | Concussive(0) | 0.5 | Rare | Existing + RT Core Table 5-4 | ImperialHuman |
| Krak Missile | — | — | 3d10+10 X | 10 | Blast(1) | 1 | Scarce | RT Core Table 5-4 | ImperialHuman |
| Photon Flash | Thrown | SBx3 | — | 0 | Blind(4), Blast(10) | 0.5 | Rare | Existing + RT Core Table 5-4 | ImperialHuman |
| Plasma | Thrown | SBx3 | 1d10+6 | 6 | Blast(1) | 4 | Very Rare | RT Core Table 5-4 | ImperialHuman |
| Smoke Grenade | Thrown | SBx3 | — | 0 | Smoke(6) †† | 0.5 | Common | Existing + RT Core Table 5-4 | ImperialHuman |
| Stun | Thrown | SBx3 | — | 0 | Blast(3) | 0.2 | Scarce | RT Core Table 5-4 | ImperialHuman |
| Virus | Thrown | SBx3 | 3d10 I | 0 | Toxic ††† | 0.5 | Extremely Rare | RT Core Table 5-4 | ImperialHuman |
| Fire Bomb | Thrown | SBx3 | 1d10+3 E | 0 | Blast(3), Flame | — | — | Existing | Universal |

---

## Weapon Upgrades

> Source: RT Core Table 5-9 (all rows). Native lighter columns `Name | Weight | Availability`; Weight is the modifier the upgrade applies to the base weapon's weight (`x1/2` = halves it, `+N kg` = adds mass). No damage/pen/range profile applies — these are attachments to existing weapons.

| Name | Weight | Availability | Source | Region |
|---|---|---|---|---|
| Compact | x1/2 | Average | RT Core Table 5-9 | ImperialHuman |
| Fire Selector | +1 kg | Scarce | RT Core Table 5-9 | ImperialHuman |
| Forearm Weapon Mounting | +1 kg | Scarce | RT Core Table 5-9 | ImperialHuman |
| Melee Attachment | +2 kg | Plentiful | RT Core Table 5-9 | ImperialHuman |
| Mono | +0 kg | Scarce | RT Core Table 5-9 | ImperialHuman |
| Motion Predictor | +0.5 kg | Very Rare | RT Core Table 5-9 | ImperialHuman |
| Photo-scope | +0.5 kg | Very Rare | RT Core Table 5-9 | ImperialHuman |
| Preysense-scope | +0.5 kg | Very Rare | RT Core Table 5-9 | ImperialHuman |
| Omni-scope | +2 kg | Near Unique | RT Core Table 5-9 | ImperialHuman |
| Overcharge Pack | +0.5 kg | Common | RT Core Table 5-9 | ImperialHuman |
| Red-Dot Laser Sight | +0.5 kg | Scarce | RT Core Table 5-9 | ImperialHuman |
| Silencer | +0.5 kg | Plentiful | RT Core Table 5-9 | ImperialHuman |
| Suspensors | x1/2 | Extremely Rare | RT Core Table 5-9 | ImperialHuman |
| Telescopic Sight | +1 kg | Average | RT Core Table 5-9 | ImperialHuman |
| Vox Operated | +0.5 kg | Rare | RT Core Table 5-9 | ImperialHuman |

---

## Ammunition

> Source: RT Core Table 5-10 (all rows). Native lighter columns `Name | Availability`. Standard munitions and power/fuel cells sold per weapon class (pistol/basic/heavy). No stat profile of their own — they supply the loaded weapon's Clip.

| Name | Availability | Source | Region |
|---|---|---|---|
| Arrows/Quarrels | Common | RT Core Table 5-10 | Universal |
| Backpack Power Pack | Rare | RT Core Table 5-10 | ImperialHuman |
| Shot | Common | RT Core Table 5-10 | Universal |
| Bullets | Plentiful | RT Core Table 5-10 | Universal |
| Shells | Common | RT Core Table 5-10 | Universal |
| Charge Pack (pistol) | Common | RT Core Table 5-10 | ImperialHuman |
| Charge Pack (basic) | Common | RT Core Table 5-10 | ImperialHuman |
| Charge Pack (heavy) | Rare | RT Core Table 5-10 | ImperialHuman |
| Fuel (pistol) | Scarce | RT Core Table 5-10 | ImperialHuman |
| Fuel (basic) | Scarce | RT Core Table 5-10 | ImperialHuman |
| Fuel (heavy) | Scarce | RT Core Table 5-10 | ImperialHuman |
| Bolt Shells | Rare | RT Core Table 5-10 | ImperialHuman |
| Melta Canister (pistol) | Very Rare | RT Core Table 5-10 | ImperialHuman |
| Melta Canister (basic) | Very Rare | RT Core Table 5-10 | ImperialHuman |
| Melta Canister (heavy) | Very Rare | RT Core Table 5-10 | ImperialHuman |
| Plasma Flask (pistol) | Rare | RT Core Table 5-10 | ImperialHuman |
| Plasma Flask (basic) | Rare | RT Core Table 5-10 | ImperialHuman |
| Plasma Flask (heavy) | Very Rare | RT Core Table 5-10 | ImperialHuman |
| Exotic | Very Rare | RT Core Table 5-10 | Universal † |

> `†` Region default-assigned (faction undeterminable from source).

---

## Unusual Ammunition

> Source: RT Core Table 5-11 (all rows). Native lighter columns `Name | Availability`. Special-issue rounds that modify the loaded weapon's behaviour; each swaps in for standard ammunition of the matching type. Source B lists these by Availability only (no per-round stat line or footnote is given in the table).

| Name | Availability | Source | Region |
|---|---|---|---|
| Amputator Shells | Extremely Rare | RT Core Table 5-11 | ImperialHuman |
| Bleeder Rounds | Rare | RT Core Table 5-11 | ImperialHuman |
| Dumdum Bullets | Scarce | RT Core Table 5-11 | Universal |
| Expander Rounds | Scarce | RT Core Table 5-11 | ImperialHuman |
| Explosive Arrows/Quarrels | Scarce | RT Core Table 5-11 | Universal |
| Hot-Shot Charge Pack | Scarce | RT Core Table 5-11 | ImperialHuman |
| Inferno Shells | Rare | RT Core Table 5-11 | ImperialHuman |
| Man-stopper Bullets | Scarce | RT Core Table 5-11 | Universal |
| Tempest Bolt Shells | Near Unique | RT Core Table 5-11 | ImperialHuman |

---

## Melee Weapons — Primitive / Low-Tech

> Source: reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-8 (B) > Existing**. Source A §6 supplies a full Low-Tech Melee Weapon Profiles table (base Damage without SB; the curated `+SB` formula is retained as the in-play convention). Source A wins field-by-field: Knife → **Piercing** (1 kg); Sword/Staff → **Primitive(6/7)** magnitudes and Staff gains **Two-Handed**; Great Weapon → Pen **2**, **Proven(2), Two-Handed** (2d10 I or R); Axe → Pen **2**, **Felling(1)** (4 kg, Common); Spear → adds **Piercing, Reach, Two-Handed**; Warhammer → 1d10+2, Pen **2**, **Concussive(1), Primitive(8), Two-Handed, Unwieldy** (5 kg); Improvised → 1d10 (I or R), **Primitive(7)**. Shield has no Source A profile and stays Existing. Groxwhip, Kraken Tooth Dagger, Truncheon have no Source A equivalent and cite Table 5-8.
> `†††` Shield provides Armour 2 to the Body and the Arm wielding the Shield.

| Name | Range | Damage | Pen | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|---|
| Knife | 5m | 1d5+SB R | 0 | Piercing | 1 | Plentiful | Liber Imperium §6 (Qualities/Weight) + Existing + RT Core Table 5-8 | Universal |
| Sword | — | 1d10+SB R | 0 | Balanced, Primitive(7) | 3 | Common | Liber Imperium §6 (Qualities) + Existing + RT Core Table 5-8 | Universal |
| Great Weapon | +1m | 2d10+SB I -or- R | 2 | Proven(2), Unbalanced, Two-Handed | 7 | Average | Liber Imperium §6 (Damage/Pen/Qualities/Availability) + Existing + RT Core Table 5-8 | Universal |
| Axe | — | 1d10+SB R | 2 | Felling(1), Unbalanced | 4 | Common | Liber Imperium §6 (Damage/Pen/Qualities/Weight/Availability) + Existing | Universal |
| Shield | — | 1d5+SB I | 0 | Defensive, Primitive(7) ††† | 3 | Common | Existing + RT Core Table 5-8 | Universal |
| Staff | +1m | 1d10+SB I | 0 | Balanced, Primitive(6), Two-Handed | 3 | Plentiful | Liber Imperium §6 (Qualities) + Existing + RT Core Table 5-8 | Universal |
| Spear | +2m | 1d10+SB R | 0 | Piercing, Primitive(8), Reach, Two-Handed | 3 | Common | Liber Imperium §6 (Qualities) + Existing + RT Core Table 5-8 | Universal |
| Warhammer | +1m | 1d10+2+SB I | 2 | Concussive(1), Primitive(8), Two-Handed, Unwieldy | 5 | Average | Liber Imperium §6 (Damage/Pen/Qualities/Weight/Availability) + Existing + RT Core Table 5-8 | Universal |
| Groxwhip | 3m | 1d10+3 R | 0 | Flexible, Tearing, Primitive | 4 | Scarce | RT Core Table 5-8 | Universal |
| Improvised | — | 1d10 I -or- R | 0 | Primitive(7), Unbalanced | — | Plentiful | Liber Imperium §6 (Damage/Qualities) + RT Core Table 5-8 | Universal |
| Kraken Tooth Dagger | 5m | 1d5+1 R | 1 | Primitive | 0.4 | Extremely Rare | RT Core Table 5-8 | Universal |
| Truncheon | — | 1d10 I | 0 | Primitive | 2 | Plentiful | RT Core Table 5-8 | Universal |

---

## Melee Weapons — Shock

> Source: reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-7 (B) > Existing**. Source A §6 supplies a Shock Weapon Profiles table; the Existing "Shock Glove" reconciles to Source A's **Shock Gauntlets** (1 kg, Scarce). Officer's Cutlass and Shock-Staff have no Source A equivalent and cite Table 5-7.

| Name | Damage | Pen | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|
| Officer's Cutlass | 1d10 R | 0 | Shocking | 3 | Scarce | RT Core Table 5-7 | ImperialHuman |
| Shock Glove | 1d10 I | 0 | Shocking | 1 | Scarce | Liber Imperium §6 (Weight/Availability, as "Shock Gauntlets") + RT Core Table 5-7 | ImperialHuman |
| Shock-Staff | 1d5+3 I | 0 | Shocking | 2 | Rare | RT Core Table 5-7 | ImperialHuman |

---

## Melee Weapons — Chain

> Source: reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-5 (B) > Existing**. Source A §6 supplies a full Chain Weapon Profiles table (base Damage without SB; the curated `+SB` formula is retained as the in-play convention). Source A wins field-by-field: Chain Axe gains **Felling(1), Two-Handed** and Availability **Scarce**; Eviscerator resolves to Source A `2d10(+SB) R, Pen 8, 15 kg, Very Rare` and gains **Vengeful(9)** (Existing 1d10+9 / Pen 5 discarded). Chainsword agrees across sources.

| Name | Damage | Pen | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|
| Chainsword | 1d10+2+SB R | 2 | Balanced, Tearing | 6 | Average | Liber Imperium §6 + Existing + RT Core Table 5-5 | ImperialHuman |
| Chain Axe | 1d10+4+SB R | 2 | Felling(1), Tearing, Two-Handed, Unbalanced | 13 | Scarce | Liber Imperium §6 (Qualities/Availability) + Existing + RT Core Table 5-5 | ImperialHuman |
| Eviscerator | 2d10+2+SB R | 8 | Tearing, Razor Sharp, Two-Handed, Unwieldy, Vengeful(9) | 15 | Very Rare | Liber Imperium §6 (Damage/Pen/Qualities/Weight/Availability) + Existing | ImperialHuman |

---

## Melee Weapons — Power

> Source: reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-6 (B) > Existing**. Source A §6 supplies a full Power Weapon Profiles table (base Damage without SB; the curated `+SB` formula is retained as the in-play convention). Source A wins field-by-field: the Existing "Power Maul" reconciles to Source A's **Power Maul (High)** profile (Concussive(0), Power Field, Unbalanced — Existing Concussive(1)/Shocking superseded); Power Axe Pen becomes **6** and gains **Felling(1), Two-Handed**; Power Fist Pen becomes **8**; Thunder Hammer becomes `2d10+SBx2+4 E, Pen 10, Concussive(2), Two-Handed, 16 kg, Near Unique`; Lightning Claw gains **Proven(4)** (Razor Sharp superseded), 30 kg, Extremely Rare; Power Maul (Low) gains **Unbalanced**, 4 kg. Omnissian Axe (Sollex) has no Source A equivalent and cites Table 5-6.
> `†` Power Fists (and the Thunder Hammer) add the user's SB×2 to the Damage.

| Name | Damage | Pen | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|
| Power Sword | 1d10+5+SB E | 5 | Balanced, Power Field | 3 | Very Rare | Liber Imperium §6 + Existing + RT Core Table 5-6 | ImperialHuman |
| Power Axe | 1d10+7+SB E | 6 | Felling(1), Power Field, Unbalanced, Two-Handed | 6 | Very Rare | Liber Imperium §6 (Pen/Qualities) + Existing + RT Core Table 5-6 | ImperialHuman |
| Power Maul | 1d10+5+SB E | 4 | Concussive(0), Power Field, Unbalanced | 4 | Very Rare | Liber Imperium §6 (Qualities/Weight, as "Power Maul (High)") + Existing + RT Core Table 5-6 | ImperialHuman |
| Power Fist | 2d10 † +SB E | 8 | Power Field, Unwieldy | 13 | Extremely Rare | Liber Imperium §6 (Pen/Weight/Availability) + Existing + RT Core Table 5-6 | ImperialHuman |
| Thunder Hammer | 2d10+SBx2+4 E | 10 | Concussive(2), Power Field, Two-Handed, Unwieldy | 16 | Near Unique | Liber Imperium §6 (Damage/Qualities/Weight/Availability) + Existing | ImperialHuman |
| Lightning Claw | 1d10+6+SB E | 8 | Power Field, Proven(4) | 30 | Extremely Rare | Liber Imperium §6 (Qualities/Weight/Availability) + Existing | ImperialHuman |
| Omnissian Axe (Sollex) | 2d10+4 E | 6 | Power Field, Unbalanced | 8 | Extremely Rare | RT Core Table 5-6 | ImperialHuman |
| Power Maul (Low) | 1d10+1 E | 2 | Shocking, Unbalanced | 4 | Very Rare | Liber Imperium §6 (Qualities/Weight, as "Power Maul (Low)") + RT Core Table 5-6 | ImperialHuman |

---

## Melee Weapons — Exotic/Xenos

> Source: reconciled applying Precedence Order **Liber Imperium §6 (A) > RT Core Table 5-7 (B) > Existing**. Source A §6's Exotic Melee Weapon Profiles cover a distinct set (Graviton Hammer, Axon Razor, Bone Maul, etc.) with **no overlap** with the xenos entries here, so no Source A override applies to this family. Choppa (Ork) ↔ Source B "Ork Choppa": Existing curated `+SB` Damage retained; differing Pen resolved to Source B (Pen 2) per Precedence (B > Existing); weight/availability added. Power Klaw, Big Choppa, 'Uge Choppa are Existing-only. Fractal Blade, Ghost Sword, Harlequin's Kiss cite RT Core Table 5-7.
> `††` Unlike other melee weapons, do not add the wielder's Strength bonus to the damage inflicted by a Harlequin's Kiss.

| Name | Damage | Pen | Qualities | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|---|---|---|
| Power Klaw (Ork) | 2d10+SB I | 7 | Power Field, Unwieldy | — | — | Existing | Ork |
| Choppa (Ork) | 1d10+2+SB R | 2 | Unbalanced | 8 | Scarce | Existing + RT Core Table 5-7 | Ork |
| Big Choppa (Ork) | 2d5+4+SB R | 2 | Unbalanced | — | — | Existing | Ork |
| 'Uge Choppa (Ork) | 2d10+SB R | 0 | Unbalanced, Unwieldy | — | — | Existing | Ork |
| Fractal Blade | 1d10+1 R | 7 | Power Field, Balanced | 1 | Extremely Rare | RT Core Table 5-7 | Eldar |
| Ghost Sword | 1d10+3 E | 6 | Power Field, Balanced | 1 | Extremely Rare | RT Core Table 5-7 | Eldar |
| Harlequin's Kiss | 1d10+8 R †† | 10 | Tearing | 1 | Extremely Rare | RT Core Table 5-7 | Eldar |

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
