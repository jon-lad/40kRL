# Design Document

## Overview

This feature layers **region (faction) data** onto equipment so lootable and spawnable
gear aligns with the race/faction that inhabits a map region. It reuses the exact
Region_Name taxonomy, the region-resolution point, and the cumulative-selection model
already established by `region-based-npc-spawns` / `bestiary-npcs`, so a level's enemies
and its loot resolve under the same Region_Name string.

The work spans three surfaces, each with its own verification approach:

1. **Reference-doc annotation** (`Reference/RT-Weapons.md`, `Reference/RT-Equipment.md`) —
   an additive Region/Faction column on every entry. Source-of-truth data; verified by
   **structured review**, not automated tests.
2. **Equipment.lua schema + C++ loader** — a new optional keyed `region` field on each
   `Equipment_Entry`, parsed by the existing sol2 load loop in `Source/Engine.cpp` into a
   new container on `EquipmentTemplate`. Verified by **automated unit + property tests**.
3. **Runtime region-weighted selection** — `Engine::selectEquipmentByTier` gains a
   Region_Name parameter and applies a cumulative-weight region filter, combined with the
   existing tier filter. Verified by **automated unit + property tests**.

The design deliberately mirrors the NPC-spawn pattern:

- NPC `chance = { Ork = 40 }` → Equipment `region = { Ork = 40 }` (single- or multi-key).
- NPC `spawnEnemy(roll, x, y, region)` reads `e.chance[region]` → equipment selection
  reads `template.regionWeights[region]` (plus `Universal`).
- NPC region comes from `Map::getRegionName()` (resolved via `regionForBiome` /
  `resolveDefaultRegion`) → equipment selection consumes the same `Map::getRegionName()`.

## Architecture

```mermaid
graph TD
    subgraph Reference["Surface 1: Reference Docs (review-verified)"]
        RW[RT-Weapons.md<br/>+ Region column]
        RE[RT-Equipment.md<br/>+ Region column]
    end

    subgraph Data["Surface 2: Data + Loader (test-verified)"]
        LUA[Scripts/Equipment.lua<br/>optional region = { ... } field]
        LOOP[Equipment load loop<br/>Source/Engine.cpp<br/>sol2 get_or parsing]
        TMPL[EquipmentTemplate<br/>+ RegionWeights regionWeights]
    end

    subgraph Runtime["Surface 3: Selection (test-verified)"]
        SEL[Engine::selectEquipmentByTier<br/>slot, weights, regionName]
        REG[Map::getRegionName]
        BIOME[regionForBiome / resolveDefaultRegion<br/>Source/WorldMap.cpp]
    end

    RW -.informs annotation.-> LUA
    RE -.informs annotation.-> LUA
    LUA --> LOOP --> TMPL
    BIOME --> REG --> SEL
    TMPL --> SEL
    SEL --> ITEM[Selected EquipmentTemplate<br/>Map::addItem / enemy loot]
```

Region_Name flows from a single shared source (`regionForBiome` /
`resolveDefaultRegion`, already stored on `Map::regionName`). Equipment selection reads
the same value NPC spawns read, guaranteeing per-level alignment (Requirements 1.3, 1.4,
7.1, 8.3, 8.4).

## Components and Interfaces

### Surface 1 — Reference-Doc Annotation

Both reference files are Markdown tables. The annotation is introduced as an **additive
column** appended to each table (never replacing or reordering existing columns), so every
existing field, Availability value, citation, and footnote marker is preserved
(Requirements 2.7, 3.7).

**Column header:** `Region` (added as the last column of every profile/equipment table).

**Column value:** exactly one Region_Name from the taxonomy, or the Universal tag.

**Taxonomy values** (identical spelling/casing to `Scripts/Enemies.lua` `chance` keys and
`regionForBiome` outputs — Requirements 1.1, 1.3, 8.4):

| Region_Name    | Meaning |
|----------------|---------|
| `Ork`          | Ork-produced/used gear (choppa, slugga, shoota, kustom weapons) |
| `Eldar`        | Craftworld Eldar gear (shuriken weapons, wraithbone, mesh armour) |
| `DarkEldar`    | Dark Eldar / Drukhari gear (splinter weapons, agoniser, ghostplate) |
| `Necron`       | Necron gear (gauss weapons, living metal) |
| `Tau`          | T'au gear (pulse weapons, ion, fusion) |
| `Tyranid`      | Tyranid bio-weapons/bio-armour (fleshborer, chitin) |
| `Kroot`        | Kroot gear (kroot rifle, kroot weapons) |
| `Chaos`        | Chaos/Renegade-specific gear and daemon weapons |
| `ImperialHuman`| Imperial / Adeptus Mechanicus / human-standard issue |
| `Servitor`     | Servitor-specific integral weapons/plating |
| `Universal`    | The Universal_Tag — faction-agnostic / primitive / civilian / broadly available |

**Faction-assignment decision rules** (Requirement 4; single value per entry — Req 4.1):

1. **Xenos-specific → that faction.** If the entry is unambiguously produced or fielded by
   one xenos faction, assign the corresponding xenos Region_Name (Reqs 2.3, 3.3, 4.2). The
   entry's name or its source section is the primary signal (e.g., "Shuriken Catapult" →
   `Eldar`; "Splinter Rifle" → `DarkEldar`; "Gauss Flayer" → `Necron`; "Pulse Rifle" →
   `Tau`; "Kroot Rifle" → `Kroot`; a fleshborer → `Tyranid`).
2. **Imperial / human-standard → `ImperialHuman`.** If unambiguously Imperial or
   human-standard issue and not xenos (lasguns, bolt weapons, flak/carapace/power armour,
   auspex, most Mechanicus tech), assign `ImperialHuman` (Reqs 2.4, 3.4, 4.3).
3. **Faction-agnostic / primitive / civilian → `Universal`.** If not tied to a single
   faction (knives, clubs, swords, stub revolvers, ropes, lamps, rations, common civilian
   tools), assign the Universal_Tag and make it **explicit** rather than implied
   (Reqs 2.5, 3.5, 4.4).
4. **Undeterminable → `Universal` (documented default).** If the faction cannot be
   determined from the name, existing fields, or the RT source material in `Reference/`,
   assign `Universal` **and add a footnote** noting it is a default-assigned Universal
   classification (Req 4.5). Footnote convention: reuse the existing `†` marker style, e.g.
   `Universal †` with `† Region default-assigned (faction undeterminable from source).`

**Force fields & cybernetics handling** (`RT-Equipment.md` categories — Req 3.1): force
fields (conversion field, refractor field, displacer field) and cybernetics (bionic limbs,
cortex implants, MIU, respirators) are overwhelmingly Imperial / Adeptus Mechanicus
technology and are annotated `ImperialHuman` by rule 2. Exceptions annotated by rule 1
where a force field or implant is explicitly xenos-produced (e.g., an Eldar personal field
→ `Eldar`). Truly generic prosthetics with no faction signal fall to rule 4 (`Universal †`).

**Availability usage** (Reqs 2.6, 3.6): the existing `Availability` column
(`Plentiful`…`Near Unique`, per `rt-mechanics-fallback` steering) is retained unchanged and
**may inform** the downstream `region` weight magnitude in `Equipment.lua` (rarer items get
smaller weights), but Availability does **not** change the Region_Annotation itself.

### Surface 2 — Equipment.lua Schema + C++ Loader

**Lua field shape** (Requirements 5.1, 5.2, 8.1) — a new optional keyed table `region`,
directly analogous to `chance` on an Enemy_Entry:

```lua
{
    name   = "Choppa",
    -- ... existing fields unchanged ...
    tier   = "common",
    region = { Ork = 100 },          -- single-faction weight
},
{
    name   = "Combat Knife",
    -- ...
    region = { Universal = 100 },    -- selectable in every region
},
{
    name   = "Lasgun",
    -- ...
    region = { ImperialHuman = 80, Chaos = 20 },  -- multi-key permitted, future-proof
},
```

- Keys are Region_Names (or `Universal`); values are **non-negative integers** — the weight
  used for cumulative selection.
- The keyed-table shape lets new Region_Name keys be added to any entry **without changing
  the loader interface or the selection signature** (Reqs 5.2, 8.1).

**C++ storage on `EquipmentTemplate`** (`Headers/Engine.hpp`):

```cpp
// Maps Region_Name (or "Universal") -> non-negative selection weight.
using RegionWeights = std::map<std::string, int>;

struct EquipmentTemplate {
    // ... existing fields unchanged ...
    RegionWeights regionWeights;   // parsed from optional "region" table; see default rule
};
```

`std::map<std::string,int>` mirrors the string-keyed Lua table, keeps the C++/Lua boundary
a plain string (Req 8.2), and imposes no ordering constraints from the caller.

**Documented default when `region` is absent** (Requirements 5.3, 6.1): when an entry omits
`region` (or it parses to empty), the loader assigns the **ImperialHuman default**:

```cpp
tmpl.regionWeights = { { "ImperialHuman", DEFAULT_REGION_WEIGHT } };  // DEFAULT_REGION_WEIGHT = 100
```

This makes the resulting template selectable in the **ImperialHuman** region — untagged
items are treated as part of the ImperialHuman Region (Req 5.3) — and preserves current
behaviour: pre-feature entries (which have no `region` field) remain loadable under
tier-based selection exactly as before (Reqs 5.3, 6.5). This is the documented default
referenced throughout. Note this is distinct from an entry that **explicitly** declares a
`Universal` key, which remains selectable in every region (Req 5.5).

**Parsing (sol2 `get_or` / optional pattern)** — inserted into the existing equipment load
loop in `Source/Engine.cpp`, after tier parsing and before template construction, following
the same defensive style as the `melee`/`ranged`/`armourLocations` parsing already there:

```cpp
// Parse optional region weighting table (default: ImperialHuman = 100).
RegionWeights regionWeights;
sol::optional<sol::table> regionTable = entry["region"];
if (regionTable) {
    for (auto& kv : *regionTable) {
        sol::optional<std::string> key = kv.first.as<sol::optional<std::string>>();
        sol::optional<int>         w   = kv.second.as<sol::optional<int>>();
        if (!key) continue;                       // non-string key -> ignore this pair
        if (isValidRegionName(*key)) {            // valid Region_Name or "Universal"
            if (w && *w >= 0) {
                regionWeights[*key] = *w;         // accept non-negative integer weight
            }
            // malformed weight (missing / non-int / negative) -> skip this pair
        }
        // unrecognized Region_Name key -> ignore for selection (Req 6.4), keep loading
    }
}
if (regionWeights.empty()) {
    // Field absent, or present-but-malformed with nothing usable -> documented default.
    regionWeights = { { "ImperialHuman", DEFAULT_REGION_WEIGHT } };  // Reqs 5.3, 6.3
}
tmpl.regionWeights = regionWeights;
```

Key behaviours:

- **Absent field** → ImperialHuman default (Reqs 5.3, 6.1).
- **Present-but-malformed** (not a table, non-int/negative weights, all-invalid keys) →
  entry still loads; unusable pairs are dropped; if nothing usable remains, the
  ImperialHuman default is applied. Loading of the remaining entries never aborts (Req 6.3).
- **Unrecognized Region_Name key** (outside the valid set and not `Universal`) → that pair
  is ignored for selection but the entry continues loading (Req 6.4).
- `isValidRegionName()` is a small free helper (in `WorldMap`/region utilities, no engine
  access) checking membership in the taxonomy set + `"Universal"`, so it is test-safe under
  `test-isolation` rules.

**Backward-compatibility equivalence guarantee** (Requirement 6.5): all pre-feature required
fields (`name`, `glyph`, `color`, `slot`, `weight`) and optional fields (`value`, `power`,
`defense`, `maxHp`, `skill`, `tier`, `melee`, `ranged`, `armourLocations`) are parsed by the
unchanged existing code; the region parsing is purely additive and only ever writes
`tmpl.regionWeights`. Therefore loading the current `Equipment.lua` under the updated loader
produces the identical set of `EquipmentTemplate`s (same field values) as the pre-feature
loader, differing only by the new `regionWeights` member defaulting to `{ImperialHuman:100}`
(Req 6.2, 6.5).

### Surface 3 — Runtime Region-Weighted Selection

**Signature change** (`Headers/Engine.hpp`, `Source/Engine.cpp`) — add a Region_Name
parameter to the existing selector; the parameter is a plain `std::string` so the boundary
never changes as new Region_Names are added (Req 8.2):

```cpp
const EquipmentTemplate* selectEquipmentByTier(
    EquipmentSlot slot,
    const EnemyEquipmentConfig::TierWeights& weights,
    const std::string& regionName);
```

Call sites (`Map::addItem`, and `Map::addMonster`'s tier-selection loop) pass
`getRegionName()` (falling back to `resolveDefaultRegion()` when empty, exactly as
`Map::addMonster` already does for spawns — Reqs 7.1, 7.6).

**Algorithm** — the existing tier roll is preserved, then a region filter and
cumulative-weight pick are applied among the region-eligible candidates:

1. Roll the ItemTier as today (normalized `common/uncommon/rare` weights).
2. Build candidates matching `slot == slot && tier == selectedTier` **and**
   region-eligible, where an entry is **region-eligible** iff its `regionWeights` contains
   the requested `regionName` key **or** the `"Universal"` key (Reqs 7.2, 7.5, 5.5).
3. If empty, apply the existing tier-fallback order (COMMON→UNCOMMON→RARE), still applying
   the region filter, so region scoping combines with tier (Req 7.5).
4. Among region-eligible candidates, select via **cumulative-weight selection** consistent
   with `Cumulative_Chance_Selection` (Req 7.3): each candidate's weight is
   `regionWeights[regionName]` if present, else `regionWeights["Universal"]`; sum the
   weights, roll `r` in `[0, total)`, walk candidates accumulating until the running total
   exceeds `r`, and return that candidate.
5. **Graceful fallback** (Req 7.4): if no region-eligible candidate exists at any tier, the
   selector returns `nullptr` (its existing "nothing matched" contract), and callers behave
   as they do today for a null result (spawn nothing / skip that slot) — no abort.

```cpp
// After the existing tier + slot filtering produces `candidates`, apply region scoping:
std::vector<const EquipmentTemplate*> regional;
std::vector<int> weightsOut;
int total = 0;
for (const auto* t : candidates) {
    auto it = t->regionWeights.find(regionName);
    int w = (it != t->regionWeights.end())
              ? it->second
              : (t->regionWeights.count("Universal") ? t->regionWeights.at("Universal") : -1);
    if (w >= 0 && w > 0) {           // eligible AND has positive weight
        regional.push_back(t);
        weightsOut.push_back(w);
        total += w;
    }
}
if (regional.empty() || total <= 0) return nullptr;   // graceful fallback (Req 7.4)

int r = TCODRandom::getInstance()->getInt(0, total - 1);   // [0, total)
int acc = 0;
for (size_t i = 0; i < regional.size(); ++i) {
    acc += weightsOut[i];
    if (r < acc) return regional[i];
}
return regional.back();   // defensive; unreachable when total > 0
```

Notes:
- A `Universal` entry is eligible in **every** region because the fallback branch reads its
  `Universal` weight whenever the exact key is absent (Req 5.5, 7.2).
- An entry whose only matching weight is `0` is treated as not selectable in that region
  (weight 0 contributes nothing to the cumulative total), mirroring the NPC rule that a
  missing/zero column is never chosen.

## Data Models

**`RegionWeights`** — `std::map<std::string,int>` on each `EquipmentTemplate`. Invariants
after loading:
- Never empty (defaults to `{"ImperialHuman": DEFAULT_REGION_WEIGHT}`).
- All keys are members of the valid taxonomy or `"Universal"` (unrecognized keys dropped at
  load).
- All values are `>= 0`.

**`DEFAULT_REGION_WEIGHT`** — compiled constant `100`, matching the terminal cumulative
value convention used by NPC `chance` columns.

**Region_Name** — plain `std::string`; the single shared taxonomy across Reference docs,
`Equipment.lua` `region` keys, `Enemies.lua` `chance` keys, and `regionForBiome` outputs
(Req 8.4). Sourced at runtime from `Map::getRegionName()`.

**Reference `Region` column** — one taxonomy string (or `Universal[ †]`) per table row;
additive, existing columns untouched.

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid
executions of a system — essentially, a formal statement about what the system should do.
Properties serve as the bridge between human-readable specifications and machine-verifiable
correctness guarantees.*

The Reference-doc annotation surface (Requirements 2, 3, 4) is source-of-truth
documentation verified by structured review, not property tests. The properties below cover
the **loader** and **selection** surfaces (Requirements 5, 6, 7), which have real runtime
behaviour amenable to property-based testing.

### Property 1: Backward-compatibility equivalence for pre-feature entries

*For any* Equipment_Entry that omits the `region` field, loading it under the updated loader
SHALL produce an EquipmentTemplate whose required fields (`name`, `glyph`, `color`, `slot`,
`weight`) and existing-optional fields (`value`, `power`, `defense`, `maxHp`, `skill`,
`tier`, `melee`, `ranged`, `armourLocations`) equal those produced by the pre-feature
loader, and whose `regionWeights` equals `{ "ImperialHuman": DEFAULT_REGION_WEIGHT }`.

**Validates: Requirements 5.3, 6.1, 6.2, 6.5**

### Property 2: Malformed region field falls back to the documented default

*For any* Equipment_Entry whose `region` field is malformed (not a table, or containing only
non-integer, negative, or unrecognized entries), the loader SHALL still load the entry and
SHALL assign `regionWeights == { "ImperialHuman": DEFAULT_REGION_WEIGHT }`, without aborting
the loading of subsequent entries.

**Validates: Requirements 6.3**

### Property 3: Unrecognized region keys are ignored, valid keys retained

*For any* Equipment_Entry whose `region` field mixes valid Region_Name/Universal keys with
keys outside the taxonomy, the loaded `regionWeights` SHALL contain exactly the valid keys
(with their non-negative weights) and none of the unrecognized keys, and the entry SHALL
load successfully.

**Validates: Requirements 5.1, 6.4**

### Property 4: Region-scoped selection only returns eligible templates

*For any* set of loaded EquipmentTemplates, any slot, and any requested Region_Name, if
`selectEquipmentByTier` returns a non-null template, that template's `regionWeights` SHALL
contain either the requested Region_Name key or the `"Universal"` key (with a positive
weight), and the template's `slot` SHALL equal the requested slot.

**Validates: Requirements 7.2, 7.5**

### Property 5: Universal entries are selectable in every region

*For any* requested Region_Name in the valid taxonomy, given a template pool in which at
least one entry (matching the slot) carries a positive `"Universal"` weight, region-scoped
selection for that slot SHALL be able to return a non-null template (it SHALL NOT return
null solely because the exact Region_Name key is absent from every entry).

**Validates: Requirements 5.5, 7.2**

### Property 6: Cumulative-weight selection is proportional to weights

*For any* pool of region-eligible templates (same slot and tier) with positive integer
weights for the requested Region_Name, running region-scoped selection many times SHALL
select each candidate with relative frequency proportional to its weight (within statistical
tolerance), matching the Cumulative_Chance_Selection model.

**Validates: Requirements 7.3**

### Property 7: Graceful fallback when nothing is region-eligible

*For any* requested Region_Name and slot for which no loaded template is region-eligible
(no matching Region_Name key and no `"Universal"` key with positive weight at any tier),
`selectEquipmentByTier` SHALL return `nullptr` and SHALL NOT abort or throw.

**Validates: Requirements 7.4**

## Error Handling

- **Missing/malformed `region` field:** handled at load time by the ImperialHuman default
  (untagged items become part of the ImperialHuman Region); never fatal (Reqs 6.1, 6.3). Consistent with the loader's existing "skip-with-warning / apply
  default" philosophy — but note the region field is optional, so its absence is *not* a
  warning, only genuinely malformed content is dropped silently per-pair.
- **Unrecognized Region_Name keys:** dropped for selection, entry still loads (Req 6.4).
- **Zero total weight at selection time:** returns `nullptr`; callers fall back to their
  existing no-item behaviour (Req 7.4).
- **Empty region name at a call site:** callers resolve via `resolveDefaultRegion()` before
  calling (same pattern as `Map::addMonster`), so the selector always receives a non-empty
  Region_Name (Reqs 7.1, 7.6).
- **Test isolation:** `isValidRegionName()` and `selectEquipmentByTier`'s region logic must
  not touch `engine.gui`/`map`/`player`. The existing `gui->message` warnings inside
  `selectEquipmentByTier` are only reached on the pre-existing zero-tier-weight and
  no-template paths; region tests exercise the selection logic with a locally-populated
  template vector and avoid those branches (or guard with `if (gui)`), per `test-isolation`.

## Testing Strategy

This feature has real runtime behaviour (loader + selection), so it gets automated tests
using the project stack: **Catch2 v3** for unit/example tests and **RapidCheck** for
property tests (C++17, MSVC, sol2, Lua 5.4). This is unlike the pure-doc
`equipment-reference-extraction` feature, which had no runtime behaviour to assert.

Split of verification by surface:

- **Reference-doc annotation (Reqs 2, 3, 4):** verified by **structured review** — a
  checklist confirming every table row has exactly one `Region` value from the taxonomy (or
  `Universal[ †]`), every existing column/citation/footnote is preserved, and each
  assignment follows the decision rules. No automated test.
- **Loader (Reqs 5, 6):** unit tests for concrete cases (absent field → ImperialHuman
  default; a single-key entry; a multi-key entry; an unrecognized-key entry; a
  malformed-weight entry) plus property tests for Properties 1–3.
- **Selection (Req 7):** unit tests for concrete cases (region-scoped pick, Universal
  fallthrough, empty pool → null) plus property tests for Properties 4–7.

### Property-Based Testing

- Library: **RapidCheck** (`rc::check`), run with **minimum 100 iterations** per property.
- Each property test is tagged with a comment referencing its design property, format:
  `// Feature: equipment-region-assignment, Property {n}: {property text}`.
- Each of the seven Correctness Properties is implemented by a **single** property-based
  test. Property 6 uses a large iteration count and asserts relative frequencies within a
  statistical tolerance band rather than exact counts.
- Generators build in-memory `std::vector<EquipmentTemplate>` pools and `RegionWeights`
  maps directly (no Lua, no Engine), keeping tests engine-isolated. Where a generator picks
  an index into a container of size `N`, it uses `rc::gen::inRange(0, N - 1)` (inclusive
  upper bound, per `test-isolation` steering); for taxonomy enums it uses
  `inRange(0, COUNT - 1)`.
- Loader properties (1–3) drive the parsing logic. To stay test-safe, the region-parsing
  logic is factored into a free function `parseRegionWeights(sol::table)` /
  `applyRegionDefault(RegionWeights&)` callable without an initialized Engine; tests feed it
  sol2 tables built in a local `sol::state`.

### Unit / Example Tests

- `region` field absent → `regionWeights == {ImperialHuman:100}` (Req 5.3/6.1).
- All existing Equipment.lua entries still load with identical non-region fields (Req 6.5
  spot-check alongside Property 1).
- Selection with a pool containing an `Ork`-keyed entry and a `Universal`-keyed entry, asked
  for `"Ork"`, can return either; asked for `"Eldar"`, returns only the `Universal` one
  (Reqs 7.2, 5.5).
- Selection asked for a region with no eligible entries returns `nullptr` (Req 7.4).

### TDD Ordering & Project Wiring

Per `tdd-workflow` steering, test tasks precede (or share a wave with) their implementation
tasks: qa-tester writes failing loader/selection tests first; developer then implements the
`RegionWeights` member, the loader parsing, and the `selectEquipmentByTier` region parameter
to make them pass.

Per `test-project` steering: any **new** `.cpp` added (e.g. a `Region.cpp`/region-utility
file hosting `isValidRegionName`/`parseRegionWeights`, if not folded into existing
`WorldMap.cpp`) must be added to **both** `40kRL.vcxproj` and
`Tests/40kRL_Tests.vcxproj` ("Game source files" ItemGroup). New test `.cpp` files are added
to `Tests/40kRL_Tests.vcxproj` only. MSBuild is invoked via the full path from the `msbuild`
steering rule.
