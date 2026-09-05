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
- **Region**: Faction/region annotation — exactly one of `Ork`, `Eldar`, `DarkEldar`, `Necron`,
  `Tau`, `Tyranid`, `Kroot`, `Chaos`, `ImperialHuman`, `Servitor`, or the `Universal` tag for
  faction-agnostic/primitive/civilian items. Casing matches `Scripts/Enemies.lua` `chance` keys
  and `regionForBiome` outputs. `Universal ††` marks a default-assigned Universal classification
  where the faction is undeterminable from the source.
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

| Name | Covered (Locations) | AP | Weight (kg) | Availability | Region |
|---|---|---|---|---|---|
| Robes ‡ | All | 0 | 4 | Plentiful (single) / Ŧ35 | Universal |
| Heavy Leathers/Furs | Arms, Body, Legs | 2 | 7 | Common | Universal |
| Grox Hide/Chainmail | Arms, Body, Legs | 3 | 15 | Common | Universal |
| Chainmail ‡ | All | 3 | 4 (single) / 16 (full set) | Common (single) / Average (full set) | Universal |
| Feudal World Plate | All | 5 | 30 | Scarce | Universal |
| Burnscour Beast Hide | Body | 6 | 20 | Very Rare | Universal |
| Xeno-Hide Pelts ‡ | Body | 6 | 10 | Very Rare | Universal |

### Flak Armour

| Name | Covered (Locations) | AP | Weight (kg) | Availability | Region |
|---|---|---|---|---|---|
| Flak Helmet | Head | 2 | 2 | Average | ImperialHuman |
| Flak Cloak | Arms, Body, Legs | 3 | 8 | Scarce | ImperialHuman |
| Flak Coat | Arms, Body, Legs | 3 | 5 | Average | ImperialHuman |
| Guard Flak Armour | All | 4 | 11 | Scarce | ImperialHuman |
| Flak (full suit) † | All | 4 | 3 (single) / 12 (full set) | Average (single) / Scarce (full set) | ImperialHuman |

> `†` Flak Armour increases its AP by 2 when hit by Explosive type Damage. (Source A §6)

### Mesh Armour

| Name | Covered (Locations) | AP | Weight (kg) | Availability | Region |
|---|---|---|---|---|---|
| Mesh Cowl | Head | 3 | 0.5 | Rare | ImperialHuman |
| Xeno Mesh | Arms, Body, Legs | 3 | 2 | Rare | Universal †† |
| Mesh Combat Cloak | Arms, Body, Legs | 4 | 1.5 | Very Rare | ImperialHuman |
| Mesh Vest | Body | 4 | 2 | Rare | ImperialHuman |
| Mesh (full suit) ‡ | All | 4 | 0.5 (single) / 3 (full set) | Average (single) / Scarce (full set) | ImperialHuman |

> `††` Region default-assigned (faction undeterminable from source). Applies to **Xeno Mesh**
> (generic "xeno" origin with no specific taxonomy faction).

### Carapace Armour

| Name | Covered (Locations) | AP | Weight (kg) | Availability | Region |
|---|---|---|---|---|---|
| Carapace Helm | Head | 4 | 2 | Rare | ImperialHuman |
| Enforcer Light Carapace | All | 5 | 15 | Rare | ImperialHuman |
| Light Carapace ‡ | All | 5 | 4 (single) / 16 (full set) | Scarce (single) / Rare (full set) | ImperialHuman |
| Carapace Chestplate | Body | 6 | 7 | Rare | ImperialHuman |
| Storm Trooper Carapace | All | 6 | 15 | Very Rare | ImperialHuman |
| Heavy Carapace ‡ | All | 6 | 5 (single) / 20 (full set) | Rare (single) / Very Rare (full set) | ImperialHuman |

### Other Armours

| Name | Covered (Locations) | AP | Weight (kg) | Availability | Region |
|---|---|---|---|---|---|
| Advanced Helmet Systems | — | — | — | Very Rare | ImperialHuman |
| Armoured Bodyglove | Arms, Body, Legs | 3 | 5 | Rare | ImperialHuman |
| Ballistic Cloak ‡§ | Arms, Body, Legs | +1 | 1 | Scarce / Ŧ275 | ImperialHuman |

> `§` Ballistic Cloak: This may be worn over other armour, but must be custom fitted for one
> type/suit of armour. When a Ballistic Cloak's Craftsmanship is raised it can only gain a bonus
> to its wearer's chosen Social Skill, not to its AP. (Source A §6)

### Power Armour

| Name | Covered (Locations) | AP | Weight (kg) | Availability | Region |
|---|---|---|---|---|---|
| Light Power Armour | All | 7 | 40 | Very Rare | ImperialHuman |
| Power Armour | All | 8 | 65 | Very Rare | ImperialHuman |
| Terminator Armour ‡¶ | All | 14 | 400 | Unique / Ŧ100,000 | ImperialHuman |

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

| Name | Protection Rating | Overload | Gear Slot | Weight/Size | Availability | Region |
|---|---|---|---|---|---|---|
| Refractor Field | 30 | 10 | Neck or Waist | 2 kg / Size (1) | Very Rare / Ŧ15,000 | ImperialHuman |
| Conversion Field | 50 | 10 | Neck or Waist | 1 kg / Size (1) | Extremely Rare / Ŧ20,000 | ImperialHuman |
| Displacer Field ‖ | 55 | 10 | Waist | 2 kg / Size (1) | Near Unique / Ŧ50,000 | ImperialHuman |
| Power Field (Personal) | 75 | 10 | Back | 50 kg / Size (3) | Near Unique / Ŧ12,000,000 | ImperialHuman |
| Power Field (Emplacement) | 80 | 10 | — | 500 kg / Size (6) | Near Unique / Ŧ10,000,000 | ImperialHuman |
| Field Wall Generator | 65 | 10 | — | 18 kg / Size (3) | Very Rare / Ŧ12,000 | ImperialHuman |
| Rosarius ※ | 50 | 10 | Neck or Waist | 0.5 kg / Size (1) | Extremely Rare / Ŧ25,000 | ImperialHuman |
| Icon of the Just | 55 | 10 | Neck or Waist | 0.5 kg / Size (1) | Near Unique / Ŧ45,000 | ImperialHuman |
| Null Blocker | 60 | 10 | Neck or Waist | 0.5 kg / Size (1) | Very Rare / Ŧ20,000 | ImperialHuman |
| Jokaerian Field | 70 | 10 | Neck or Waist | 0.5 kg / Size (1) | Near Unique / Ŧ50,000 | Universal †† |
| Personal Flare Shield ✱ | 25/50 | 10 | Back | 3 kg / Size (2) | Extremely Rare / Ŧ100,000 | ImperialHuman |
| Refraction Bracer ✚ | 30 | 10 | Wrist | 0.5 kg / Size (2) | Rare / Ŧ5,000 | ImperialHuman |

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

**Region marker used above:**
- `††` Region default-assigned (faction undeterminable from source). Applies to the
  **Jokaerian Field** (Jokaero-derived xenos tech with no taxonomy faction).

---

## Gear

> Source: `RT Core Table 5-13` for the base profiles below. Per the Precedence Order
> (Source A > Source B), where the same item also appears in Source A §6's "Clothing &
> Personal Gear Summary" with a differing value, the Source A value is recorded and the
> differing Source B value discarded; those rows carry an inline `‡` and cite `Liber Imperium §6`.
> `†` Clothing and Adornment may have any appropriate Availability. There is always a way to
> spend more on appearance, to find yet more exclusive fabrics, clothiers, and jewelers, and
> otherwise display the ways that your wealth glitters more brightly than that of your rivals.

| Name | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|
| Backpack ‡ | 2 | Abundant | Liber Imperium §6 | Universal |
| Cameleoline Cloak ‡ | 3 | Very Rare | Liber Imperium §6 | ImperialHuman |
| Charm | — | Average | RT Core Table 5-13 | Universal |
| Chrono ‡ | — | Plentiful | Liber Imperium §6 | Universal |
| Clip Harness/Drop Harness ‡ | 2 | Common | Liber Imperium §6 | Universal |
| Clothing and Adornment (Common) † | — | Abundant | RT Core Table 5-13 | Universal |
| Clothing and Adornment (Merchant Guilder) † | — | Average | RT Core Table 5-13 | Universal |
| Clothing and Adornment (Noble) † | — | Scarce | RT Core Table 5-13 | Universal |
| Filtration Plugs | — | Common | RT Core Table 5-13 | ImperialHuman |
| Night Cloak | 2 | Average | RT Core Table 5-13 | Universal |
| Photo-visors/Photo-contacts | 0.5 | Scarce | RT Core Table 5-13 | ImperialHuman |
| Preysense Goggles | 0.5 | Rare | RT Core Table 5-13 | ImperialHuman |
| Rebreather | 1 | Scarce | RT Core Table 5-13 | ImperialHuman |
| Recoil Gloves | 0.5 | Rare | RT Core Table 5-13 | ImperialHuman |
| Respirator/Gas Mask | 0.5 | Average | RT Core Table 5-13 | ImperialHuman |
| Shifting Fabric | — | Very Rare | RT Core Table 5-13 | Universal |
| Survival Suit ‡ | 10 | Average | Liber Imperium §6 | ImperialHuman |
| Synskin ‡ | 2 | Very Rare | Liber Imperium §6 | ImperialHuman |
| Void Suit ‡ | 8 | Scarce | Liber Imperium §6 | ImperialHuman |
| Void Suit (Selenite-pattern) ‡ | 25 | Very Rare | Liber Imperium §6 | ImperialHuman |

**Footnote markers used above:**
- `†` Clothing & Adornment may have any appropriate Availability (see section note).
- `‡` Multi-source item — value recorded from Source A §6 "Clothing & Personal Gear Summary"
  per the Precedence Order (Source A > Source B). Source A overrides applied: **Backpack**
  (Plentiful → Abundant, 1 → 2 kg), **Cameleoline Cloak** (Rare → Very Rare, 0.5 → 3 kg),
  **Chrono** (Abundant → Plentiful), **Clip/Drop Harness** ("Drop Harness" Common, 2 kg),
  **Survival Suit** (Plentiful → Average, — → 10 kg), **Synskin** (Extremely Rare → Very Rare),
  **Void Suit** (Plentiful → Scarce), **Void Suit (Selenite-pattern)** (Scarce → Very Rare).

---

## Tools

> Source: `RT Core Table 5-15` for the base profiles below. Per the Precedence Order
> (Source A > Source B), where the same item also appears in Source A §6's "Tools Summary"
> with a differing Availability, the Source A value is recorded; those rows carry an inline `‡`
> and cite `Liber Imperium §6`.

| Name | Weight (kg) | Availability | Source | Region |
|---|---|---|---|---|
| Almanac Astrae Divinitus | 4 | Extremely Rare | RT Core Table 5-15 | ImperialHuman |
| Arms Coffer | 6 | Average | RT Core Table 5-15 | ImperialHuman |
| Auspex/Scanner | 0.5 | Scarce | RT Core Table 5-15 | ImperialHuman |
| Auto-quill | — | Scarce | RT Core Table 5-15 | ImperialHuman |
| Calculance Array | 120 | Scarce | RT Core Table 5-15 | ImperialHuman |
| Combi-tool | 1 | Rare | RT Core Table 5-15 | ImperialHuman |
| Data-loom (Hadd-pattern) | 13 | Very Rare | RT Core Table 5-15 | ImperialHuman |
| Data-slate | 0.5 | Common | RT Core Table 5-15 | ImperialHuman |
| Demolition Charge | 1 | Scarce | RT Core Table 5-15 | ImperialHuman |
| Diagnostor ‡ | 4 | Rare | Liber Imperium §6 | ImperialHuman |
| Glow-globe/Lamp Pack | 0.5 | Abundant | RT Core Table 5-15 | Universal |
| Grapnel ‡ | 2 | Common | Liber Imperium §6 | Universal |
| Grapplehawk | — | Very Rare | RT Core Table 5-15 | ImperialHuman |
| Grav Chute | 15 | Rare | RT Core Table 5-15 | ImperialHuman |
| Jump Pack | 25 | Rare | RT Core Table 5-15 | ImperialHuman |
| Lord-Captain's Baton | 1 | Very Rare | RT Core Table 5-15 | ImperialHuman |
| Magboots | 2 | Rare | RT Core Table 5-15 | ImperialHuman |
| Magnoculars | 0.5 | Average | RT Core Table 5-15 | Universal |
| Manacles | 1 | Plentiful | RT Core Table 5-15 | Universal |
| Mefonte's Orthodoxy | 2 | Scarce | RT Core Table 5-15 | ImperialHuman |
| Melta-bomb | 12 | Very Rare | RT Core Table 5-15 | ImperialHuman |
| Micro-bead | — | Average | RT Core Table 5-15 | ImperialHuman |
| Multikey | — | Scarce | RT Core Table 5-15 | ImperialHuman |
| Multicompass | 4 | Near Unique | RT Core Table 5-15 | ImperialHuman |
| Navis Prima | 1 | Very Rare | RT Core Table 5-15 | ImperialHuman |
| Pict Recorder ‡ | 1 | Average | Liber Imperium §6 | ImperialHuman |
| Psy-focus ‡ | — | Average | Liber Imperium §6 | ImperialHuman |
| Renumeration Engine | 7 | Very Rare | RT Core Table 5-15 | ImperialHuman |
| Shipboard Emergency Kit | 6 | Common | RT Core Table 5-15 | ImperialHuman |
| Screamer ‡ | 1.5 | Scarce | Liber Imperium §6 | ImperialHuman |
| Servitor (Labor, Simple Monotask) | — | Scarce | RT Core Table 5-15 | Servitor |
| Servitor (Combat) | — | Rare | RT Core Table 5-15 | Servitor |
| Servitor (Complex Multitask) | — | Rare | RT Core Table 5-15 | Servitor |
| Servo-Skull | — | Scarce | RT Core Table 5-15 | Servitor |
| Stummer ‡ | 2 | Very Rare | Liber Imperium §6 | ImperialHuman |
| Venom Ring | — | Very Rare | RT Core Table 5-15 | Universal |
| Vox Caster | — | Scarce | RT Core Table 5-15 | ImperialHuman |

**Footnote markers used above:**
- `‡` Multi-source item — Availability recorded from Source A §6 "Tools Summary" per the
  Precedence Order (Source A > Source B). Source A overrides applied: **Diagnostor**
  (Very Rare → Rare), **Grapnel** ("Grapnel & Line" Average → Common), **Pict Recorder**
  (Common → Average), **Psy-focus** ("Psy Focus" Rare → Average), **Screamer** (Average → Scarce),
  **Stummer** (Average → Very Rare).

---

## Drugs & Consumables

> Source: `RT Core Table 5-14` for the base profiles below. Per the Precedence Order
> (Source A > Source B), where the same item also appears in Source A §6's "Drugs Summary"
> or §6.6.1 "Consumables" with a differing Availability, the Source A value is recorded; those
> rows carry an inline `‡` and cite `Liber Imperium §6`.

| Name | Weight | Availability | Source | Region |
|---|---|---|---|---|
| Amasec ‡ | — | Average | Liber Imperium §6 | Universal |
| De-tox | — | Rare | RT Core Table 5-14 | ImperialHuman |
| Frenzon ‡ | — | Rare | Liber Imperium §6 | ImperialHuman |
| Injector | — | Plentiful | RT Core Table 5-14 | ImperialHuman |
| Lho-sticks | — | Common | RT Core Table 5-14 | Universal |
| Medikit | 2 kg | Common | RT Core Table 5-14 | ImperialHuman |
| Medikit (Advanced) | 5 kg | Rare | RT Core Table 5-14 | ImperialHuman |
| High Provender | — | Very Rare | RT Core Table 5-14 | Universal |
| Obscura | — | Rare | RT Core Table 5-14 | Universal |
| Ration Packs | — | Ubiquitous | RT Core Table 5-14 | Universal |
| Recaf | — | Abundant | RT Core Table 5-14 | Universal |
| Sacred Unguents | — | Very Rare | RT Core Table 5-14 | ImperialHuman |
| Slaught ‡ | — | Very Rare | Liber Imperium §6 | ImperialHuman |
| Stimm | — | Average | RT Core Table 5-14 | ImperialHuman |
| Thosophist's Philtre | — | Very Rare | RT Core Table 5-14 | ImperialHuman |
| Tranq | — | Abundant | RT Core Table 5-14 | Universal |

**Footnote markers used above:**
- `‡` Multi-source item — Availability recorded from Source A §6 per the Precedence Order
  (Source A > Source B). Source A overrides applied: **Amasec** (Scarce → Average, §6.6.1),
  **Frenzon** (Very Rare → Rare, Drugs Summary), **Slaught** (Scarce → Very Rare, Drugs Summary).

---

## Cybernetics

> Source: `RT Core Table 5-16` for the base profiles below. Per the Precedence Order
> (Source A > Source B), where the same item also appears in Source A §6.9's "Cybernetics
> Summary" with a differing Availability, the Source A value is recorded; those rows carry an
> inline `‡` and cite `Liber Imperium §6`.
> `†` Some cybernetic systems are only provided to tech-adepts of the Adeptus Mechanicus —
> though it is possible that skilled hereteks might risk the Machine Cult's wrath by implanting
> crude versions of these systems in anyone willing to pay their price. Mechanicus-restricted
> cybernetics: Ballistic/Manipulator/Medicae/Optical/Utility Mechadendrite.

| Name | Availability | Source | Region |
|---|---|---|---|
| Augur Array | Rare | RT Core Table 5-16 | ImperialHuman |
| Augmented Senses | Rare | RT Core Table 5-16 | ImperialHuman |
| Baleful Eye | Near Unique | RT Core Table 5-16 | ImperialHuman |
| Ballistic Mechadendrite † | Very Rare | RT Core Table 5-16 | ImperialHuman |
| Bionic Limb | Scarce | RT Core Table 5-16 | ImperialHuman |
| Bionic Locomotion | Scarce | RT Core Table 5-16 | ImperialHuman |
| Bionic Respiratory System | Rare | RT Core Table 5-16 | ImperialHuman |
| Bionic Heart ‡ | Very Rare | Liber Imperium §6 | ImperialHuman |
| Calculus Logi Upgrade | Very Rare | RT Core Table 5-16 | ImperialHuman |
| Cortex Implants | Very Rare | RT Core Table 5-16 | ImperialHuman |
| Cranial Armour | Scarce | RT Core Table 5-16 | ImperialHuman |
| Cybernetic Senses | Rare | RT Core Table 5-16 | ImperialHuman |
| Locator Matrix | Rare | RT Core Table 5-16 | ImperialHuman |
| Manipulator Mechadendrite † | Very Rare | RT Core Table 5-16 | ImperialHuman |
| Medicae Mechadendrite † | Very Rare | RT Core Table 5-16 | ImperialHuman |
| Memorance Implant | Rare | RT Core Table 5-16 | ImperialHuman |
| Mind Impulse Unit | Rare | RT Core Table 5-16 | ImperialHuman |
| MIU Weapon Interface | Rare | RT Core Table 5-16 | ImperialHuman |
| Optical Mechadendrite † | Very Rare | RT Core Table 5-16 | ImperialHuman |
| Respiratory Filter Implant | Rare | RT Core Table 5-16 | ImperialHuman |
| Scribe-tines | Scarce | RT Core Table 5-16 | ImperialHuman |
| Subskin Armour | Very Rare | RT Core Table 5-16 | ImperialHuman |
| Synthetic Muscle Grafts | Rare | RT Core Table 5-16 | ImperialHuman |
| Utility Mechadendrite † | Very Rare | RT Core Table 5-16 | ImperialHuman |
| Voidskin | Scarce | RT Core Table 5-16 | ImperialHuman |
| Volitor Implant | Rare | RT Core Table 5-16 | ImperialHuman |
| Vox Implant ‡ | Average | Liber Imperium §6 | ImperialHuman |

**Footnote markers used above:**
- `†` Mechanicus-restricted cybernetic (see section note): Ballistic, Manipulator, Medicae,
  Optical, and Utility Mechadendrites are provided only to tech-adepts of the Adeptus Mechanicus.
- `‡` Multi-source item — Availability recorded from Source A §6.9 "Cybernetics Summary" per the
  Precedence Order (Source A > Source B). Source A overrides applied: **Bionic Heart**
  (Rare → Very Rare), **Vox Implant** (Scarce → Average).
