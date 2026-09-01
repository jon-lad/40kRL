# Requirements Document

## Introduction

This feature completes the persistence and test coverage for the player's `CharacterSheet` component. During investigation it was discovered that `CareerProgression::save()` and `CareerProgression::load()` are empty stubs (marked "Stub — full implementation in task 3.3" in `Source/CareerProgression.cpp`). Because `CharacterSheet::save()`/`load()` delegate to `Characteristics` and `CareerProgression`, the empty career stubs mean that all career data (rank, XP pool, spent XP, skills, talents, traits) is silently lost across a save/load cycle — only the nine `Characteristics` survive.

The scope of this feature is twofold:

1. **Implement** `CareerProgression::save()` and `CareerProgression::load()` so that every career field is fully serialized and deserialized, using the project's existing `TCODZip` collection-serialization patterns.
2. **Test** the full `CharacterSheet` and `CareerProgression` surface with Catch2 v3 unit tests and RapidCheck property-based tests, covering serialization round-trips, purchase eligibility rules, purchase mutation logic, rank evaluation, and available-XP arithmetic.

This is a serialization and logic feature with no GUI involvement, so all components and tests are engine-independent (no `engine.gui`, `engine.map`, or `engine.player` access) per the test-isolation steering rules.

The existing `CareerProgression::canPurchase`, `purchase`, `evaluateRankUp`, and `availableXp` logic is already implemented; this feature treats their current behavior as the specification to be locked in by tests, and fills the serialization gap. Skill-rank convention follows the existing implementation: skill ranks are integers where a first purchase places a skill at rank 1 and each subsequent purchase increments by 1, capped at rank 2 (+20).

## Glossary

- **CharacterSheet**: The unified player character component (`Headers/CharacterSheet.hpp`) that owns a `Characteristics` instance and a `CareerProgression` instance, and delegates its `save`/`load` to both in fixed order (characteristics first, career second).
- **Characteristics**: The component (`Headers/Characteristics.hpp`) storing the nine Rogue Trader characteristics as base values clamped to `[1, 99]`, with `save`/`load` writing/reading nine ints in fixed `CharId` order. Already implemented and working.
- **CareerProgression**: The component (`Headers/CareerProgression.hpp`) tracking career identity and advancement: `homeworldName`, `careerName`, `currentRank`, `xpPool`, `spentXp`, `skills`, `talents`, and `traits`, plus the `canPurchase`, `purchase`, `evaluateRankUp`, `availableXp`, `save`, and `load` methods.
- **CharId**: The enum (`WS, BS, S, T, Ag, Int, Per, WP, Fel`) indexing the nine characteristics in fixed serialization order.
- **AdvanceEntry**: A purchasable advance (`Headers/CharacterData.hpp`) with a `type` (`CHARACTERISTIC`, `SKILL`, or `TALENT`), a `name`, an XP `cost`, and an `amount` (default 5, used for characteristic increases).
- **CareerTemplate**: A career definition (`Headers/CharacterData.hpp`) holding an ordered vector of `RankDefinition` records.
- **RankDefinition**: One rank within a career, holding `rankNumber`, `rankTitle`, `xpThreshold` (cumulative spent XP required to enter the rank), and an `advances` vector.
- **Skills_Map**: The `std::unordered_map<std::string, int>` on `CareerProgression` mapping skill name to integer skill rank (first purchase sets rank 1; increments up to a maximum of 2 representing +20).
- **Talents_Set**: The `std::unordered_set<std::string>` on `CareerProgression` holding acquired talent names.
- **Traits_Vector**: The `std::vector<std::string>` on `CareerProgression` holding trait names (no mechanical effect in this spec).
- **Available_XP**: The value `xpPool - spentXp`, returned by `CareerProgression::availableXp()`.
- **TCODZip**: The libtcod serialization buffer used by all `Persistent` components; supports `putInt`/`getInt` for integers and `putString`/`getString` for C-strings.
- **Round_Trip**: A serialization test in which an object is written to a `TCODZip`, then read back into a fresh object, and the two objects are asserted field-for-field equal.
- **Test_Suite**: The Catch2 v3 test file `Tests/test_character_sheet.cpp`, tagged `[character-sheet]`, added to `Tests/40kRL_Tests.vcxproj`.

## Requirements

### Requirement 1: CareerProgression Serialization

**User Story:** As a developer, I want `CareerProgression` to serialize and deserialize every field, so that a saved character retains its full career state (rank, XP, skills, talents, traits) after loading.

#### Acceptance Criteria

1. WHEN `CareerProgression::save` is called, THE CareerProgression SHALL write `homeworldName` and `careerName` to the TCODZip as strings.
2. WHEN `CareerProgression::save` is called, THE CareerProgression SHALL write `currentRank`, `xpPool`, and `spentXp` to the TCODZip as integers.
3. WHEN `CareerProgression::save` is called, THE CareerProgression SHALL write the Skills_Map by first writing an integer entry count, then writing each entry as a skill-name string followed by an integer rank.
4. WHEN `CareerProgression::save` is called, THE CareerProgression SHALL write the Talents_Set by first writing an integer element count, then writing each talent name as a string.
5. WHEN `CareerProgression::save` is called, THE CareerProgression SHALL write the Traits_Vector by first writing an integer element count, then writing each trait name as a string in order.
6. WHEN `CareerProgression::load` is called on data produced by `CareerProgression::save`, THE CareerProgression SHALL restore `homeworldName`, `careerName`, `currentRank`, `xpPool`, and `spentXp` to the values that were saved.
7. WHEN `CareerProgression::load` is called on data produced by `CareerProgression::save`, THE CareerProgression SHALL restore the Skills_Map to contain exactly the saved skill-name-to-rank entries.
8. WHEN `CareerProgression::load` is called on data produced by `CareerProgression::save`, THE CareerProgression SHALL restore the Talents_Set to contain exactly the saved talent names.
9. WHEN `CareerProgression::load` is called on data produced by `CareerProgression::save`, THE CareerProgression SHALL restore the Traits_Vector to contain exactly the saved trait names in the saved order.
10. WHEN `CareerProgression::load` reads collection data, THE CareerProgression SHALL clear any pre-existing entries in the Skills_Map, Talents_Set, and Traits_Vector before populating them from the TCODZip.

### Requirement 2: CareerProgression Serialization Round-Trip Property

**User Story:** As a developer, I want a property-based guarantee that any valid `CareerProgression` state survives a save/load cycle unchanged, so that no career field is silently dropped for any combination of data.

#### Acceptance Criteria

1. FOR ALL CareerProgression instances with arbitrary `homeworldName`, `careerName`, `currentRank`, `xpPool`, `spentXp`, Skills_Map, Talents_Set, and Traits_Vector, saving to a TCODZip and loading into a fresh CareerProgression SHALL produce an instance whose fields are equal to the original (Round_Trip property).
2. WHEN the Round_Trip property is evaluated, THE Test_Suite SHALL run at least 100 generated iterations.
3. WHERE a generated CareerProgression has an empty Skills_Map, empty Talents_Set, and empty Traits_Vector, THE Round_Trip property SHALL hold with all collections restored empty.

### Requirement 3: CharacterSheet Serialization Round-Trip

**User Story:** As a developer, I want `CharacterSheet` save/load to preserve both characteristics and career data together, so that the composed player component is fully persisted.

#### Acceptance Criteria

1. WHEN `CharacterSheet::save` is called, THE CharacterSheet SHALL write the Characteristics data followed by the CareerProgression data to the TCODZip in that order.
2. WHEN `CharacterSheet::load` is called on data produced by `CharacterSheet::save`, THE CharacterSheet SHALL restore all nine characteristic base values to the values that were saved.
3. WHEN `CharacterSheet::load` is called on data produced by `CharacterSheet::save`, THE CharacterSheet SHALL restore every CareerProgression field to the values that were saved.
4. FOR ALL CharacterSheet instances with arbitrary characteristic base values in `[1, 99]` and arbitrary CareerProgression state, saving to a TCODZip and loading into a fresh CharacterSheet SHALL produce an instance equal to the original across all characteristic and career fields (Round_Trip property), evaluated over at least 100 generated iterations.

### Requirement 4: Purchase Eligibility (canPurchase)

**User Story:** As a player, I want the game to correctly determine whether I can afford and am allowed an advance, so that invalid purchases are rejected before any state changes.

#### Acceptance Criteria

1. IF Available_XP is less than the AdvanceEntry `cost`, THEN THE CareerProgression SHALL return `false` from `canPurchase` regardless of advance type.
2. WHERE the AdvanceEntry type is SKILL and the named skill already exists in the Skills_Map at rank 2 or higher, THE CareerProgression SHALL return `false` from `canPurchase`.
3. WHERE the AdvanceEntry type is TALENT and the named talent already exists in the Talents_Set, THE CareerProgression SHALL return `false` from `canPurchase`.
4. WHERE the AdvanceEntry type is CHARACTERISTIC and Available_XP is greater than or equal to the `cost`, THE CareerProgression SHALL return `true` from `canPurchase`.
5. WHERE the AdvanceEntry type is SKILL, the named skill is absent or below rank 2, and Available_XP is greater than or equal to the `cost`, THE CareerProgression SHALL return `true` from `canPurchase`.
6. WHERE the AdvanceEntry type is TALENT, the named talent is absent from the Talents_Set, and Available_XP is greater than or equal to the `cost`, THE CareerProgression SHALL return `true` from `canPurchase`.

### Requirement 5: Purchase Application (purchase)

**User Story:** As a player, I want purchasing an advance to deduct its XP cost and apply its effect, so that my character progresses correctly.

#### Acceptance Criteria

1. IF `canPurchase` returns `false` for an AdvanceEntry, THEN THE CareerProgression SHALL return `false` from `purchase` and SHALL leave `spentXp`, the Skills_Map, the Talents_Set, and the target Characteristics unchanged.
2. WHEN a purchasable AdvanceEntry is purchased, THE CareerProgression SHALL increase `spentXp` by the AdvanceEntry `cost` and return `true`.
3. WHEN a purchasable CHARACTERISTIC AdvanceEntry naming a recognized CharId is purchased, THE CareerProgression SHALL set that characteristic to its current value plus the AdvanceEntry `amount`, clamped to the range `[1, 99]`.
4. WHEN a purchasable SKILL AdvanceEntry naming a skill absent from the Skills_Map is purchased, THE CareerProgression SHALL insert the skill into the Skills_Map at rank 1.
5. WHEN a purchasable SKILL AdvanceEntry naming a skill already present in the Skills_Map is purchased, THE CareerProgression SHALL increase that skill's rank by 1.
6. WHEN a purchasable TALENT AdvanceEntry is purchased, THE CareerProgression SHALL insert the talent name into the Talents_Set.
7. WHEN a purchasable CHARACTERISTIC AdvanceEntry naming an unrecognized characteristic abbreviation is purchased, THE CareerProgression SHALL deduct the `cost` from `spentXp`, leave all characteristics unchanged, and return `true`.

### Requirement 6: Rank Evaluation (evaluateRankUp)

**User Story:** As a player, I want my rank to advance automatically once I have spent enough XP, so that reaching a career milestone promotes my character.

#### Acceptance Criteria

1. WHEN `evaluateRankUp` is called, THE CareerProgression SHALL set `currentRank` to the highest `rankNumber` among the CareerTemplate ranks whose `xpThreshold` is less than or equal to `spentXp` and whose `rankNumber` is greater than the current `currentRank`.
2. IF no CareerTemplate rank has both an `xpThreshold` less than or equal to `spentXp` and a `rankNumber` greater than `currentRank`, THEN THE CareerProgression SHALL leave `currentRank` unchanged.
3. WHEN `evaluateRankUp` is called with a CareerTemplate whose ranks are all above the current `spentXp` threshold, THE CareerProgression SHALL leave `currentRank` unchanged.
4. WHEN `evaluateRankUp` is called twice in succession with the same CareerTemplate and no intervening change to `spentXp`, THE CareerProgression SHALL produce the same `currentRank` after the second call as after the first call (idempotence).

### Requirement 7: Available XP Calculation (availableXp)

**User Story:** As a player, I want an accurate count of XP I have left to spend, so that I know what advances I can afford.

#### Acceptance Criteria

1. THE CareerProgression SHALL return `xpPool` minus `spentXp` from `availableXp`.
2. FOR ALL integer values of `xpPool` and `spentXp`, `availableXp` SHALL equal `xpPool - spentXp` (evaluated over at least 100 generated iterations).

### Requirement 8: Test Suite Integration

**User Story:** As a developer, I want the new tests wired into the test project and consistently tagged, so that they run as part of the standard test build.

#### Acceptance Criteria

1. THE Test_Suite SHALL be located at `Tests/test_character_sheet.cpp`.
2. THE Test_Suite SHALL tag every test case with `[character-sheet]`.
3. THE Test_Suite SHALL be registered in `Tests/40kRL_Tests.vcxproj` within the test-sources ItemGroup so that it compiles and links into the test binary.
4. THE Test_Suite SHALL exercise only `CharacterSheet`, `CareerProgression`, `Characteristics`, and `CharacterData` types without invoking `engine.gui`, `engine.map`, or `engine.player` (engine-independent tests).
5. WHERE a property-based test generates an index into a container of size N, THE Test_Suite SHALL use `rc::gen::inRange(0, N - 1)` to respect this project's inclusive-bounds convention.
