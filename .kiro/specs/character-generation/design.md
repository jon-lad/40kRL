# Design Document — Character Generation

## Overview

This feature replaces the existing `Classes.lua`-based player creation with a multi-step character generation flow based on the Rogue Trader RPG NPC creation rules. The player selects a homeworld (determines characteristic modifiers, starting skills, and traits) and a career path (determines rank-gated advances). A skills/talents tracking system is introduced. XP spending replaces the old auto-level system: players purchase characteristic advances, skills, and talents from their current rank's advance table. The XP bar is reworked to show `Available_XP / XP_Pool` instead of progress-to-next-level.

The implementation integrates with the existing Engine singleton, component-based Actor system, `Persistent`/`TCODZip` serialization, and sol2 Lua scripting. Four new Lua data files define content. A new `CHARACTER_GEN` game state drives a modal overlay flow before gameplay begins.

---

## Architecture

### System Integration Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                           Engine                                     │
│                                                                     │
│  GameStatus::CHARACTER_GEN  ──►  CharacterGenerator (new)           │
│       │                            │                                │
│       │   ┌────────────────────────┼────────────────────────────┐   │
│       │   │  Step 1: Homeworld     │  Step 2: Career Path       │   │
│       │   │  Step 3: Initial Advances (spend starting XP)       │   │
│       │   └────────────────────────┼────────────────────────────┘   │
│       │                            │                                │
│       ▼                            ▼                                │
│  Engine::init()               Player Actor created with:            │
│  (after chargen completes)      - Characteristics (base 25 + mods) │
│                                 - CareerProgression component       │
│                                 - Skills collection                 │
│                                 - Talents collection                │
│                                                                     │
├─────────────────────────────────────────────────────────────────────┤
│                        Lua Data Layer                                │
│                                                                     │
│  Scripts/Homeworlds.lua   Scripts/Careers.lua                       │
│  Scripts/Skills.lua       Scripts/Talents.lua                       │
│                                                                     │
│  Loaded at Engine::init() into vectors of parsed structs            │
└─────────────────────────────────────────────────────────────────────┘
```

### Game State Flow

```
Engine::load()
  └─ Main Menu
       └─ "New Game" selected
            └─ gameStatus = CHARACTER_GEN
                 └─ CharacterGenerator modal overlay
                      ├─ Step 1: selectHomeworld()
                      ├─ Step 2: selectCareer()
                      └─ Step 3: purchaseInitialAdvances()
                           └─ On confirm: build player Actor
                                └─ gameStatus = STARTUP (normal gameplay begins)
```

### Modal Overlay Pattern

The character generation follows the same overlay pattern used by `TARGETING`, `INVENTORY`, `LOOK`, and `CHARACTER_SHEET`:

1. A new `GameStatus::CHARACTER_GEN` enum value is added.
2. An `std::optional<CharGenState>` field is added to `Engine`.
3. `Engine::update()` dispatches to `updateCharGen()` when in this state.
4. `Engine::render()` dispatches to `renderCharGen()` when in this state.
5. Input is consumed entirely by the chargen overlay — no game world interaction.

---

## Components and Interfaces

### New Data Structures (Lua-Loaded Templates)

```cpp
// Loaded from Scripts/Homeworlds.lua
struct HomeworldTemplate {
    std::string name;
    std::array<int, 9> characteristicMods;  // indexed by CharId, can be negative
    std::vector<std::string> startingSkills; // skill names granted at Trained
    std::vector<std::string> startingTraits; // trait names (recorded, not mechanically active)
};

// A single advance entry within a rank table
struct AdvanceEntry {
    enum class Type { CHARACTERISTIC, SKILL, TALENT };
    Type type;
    std::string name;       // CharId abbreviation, skill name, or talent name
    int cost;               // XP cost to purchase
    int amount = 5;         // for CHARACTERISTIC: amount to add (default +5)
};

// One rank's data within a career path
struct RankDefinition {
    int rankNumber;         // 1-based
    std::string rankTitle;  // display name (e.g. "Void-Master")
    int xpThreshold;        // cumulative Spent_XP required to enter this rank
    std::vector<AdvanceEntry> advances;
    std::vector<std::string> startingSkills;  // granted free at rank entry (Rank 1 only)
    std::vector<std::string> startingTalents; // granted free at rank entry (Rank 1 only)
};

// Loaded from Scripts/Careers.lua
struct CareerTemplate {
    std::string name;
    std::vector<RankDefinition> ranks;  // ordered by rankNumber
};

// Loaded from Scripts/Skills.lua
struct SkillDefinition {
    std::string name;
    bool isCombat;  // true = combat skill, false = non-combat (deferred mechanics)
};

// Loaded from Scripts/Talents.lua
struct TalentDefinition {
    std::string name;
    std::string description;
};
```

### New Component: CareerProgression

Attached to the player Actor. Owns all career/rank/XP/skill/talent state.

```cpp
// Tracks career path progression, XP spending, skills, and talents.
class CareerProgression : public Persistent {
public:
    // Identifiers (index into Engine template vectors, plus string for serialization)
    std::string homeworldName;
    std::string careerName;

    // Rank tracking
    int currentRank = 1;

    // XP
    int xpPool = 0;       // total XP earned
    int spentXp = 0;      // total XP spent on advances
    int availableXp() const { return xpPool - spentXp; }

    // Skills: maps skill name → rank (0 = Trained, 1 = +10, 2 = +20)
    std::unordered_map<std::string, int> skills;

    // Talents: set of acquired talent names
    std::unordered_set<std::string> talents;

    // Traits: set of trait names (no mechanical effect this spec)
    std::vector<std::string> traits;

    // Purchase logic
    bool canPurchase(const AdvanceEntry& advance) const;
    bool purchase(const AdvanceEntry& advance, Characteristics& chars);

    // Rank evaluation
    void evaluateRankUp(const CareerTemplate& career);

    // Persistence
    void save(TCODZip& zip) override;
    void load(TCODZip& zip) override;
};
```

### Character Generation State

```cpp
struct CharGenState {
    enum class Step { HOMEWORLD, CAREER, ADVANCES, DONE };
    Step currentStep = Step::HOMEWORLD;

    int selectedIndex = 0;     // cursor position in current list
    int scrollOffset = 0;      // for scrollable lists

    // Accumulated choices (indices into Engine template vectors)
    int homeworldIndex = -1;
    int careerIndex = -1;

    // Temporary characteristics built up during chargen
    Characteristics workingChars{25};  // starts at base 25

    // Working skills/talents from homeworld + career grants
    std::unordered_map<std::string, int> workingSkills;
    std::unordered_set<std::string> workingTalents;
    std::vector<std::string> workingTraits;

    // XP for initial advance purchases (granted by career at creation)
    int startingXp = 0;
    int spentXp = 0;
};
```

### Engine Modifications

```cpp
// New fields on Engine:
std::vector<HomeworldTemplate> homeworldTemplates;   // loaded from Homeworlds.lua
std::vector<CareerTemplate> careerTemplates;         // loaded from Careers.lua
std::vector<SkillDefinition> skillDefinitions;       // loaded from Skills.lua
std::vector<TalentDefinition> talentDefinitions;     // loaded from Talents.lua

std::optional<CharGenState> charGenState;            // active during CHARACTER_GEN

// New GameStatus value:
// CHARACTER_GEN  — character generation overlay is active

// New methods:
void beginCharGen();
void updateCharGen();
void renderCharGen();
```

### Actor Modifications

```cpp
// New component slot on Actor:
std::shared_ptr<CareerProgression> career;  // non-null for player after chargen
```

---

## Data Models

### Lua File: Scripts/Homeworlds.lua

```lua
homeworlds = {
    {
        name = "Void Born",
        charMods = { ws = 0, bs = 0, s = -5, t = 0, ag = 0, int = 0, per = 0, wp = 5, fel = 0 },
        startingSkills = { "Navigation (Stellar)", "Pilot (Spacecraft)" },
        startingTraits = { "Void Accustomed", "Charmed" },
    },
    {
        name = "Hive World",
        charMods = { ws = 0, bs = 0, s = 0, t = 0, ag = 5, int = 0, per = 0, wp = 0, fel = -5 },
        startingSkills = { "Awareness", "Deceive" },
        startingTraits = { "Hivebound", "Wary" },
    },
    -- ... more homeworlds
}
```

### Lua File: Scripts/Careers.lua

```lua
careers = {
    {
        name = "Rogue Trader",
        ranks = {
            {
                rankNumber = 1,
                rankTitle = "Rogue Trader",
                xpThreshold = 0,
                startingSkills = { "Command", "Commerce" },
                startingTalents = { "Pistol Weapon Training (Universal)" },
                advances = {
                    { type = "characteristic", name = "WS",  cost = 100, amount = 5 },
                    { type = "characteristic", name = "BS",  cost = 100, amount = 5 },
                    { type = "characteristic", name = "Fel", cost = 100, amount = 5 },
                    { type = "skill",          name = "Command",   cost = 100 },
                    { type = "skill",          name = "Commerce",  cost = 100 },
                    { type = "talent",         name = "Air of Authority", cost = 200 },
                },
            },
            {
                rankNumber = 2,
                rankTitle = "Master Trader",
                xpThreshold = 500,
                startingSkills = {},
                startingTalents = {},
                advances = {
                    { type = "characteristic", name = "Fel", cost = 250, amount = 5 },
                    { type = "skill",          name = "Barter", cost = 200 },
                    { type = "talent",         name = "Iron Discipline", cost = 300 },
                },
            },
            -- ... more ranks
        },
    },
    -- ... more career paths
}
```

### Lua File: Scripts/Skills.lua

```lua
skills = {
    { name = "Awareness",              isCombat = false },
    { name = "Command",                isCombat = false },
    { name = "Commerce",               isCombat = false },
    { name = "Deceive",                isCombat = false },
    { name = "Navigation (Stellar)",   isCombat = false },
    { name = "Pilot (Spacecraft)",     isCombat = false },
    { name = "Barter",                 isCombat = false },
    -- ... more skills
}
```

### Lua File: Scripts/Talents.lua

```lua
talents = {
    { name = "Pistol Weapon Training (Universal)", description = "May use any pistol weapon without penalty." },
    { name = "Air of Authority",                   description = "Affect more targets with Command tests." },
    { name = "Iron Discipline",                    description = "Followers may re-roll failed Willpower tests." },
    -- ... more talents
}
```

### Skill Rank Encoding

| Rank Value | Display Name | Meaning |
|-----------|-------------|---------|
| 0 | Trained | Skill is known, no bonus |
| 1 | +10 | Adds +10 to relevant tests |
| 2 | +20 | Adds +20 to relevant tests |

Purchasing a skill advance increments the rank value by 1. Maximum is 2 (+20). Skills not in the map are untrained.

### XP Model

| Field | Derivation |
|-------|-----------|
| `xpPool` | Accumulated from enemy kills (existing `destructible->xp` award path) |
| `spentXp` | Sum of all advance costs ever purchased |
| `availableXp` | `xpPool - spentXp` |

Rank thresholds are evaluated against `spentXp` (not `xpPool`). This matches the Rogue Trader rules where spending XP is what unlocks higher ranks.

### Serialization Layout (CareerProgression)

Appended to the player Actor's save block after existing components:

```
int     hasCareer (0 or 1)
[if hasCareer:]
  string  homeworldName
  string  careerName
  int     currentRank
  int     xpPool
  int     spentXp
  int     skillCount
  foreach skill:
    string  skillName
    int     skillRank
  int     talentCount
  foreach talent:
    string  talentName
  int     traitCount
  foreach trait:
    string  traitName
```

### XP Bar Display Format

Old: `XP(1)  ████████░░░░░░░░░░░░  [current / nextLevel]`

New: `XP: 150 / 500  Rank 2`

Rendered as text on the HUD panel at the same position (row 5), replacing the progress bar with a plain text label.

---

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system — essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

---

### Property 1: Homeworld modifiers produce clamped characteristics

*For any* array of 9 characteristic modifiers (each in [-74, +74]) applied to a base value of 25, the resulting characteristic value SHALL equal `max(1, min(99, 25 + modifier))` for each characteristic.

**Validates: Requirements 1.2, 3.1, 3.2**

---

### Property 2: Homeworld and career grants are fully applied

*For any* homeworld template with an arbitrary set of starting skills and traits, and *for any* career template with an arbitrary set of Rank 1 starting skills and talents, after applying both: every homeworld skill SHALL appear in the character's skill collection at Trained rank (0), every homeworld trait SHALL appear in the traits collection, every career starting skill SHALL appear at Trained rank, and every career starting talent SHALL appear in the talents collection.

**Validates: Requirements 1.3, 1.4, 2.3, 4.2, 5.2**

---

### Property 3: Skill rank invariant and purchase semantics

*For any* sequence of skill advance purchases on a character, the skill rank SHALL remain in the range [0, 2] at all times. Purchasing an advance on a skill at rank 0 or 1 SHALL increment the rank by exactly 1. Purchasing an advance on a skill at rank 2 SHALL be rejected and leave the rank unchanged.

**Validates: Requirements 4.1, 4.3, 4.4**

---

### Property 4: Talent collection invariant and purchase semantics

*For any* sequence of talent acquisitions, the talent collection SHALL never contain duplicate entries. Purchasing a talent not already in the collection SHALL add it. Purchasing a talent already in the collection SHALL be rejected and leave the collection unchanged.

**Validates: Requirements 5.1, 5.3, 5.4**

---

### Property 5: XP conservation on advance purchase

*For any* character state where `availableXp >= advance.cost`, purchasing that advance SHALL result in `new_spentXp == old_spentXp + advance.cost` and `new_availableXp == old_availableXp - advance.cost`, while `xpPool` remains unchanged. If `availableXp < advance.cost`, the purchase SHALL be rejected and all XP fields remain unchanged.

**Validates: Requirements 6.3, 6.4**

---

### Property 6: Characteristic advance increases value by defined amount

*For any* characteristic at value V in [1, 99] and *for any* advance amount A in [1, 20], purchasing a characteristic advance SHALL set the new value to `max(1, min(99, V + A))`.

**Validates: Requirements 6.5**

---

### Property 7: Displayed advances restricted to current rank

*For any* career template with multiple ranks and *for any* current rank R, the set of advances available for purchase SHALL exactly equal the advance table entries defined for rank R in the career template — no entries from ranks other than R SHALL appear.

**Validates: Requirements 6.1, 6.6, 12.1, 12.2**

---

### Property 8: Rank-up evaluates correctly against XP thresholds

*For any* career template with rank thresholds [T1=0, T2, T3, ...] and *for any* value of spentXp, the character's rank SHALL equal the highest rank R where `spentXp >= T_R`. Rank SHALL never decrease.

**Validates: Requirements 7.1, 7.2, 7.3**

---

### Property 9: XP award increases pool without side effects

*For any* XP award amount A > 0, adding XP to the character SHALL increase `xpPool` by exactly A, leave `spentXp` unchanged, leave `currentRank` unchanged, and leave all nine characteristic values unchanged.

**Validates: Requirements 8.4, 13.3, 13.7**

---

### Property 10: CareerProgression serialization round-trip

*For any* valid CareerProgression state (arbitrary homeworldName, careerName, currentRank in [1, 8], xpPool >= spentXp >= 0, arbitrary skill map with ranks in [0, 2], arbitrary talent set, arbitrary traits list), serializing to a TCODZip archive then deserializing into a fresh instance SHALL produce a state with all fields equal to the original.

**Validates: Requirements 11.1, 11.2, 11.3**



---

## Error Handling

| Scenario | Behaviour |
|----------|-----------|
| Homeworlds.lua missing or malformed | Log warning via `gui->message`, use empty template vector. Chargen shows "No homeworlds available" and cannot proceed. |
| Careers.lua missing or malformed | Same as above — chargen cannot proceed without career data. |
| Skills.lua / Talents.lua missing | Log warning, continue with empty definitions. Skills/talents referenced by homeworld/career that don't exist in definitions are still granted (names are stored as strings). |
| Invalid Lua entry (missing required field) | Skip entry, log warning with entry index. Do not crash. Continue loading remaining entries. |
| Characteristic modifier produces value outside [1, 99] | Clamped silently by `Characteristics::set()` — existing behaviour. |
| Advance purchase with insufficient XP | `canPurchase()` returns false. UI displays "Insufficient XP" message. State unchanged. |
| Skill purchase at max rank (+20) | `canPurchase()` returns false for skill-type advances when rank == 2. UI displays "Skill at maximum rank". |
| Duplicate talent purchase | `canPurchase()` returns false when talent already in set. UI displays "Talent already acquired". |
| Save file missing career data | Not applicable — no legacy saves exist. The `hasCareer` flag is always 1 for valid saves produced by this version. |
| Career template has no ranks defined | Log warning during load, skip that career. Chargen won't show it. |
| Rank threshold not met for any rank > 1 | Character stays at Rank 1. `evaluateRankUp` is a no-op. |

---

## Testing Strategy

### PBT Applicability Assessment

This feature has significant pure-function logic amenable to property-based testing:
- Characteristic modification and clamping (pure arithmetic)
- Skill/talent collection management (set/map operations)
- XP accounting (arithmetic invariants)
- Rank threshold evaluation (comparison logic)
- Serialization round-trips (encode/decode)
- Advance restriction logic (filtering)

The Lua data loading is integration-level (file I/O + sol2 parsing) and is better tested with example-based integration tests. The UI rendering (chargen overlay display) is tested with example-based tests verifying state, not visual output.

### Property-Based Testing Library

**RapidCheck** (already integrated — see `Tests/test_rapidcheck_smoke.cpp` and `Tests/lib/`).

Each property test runs a minimum of **100 iterations** via `rc::check(...)` or `rc::prop(...)`.

Tag format: `// Feature: character-generation, Property N: <property text>`

### Property Tests (10 properties)

| Property | Test File | What's Generated |
|----------|-----------|-----------------|
| 1: Homeworld modifiers | `test_character_generation.cpp` | Random modifier arrays [-74, +74] per characteristic |
| 2: Grants applied | `test_character_generation.cpp` | Random skill/talent name lists |
| 3: Skill rank invariant | `test_character_generation.cpp` | Random sequences of skill operations |
| 4: Talent set invariant | `test_character_generation.cpp` | Random sequences of talent acquisitions |
| 5: XP conservation | `test_character_generation.cpp` | Random xpPool, spentXp, advance cost |
| 6: Characteristic advance | `test_character_generation.cpp` | Random characteristic values + advance amounts |
| 7: Advances restricted to rank | `test_character_generation.cpp` | Random career templates with multiple ranks |
| 8: Rank-up thresholds | `test_character_generation.cpp` | Random threshold sequences + spentXp values |
| 9: XP award no side effects | `test_character_generation.cpp` | Random xpPool + award amounts |
| 10: Serialization round-trip | `test_character_generation.cpp` | Random CareerProgression states |

### Unit Tests (Example-Based)

| Test | Coverage |
|------|----------|
| Chargen initializes with base 25 for all characteristics | Req 3.1 |
| Chargen step order: homeworld → career → advances | Req 9.2 |
| Back-navigation resets to previous step | Req 9.4 |
| Initial rank is 1 after career selection | Req 2.2 |
| XP bar format string matches "XP: N / M  Rank R" | Req 8.1, 8.2 |
| Old xpLevel and getNextLevelXp are removed (compile test) | Req 13.1, 13.2 |
| Old level-up message does not appear after XP award | Req 13.5 |
| Legacy save without career data loads gracefully | Req 13.8 — removed (no legacy saves exist) |
| Invalid Lua entry (missing name) is skipped without crash | Req 10.5 |

### Integration Tests

| Test | Coverage |
|------|----------|
| Full chargen flow: select homeworld → career → purchase advance → verify player state | Req 9.3 |
| Load Homeworlds.lua with known content, verify all templates parsed | Req 10.1 |
| Load Careers.lua with known content, verify ranks and advances parsed | Req 10.2 |
| Load Skills.lua and Talents.lua, verify definitions parsed | Req 10.3, 10.4 |
| Save/load full game with CareerProgression attached | Req 11.1, 11.2 |

### Test Configuration

- All property tests: minimum 100 iterations (`rc::check` / `rc::prop`)
- Test file: `Tests/test_character_generation.cpp`
- Test tags: `[pbt][property][Feature: character-generation]` for property tests
- Test tags: `[unit][Feature: character-generation]` for example-based tests
- Test tags: `[integration][Feature: character-generation]` for integration tests
