# Requirements Document

## Introduction

This feature populates the game's enemy data (`Scripts/Enemies.lua`) with every Troop-tier NPC drawn from `Reference/RT-Bestiary.md`, and wires those NPCs into the shipped region-based spawn system so they appear in play. Spawn wiring is organised by faction: each faction is designated its own Faction_Region, a whole level can be assigned a Faction_Region, and each NPC declares a cumulative spawn chance in its faction's region column. The existing `spawnEnemy(roll, x, y, region)` signature and the cumulative-selection algorithm are unchanged.

All NPC stat blocks are derived from `Reference/RT-Bestiary.md`, cited per the project's Rogue Trader mechanics fallback (see `Reference/RT-Bestiary.md`, authoritative). The in-scope roster is every heading tagged `(Troop)` in the reference — 69 entries in total: 56 entries across Chapter IV sections IV.1 (Renegades, Heretics, & Mutants), IV.3 (Daemons of Chaos), IV.4 (Craftworld Eldar), IV.5 (Harlequins & Dark Eldar), IV.6 (Necrons), IV.7 (Orks), IV.8 (Tau Empire Infantry, Kroot, Vespid), and IV.9 (Tyranids); plus 13 Troop-tagged Chapter V core-rulebook profiles (Imperial Humans, Servitors, Xenos, and the Warp Predator). The six Colonist variants (Adept, Bloodskinner, Entertainer, Hired Gun, Scum, Voidfarer) are additionally built as modifications of the in-scope Colonist base template.

This feature is data-and-mapping only: it uses the existing enemy schema, the existing equipment schema, and the existing region/spawn architecture. It does not change the C++/Lua spawn boundary or the cumulative-selection algorithm.

### In Scope

- Adding an Enemy_Entry to `Scripts/Enemies.lua` for every heading tagged `(Troop)` in `Reference/RT-Bestiary.md` (69 entries), using the existing enemy schema.
- Building the six Colonist_Variants (Adept, Bloodskinner, Entertainer, Hired Gun, Scum, Voidfarer) as modifications of the Colonist base template, per `Reference/RT-Bestiary.md` §5.1.
- Deriving each in-scope NPC's characteristics, wounds, skills, talents, traits, and equipment from its cited Troop_Profile in `Reference/RT-Bestiary.md`.
- Assigning each faction its own Faction_Region column (`Chaos`, `Eldar`, `DarkEldar`, `Necron`, `Ork`, `Tau`, `Tyranid`, `ImperialHuman`, `Servitor`, `Warp`), assigning every Troop Enemy_Entry to exactly one Faction_Region, and declaring a per-region cumulative chance table on each Enemy_Entry keyed by its Faction_Region.
- Reconciling the existing four legacy Ork entries (Gretchin, Ork, Shoota Boy, Nob) with the bestiary's Ork Troop set so the `Ork` Faction_Region is rebuilt from the bestiary Ork Troops.
- Adding new weapon and armour entries to `Scripts/Equipment.lua` so in-scope NPCs' equipment lists resolve to defined gear.
- Handling Unnatural characteristic multipliers, em-dash characteristics, and machine/daemonic/fear/flyer/size traits present in the reference profiles.

### Out of Scope (Future Expansion)

- Elite-tier and Master-tier NPCs. Only Troop-tagged profiles and the six Colonist variants are in scope; Elite and Master profiles are deferred.
- Vehicles and war machines, which live in `Reference/vehicles.md`, not the bestiary creature set.
- Full simulation of psychic powers, Dark Pact daemon summoning, bio-morph selection, and other narrative abilities beyond their representation as traits or talents strings.
- Weighted multi-faction regions and per-tile spawn granularity. A whole level is designated a single Faction_Region; the policy that assigns a specific level to a specific Faction_Region MAY be deferred.
- Changes to the C++/Lua spawn call boundary, the `spawnEnemy` signature, or the cumulative-selection algorithm.

## Glossary

- **Bestiary**: The complete set of Enemy_Entry records defined in the Enemy_Script.
- **Enemy_Script**: The Lua script `Scripts/Enemies.lua`, which defines enemy templates and the `spawnEnemy(roll, x, y, region)` function.
- **Enemy_Entry**: A single enemy definition table within the Enemy_Script. Fields: `glyph`, `name`, `color`, `hp`, `defense`, `corpse`, `xp`, `power`, `skill`, the nine Characteristics (`ws`, `bs`, `s`, `t`, `ag`, `int`, `per`, `wp`, `fel`), the per-region `chance` table, and the optional fields `equipment`, `dropChance`, `equipTier`, `skills`, `talents`, and `traits`.
- **Equipment_Script**: The Lua script `Scripts/Equipment.lua`, which defines equippable item templates (weapons and armour) loaded and validated at initialization.
- **Reference_Profile**: A single named stat block in `Reference/RT-Bestiary.md`.
- **Troop_Profile**: A Reference_Profile whose heading is tagged `(Troop)` in `Reference/RT-Bestiary.md`.
- **Troop_Tier**: The Rogue Trader adversary power band whose profiles are tagged `(Troop)` in `Reference/RT-Bestiary.md`, distinct from the Elite and Master tiers.
- **Colonist_Variant**: An Imperial Human Minor NPC (Adept, Bloodskinner, Entertainer, Hired Gun, Scum, Voidfarer) built on the Colonist base template with the modifications listed in its Reference_Profile in `Reference/RT-Bestiary.md` §5.1.
- **Characteristic**: One of the nine Rogue Trader characteristics stored as an integer 0–100 on an Enemy_Entry: Weapon Skill (`ws`), Ballistic Skill (`bs`), Strength (`s`), Toughness (`t`), Agility (`ag`), Intelligence (`int`), Perception (`per`), Willpower (`wp`), Fellowship (`fel`).
- **Unnatural_Multiplier**: A multiplier applied to a Characteristic, shown in parentheses in a Reference_Profile (for example `S 40 (8)`), represented in the Bestiary as a trait such as `Unnatural Strength (x2)`.
- **Faction**: One of the ten adversary groupings this feature spawns: Chaos, Craftworld Eldar, Dark Eldar, Necrons, Orks, the Tau Empire, Tyranids, Imperial Humans, Servitors, and Denizens of the Warp.
- **Faction_Region**: A Region_Name that names a single Faction. The ten Faction_Region values are `Chaos`, `Eldar`, `DarkEldar`, `Necron`, `Ork`, `Tau`, `Tyranid`, `ImperialHuman`, `Servitor`, and `Warp`. Each Faction is designated its own Faction_Region, and a whole level can be designated a Faction_Region.
- **Region_Name**: The string identifier of a Faction_Region, used as a key in an Enemy_Entry's per-region chance table.
- **Race_Group**: The set of Enemy_Entry records belonging to one Faction that share a Faction_Region column.
- **Region_Chance_Table**: The per-region `chance` table on an Enemy_Entry that maps a Faction_Region to a cumulative chance integer in the range 0 to 100 inclusive.
- **Cumulative_Chance_Selection**: The existing selection algorithm in `spawnEnemy` that iterates enemy entries in declaration order and selects the first entry whose region-scoped cumulative threshold is strictly greater than the roll.
- **Equipment_Reference**: A string in an Enemy_Entry `equipment` list that names an entry in the Equipment_Script.
- **Spawn_System**: The C++ code path (`Map::addMonster`) that calls `spawnEnemy` with a roll and the active level's Faction_Region.

## Requirements

### Requirement 1: Troop Roster Completeness

**User Story:** As a game designer, I want every Troop-tier NPC defined from the reference profiles, so that the full Troop-tier bestiary appears in the game.

#### Acceptance Criteria

1. THE Enemy_Script SHALL define exactly one Enemy_Entry for each of the 69 Troop_Profiles cited in `Reference/RT-Bestiary.md`, and each such Enemy_Entry SHALL use the nine Characteristics cited for its Troop_Profile.
2. THE Enemy_Script SHALL define one Enemy_Entry each for the six Chaos and Renegade Troop_Profiles cited in `Reference/RT-Bestiary.md` section IV.1: Chaos Cultist, Mutant Fighter, Dark Disciple, Mutant Wretch, Renegade Soldier, and Renegade Veteran.
3. THE Enemy_Script SHALL define one Enemy_Entry each for the eight Daemon Troop_Profiles cited in `Reference/RT-Bestiary.md` section IV.3: Nurgling, Brimstone Horror, Blue Horror, Screamer of Tzeentch, Ebon Gheist, Chaos Fury, Nether Spawn, and Gibbering Malingerer.
4. THE Enemy_Script SHALL define one Enemy_Entry each for the two Craftworld Eldar Troop_Profiles cited in `Reference/RT-Bestiary.md` section IV.4: Eldar Guardian and Eldar Ranger.
5. THE Enemy_Script SHALL define one Enemy_Entry each for the five Harlequin and Dark Eldar Troop_Profiles cited in `Reference/RT-Bestiary.md` section IV.5: Wych, Razorwing Flock, Ariadne Helspider, Kabalite, and Hellion.
6. THE Enemy_Script SHALL define one Enemy_Entry each for the three Necron Troop_Profiles cited in `Reference/RT-Bestiary.md` section IV.6: Necron Warrior, Canoptek Scarab, and Canoptek Scarab Swarm.
7. THE Enemy_Script SHALL define one Enemy_Entry each for the thirteen Ork Troop_Profiles cited in `Reference/RT-Bestiary.md` section IV.7: Snotling, Snotling Mob, Gretchin, Ork Boy, Burna Boy, Tankbusta, Loota, Storm Boy, Kommando, 'Ard Boy, Skar Boy, Runtherd, and Attack Squig.
8. THE Enemy_Script SHALL define one Enemy_Entry each for the seven Tau, Kroot, and Vespid Troop_Profiles cited in `Reference/RT-Bestiary.md` section IV.8: Fire Warrior, Pathfinder, Water Caste Diplomat, Earth Caste Engineer, Kroot Carnivore, Kroot Hound, and Vespid Stingwing.
9. THE Enemy_Script SHALL define one Enemy_Entry each for the twelve Tyranid Troop_Profiles cited in `Reference/RT-Bestiary.md` section IV.9: Late Generation Hybrid, Early Generation Hybrid, Ripper, Ripper Swarm, Hormagaunt, Termagant, Mucolid Spore, Mieotic Spore, Biovore, Pyrovore, Gargoyle, and Mycetic Spore.
10. THE Enemy_Script SHALL define one Enemy_Entry each for the thirteen Chapter V Troop_Profiles cited in `Reference/RT-Bestiary.md` sections 5.1 through 5.4: Colonist, Mutant Outcast, Oathsworn Bodyguard, Renegade, Warp Witch, Battle Servitor (Charron-Pattern), Grapplehawk (Falax-Pattern), Servitor Drone, Servo Skull, Eldar Corsair, Ork Freebooter, Kroot Mercenary, and Warp Predator (Ebon Geist).
11. THE Enemy_Script SHALL give each Enemy_Entry a unique `name` value across the entire Bestiary.
12. THE Enemy_Script SHALL give each Enemy_Entry a visually distinct appearance from every other Enemy_Entry, such that no two Enemy_Entries share the same combination of `glyph` and `color`; each `color` string SHALL name a color defined in `Headers/Colors.hpp` (resolvable by `colorFromName`).

### Requirement 2: Characteristic Mapping

**User Story:** As a developer, I want reference characteristics mapped consistently into the integer schema, so that stat blocks translate faithfully.

#### Acceptance Criteria

1. THE Enemy_Script SHALL assign each Characteristic field (`ws`, `bs`, `s`, `t`, `ag`, `int`, `per`, `wp`, `fel`) an integer in the range 0 to 100 inclusive, copied as the base value cited in the Reference_Profile.
2. WHERE a Reference_Profile lists a Characteristic with no value (shown as an em dash), THE Enemy_Script SHALL set that Characteristic to 0 in the corresponding Enemy_Entry.
3. WHERE a Reference_Profile lists an Unnatural_Multiplier on a Characteristic, THE Enemy_Script SHALL record that multiplier as a trait in the Enemy_Entry `traits` field using the form `Unnatural <Characteristic> (x<N>)`.
4. THE Enemy_Script SHALL store each Characteristic as its base value from the Reference_Profile, excluding the Unnatural_Multiplier from the stored integer.

### Requirement 3: Derived Fields

**User Story:** As a game designer, I want each NPC's wounds, skills, talents, and traits derived from its profile, so that the entries reflect the reference stat blocks.

#### Acceptance Criteria

1. THE Enemy_Script SHALL set each Enemy_Entry `hp` value equal to the Wounds value cited for its Troop_Profile in `Reference/RT-Bestiary.md`.
2. THE Enemy_Script SHALL populate the `skills`, `talents`, and `traits` fields of each Enemy_Entry from the corresponding Troop_Profile in `Reference/RT-Bestiary.md`.
3. WHERE a Reference_Profile lists a Machine, Daemonic, Fear, Flyer, or Size trait, THE Enemy_Script SHALL record each such trait as a string entry in the Enemy_Entry `traits` field.
4. IF a Troop_Profile cites no entries for a given field (`skills`, `talents`, `traits`, or `equipment`), THEN THE Enemy_Script SHALL set that field of the corresponding Enemy_Entry to an empty collection rather than omitting the field.
5. IF a Troop_Profile cites a Wounds value of zero or cites the value as omitted, THEN THE Enemy_Script SHALL set that Enemy_Entry `hp` to 1 or greater and SHALL document the source-omitted Wounds value in a comment.

### Requirement 4: Colonist Template and Variants

**User Story:** As a game designer, I want the six Colonist variants built from the Colonist base, so that Imperial Human minor NPCs share a consistent template.

#### Acceptance Criteria

1. THE Enemy_Script SHALL define exactly one Enemy_Entry for the Colonist base template with the nine Characteristics WS 25, BS 20, S 30, T 30, Ag 30, Int 25, Per 25, WP 25, Fel 30 as cited in `Reference/RT-Bestiary.md` §5.1.
2. THE Enemy_Script SHALL define exactly one Enemy_Entry for each of the six Colonist_Variants (Adept, Bloodskinner, Entertainer, Hired Gun, Scum, Voidfarer), where each variant's Characteristics equal the Colonist base Characteristics with the cited overrides applied (Adept: Int 30; Bloodskinner: WS 35, BS 30, Per 35; Entertainer: Fel 35; Hired Gun: BS 35; Scum: WS 30, Per 30; Voidfarer: S 38, T 38) as cited in `Reference/RT-Bestiary.md` §5.1, and every non-overridden Characteristic SHALL retain the Colonist base value.
3. THE Enemy_Script SHALL set each Colonist_Variant Enemy_Entry `hp` equal to the Colonist base Wounds value (9), except the Hired Gun Enemy_Entry, whose `hp` SHALL equal the cited Wounds override (12), as cited in `Reference/RT-Bestiary.md` §5.1.
4. THE Enemy_Script SHALL populate each Colonist_Variant Enemy_Entry `skills`, `talents`, and `traits` fields with the Colonist base entries plus the additional entries cited for that variant in `Reference/RT-Bestiary.md` §5.1.

### Requirement 5: Enemy Schema Compliance

**User Story:** As a developer, I want every new NPC to use the existing enemy schema, so that the C++ loader and spawn path accept the entries without code changes.

#### Acceptance Criteria

1. THE Enemy_Script SHALL define each new Enemy_Entry with the fields `glyph`, `name`, `color`, `hp`, `defense`, `corpse`, `xp`, `power`, `skill`, `ws`, `bs`, `s`, `t`, `ag`, `int`, `per`, `wp`, `fel`, and a per-region `chance` table.
2. WHERE an Enemy_Entry carries equipment, THE Enemy_Script SHALL populate the `equipment` list, the `dropChance` value, and the `equipTier` table using the same field forms as the existing Ork enemy set.
3. WHERE an Enemy_Entry declares `skills`, `talents`, or `traits`, THE Enemy_Script SHALL use the same field forms as the existing Ork enemy set: `skills` as a name-to-rank table, `talents` and `traits` as string lists.
4. THE Enemy_Script SHALL give every Enemy_Entry a `(glyph, color)` pair that is distinct from that of every other Enemy_Entry across the entire Bestiary, so that no two NPCs render with both the same glyph and the same color, using only `color` names defined in `Headers/Colors.hpp`.

### Requirement 6: Faction Spawn Regions

**User Story:** As a game designer, I want each faction assigned its own spawn region, so that a level designated a faction-region spawns only that faction's NPCs.

#### Acceptance Criteria

1. THE Enemy_Script SHALL define exactly ten Faction_Region columns, one per Faction, with the Region_Name values `Chaos`, `Eldar`, `DarkEldar`, `Necron`, `Ork`, `Tau`, `Tyranid`, `ImperialHuman`, `Servitor`, and `Warp`.
2. THE Enemy_Script SHALL assign every Troop Enemy_Entry to exactly one Faction_Region, mapping each Enemy_Entry to its Faction as follows: sections IV.1 and IV.3 to `Chaos`; section IV.4 and the Eldar Corsair (§5.3) to `Eldar`; section IV.5 to `DarkEldar`; section IV.6 to `Necron`; section IV.7 and the Ork Freebooter (§5.3) to `Ork`; section IV.8 and the Kroot Mercenary (§5.3) to `Tau`; section IV.9 to `Tyranid`; the Imperial Human profiles including the Colonist and its variants (§5.1) to `ImperialHuman`; the Servitor profiles (§5.2) to `Servitor`; and the Warp Predator (§5.4) to `Warp`.
3. THE Enemy_Script SHALL make the Faction-to-Faction_Region mapping the single point at which a level or region is designated to a Faction, so that designating a level a Faction_Region determines which Faction spawns there.
4. WHEN Cumulative_Chance_Selection requests a spawn for a Faction_Region, THE Enemy_Script SHALL select only Enemy_Entries whose Region_Chance_Table contains that Faction_Region key.
5. THE Spawn_System MAY designate a whole level as a single Faction_Region, and the mechanism that assigns a specific level to a specific Faction_Region MAY be deferred without changing the `spawnEnemy` signature.
6. IF Cumulative_Chance_Selection requests a spawn for a Region_Name that is not a defined Faction_Region, THEN THE Spawn_System SHALL supply the fallback Region_Name `Ork`, which is a defined Faction_Region.

### Requirement 7: Per-Region Cumulative Chance Tables

**User Story:** As a game designer, I want each faction column to be a controlled cumulative distribution, so that spawns within a faction are deterministic and every faction can reach 100.

#### Acceptance Criteria

1. THE Enemy_Script SHALL declare on each in-scope Enemy_Entry a Region_Chance_Table that maps its Faction_Region key to a cumulative chance integer in the range 0 to 100 inclusive.
2. WHERE a Faction_Region column contains more than one Enemy_Entry, THE Enemy_Script SHALL assign the cumulative chance values within that column in strictly ascending order in declaration order, with each entry's value greater than the previous entry's value.
3. THE Enemy_Script SHALL set the final Enemy_Entry's cumulative chance value in each Faction_Region column equal to 100.
4. WHEN Cumulative_Chance_Selection requests a spawn with a roll integer in the range 0 to 100 inclusive for a Faction_Region, THE Enemy_Script SHALL select the first Enemy_Entry in that Faction_Region column, in declaration order, whose cumulative chance value is strictly greater than the roll.
5. IF Cumulative_Chance_Selection requests a spawn for a Faction_Region that has no Enemy_Entry, or the roll is not strictly less than any cumulative chance value in that column, THEN THE Enemy_Script SHALL select no Enemy_Entry and SHALL leave the target tile unoccupied.

### Requirement 8: Ork Reconciliation

**User Story:** As a maintainer, I want the four legacy Ork entries reconciled with the bestiary Ork Troop set, so that the Ork faction-region reflects the bestiary rather than stale duplicates.

#### Acceptance Criteria

1. THE Enemy_Script SHALL rebuild the `Ork` Faction_Region column from the bestiary Ork Troop_Profiles cited in `Reference/RT-Bestiary.md` section IV.7, replacing the legacy four-entry flat distribution (Gretchin, Ork, Shoota Boy, Nob).
2. THE Enemy_Script SHALL align each retained or renamed Ork Enemy_Entry's Characteristics, `hp`, `skills`, `talents`, and `traits` with the corresponding Ork Troop_Profile cited in `Reference/RT-Bestiary.md` section IV.7.
3. IF a legacy Ork entry name (Ork, Shoota Boy, Nob) has no matching bestiary Ork Troop_Profile name, THEN THE Enemy_Script SHALL map that legacy entry to the closest bestiary Ork Troop_Profile or remove the stale entry, so that no stale duplicate Ork Enemy_Entry remains.
4. THE Enemy_Script SHALL declare the `Ork` Faction_Region column with cumulative chance values in strictly ascending order and a final value of 100 across the bestiary Ork Troop set and the Ork Freebooter.

### Requirement 9: New Equipment Definitions

**User Story:** As a developer, I want the weapons and armour referenced by new NPCs to exist in the equipment script, so that equipped NPCs resolve their gear.

#### Acceptance Criteria

1. FOR EACH Equipment_Reference that appears in a new Enemy_Entry `equipment` list, THE Equipment_Script SHALL define exactly one entry whose `name` is byte-for-byte string-equal to that Equipment_Reference.
2. THE Equipment_Script SHALL define each new weapon and armour entry with a non-empty `name` string, a single-character `glyph`, a `color` string, a `slot` value drawn from the existing Equipment_Script slot values (`weapon`, `body`, `head`, `offhand`), and a `weight` value of zero or greater, following the existing Equipment_Script schema.
3. WHERE a new weapon entry represents a ranged weapon in `Reference/RT-Bestiary.md`, THE Equipment_Script SHALL populate the `ranged` table (`damageDice`, `penetration`, `range`, `rateOfFire`, `clipSize`, `reloadTime`) with the values cited in that weapon's profile in `Reference/RT-Bestiary.md`.
4. WHERE a new weapon entry represents a melee weapon in `Reference/RT-Bestiary.md`, THE Equipment_Script SHALL populate the `melee` table (`damageDice`, `penetration`, `qualities`) with the values cited in that weapon's profile in `Reference/RT-Bestiary.md`.
5. WHERE a new armour entry represents body protection in `Reference/RT-Bestiary.md`, THE Equipment_Script SHALL populate the `armourLocations` table (`head`, `body`, `leftArm`, `rightArm`, `leftLeg`, `rightLeg`) with the per-location Armour Points cited in `Reference/RT-Bestiary.md`, using 0 for any location the profile does not cite.
6. IF an Equipment_Reference in a new Enemy_Entry `equipment` list has no citable weapon or armour profile in `Reference/RT-Bestiary.md`, THEN THE Equipment_Script SHALL either omit that reference from the Enemy_Entry `equipment` list or define the entry so the C++ loader accepts it.
7. THE Equipment_Script SHALL retain every existing equipment entry with its `name` and all field values unchanged.

### Requirement 10: Preserving Existing Behaviour

**User Story:** As a player, I want the game to keep working after the bestiary expansion, so that spawning and loading remain stable.

#### Acceptance Criteria

1. THE Enemy_Script SHALL retain the `spawnEnemy(roll, x, y, region)` function signature unchanged.
2. WHEN the Enemy_Script selects an Enemy_Entry for a Faction_Region, THE Enemy_Script SHALL call the injected `addActor` function with the selected Enemy_Entry.
3. THE Enemy_Script SHALL leave the Cumulative_Chance_Selection algorithm in `spawnEnemy` unchanged in behaviour.
4. THE Region_Name values SHALL remain strings decoupled from the C++/Lua spawn call boundary, so that a Faction_Region can differ from a race name without changing the spawn function signature.
5. IF the Enemy_Script fails to load or execute, THEN THE Spawn_System SHALL fall back to the existing hard-coded Ork spawn behaviour.

### Requirement 11: Deferred Scope Documentation

**User Story:** As a maintainer, I want deferred scope flagged as future work, so that the gap is tracked rather than silently omitted.

#### Acceptance Criteria

1. THE Bestiary SHALL omit Elite-tier and Master-tier Enemy_Entry records in this feature, because only Troop-tagged profiles and the six Colonist variants are in scope.
2. THE Enemy_Script SHALL document, in a comment, that Elite-tier and Master-tier NPCs, weighted multi-faction regions, and per-tile spawn granularity are deferred, and that vehicles live in `Reference/vehicles.md` and are out of scope.
3. WHERE Elite-tier or Master-tier profiles are added to the Bestiary in the future, THE Bestiary SHALL be extendable to include them without changing the enemy schema or the `spawnEnemy` function signature.
