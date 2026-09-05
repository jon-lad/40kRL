# Requirements Document

## Introduction

This feature layers **region (faction) data** onto equipment so that lootable and
spawnable gear aligns with the race/faction that inhabits a given map region. It
mirrors the pattern established by the `region-based-npc-spawns` and `bestiary-npcs`
features, where a Region is a race/faction name derived from the map biome, and each
Enemy_Entry declares a per-region spawn chance keyed by Region_Name. In this feature,
equipment gains an analogous per-region weighting field so that the equipment
spawn/drop path can select gear scoped to the active level's Region_Name.

The work spans three surfaces:

1. **Reference documentation** — annotate every entry in `Reference/RT-Weapons.md`
   and `Reference/RT-Equipment.md` with a Region/Faction tag, so the source-of-truth
   catalogue records which faction each weapon or piece of equipment belongs to
   (including an explicit tag for items that are universal/common across factions).
2. **Equipment data** — add an optional per-region weighting field to the equipment
   schema in `Scripts/Equipment.lua` and populate it on entries, keyed by the same
   Region_Name taxonomy used by NPC spawns.
3. **Runtime selection** — consume the region weighting field in the equipment
   spawn/drop path so gear is selected region-scoped, consistent with the
   Cumulative-Chance selection model used for enemies.

The Region taxonomy is deliberately shared with NPC spawns so a region's enemies and
its lootable equipment stay aligned (an Ork region drops Ork gear; an Eldar region
drops Eldar gear). Universal items (e.g. common Imperial-standard gear that any
faction might carry) are tagged so they remain broadly available rather than being
locked to a single faction.

### In Scope

- Defining the equipment Region taxonomy as the same race/faction Region_Name set
  used by `region-based-npc-spawns` / `bestiary-npcs`.
- Annotating ALL entries in BOTH `Reference/RT-Weapons.md` and
  `Reference/RT-Equipment.md` with a Region/Faction annotation, including an explicit
  Universal tag for faction-agnostic items.
- Adding an optional per-region weighting field to the `Scripts/Equipment.lua` schema
  and populating existing entries, with a sensible default that preserves current
  behaviour for entries that omit the field.
- Backward compatibility: existing `Scripts/Equipment.lua` entries that predate the
  new field continue to load and validate under the existing C++ loader
  (`Headers/Equipment.hpp`, `Source/Equipment.cpp`, and the equipment load path in
  `Source/Engine.cpp`).
- Runtime region-weighted equipment selection consistent with the NPC-spawn
  cumulative-chance model, consuming the active level's Region_Name.

### Out of Scope (Future Expansion)

- Per-tile equipment region granularity within a single level (regions remain
  whole-level, matching `region-based-npc-spawns`).
- Multi-faction weighted equipment blends beyond the keyed per-region field
  (the data structure permits later expansion without a signature change).
- New biome→region mappings or changes to `regionForBiome` in `Source/WorldMap.cpp`.
- Rebalancing the existing tier-weighted selection algorithm
  (`Engine::selectEquipmentByTier`) beyond adding a region filter/weight.
- Persisting new per-item region state to save files beyond what the existing
  equipment/actor save path already covers.

## Glossary

- **Region**: A named classification applied to an entire level that determines which
  NPCs spawn and which equipment is available on that level. A Region name is a
  race/faction name (for example `"Ork"`), identical to the taxonomy used by
  `region-based-npc-spawns`.
- **Region_Name**: The string identifier of a Region. Equal to a race/faction name.
  The valid set is the same one used by NPC spawns: `Ork`, `Eldar`, `DarkEldar`,
  `Necron`, `Tau`, `Tyranid`, `Kroot`, `Chaos`, `ImperialHuman`, and `Servitor`.
- **Universal_Tag**: A distinguished Region_Name annotation value, `"Universal"`,
  applied to faction-agnostic items (common Imperial-standard or broadly-available
  gear) that are not restricted to a single faction Region.
- **Biome_Region**: The subset of Region_Names that `regionForBiome` in
  `Source/WorldMap.cpp` can currently produce from the five BiomeType values
  (`ImperialHuman`, `Ork`, `Eldar`, `Servitor`, `Chaos`). Every level resolves to one
  Biome_Region (or the Default_Region) at runtime.
- **Default_Region**: The configurable Region_Name (`config.defaultRegion` in
  `Scripts/Config.lua`, compiled fallback `"Ork"`) applied to a level with no
  world-map biome context, resolved via `resolveDefaultRegion()`.
- **Reference_Docs**: The two Markdown catalogues `Reference/RT-Weapons.md` (weapon
  profiles) and `Reference/RT-Equipment.md` (non-weapon equipment: armour, force
  fields, gear, tools, consumables, cybernetics).
- **Region_Annotation**: A Region/Faction tag recorded against each Reference_Docs
  entry, holding a Region_Name from the valid set or the Universal_Tag.
- **Availability**: The existing rarity column in the Reference_Docs (values such as
  `Plentiful`, `Common`, `Average`, `Scarce`, `Rare`, `Very Rare`, `Near Unique`)
  drawn from Rogue Trader rules (`Reference/RT-Weapons.md` and
  `Reference/RT-Equipment.md`), which MAY inform region weighting.
- **Equipment_Script**: The Lua data file `Scripts/Equipment.lua`, which defines the
  `equipment` table of item templates loaded and validated at game initialization.
- **Equipment_Entry**: A single item template table within Equipment_Script.
- **Region_Weight_Field**: The new optional field on an Equipment_Entry that maps a
  Region_Name (or the Universal_Tag) to a weighting value used for region-scoped
  selection, analogous to the `chance` table on an Enemy_Entry.
- **Untagged_Default**: The documented default Region weighting the Equipment_Loader
  applies to an Equipment_Entry whose Region_Weight_Field is ABSENT or fully malformed,
  namely `{ ImperialHuman = <default weight> }`. Untagged items are treated as part of
  the `ImperialHuman` Region and are therefore selectable in the `ImperialHuman` Region.
  This is distinct from the Universal_Tag: an entry that EXPLICITLY declares the
  Universal_Tag remains selectable in every Region, whereas an Untagged_Default entry is
  scoped to `ImperialHuman` only.
- **Equipment_Loader**: The C++ code path that reads the `equipment` table from
  Equipment_Script and constructs `EquipmentTemplate` records
  (`Source/Engine.cpp` equipment load loop; `Headers/Equipment.hpp`;
  `Source/Equipment.cpp`).
- **EquipmentTemplate**: The C++ record produced by the Equipment_Loader for each
  Equipment_Entry.
- **Equipment_Selection**: The C++ runtime path that chooses an EquipmentTemplate for
  a spawn or drop, currently `Engine::selectEquipmentByTier`.
- **Cumulative_Chance_Selection**: The selection algorithm used by NPC spawns that
  iterates candidate entries and selects by cumulative threshold within a single
  Region_Name column; a missing Region_Name key means the entry is never selected.
- **ItemTier**: The existing rarity tier on an Equipment_Entry (`common`, `uncommon`,
  `rare`), consumed by Equipment_Selection.

## Requirements

### Requirement 1: Shared Region Taxonomy

**User Story:** As a game designer, I want equipment regions to use the same
race/faction taxonomy as NPC spawns, so that a region's enemies and its lootable
equipment stay aligned.

#### Acceptance Criteria

1. THE Equipment_Script SHALL express a Region_Name using the same string taxonomy
   used by the `region-based-npc-spawns` feature, namely one of `Ork`, `Eldar`,
   `DarkEldar`, `Necron`, `Tau`, `Tyranid`, `Kroot`, `Chaos`, `ImperialHuman`, or
   `Servitor`.
2. THE Equipment_Script SHALL additionally accept the Universal_Tag value
   `"Universal"` as a Region_Annotation for faction-agnostic items.
3. THE Region_Annotation SHALL use the same spelling and casing for a given faction as
   the `chance` table keys in `Scripts/Enemies.lua` and the outputs of `regionForBiome`
   in `Source/WorldMap.cpp`, so a region's equipment and enemies resolve under the same
   Region_Name string.
4. WHERE a level resolves to a Biome_Region at runtime via `regionForBiome` or
   `resolveDefaultRegion`, THE Equipment_Selection SHALL use that resolved Region_Name
   as the selection key, consistent with the NPC-spawn region resolution.

### Requirement 2: Reference Weapon Region Annotation

**User Story:** As a game designer, I want every weapon in the weapons reference to
carry a region tag, so that the catalogue records which faction each weapon belongs to.

#### Acceptance Criteria

1. THE Reference_Docs weapons file `Reference/RT-Weapons.md` SHALL record a
   Region_Annotation for every weapon profile entry in the file.
2. THE Region_Annotation for a weapon entry SHALL be a single Region_Name from the
   valid set defined in Requirement 1, or the Universal_Tag.
3. WHERE a weapon entry is xenos-specific (for example an Ork, Eldar, Dark Eldar,
   Necron, Tau, Tyranid, or Kroot weapon), THE Reference_Docs SHALL record that weapon
   Region_Annotation as the corresponding xenos Region_Name.
4. WHERE a weapon entry is Imperial or human-standard issue, THE Reference_Docs SHALL
   record that weapon Region_Annotation as `ImperialHuman`.
5. WHERE a weapon entry is faction-agnostic or broadly available across factions
   (for example a common primitive or civilian weapon), THE Reference_Docs SHALL record
   that weapon Region_Annotation as the Universal_Tag and SHALL make the Universal
   classification explicit rather than implied.
6. WHERE a weapon entry supplies an Availability value drawn from Rogue Trader rules
   (`Reference/RT-Weapons.md`), THE Reference_Docs MAY use that Availability value to
   inform the Region_Annotation and any downstream region weighting, and SHALL retain
   the existing Availability value unchanged.
7. THE Reference_Docs SHALL introduce the Region_Annotation as an additive column or
   inline annotation and SHALL preserve every existing weapon field, source citation,
   and value already recorded in `Reference/RT-Weapons.md`.

### Requirement 3: Reference Equipment Region Annotation

**User Story:** As a game designer, I want every non-weapon equipment entry in the
equipment reference to carry a region tag, so that armour and gear availability aligns
with faction regions.

#### Acceptance Criteria

1. THE Reference_Docs equipment file `Reference/RT-Equipment.md` SHALL record a
   Region_Annotation for every equipment entry in the file, across all equipment
   categories present (armour, force fields, gear, tools, drugs and consumables, and
   cybernetics).
2. THE Region_Annotation for an equipment entry SHALL be a single Region_Name from the
   valid set defined in Requirement 1, or the Universal_Tag.
3. WHERE an equipment entry is xenos-specific, THE Reference_Docs SHALL record that
   entry Region_Annotation as the corresponding xenos Region_Name.
4. WHERE an equipment entry is Imperial or human-standard issue, THE Reference_Docs
   SHALL record that entry Region_Annotation as `ImperialHuman`.
5. WHERE an equipment entry is faction-agnostic or broadly available across factions,
   THE Reference_Docs SHALL record that entry Region_Annotation as the Universal_Tag and
   SHALL make the Universal classification explicit rather than implied.
6. WHERE an equipment entry supplies an Availability value drawn from Rogue Trader
   rules (`Reference/RT-Equipment.md`), THE Reference_Docs MAY use that Availability
   value to inform the Region_Annotation and any downstream region weighting, and SHALL
   retain the existing Availability value unchanged.
7. THE Reference_Docs SHALL introduce the Region_Annotation as an additive column or
   inline annotation and SHALL preserve every existing equipment field, source
   citation, and value already recorded in `Reference/RT-Equipment.md`.

### Requirement 4: Consistent Faction Assignment Rules

**User Story:** As a game designer, I want a single consistent rule for deciding a
faction for each reference entry, so that annotations are unambiguous and repeatable.

#### Acceptance Criteria

1. THE Reference_Docs SHALL assign exactly one Region_Annotation value to each entry,
   such that no entry carries more than one Region_Annotation.
2. IF an entry is unambiguously produced or used by a single xenos faction, THEN THE
   Reference_Docs SHALL assign that entry the corresponding xenos Region_Name.
3. IF an entry is unambiguously Imperial or human-standard issue and is not xenos, THEN
   THE Reference_Docs SHALL assign that entry the `ImperialHuman` Region_Name.
4. IF an entry is not tied to any single faction, or is a primitive or civilian item
   available across many cultures, THEN THE Reference_Docs SHALL assign that entry the
   Universal_Tag.
5. WHERE the faction for an entry cannot be determined from the entry name, existing
   fields, or the Rogue Trader source material in `Reference/`, THE Reference_Docs SHALL
   assign the Universal_Tag as the documented default and SHALL note that the entry is a
   default-assigned Universal classification.

### Requirement 5: Optional Region Weighting Field in Equipment Data

**User Story:** As a game designer, I want each equipment entry to declare an optional
per-region weighting, so that region-scoped equipment selection can be data-driven.

#### Acceptance Criteria

1. THE Equipment_Script SHALL allow each Equipment_Entry to declare an optional
   Region_Weight_Field that maps a Region_Name (or the Universal_Tag) to a
   non-negative integer weighting value.
2. THE Region_Weight_Field SHALL be represented as a keyed table so that additional
   Region_Name keys can be added to an Equipment_Entry without changing the
   Equipment_Loader interface or the Equipment_Selection signature.
3. WHERE an Equipment_Entry omits the Region_Weight_Field, THE Equipment_Loader SHALL
   apply a documented default of `{ ImperialHuman = <default weight> }`, so that the
   resulting EquipmentTemplate is selectable in the `ImperialHuman` Region under the
   existing tier-based selection behaviour, treating untagged items as part of the
   ImperialHuman Region.
4. THE Equipment_Script SHALL populate the Region_Weight_Field on the Equipment_Entries
   using the Region_Name that matches the corresponding Reference_Docs Region_Annotation
   for the same item, and SHALL use the Universal_Tag for faction-agnostic entries.
5. WHERE an Equipment_Entry carries the Universal_Tag in its Region_Weight_Field, THE
   Equipment_Selection SHALL treat that entry as selectable in every Region.

### Requirement 6: Backward-Compatible Loading of Equipment Data

**User Story:** As a developer, I want the new region field to be optional so existing
equipment entries keep loading, so that adding regions does not break the game.

#### Acceptance Criteria

1. WHEN the Equipment_Loader reads an Equipment_Entry that omits the
   Region_Weight_Field, THE Equipment_Loader SHALL load that entry successfully and
   SHALL NOT reject or skip the entry for the absence of the field.
2. THE Equipment_Loader SHALL preserve loading and validation of all existing required
   fields (`name`, `glyph`, `color`, `slot`, `weight`) and existing optional fields
   (`value`, `power`, `defense`, `maxHp`, `skill`, `tier`, `melee`, `ranged`,
   `armourLocations`) exactly as before the Region_Weight_Field is added.
3. IF the Region_Weight_Field is present but fully malformed (for example not a table,
   or containing a non-integer or negative weight), THEN THE Equipment_Loader SHALL apply
   the documented default of `{ ImperialHuman = <default weight> }` for that entry,
   making it selectable in the `ImperialHuman` Region, and SHALL continue loading without
   aborting the remaining entries.
4. IF the Region_Weight_Field references a Region_Name outside the valid set defined in
   Requirement 1 and outside the Universal_Tag, THEN THE Equipment_Loader SHALL ignore
   that unrecognized key for selection purposes and SHALL continue loading the entry.
5. FOR ALL Equipment_Entries present before this feature, loading the current
   `Scripts/Equipment.lua` under the updated Equipment_Loader SHALL produce the same set
   of EquipmentTemplates, with the same required and existing-optional field values, as
   loading it under the pre-feature Equipment_Loader, EXCEPT that each such pre-feature
   entry now carries the `{ ImperialHuman = <default weight> }` default Region weighting
   (backward-compatibility property).

### Requirement 7: Region-Weighted Equipment Selection at Runtime

**User Story:** As a player, I want equipment that spawns or drops to match the region
I am in, so that loot feels consistent with the enemies and environment.

#### Acceptance Criteria

1. WHEN Equipment_Selection chooses an EquipmentTemplate for a spawn or drop on a level,
   THE Equipment_Selection SHALL use that level resolved Region_Name as the selection
   key, obtained from the same region resolution used by NPC spawns (`regionForBiome`
   or `resolveDefaultRegion`).
2. THE Equipment_Selection SHALL restrict candidate EquipmentTemplates for a given
   Region_Name to those whose Region_Weight_Field contains that Region_Name key or the
   Universal_Tag, mirroring the NPC-spawn rule that an entry without the requested key is
   not selected for that Region.
3. WHEN multiple EquipmentTemplates are selectable for the requested Region_Name, THE
   Equipment_Selection SHALL choose among them using a cumulative-weight selection
   consistent with the Cumulative_Chance_Selection model used for NPC spawns.
4. IF no EquipmentTemplate is selectable for the requested Region_Name after including
   Universal_Tag entries, THEN THE Equipment_Selection SHALL select no region-scoped
   EquipmentTemplate for that request and SHALL fall back to the documented default
   behaviour without aborting.
5. WHERE the existing tier-weighted selection (`Engine::selectEquipmentByTier`) applies,
   THE Equipment_Selection SHALL apply the Region_Name filter in combination with the
   existing ItemTier filter, such that a selected EquipmentTemplate satisfies both the
   requested Region_Name (or Universal_Tag) and the selected ItemTier.
6. WHILE a level Region_Name equals the Default_Region because no biome context was
   available, THE Equipment_Selection SHALL select equipment scoped to that
   Default_Region using the same rules as any other Region_Name.

### Requirement 8: Extensibility and Alignment Guarantees

**User Story:** As a developer, I want the region equipment data structure to avoid
painting future iterations into a corner, so that later features extend it without a
breaking redesign.

#### Acceptance Criteria

1. THE Equipment_Script SHALL represent per-region weighting as a keyed table so that a
   Region_Name can later differ from a race name without changing the Equipment_Loader
   interface.
2. THE Equipment_Selection SHALL receive the Region as a Region_Name string so that the
   C++/Lua selection boundary does not change when new Region_Names are added.
3. THE feature SHALL treat the Region_Name derivation point (`regionForBiome` in
   `Source/WorldMap.cpp`) as the single shared source of the active Region_Name for both
   NPC spawns and equipment selection, so equipment and enemies for a level always
   resolve under the same Region_Name.
4. THE feature SHALL keep the Reference_Docs Region_Annotation, the Equipment_Script
   Region_Weight_Field, and the runtime Region_Name taxonomy mutually consistent, such
   that a faction named in the Reference_Docs maps to the same Region_Name string in
   both `Scripts/Equipment.lua` and the runtime selection path.
