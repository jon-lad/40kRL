# Working Canonical Inventory (Task 1.1)

> Intermediate curation artifact for the `equipment-reference-extraction` spec.
> This is process scaffolding used by downstream tasks (2.x weapons, 4.x equipment).
> It is NOT a deliverable reference file. The deliverables are `Reference/RT-Weapons.md`
> (reconciled) and `Reference/RT-Equipment.md` (new).
>
> Precedence Order (Req 3): **Source A (Liber Imperium §6) > Source B (RT Core Table 5-N) > Existing Target (RT-Weapons.md)**.
> Item Identity (design): trim/collapse spacing, case-insensitive; strip Source B pattern
> qualifiers `(Locke)`/`(Solar)`/`(Mars)`/`(Ceres)`/`(Mezoa)`/`(Ryza)`/`(High)`/`(Low)`/`(Hecate)`/`(Mordian)`/`(Orthlack)`/`(Ursid)`/`(Voss)`/`(Retobi)`/`(Sollex)`
> when reconciling to the shorter canonical names already used in the Existing Target;
> keep genuinely distinct patterns (differing Clip/Reload/Availability) as separate items.

## Legend

- **Cat**: Weapon (ranged/melee), Upgrade, Ammo → `RT-Weapons.md`; Armour, ForceField, Gear, Tool, Drug, Cybernetic → `RT-Equipment.md`.
- **Cite**: `LI §6` = Liber Imperium §6 (Source A); `5-N` = RT Core Table 5-N (Source B); `existing` = current RT-Weapons.md row.
- **Winner**: source recorded after precedence merge (only shown where multiple sources present).

---

## 1. Named Conflicts (Req 3.4) — Source A wins all three

Confirmed Source A stat lines (OCR):
- Inferno Pistol — LI §6 line 2249: `Pistol 10m S/-/- 2d10+10E Pen 12 Clip 3 Full  Melta  3kg` — N.Unique/Ŧ7,500
- Multi-Melta   — LI §6 line 2261: `Heavy 60m S/-/- 2d10+16E Pen 12 Clip 12 Full  Blast(1), Melta  40kg` — Ex.Rare/Ŧ8,500
- Heavy Bolter  — LI §6 line 1900: `Heavy 150m -/-/6 1d10+8X Pen 5 Clip 60 Full  Tearing  40kg` — Very Rare/Ŧ4,000

| Item | Source A (§6) | Source B (5-4) | Existing | Recorded (winner) | Cite |
|---|---|---|---|---|---|
| Inferno Pistol — Pen | Pen 12 | Pen 13 (Mars) | Pen 12 | **Pen 12** | LI §6 |
| Inferno Pistol — Damage | 2d10+10 E | 2d10+8 E | 2d10+4 E | **2d10+10 E** | LI §6 |
| Multi-Melta — Damage | 2d10+16 E | 4d10+5 E | 2d10+8 E | **2d10+16 E** | LI §6 |
| Multi-Melta — Pen | Pen 12 | Pen 13 | Pen 12 | **Pen 12** | LI §6 |
| Heavy Bolter — Damage | 1d10+8 X | 2d10+2 X | 1d10+8 X | **1d10+8 X** | LI §6 |

> NOTE for downstream: The design's Known-Conflicts table records Inferno Pistol as "Pen 12" (the
> single named field in Req 3.4). Source A's damage line is `2d10+10 E`, which also differs from the
> Existing `2d10+4 E`; by precedence Source A wins that field too. Downstream task 2.2 should set at
> minimum the three named fields; the Inferno Pistol damage discrepancy is flagged here for the
> checkpoint reviewer (task 3 / 5.1) since it is a precedence consequence, not a named conflict.

---

## 2. Ranged Weapons → RT-Weapons.md (Cat: Weapon)

Reconciliation of Existing Target rows against Source B patterns (Item Identity applied). Where an
item exists in both, precedence B > Existing for differing fields (no Source A ranged stat lines
except the three named conflicts above, which override).

### Las
| Canonical | Existing | Source B (pattern) | Winner/Notes | Cite |
|---|---|---|---|---|
| Laspistol | 30m S/2/- 1d10+2E Pen0 Clip30 Half Reliable | Laspistol 30m S/-/- 1d10+2E Pen0 Clip30 Full Reliable, 1.5kg, Common | keep Existing RoF/Reload (curated); add wt 1.5 / Avail Common | existing + 5-4 |
| Lasgun | 100m S/3/- 1d10+3E Pen0 Clip60 Half Reliable | Lasgun 100m S/3/- 1d10+3E Pen0 Clip60 Full 4kg Common | add wt 4 / Avail Common | existing + 5-4 |
| Long-Las | 150m S/-/- 1d10+3E Pen1 Clip40 Full Accurate,Reliable,Felling(4) | Long-las 150m S/-/- 1d10+3E Pen1 Clip40 Full Accurate,Reliable 4.5kg Scarce | keep Existing Felling(4); add wt 4.5 / Scarce | existing + 5-4 |
| Hot-Shot Laspistol | 20m S/2/- 1d10+4E Pen7 Clip40 2Full — | (Hellpistol Lucius ≈ variant, kept distinct) | keep Existing | existing |
| Hot-Shot Lasgun | 60m S/3/- 1d10+4E Pen7 Clip30 2Full — | (Hellgun Lucius ≈ variant, kept distinct) | keep Existing | existing |
| Lascannon | 300m S/-/- 5d10+10E Pen10 Clip5 2Full Proven(3),Reliable | Man Portable Lascannon 300m 5d10+10E Pen10 Clip5 2Full 55kg VeryRare | add wt 55 / VeryRare | existing + 5-4 |
| Multi-laser | 150m -/-/5 2d10+10E Pen2 Clip100 2Full Reliable | (not in B) | keep Existing | existing |
| Hellpistol (Lucius) | — | 35m S/2/- 1d10+4E Pen7 Clip40 2Full 4kg Rare | NEW (distinct pattern; ≈ Hot-Shot family) | 5-4 |
| Hellgun (Lucius) | — | 110m S/3/- 1d10+4E Pen7 Clip30 2Full 6kg Rare | NEW | 5-4 |
| Archeotech Laspistol | — | 90m S/3/- 1d10+3E Pen2 Clip70 Full Accurate,Reliable 4kg NearUnique | NEW | 5-4 |
| Belasco Dueling Pistol | — | 45m S/-/- 1d10+5E Pen4 Clip1 Full Accurate 1.5kg VeryRare | NEW | 5-4 |
| Las Gauntlets | — | 50m S/4/- 1d10+4E Pen1 Clip20 Full Reliable 3kg VeryRare | NEW | 5-4 |
| Lascarbine (Locke) | — | 60m S/2/- 1d10+3E Pen0 Clip40 2Full Reliable 2.5kg Scarce | NEW (distinct pattern) | 5-4 |

### Solid Projectile (SP)
| Canonical | Existing | Source B | Winner/Notes | Cite |
|---|---|---|---|---|
| Stub Automatic | 30m S/3/- 1d10+3I Pen0 Clip9 Half — | 30m S/3/- 1d10+3I Pen0 Clip9 Full 1.5kg Plentiful | add wt/avail | existing + 5-4 |
| Stub Revolver | 30m S/-/- 1d10+3I Pen0 Clip6 2Half Reliable | 30m S/-/- ... Clip6 2Full 1kg Plentiful | add wt/avail | existing + 5-4 |
| Autopistol | 30m S/-/6 1d10+2I Pen0 Clip18 Half — | 30m S/-/6 ... 2.5kg Common | add wt/avail | existing + 5-4 |
| Autogun | 100m S/3/- 1d10+3I Pen0 Clip30 Half — | 90m S/3/10 ... 3.5kg Average | keep Existing 100m/S/3/- (curated); add wt/avail | existing + 5-4 |
| Combat Shotgun | 30m S/3/- 1d10+4I Pen0 Clip18 Half Scatter | (Shotgun/Pump variants in B) | keep Existing | existing |
| Pump Shotgun | 30m S/-/- 1d10+4I Pen0 Clip8 2Half Scatter,Reliable | Pump-Action Shotgun 30m ... Clip8 2Full Scatter 5kg Average | add wt/avail | existing + 5-4 |
| Heavy Stubber | 100m -/-/8 1d10+4I Pen3 Clip200 2Full — | Heavy Stubber (Orthlack) 120m -/-/10 1d10+5I Pen3 Clip200 2Full 35kg Average | keep Existing base; NEW patterns Orthlack/Ursid distinct | existing + 5-4 |
| Autocannon | 300m S/3/- 3d10+8I Pen6 Clip20 2Full Reliable | (not in B) | keep Existing | existing |
| Hand Cannon | — | 35m S/-/- 1d10+4I Pen2 Clip5 2Full 3kg Average | NEW | 5-4 |
| Heavy Stubber (Orthlack) | — | 120m -/-/10 1d10+5I Pen3 Clip200 2Full 35kg Average | NEW (distinct: Clip200/2Full) | 5-4 |
| Heavy Stubber (Ursid) | — | 120m -/-/10 1d10+5I Pen3 Clip40 Full 35kg Scarce | NEW (distinct: Clip40/Full) | 5-4 |
| Naval Pistol (Mars) | — | 20m S/3/- 1d10+4I Pen0 Clip6 Full Tearing 3kg Scarce | NEW | 5-4 |
| Naval Shotcannon | — | 40m S/3/- 2d10+4I Pen0 Clip24 2Full Scatter,Unreliable 7kg Scarce | NEW | 5-4 |
| Shotgun | — | 30m S/-/- 1d10+4I Pen0 Clip2 2Full Scatter 5kg Common | NEW | 5-4 |
| Shotgun Pistol | — | 10m S/-/- 1d10+4I Pen0 Clip1 Full Reliable,Scatter 1kg Average | NEW | 5-4 |

### Bolt
| Canonical | Existing | Source B | Winner/Notes | Cite |
|---|---|---|---|---|
| Bolt Pistol | 30m S/2/- 1d10+5X Pen4 Clip8 Full Tearing | Bolt Pistol (Ceres) 30m S/2/- ... 3.5kg Rare | add wt 3.5/Rare | existing + 5-4 |
| Boltgun | 100m S/2/- 1d10+5X Pen4 Clip24 Full Tearing | Boltgun (Locke) 90m S/2/4 ... 7kg VeryRare | keep Existing 100m/S/2/-; add wt 7 | existing + 5-4 |
| Heavy Bolter | 150m -/-/6 1d10+8X Pen5 Clip60 2Full Tearing | Heavy Bolter (Solar) 120m -/-/10 2d10+2X Pen5 Clip60 Full 40kg VeryRare | **Damage 1d10+8X wins (LI §6 named conflict)**; add wt 40 | LI §6 (named) |
| Storm Bolter | 90m S/2/4 1d10+5X Pen4 Clip60 2Full Tearing,Storm | Storm Bolter (Mars) 90m S/2/4 ... 9kg Ex.Rare | add wt 9 | existing + 5-4 |

### Plasma
| Canonical | Existing | Source B | Winner/Notes | Cite |
|---|---|---|---|---|
| Plasma Pistol | 30m S/2/- 1d10+6E Pen6 Clip10 3Full Overheats | Plasma Pistol (Ryza) ... 4kg VeryRare | add wt 4/VeryRare | existing + 5-4 |
| Plasma Gun | 90m S/2/- 1d10+7E Pen6 Clip20 4Full Overheats | Plasma Gun (Mezoa) Clip40 5Full 18kg VeryRare | keep Existing Clip20/4Full (curated); add wt 18 | existing + 5-4 |
| Plasma Cannon | 120m S/-/- 2d10+10E Pen8 Clip16 5Full Blast(1),Overheats | Plasma Cannon (Ryza) ... 40kg VeryRare (adds Unreliable) | keep Existing quals; add wt 40 | existing + 5-4 |

### Melta
| Canonical | Existing | Source B | Winner/Notes | Cite |
|---|---|---|---|---|
| Inferno Pistol | 10m S/-/- 2d10+4E Pen12 Clip3 Full Melta | Inferno Pistol (Mars) 2d10+8E Pen13 Clip3 Full 2.5kg VeryRare | **Damage 2d10+10E, Pen 12 win (LI §6)**; add wt 2.5/VeryRare | LI §6 (named) |
| Meltagun | 20m S/-/- 2d10+4E Pen12 Clip5 2Full Melta | Meltagun (Mars) 2d10+8E Pen13 Clip5 2Full 40kg Rare | Source B > Existing: Damage 2d10+8E Pen13 win; add wt 40/Rare | 5-4 |
| Multi-Melta | 60m S/-/- 2d10+8E Pen12 Clip12 2Full Blast(1),Melta | Multi-Melta (Mars) 60m S/3/- 4d10+5E Pen13 Clip10 2Full Blast(1) 40kg VeryRare | **Damage 2d10+16E, Pen 12 win (LI §6 named)**; add wt 40/VeryRare | LI §6 (named) |
| Meltagun (Mezoa) | — | 20m S/-/- 2d10+8E Pen13 Clip10 3Full 46kg Rare | NEW (distinct: Clip10/3Full) | 5-4 |
| Thermal Lance (Mars) | — | Heavy 10m S/-/- 2d10+10E Pen12 Clip2 2Full Accurate 40kg Rare | NEW | 5-4 |

### Flame
| Canonical | Existing | Source B | Winner/Notes | Cite |
|---|---|---|---|---|
| Hand Flamer | 10m S/-/- 1d10+4E Pen2 Clip2 2Full Flame,Spray | Hand Flamer (Mezoa) ... 3.5kg Rare | keep Existing Spray; add wt 3.5/Rare | existing + 5-4 |
| Flamer | 20m S/-/- 1d10+4E Pen2 Clip6 2Full Flame,Spray | Flamer (Mezoa) ... 6kg Scarce | add wt 6/Scarce | existing + 5-4 |
| Heavy Flamer | 30m S/-/- 1d10+5E Pen4 Clip10 2Full Flame,Spray | Heavy Flamer (Locke) 2d10+4E Pen4 Clip10 2Full 20kg Rare | Source B Damage 2d10+4E > Existing 1d10+5E; keep Spray; add wt 20 | 5-4 |

### Primitive Ranged (Source B only) — NEW
Bolas, Bow, Crossbow, Hand Bow, Flintlock Pistol, Musket, Sling — all from 5-4, ported verbatim.

### Launchers (Source B only) — NEW
Grenade Launcher (Mezoa), Grenade Launcher (Voss), Missile Launcher (Locke), Missile Launcher (Retobi).
Footnote `†` Damage/Pen/Special vary with ammunition loaded.

### Exotic Ranged (Source B only) — NEW
Crux Beam Gun, Dartcaster, Digi-laser, Digi-melta, Digi-needler, Digi-flamer, Graviton Gun,
Kroot Rifle (+ melee profile), Needle Pistol, Needle Rifle, Ork Shoota, Ork Slugga,
Shuriken Catapult, Shuriken Pistol. Footnote `†` Ork weapons: Inaccurate, Unreliable.

### Grenades & Missiles
Existing has: Frag, Krak, Photon Flash, Smoke, Fire Bomb, Hallucinogen.
Source B (5-4) adds/reconciles: Anti-Plant, Blind, Filament, Frag, Frag Missile, Geode,
Hallucinogen, Krak, Krak Missile, Photon Flash, Plasma, Smoke, Stun, Virus.
Footnotes preserved verbatim (Req 7.7):
- `†` Anti-Plant rounds only affect flora and have no other effect.
- `††` Blind grenades: unlike smoke, this only interferes with visual sight.
- `†††` Virus: if the grenade causes any Damage, then each round after the first, check for another
  target at random (friend or foe) within d5 metres to see if any Damage is caused (roll new Damage
  for the mutated virus). Continue until Damage is not taken or after d10 rounds have passed.

---

## 3. Melee Weapons → RT-Weapons.md (Cat: Weapon)

Existing melee families: Primitive/Low-Tech, Chain, Power, Exotic/Xenos.
Note: Existing uses `+SB` inline in Damage; Source B lists base Damage without SB (SB added in play).
Merge keeps Existing curated `+SB` formulas; adds wt/avail from Source B where matched.

### Chain
| Canonical | Existing | Source B | Notes | Cite |
|---|---|---|---|---|
| Chainsword | 1d10+2+SB R Pen2 Balanced,Tearing | Chainsword (Hecate) 1d10+2R Pen2 Tearing,Balanced 6kg Average | add wt 6/Average | existing + 5-5 |
| Chain Axe | 1d10+4+SB R Pen2 Tearing,Unbalanced | Chain Axe 1d10+4R Pen2 Tearing 13kg Average | keep Existing Unbalanced; add wt 13/Average | existing + 5-5 |
| Eviscerator | 1d10+9+SB R Pen5 Tearing,Razor Sharp,Unwieldy | (not in B) | keep Existing | existing |

### Power (Power Fist footnote Req 7.7 preserved)
| Canonical | Existing | Source B | Notes | Cite |
|---|---|---|---|---|
| Power Sword | 1d10+5+SB E Pen5 Balanced,Power Field | Power Sword (Mordian) 1d10+5E Pen5 Power Field,Balanced 3kg VeryRare | add wt 3/VeryRare | existing + 5-6 |
| Power Axe | 1d10+7+SB E Pen7 Power Field,Unbalanced | Power Axe (Mezoa) 1d10+7E Pen7 ... 6kg VeryRare | add wt 6/VeryRare | existing + 5-6 |
| Power Maul | 1d10+5+SB E Pen4 Power Field,Concussive(1),Shocking | Power Maul (High) 1d10+5E Pen4 Power Field,Shocking 3.5kg VeryRare | keep Existing Concussive(1); add wt 3.5 | existing + 5-6 |
| Power Fist | 2d10+SB E Pen9 Power Field,Unwieldy | Power Fist (Mezoa) 2d10† E Pen9 ... 13kg VeryRare | footnote `†` Power Fists add user's SB×2 to Damage; add wt 13 | existing + 5-6 |
| Thunder Hammer | 2d10+5+SB E Pen10 Power Field,Concussive(3),Unwieldy | (not in B) | keep Existing | existing |
| Lightning Claw | 1d10+6+SB E Pen8 Power Field,Razor Sharp | (not in B) | keep Existing | existing |
| Omnissian Axe (Sollex) | — | 2d10+4E Pen6 Power Field,Unbalanced 8kg Ex.Rare | NEW | 5-6 |
| Power Maul (Low) | — | 1d10+1E Pen2 Shocking 3.5kg VeryRare | NEW (distinct, no Power Field) | 5-6 |

### Exotic / Xenos
| Canonical | Existing | Source B | Notes | Cite |
|---|---|---|---|---|
| Power Klaw (Ork) | 2d10+SB I Pen7 Power Field,Unwieldy | (not in B) | keep Existing | existing |
| Choppa (Ork) | 1d10+2+SB R Pen0 Unbalanced | Ork Choppa 1d10+1R Pen2 Unbalanced 8kg Scarce | keep both? Item Identity: same item → Existing curated wins name/formula; add wt 8/Scarce | existing + 5-7 |
| Big Choppa (Ork) | 2d5+4+SB R Pen2 Unbalanced | (not in B) | keep Existing | existing |
| 'Uge Choppa (Ork) | 2d10+SB R Pen0 Unbalanced,Unwieldy | (not in B) | keep Existing | existing |
| Fractal Blade | — | 1d10+1R Pen7 Power Field,Balanced 1kg Ex.Rare | NEW | 5-7 |
| Ghost Sword | — | 1d10+3E Pen6 Power Field,Balanced 1kg Ex.Rare | NEW | 5-7 |
| Harlequin's Kiss | — | 1d10+8R†† Pen10 Tearing 1kg Ex.Rare | NEW; footnote `††` do not add wielder's SB to damage | 5-7 |

### Shock (Source B only) — NEW
Officer's Cutlass, Shock Glove, Shock-Staff (Table 5-7/5-8).

### Primitive Melee
Existing: Knife, Sword, Great Weapon, Axe, Shield, Staff, Spear, Warhammer (with +SB formulas).
Source B (5-8) reconciles wt/avail and adds: Groxwhip, Improvised, Kraken Tooth Dagger, Truncheon.
Footnote `†††` Shield provides Armour 2 to the Body and the Arm wielding the Shield.

---

## 4. Weapon Upgrades → RT-Weapons.md (Cat: Upgrade, Table 5-9) — NEW
Columns `Name | Weight | Availability`. 15 rows: Compact, Fire Selector, Forearm Weapon Mounting,
Melee Attachment, Mono, Motion Predictor, Photo-scope, Preysense-scope, Omni-scope, Overcharge Pack,
Red-Dot Laser Sight, Silencer, Suspensors, Telescopic Sight, Vox Operated. Cite 5-9.

## 5. Ammunition → RT-Weapons.md (Cat: Ammo, Tables 5-10, 5-11) — NEW
- 5-10 Ammo (19 rows): Arrows/Quarrels, Backpack Power Pack, Shot, Bullets, Shells, Charge Pack
  (pistol/basic/heavy), Fuel (pistol/basic/heavy), Bolt Shells, Melta Canister (pistol/basic/heavy),
  Plasma Flask (pistol/basic/heavy), Exotic. Columns `Name | Availability`. Cite 5-10.
- 5-11 Unusual Ammunition (9 rows): Amputator Shells, Bleeder Rounds, Dumdum Bullets, Expander Rounds,
  Explosive Arrows/Quarrels, Hot-Shot Charge Pack, Inferno Shells, Man-stopper Bullets,
  Tempest Bolt Shells. Columns `Name | Availability`. Cite 5-11.

---

## 6. Armour → RT-Equipment.md (Cat: Armour, Table 5-12) — NEW file
Columns `Name | Covered (Locations) | AP | Weight (kg) | Availability`. Grouped: Primitive, Flak,
Mesh, Carapace, Other, Power. 20 rows from 5-12 (verbatim AP-by-location where "Covered" lists
specific locations). Source A §6 adds power/carapace prose but Table 5-12 supplies the numeric
profiles. Cite 5-12 (+ LI §6 where §6 adds an entry not in B). AP-by-location preserved per Req 7.4.

## 7. Force Fields → RT-Equipment.md (Cat: ForceField, Source A §6 only) — NEW
Columns `Name | Protection Rating | Overload | Gear Slot | Weight/Size | Availability`. Source A
"Force Field Profiles" table (LI §6, lines ~6842-6890), 12 rows:
| Name | Prot. Rating | Overload | Gear Slot | Weight/Size | Availability |
|---|---|---|---|---|---|
| Refractor Field | 30 | 10 | Neck or Waist | 2 kg/Size(1) | Very Rare/Ŧ15,000 |
| Conversion Field | 50 | 10 | Neck or Waist | 1 kg/Size(1) | Ex. Rare/Ŧ20,000 |
| Displacer Field | 55 | 10 | Waist | 2 kg/Size(1) | N. Unique/Ŧ50,000 |
| Power Field (Personal) | 75 | 10 | Back | 50 kg/Size(3) | N. Unique/Ŧ12,000,000 |
| Power Field (Emplacement) | 80 | 10 | - | 500 kg/Size(6) | N. Unique/Ŧ10,000,000 |
| Field Wall Generator | 65 | 10 | - | 18 kg/Size(3) | Very Rare/Ŧ12,000 |
| Rosarius | 50 | 10 | Neck or Waist | 0.5 kg/Size(1) | Ex. Rare/Ŧ25,000 |
| Icon of the Just | 55 | 10 | Neck or Waist | 0.5 kg/Size(1) | N. Unique/Ŧ45,000 |
| Null Blocker | 60 | 10 | Neck or Waist | 0.5 kg/Size(1) | Very Rare/Ŧ20,000 |
| Jokaerian Field | 70 | 10 | Neck or Waist | 0.5 kg/Size(1) | N. Unique/Ŧ50,000 |
| Personal Flare Shield | 25/50 | 10 | Back | 3 kg/Size(2) | Ex. Rare/Ŧ100,000 |
| Refraction Bracer | 30 | 10 | Wrist | 0.5 kg/Size(2) | Rare/Ŧ5,000 |

Footnotes/special rules to preserve verbatim (Req 7.7): Personal Flare Shield 25/50 (doubles to 50
vs Blast/Scatter/Spray/Full-Auto); Refraction Bracer protects Body/Arms only, no Blast, half vs
Spray, +10 Parry; Rosarius Corruption 20+ cannot use; Displacer random-teleport rule.

## 8. Gear → RT-Equipment.md (Cat: Gear, Table 5-13 + LI §6 §6.5) — NEW
Columns `Name | Weight (kg) | Availability`. 5-13 has 20 rows. Footnote `†` Clothing & Adornment
may have any appropriate Availability. LI §6 §6.5 adds prose gear (Arms Coffer, Auto Sense Goggles,
Blast Goggles, Chrono, Cameleoline Cloak, etc.) with per-item Availability/Burden/Slot — reconcile
by Item Identity (e.g., Cameleoline Cloak, Chrono, Arms Coffer appear in both).

## 9. Tools → RT-Equipment.md (Cat: Tool, Table 5-15) — NEW
Columns `Name | Weight (kg) | Availability`. 38 rows from 5-15.

## 10. Drugs & Consumables → RT-Equipment.md (Cat: Drug, Table 5-14) — NEW
Columns `Name | Weight | Availability`. 16 rows from 5-14.

## 11. Cybernetics → RT-Equipment.md (Cat: Cybernetic, Table 5-16) — NEW
Columns `Name | Availability`. 27 rows from 5-16. Footnote `†` Mechanicus-restricted (Ballistic/
Manipulator/Medicae/Optical/Utility Mechadendrite) preserved verbatim (Req 7.7).

---

## Item Identity decisions log (Req 4 / design)

**Stripped pattern qualifiers (reconciled to Existing canonical short names):**
- Bolt Pistol (Ceres) → Bolt Pistol; Storm Bolter (Mars) → Storm Bolter; Heavy Bolter (Solar) → Heavy Bolter
- Inferno Pistol (Mars) → Inferno Pistol; Multi-Melta (Mars) → Multi-Melta; Meltagun (Mars) → Meltagun
- Plasma Pistol (Ryza) → Plasma Pistol; Plasma Gun (Mezoa) → Plasma Gun; Plasma Cannon (Ryza) → Plasma Cannon
- Hand Flamer (Mezoa) → Hand Flamer; Flamer (Mezoa) → Flamer; Heavy Flamer (Locke) → Heavy Flamer
- Boltgun (Locke) → Boltgun; Chainsword (Hecate) → Chainsword; Power Sword (Mordian) → Power Sword
- Power Axe (Mezoa) → Power Axe; Ork Choppa → Choppa (Ork)

**Kept DISTINCT (genuinely different patterns — differing Clip/Reload/Availability):**
- Heavy Stubber (Orthlack) [Clip 200 / 2 Full] vs Heavy Stubber (Ursid) [Clip 40 / Full] — both distinct from Existing base Heavy Stubber
- Meltagun (Mezoa) [Clip 10 / 3 Full] distinct from Meltagun (base/Mars) [Clip 5 / 2 Full]
- Power Maul (High) [Power Field] vs Power Maul (Low) [no Power Field] — Existing Power Maul reconciles to High
- Grenade Launcher (Mezoa) vs (Voss); Missile Launcher (Locke) vs (Retobi)
- Lascarbine (Locke), Hellpistol/Hellgun (Lucius), Naval Pistol (Mars) — no short canonical equivalent, retained

**Single-source values recorded as-is (absence ≠ conflict, Req 3.5):** all weight/availability added
from Source B where Existing lacked them; all Existing curated Qualities (Felling(4), Concussive,
Storm, Reliable, etc.) retained where Source B omitted them.

---

## CORRECTION LOG — Task 2.1 re-run (defect fix from review 2.4)

**Defect:** The original Section 2 / 3 above assumed Source A §6 contained *no* ranged/melee
weapon stat lines except the three named conflicts (Inferno Pistol, Multi-Melta, Heavy Bolter).
This was WRONG. Source A §6 contains **full Weapon Profile tables for every family**:

- Las Weapon Profiles (§6.2.1, LI lines ~1261–1336 and ~1337–1389)
- Solid Projectile Weapon Profiles (§6.2.2, LI lines ~1454–1690)
- Bolt Weapon Profiles (§6.2.3, LI lines ~1874–1912)
- Flame Weapon Profiles (§6.2.4, LI lines ~2116–2175)
- Melta Weapon Profiles (§6.2.5, LI lines ~2244–2280)
- Plasma Weapon Profiles (§6.2.6, LI lines ~2387–2455)
- Launcher Weapon Profiles (§6.2.7, LI lines ~2509–2600; Damage varies by ammo → no numeric override)
- Low-Tech Ranged Weapon Profiles (§6.2.9, LI lines ~3050–3110; Bow/Crossbow override)
- Exotic Ranged Weapon Profiles (§6.2.10, LI lines ~3222–3480; distinct Mechanicus set, no overlap)
- Chain Weapon Profiles (§6.2.11, LI lines ~3660–3690)
- Power Weapon Profiles (§6.2.12, LI lines ~3775–3826)
- Shock Weapon Profiles (§6.2.13, LI lines ~3831–3898)
- Force Weapon Profiles (§6.2.14, LI lines ~3934–3964; no Existing/Source-B counterparts → not added)
- Low-Tech Melee Weapon Profiles (§6.2.15, LI lines ~4020–4078)
- Exotic Melee Weapon Profiles (§6.2.16, LI lines ~4160+; distinct set, no overlap)

**Precedence Order A > B > Existing re-applied field-by-field.** Corrected winning values now in
`Reference/RT-Weapons.md`:

### Las (Source A wins)
- Laspistol: +Variable quality, 2 kg, Average
- Lasgun: RoF **S/2/3**, Reload **Full**, +Variable
- Long-Las: Class **Heavy**, **Felling(2)** (was Felling(4)), +Deadly, +Variable, 5 kg (Reliable kept from Existing)
- Hot-Shot Laspistol/Lasgun: +Variable, weights 4/6 kg, Rare
- Lascannon: Proven(3) + Reliable (kept), 55 kg
- Multi-Laser: 35 kg, Very Rare

### Solid Projectile (Source A wins)
- Stub Automatic: +Reliable, Reload Full, 2 kg, Common
- Stub Revolver: 2 Full, 2 kg
- Autopistol: RoF **S/3/6**, Full, 2 kg, Average
- Autogun: RoF **S/3/10**, Full, 5 kg
- Combat Shotgun (= A "Automatic Shotgun"): +Reliable, Full, 5 kg, Scarce
- Pump Shotgun (= A "Shotgun"): 2 Full, 5 kg, Average
- Heavy Stubber: **Clip 80**, +Reliable, 30 kg, Rare
- Autocannon: **Clip 15**, Reload Full, 40 kg, Very Rare
- Hand Cannon: Availability Scarce
- Shotgun Pistol (= A "Shotpistol"): 15m, 1d10+3 I, Clip 4, 2 Full, 3 kg, Average

### Bolt (Source A wins)
- Bolt Pistol: 4 kg, Very Rare
- Boltgun: RoF **S/2/3**
- Heavy Bolter: Reload **Full** (Damage 1d10+8 X already named-conflict)
- Storm Bolter: Damage **1d10+4 X**, Pen **2**, 10 kg

### Plasma (Source A wins)
- Pen **8** family-wide; Qualities **Maximal, Overheat**
- Plasma Gun: Clip **40**, Reload **5 Full**

### Melta (Source A wins)
- Inferno Pistol: 3 kg, Near Unique
- Meltagun: 2d10+10 E, Pen 12, Clip 5, Full, 15 kg, Very Rare (verified — matches reviewer fix)
- Multi-Melta: Reload **Full**, Availability **Extremely Rare**

### Flame (Source A wins)
- Hand Flamer: 4 kg
- Heavy Flamer: 1d10+5 E, 45 kg, Rare (verified — matches reviewer fix)

### Primitive Ranged (Source A wins for Bow/Crossbow)
- Bow: SBx5m, S/2/-, 1d10+1 R, Primitive(6)
- Crossbow: 1d10+2 R, Clip 8, Primitive(7)

### Chain (Source A wins)
- Chain Axe: +Felling(1), +Two-Handed, Scarce
- Eviscerator: 2d10+2(+SB) R, Pen 8, +Vengeful(9), 15 kg, Very Rare

### Power (Source A wins)
- Power Axe: Pen **6**, +Felling(1), +Two-Handed
- Power Maul (= A "Power Maul (High)"): Concussive(0) (not (1)), +Unbalanced, Shocking removed, 4 kg
- Power Fist: Pen **8**, Extremely Rare
- Thunder Hammer: 2d10+SBx2+4 E, Concussive(2), +Two-Handed, 16 kg, Near Unique
- Lightning Claw: +Proven(4) (Razor Sharp removed), 30 kg, Extremely Rare
- Power Maul (Low): +Unbalanced, 4 kg

### Shock (Source A wins)
- Shock Glove (= A "Shock Gauntlets"): 1 kg, Scarce

### Low-Tech Melee (Source A wins)
- Knife: quality **Piercing** (not Primitive)
- Sword: Primitive(7); Staff: Primitive(6) + Two-Handed
- Great Weapon: Pen 2, Proven(2), Two-Handed, 2d10 I/R, Average
- Axe: Pen 2, Felling(1), 4 kg, Common
- Spear: +Piercing, +Reach, +Two-Handed
- Warhammer: 1d10+2, Pen 2, Concussive(1), Primitive(8), Two-Handed, Unwieldy, 5 kg
- Improvised: 1d10 I/R, Primitive(7)

### Not overridden (Source A has no overlapping entry)
- Exotic Ranged (xenos: Shuriken/Kroot/Ork/Needle/Digi) — Source A exotic is a distinct Mechanicus set
- Exotic Melee (Ork weapons, Fractal Blade, Ghost Sword, Harlequin's Kiss) — Source A exotic melee is a distinct set
- Launchers — Source A entries vary by ammo (no numeric override)
- Grenades, Weapon Upgrades (5-9), Ammunition (5-10/5-11) — no Source A numeric equivalent
- Shield (Primitive melee) — no Source A profile

### Deliberately NOT added (scope: reconcile existing families, not import all of Source A)
- Force Weapons (§6.2.14): a Source A-only family with no Existing or Source B counterpart in the
  current file; adding a whole new Force section is new content beyond precedence reconciliation.
- Numerous Source A-only weapons (Meltablaster, Meltabeamer, Melta Lance, Chainflail, Chainglaive,
  Power Greatsword/Greataxe/etc., Arc/Volkite/Radium/Phosphor families) — out of scope for this
  reconciliation pass; may be captured by a future coverage-expansion task if desired.
