# Requirements Document

## Introduction

This feature curates and extracts Warhammer 40,000 Rogue Trader (RT) equipment **data** — as human-and-AI-readable Markdown reference documents — from three sources into the project's `Reference/` folder. The output is structured reference data only; it is **not** game code. Wiring the extracted data into `Scripts/Equipment.lua` is explicitly out of scope and will be handled by a separate future spec.

The extraction draws from three sources with a defined precedence order and splits the result across two target files: weapon profiles into the existing `Reference/RT-Weapons.md`, and all non-weapon equipment into a new `Reference/RT-Equipment.md`. Existing entries in `RT-Weapons.md` are de-duplicated and reconciled rather than duplicated.

The work is a documentation/data-curation task. There is no runtime behavior, no compiled code, and therefore no automated property-based testing; verification is by manual review of the produced Markdown against the sources.

## Glossary

- **Curator**: The agent performing the extraction and curation of equipment data into the reference Markdown files. This is the "system" referenced by the acceptance criteria.
- **Source A / Liber Imperium**: `Reference/LiberImperium/06-armoury-and-acquisitions.md` — Chapter 6 "Armoury & Acquisitions" of the Liber Imperium, containing weapons, armour, force fields, tools, gear, drugs, cybernetics, and ammunition.
- **Source B / RT Core Equipment**: `Reference/RT-CoreEquipment.md` — the reformatted Rogue Trader core rulebook Chapter V "Armoury" data, containing Tables 5-4 (ranged weapons), 5-5 through 5-8 (melee weapons), 5-9 (weapon upgrades), 5-10 (ammo), 5-11 (unusual ammunition), 5-12 (armour), 5-13 (gear), 5-14 (drugs & consumables), 5-15 (tools), and 5-16 (cybernetics).
- **Existing Target / RT-Weapons.md**: `Reference/RT-Weapons.md` — the pre-existing curated weapon profile document that already contains some weapon profiles.
- **New Target / RT-Equipment.md**: `Reference/RT-Equipment.md` — the new document created by this feature for all non-weapon equipment.
- **Weapon Profile**: A single equipment entry describing a ranged weapon, melee weapon, weapon upgrade, or ammunition type.
- **Non-Weapon Equipment**: Armour, force fields, gear, tools, drugs & consumables, and cybernetics/augmetics.
- **Precedence Order**: The ranking used to resolve conflicting values for the same item: Source A (Liber Imperium) is highest, then Source B (RT Core Equipment), then the Existing Target value.
- **Conflict**: A situation where the same item appears in more than one source with one or more differing stat values (for example, differing Penetration, Damage, Range, or Qualities).
- **Weapon Table Format**: The AI-consumption-optimized Markdown table style used by `RT-Weapons.md` and `RT-Bestiary.md`, with weapon columns: Name, Class, Range, RoF, Damage, Pen, Clip, Reload, Qualities — plus weight (kg) and Availability columns where the source provides them.
- **Source Fidelity**: Faithful preservation of a source's damage formulas, penetration values, weapon qualities, armour AP-by-location, weights, availability, and footnotes/special rules.
- **Source Citation**: A short attribution identifying where an entry or section originates (for example, "Liber Imperium §6" or "RT Core Table 5-12").

## Requirements

### Requirement 1: Two-File Output Split

**User Story:** As a game designer, I want weapon data and non-weapon equipment data in separate reference files, so that I can locate and later wire each category independently.

#### Acceptance Criteria

1. THE Curator SHALL write all Weapon Profiles into `Reference/RT-Weapons.md`.
2. WHERE an extracted entry is a ranged weapon, a melee weapon, a weapon upgrade, or an ammunition type, THE Curator SHALL place the entry in `Reference/RT-Weapons.md`.
3. THE Curator SHALL create the file `Reference/RT-Equipment.md`.
4. WHERE an extracted entry is armour, a force field, gear, a tool, a drug or consumable, or a cybernetic or augmetic, THE Curator SHALL place the entry in `Reference/RT-Equipment.md`.
5. THE Curator SHALL exclude all Non-Weapon Equipment from `Reference/RT-Weapons.md`.
6. THE Curator SHALL exclude all Weapon Profiles from `Reference/RT-Equipment.md`.

### Requirement 2: Source Coverage

**User Story:** As a game designer, I want every relevant equipment category from all three sources represented, so that the reference set is complete.

#### Acceptance Criteria

1. THE Curator SHALL extract weapons, armour, force fields, tools, gear, drugs, cybernetics, and ammunition from Source A (`Reference/LiberImperium/06-armoury-and-acquisitions.md`).
2. THE Curator SHALL extract entries from Source B Tables 5-4, 5-5, 5-6, 5-7, 5-8, 5-9, 5-10, 5-11, 5-12, 5-13, 5-14, 5-15, and 5-16 of `Reference/RT-CoreEquipment.md`.
3. THE Curator SHALL reconcile extracted Weapon Profiles against the entries already present in `Reference/RT-Weapons.md`.

### Requirement 3: Conflict Resolution by Precedence

**User Story:** As a game designer, I want conflicting stats resolved by a fixed precedence, so that the reference data is consistent and predictable.

#### Acceptance Criteria

1. IF the same item appears in Source A and Source B with one or more differing stat values, THEN THE Curator SHALL record the Source A value and discard the differing Source B value.
2. IF the same item appears in Source B and the Existing Target with one or more differing stat values, AND the item is absent from Source A, THEN THE Curator SHALL record the Source B value and discard the differing Existing Target value.
3. IF the same item appears in Source A and the Existing Target with one or more differing stat values, THEN THE Curator SHALL record the Source A value and overwrite the differing Existing Target value.
4. WHEN resolving a Conflict for the Inferno Pistol Penetration, the Multi-Melta Damage, or the Heavy Bolter Damage, THE Curator SHALL apply the Source A (Liber Imperium) value.
5. WHERE a stat value is present in one source and absent in the others for the same item, THE Curator SHALL record the present value without treating the absence as a Conflict.

### Requirement 4: De-Duplication

**User Story:** As a game designer, I want each item represented once, so that the reference files contain no duplicate entries.

#### Acceptance Criteria

1. IF an extracted item is already present in `Reference/RT-Weapons.md`, THEN THE Curator SHALL reconcile the existing entry per the Precedence Order rather than adding a second entry for that item.
2. THE Curator SHALL produce at most one entry per distinct item within `Reference/RT-Weapons.md`.
3. THE Curator SHALL produce at most one entry per distinct item within `Reference/RT-Equipment.md`.

### Requirement 5: Weapon Table Format Consistency

**User Story:** As a developer, I want weapon entries in the established table format, so that the data is consistent and easy to parse for later wiring.

#### Acceptance Criteria

1. THE Curator SHALL format each Weapon Profile using the Weapon Table Format columns Name, Class, Range, RoF, Damage, Pen, Clip, Reload, and Qualities.
2. WHERE a source provides a weight value for a weapon, THE Curator SHALL include a weight (kg) column value for that weapon.
3. WHERE a source provides an availability value for a weapon, THE Curator SHALL include an Availability column value for that weapon.
4. THE Curator SHALL match the AI-consumption-optimized Markdown table style used by `Reference/RT-Weapons.md` and `Reference/RT-Bestiary.md`.

### Requirement 6: Non-Weapon Equipment Format Consistency

**User Story:** As a developer, I want non-weapon equipment presented in clear tables preserving each category's native columns, so that the data is readable and later machine-consumable.

#### Acceptance Criteria

1. THE Curator SHALL organize `Reference/RT-Equipment.md` into sections for armour, force fields, gear, tools, drugs and consumables, and cybernetics.
2. WHERE an entry is armour, THE Curator SHALL preserve the armour points by covered location.
3. THE Curator SHALL match the AI-consumption-optimized Markdown table style used by `Reference/RT-Weapons.md` and `Reference/RT-Bestiary.md`.

### Requirement 7: Source Fidelity

**User Story:** As a game designer, I want extracted values to faithfully match the source, so that the reference data is trustworthy.

#### Acceptance Criteria

1. THE Curator SHALL preserve damage formulas as written in the selected source.
2. THE Curator SHALL preserve penetration values as written in the selected source.
3. THE Curator SHALL preserve weapon qualities as written in the selected source.
4. WHERE an armour entry defines armour points by location, THE Curator SHALL preserve the armour points for each covered location.
5. WHERE a source provides a weight value for an entry, THE Curator SHALL preserve the weight value.
6. WHERE a source provides an availability value for an entry, THE Curator SHALL preserve the availability value.
7. WHERE a source provides a footnote or special rule for an entry, THE Curator SHALL preserve the footnote or special rule.

### Requirement 8: Source Citation

**User Story:** As a game designer, I want each entry or section attributed to its source, so that I can trace values back to the original material.

#### Acceptance Criteria

1. WHERE an entry originates from Source A, THE Curator SHALL record a Source Citation identifying the Liber Imperium chapter (for example, "Liber Imperium §6").
2. WHERE an entry originates from Source B, THE Curator SHALL record a Source Citation identifying the RT Core table (for example, "RT Core Table 5-12").
3. WHEN a Conflict is resolved in favor of a source, THE Curator SHALL cite the source whose value was recorded.

### Requirement 9: Scope Boundary

**User Story:** As a project maintainer, I want this feature limited to reference data, so that game-code wiring remains a separate, controlled change.

#### Acceptance Criteria

1. THE Curator SHALL limit its output to Markdown reference documents in the `Reference/` folder.
2. THE Curator SHALL exclude changes to `Scripts/Equipment.lua`.
3. THE Curator SHALL exclude changes to compiled game source code.
