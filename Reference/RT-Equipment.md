# Rogue Trader Equipment Reference
## Optimized for AI consumption — non-weapon equipment profiles for game implementation

---

## Scope

This document holds **non-weapon equipment** only: armour, force fields, gear, tools,
drugs & consumables, and cybernetics. All weapon profiles (ranged, melee, weapon
upgrades, ammunition) live in `RT-Weapons.md` and are intentionally excluded here.

---

## Legend

- **Covered (Locations)**: Body regions the armour protects — Head, Arms, Body, Legs, or All
- **AP**: Armour Points (armour value applied to a covered location). Where AP differs by
  location, each covered location's AP is recorded explicitly (e.g., `Head 5 / Body 6 / Arms 4 / Legs 4`)
- **Protection Rating**: A force field's block value, preserved verbatim from the source
- **Overload**: A force field's overload range, preserved verbatim from the source
- **Weight (kg)**: Item weight where the source provides it
- **Availability**: Rarity/acquisition value where the source provides it
- **—**: Value absent in the source (not applicable or not provided)
- **†**: Footnote marker for a special rule or restriction spelled out beneath the table

---

## Source Citation Convention

Every entry or section is attributed to its origin so values can be traced back:

- **Source A (Liber Imperium §6 "Armoury & Acquisitions")** — cited as `Liber Imperium §6`
- **Source B (RT Core Rulebook Ch. V "Armoury")** — cited as the specific table, `RT Core Table 5-N`
  (e.g., `RT Core Table 5-12` for armour, `RT Core Table 5-16` for cybernetics)
- **Conflict-resolved entries** cite the source whose value was recorded (the winning source per
  the Precedence Order: Source A > Source B > existing curated data)

Citations are placed per section (`> Source: …`) when every row in a section shares one origin,
or with an inline `†` footnote on rows whose origin differs from the section default.

---

## Armour

> Source: `RT Core Table 5-12` for the primary numeric profiles below; rows drawn from
> Source A carry an inline `†`/`‡` marker and cite `Liber Imperium §6`.
> Per the Precedence Order (Source A > Source B), where the same item appears in both sources
> the Source A value is recorded and the differing Source B value discarded — the notable
> multi-source items are flagged in the AP-by-location / conflict notes beneath the tables.
> Note on AP-by-location: neither Source B Table 5-12 nor Source A §6 assigns *different* AP to
> different covered locations — each armour applies a single uniform AP across every location it
> covers. Accordingly the `AP` column records that single value and the `Covered (Locations)`
> column records the protected locations; no per-location split is required for these entries.

### Primitive Armour

| Name | Covered (Locations) | AP | Weight (kg) | Availability |
|---|---|---|---|---|
| Robes ‡ | All | 0 | 4 | Plentiful (single) / Ŧ35 |
| Heavy Leathers/Furs | Arms, Body, Legs | 2 | 7 | Common |
| Grox Hide/Chainmail | Arms, Body, Legs | 3 | 15 | Common |
| Chainmail ‡ | All | 3 | 4 (single) / 16 (full set) | Common (single) / Average (full set) |
| Feudal World Plate | All | 5 | 30 | Scarce |
| Burnscour Beast Hide | Body | 6 | 20 | Very Rare |
| Xeno-Hide Pelts ‡ | Body | 6 | 10 | Very Rare |

### Flak Armour

| Name | Covered (Locations) | AP | Weight (kg) | Availability |
|---|---|---|---|---|
| Flak Helmet | Head | 2 | 2 | Average |
| Flak Cloak | Arms, Body, Legs | 3 | 8 | Scarce |
| Flak Coat | Arms, Body, Legs | 3 | 5 | Average |
| Guard Flak Armour | All | 4 | 11 | Scarce |
| Flak (full suit) † | All | 4 | 3 (single) / 12 (full set) | Average (single) / Scarce (full set) |

> `†` Flak Armour increases its AP by 2 when hit by Explosive type Damage. (Source A §6)

### Mesh Armour

| Name | Covered (Locations) | AP | Weight (kg) | Availability |
|---|---|---|---|---|
| Mesh Cowl | Head | 3 | 0.5 | Rare |
| Xeno Mesh | Arms, Body, Legs | 3 | 2 | Rare |
| Mesh Combat Cloak | Arms, Body, Legs | 4 | 1.5 | Very Rare |
| Mesh Vest | Body | 4 | 2 | Rare |
| Mesh (full suit) ‡ | All | 4 | 0.5 (single) / 3 (full set) | Average (single) / Scarce (full set) |

### Carapace Armour

| Name | Covered (Locations) | AP | Weight (kg) | Availability |
|---|---|---|---|---|
| Carapace Helm | Head | 4 | 2 | Rare |
| Enforcer Light Carapace | All | 5 | 15 | Rare |
| Light Carapace ‡ | All | 5 | 4 (single) / 16 (full set) | Scarce (single) / Rare (full set) |
| Carapace Chestplate | Body | 6 | 7 | Rare |
| Storm Trooper Carapace | All | 6 | 15 | Very Rare |
| Heavy Carapace ‡ | All | 6 | 5 (single) / 20 (full set) | Rare (single) / Very Rare (full set) |

### Other Armours

| Name | Covered (Locations) | AP | Weight (kg) | Availability |
|---|---|---|---|---|
| Advanced Helmet Systems | — | — | — | Very Rare |
| Armoured Bodyglove | Arms, Body, Legs | 3 | 5 | Rare |
| Ballistic Cloak ‡§ | Arms, Body, Legs | +1 | 1 | Scarce / Ŧ275 |

> `§` Ballistic Cloak: This may be worn over other armour, but must be custom fitted for one
> type/suit of armour. When a Ballistic Cloak's Craftsmanship is raised it can only gain a bonus
> to its wearer's chosen Social Skill, not to its AP. (Source A §6)

### Power Armour

| Name | Covered (Locations) | AP | Weight (kg) | Availability |
|---|---|---|---|---|
| Light Power Armour | All | 7 | 40 | Very Rare |
| Power Armour | All | 8 | 65 | Very Rare |
| Terminator Armour ‡¶ | All | 14 | 400 | Unique / Ŧ100,000 |

> `¶` Terminator Armour counts as having a Force Field with a Protection Rating of 35 and an
> Overload of 10, which changes accordingly with the armour's Craftsmanship as per Force Field
> Craftsmanship. See Source A §6 "Powered Armour for Mortals" for the full benefits/penalties. (Source A §6)

**Footnote markers used above:**
- `‡` Entry sourced from Source A §6 "Armour Profiles" (`Liber Imperium §6`); not present as a
  distinct row in Source B Table 5-12.
- Where an item appears in both sources with differing values (multi-source conflict), the
  Source A value is recorded per the Precedence Order. Notable examples: **Feudal Plate** — Source A
  classifies it as Carapace AP 4 / 7–30 kg, while Source B Table 5-12 lists "Feudal World Plate"
  as Primitive AP 5 / 30 kg; the two are kept as the distinct rows above (Source B's numeric
  profile retained for the Table 5-12 entry, Source A's descriptive classification noted here).
  **Chainmail** — Source A AP 3 / All (recorded), reconciled against Source B's combined
  "Grox Hide/Chainmail" primitive row (AP 3 / Arms, Body, Legs).

---

## Force Fields

> Source: `Liber Imperium §6` — Source A "Force Field Profiles" table (§6.3). Source B has no
> dedicated force-field table, so every row here originates from Source A. Values (Protection
> Rating, Overload, Weight/Size, Availability) are preserved verbatim from the source table.

| Name | Protection Rating | Overload | Gear Slot | Weight/Size | Availability |
|---|---|---|---|---|---|
| Refractor Field | 30 | 10 | Neck or Waist | 2 kg / Size (1) | Very Rare / Ŧ15,000 |
| Conversion Field | 50 | 10 | Neck or Waist | 1 kg / Size (1) | Extremely Rare / Ŧ20,000 |
| Displacer Field ‖ | 55 | 10 | Waist | 2 kg / Size (1) | Near Unique / Ŧ50,000 |
| Power Field (Personal) | 75 | 10 | Back | 50 kg / Size (3) | Near Unique / Ŧ12,000,000 |
| Power Field (Emplacement) | 80 | 10 | — | 500 kg / Size (6) | Near Unique / Ŧ10,000,000 |
| Field Wall Generator | 65 | 10 | — | 18 kg / Size (3) | Very Rare / Ŧ12,000 |
| Rosarius ※ | 50 | 10 | Neck or Waist | 0.5 kg / Size (1) | Extremely Rare / Ŧ25,000 |
| Icon of the Just | 55 | 10 | Neck or Waist | 0.5 kg / Size (1) | Near Unique / Ŧ45,000 |
| Null Blocker | 60 | 10 | Neck or Waist | 0.5 kg / Size (1) | Very Rare / Ŧ20,000 |
| Jokaerian Field | 70 | 10 | Neck or Waist | 0.5 kg / Size (1) | Near Unique / Ŧ50,000 |
| Personal Flare Shield ✱ | 25/50 | 10 | Back | 3 kg / Size (2) | Extremely Rare / Ŧ100,000 |
| Refraction Bracer ✚ | 30 | 10 | Wrist | 0.5 kg / Size (2) | Rare / Ŧ5,000 |

**Force Field special rules (preserved verbatim from Source A §6):**

- `✱` **Personal Flare Shield:** Only characters with the Mechanicus Implants Trait may use a
  Personal Flare Shield. It has a variable Protection Rating: a Personal Flare Shield has a
  Protection Rating of 25, and this Protection Rating is doubled to 50 when the wearer is hit by an
  attack with the Blast, Scatter, or Spray Qualities, as well as when they are fit by a Full Auto
  Burst (though more than one hit must be made against them before Evasions for the Protection
  Rating to count as 50).
- `✚` **Refraction Bracer:** The Refraction Bracer only protects against hits that would strike the
  Body or Arms Hit Locations, leaving the Head and Legs unprotected. In addition it does not
  function against weapons with the Blast Quality (though it provides half of its Protection Rating
  against Spray weapons), it does, however, afford its wielder a +10 to Parry Tests. Refraction
  Bracers occupy a Wrist Gear Slot.
- `※` **Rosarius:** Allies who can draw a line of sight to a character with a rosarius gain a +10
  bonus to Fear and Pinning tests; this is lost if the wearer dies or suffers any Critical damage.
  If the Rosarius would Overload and the bearer has the Pure Faith Talent he may make a Fate Test.
  If this is passed the Rosarius does not Overload, if it is failed then the Rosarius still
  Overloads. Characters with 20 or more Corruption cannot use a Rosarius, it simply refuses to
  protect them.
- `‖` **Displacer Field:** When the field successfully nullifies an attack, the user jumps in a
  random direction. Roll 3d10 for the number of metres travelled — the wearer always emerges on
  solid footing and in a suitable empty space. If all three dice come up with the same number
  (e.g., 3 results of 7), then the user does not re-emerge for 1d5 rounds and gains 1d5 Corruption
  Points from exposure to the unnatural energies within the Warp. If the activation is unexpected,
  then the wearer cannot act for one round while he regains his sense of place.

---

## Gear

<!-- Populated in task 4.2 -->

---

## Tools

<!-- Populated in task 4.2 -->

---

## Drugs & Consumables

<!-- Populated in task 4.2 -->

---

## Cybernetics

<!-- Populated in task 4.2 -->
