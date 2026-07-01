# Design Document — 40kRL Game Architecture

## Overview

40kRL is a Warhammer 40K themed roguelike written in C++17. It uses **libtcod** for terminal
rendering and input, and **sol2** to embed a Lua 5.x scripting engine for data-driven item
effects. The architecture follows the classic libtcod tutorial pattern — a single global `Engine`
singleton coordinates a flat actor list, a procedurally generated dungeon `Map`, and a HUD overlay
(`Gui`). Components attached to Actors (`Attacker`, `Destructible`, `Ai`, `Pickable`, `Container`)
define each entity's capabilities without inheritance on the Actor class itself. Every persistent
object derives from the `Persistent` abstract base and round-trips through a `TCODZip` binary
archive.

The design priorities are:

1. **Simplicity** — a single-pass synchronous game loop with no multithreading.
2. **Composability** — entity behaviour comes from component combinations, not deep inheritance.
3. **Scriptability** — Lua scripts control item effect values so designers can tune without
   recompiling.
4. **Extensibility** — clear seams for guns, equipment, character generation, Lua AI, and Lua map
   generation, each of which maps to an existing extension point.

---

## Architecture

### System Diagram

```
┌─────────────────────────────────────────────────────────┐
│                        main.cpp                         │
│  Engine engine(80, 50)  ← global singleton              │
│  engine.load()  → main menu / save-file restore         │
│  loop:                                                  │
│    engine.update()   ← input + state machine            │
│    engine.render()   ← map + actors + gui               │
│    TCODConsole::flush()                                 │
│  engine.save()  → on window-close                       │
└────────────────────────┬────────────────────────────────┘
                         │ owns
         ┌───────────────┼───────────────┐
         ▼               ▼               ▼
   ┌──────────┐   ┌────────────┐   ┌──────────┐
   │   Map    │   │ actors     │   │   Gui    │
   │  (BSP)   │   │ list<uptr> │   │  + Menu  │
   │  TCODMap │   │  ┌───────┐ │   │ + Camera │
   │  tiles[] │   │  │Actor  │ │   └──────────┘
   │  scent   │   │  │ + cpts│ │
   └──────────┘   │  └───────┘ │
                  └────────────┘
```

### Game Loop Flow

```
engine.load()
  └─ show main menu (or restore save)
  └─ engine.init() or engine.term() + restore

main loop:
  engine.update()
    ├─ if STARTUP → map->computeFOV()
    ├─ set gameStatus = IDLE
    ├─ checkForEvent (keyboard + mouse)
    ├─ if ESC → save() → load() [show menu]
    ├─ player->update()           ← always (unless STARTUP/VICTORY/DEFEAT)
    │    └─ PlayerAi::update      ← handles input, level-up, sets NEW_TURN
    └─ if NEW_TURN:
         ├─ map->currentScentValue++
         └─ foreach non-player actor → actor->update()
              └─ MonsterAi::update / ConfusedMonsterAi::update

  engine.render()
    ├─ TCODConsole::root->clear()
    ├─ map->render()              ← tiles with camera offset, box-drawing walls
    ├─ foreach actor → actor->render()   ← if in FOV or explored
    └─ gui->render()              ← HUD panel blitted to bottom of screen

  TCODConsole::flush()

on window close:
  engine.save()
```

### State Machine

```
         ┌─────────────────────────────────┐
         │              STARTUP            │
         │  (FOV recomputed, no AI update) │
         └─────────────┬───────────────────┘
                       │ player moves / acts
                       ▼
         ┌─────────────────────────────────┐
         │             NEW_TURN            │◄──┐
         │  (scent++, all monsters update) │   │ player acts
         └─────────────┬───────────────────┘   │
                       │ after monster updates  │
                       ▼                       │
         ┌─────────────────────────────────┐   │
         │              IDLE               │───┘
         │  (only player update runs)      │
         └────┬────────────────────────────┘
              │                  │
        player dies          player wins
              ▼                  ▼
          DEFEAT              VICTORY
```

---

## Components and Interfaces

### Component Overview

Each `Actor` holds five optional `shared_ptr` component slots. A null slot means "does not have
this capability". Components are the only place where type-specific behaviour lives.

| Component      | Role                                              | Stored as         |
|----------------|---------------------------------------------------|-------------------|
| `Attacker`     | Can deal melee damage                             | `shared_ptr`      |
| `Destructible` | Has HP, defence, death behaviour                  | `shared_ptr`      |
| `Ai`           | Has autonomous update behaviour                   | `shared_ptr`      |
| `Pickable`     | Can be picked up, used, dropped; owns TargetSelector + Effect | `shared_ptr` |
| `Container`    | Holds an inventory of Actors                      | `shared_ptr`      |

### Component Ownership Diagram

```
Engine (global singleton)
│
├── actors : std::list<std::unique_ptr<Actor>>
│     │
│     ├── Actor  ←─── engine.player (raw ptr alias)
│     │   ├── shared_ptr<Attacker>
│     │   ├── shared_ptr<Destructible>   ── PlayerDestructible / MonsterDestructible
│     │   ├── shared_ptr<Ai>             ── PlayerAi / MonsterAi / ConfusedMonsterAi
│     │   │                                  ConfusedMonsterAi.oldAi : shared_ptr<Ai>
│     │   ├── shared_ptr<Pickable>
│     │   │   ├── unique_ptr<TargetSelector>
│     │   │   └── unique_ptr<Effect>     ── HealthEffect / AiChangeEffect
│     │   │         AiChangeEffect.newAi : unique_ptr<TemporaryAi>
│     │   └── shared_ptr<Container>
│     │         └── inventory : std::list<std::unique_ptr<Actor>>
│     │
│     └── Actor  ←─── engine.stairs (raw ptr alias)
│
├── map     : std::unique_ptr<Map>
│     ├── tiles   : std::vector<Tile>     (width × height, flat)
│     ├── map     : std::unique_ptr<TCODMap>
│     └── rng     : std::unique_ptr<TCODRandom>
│
├── camera  : std::unique_ptr<Camera>
│
└── gui     : std::unique_ptr<Gui>
      ├── con     : std::unique_ptr<TCODConsole>  (HUD panel)
      ├── log     : std::list<std::unique_ptr<Message>>
      └── menu    : Menu
            └── items : std::list<std::unique_ptr<MenuItem>>
```

### Persistent Interface

Every object that participates in save/load derives from:

```cpp
class Persistent {
public:
    virtual void save(TCODZip& zip) = 0;
    virtual void load(TCODZip& zip) = 0;
};
```

Polymorphic types (`Destructible`, `Ai`, `Effect`) prepend a type-discriminator integer before
their own fields so the corresponding `create(TCODZip&)` factory can reconstruct the correct
subclass. The load path always calls `create` first, then delegates to the base `load` for shared
fields.

---

## Class Hierarchies

### Ai Hierarchy

```
Ai  (abstract: update(Actor*), create(TCODZip&))
│   enum AiType { PLAYER=0, MONSTER, CONFUSED_MONSTER }
│
├── PlayerAi
│     fields: xpLevel
│     update: level-up check → move/attack → handle action keys
│
├── MonsterAi
│     update: moveOrAttack(owner, player.x, player.y)
│     moveOrAttack:
│       distance < 2  → attack
│       in FOV        → step toward player (normalised int delta)
│       out of FOV    → follow scent gradient (8-neighbour, threshold)
│
└── TemporaryAi  (abstract wrapper)
      fields: nbTurns, oldAi : shared_ptr<Ai>
      update: decrement nbTurns; if 0 → owner->ai = move(oldAi)
      applyToActor(unique_ptr<TemporaryAi> self, Actor*):
        self->oldAi = move(actor->ai); actor->ai = move(self)
      │
      └── ConfusedMonsterAi
            update: random move or attack + TemporaryAi::update
```

**Extension point**: add a new `AiType` enum value, subclass `Ai` or `TemporaryAi`, implement
`update` / `save` / `load`, register in `Ai::create`.

### Destructible Hierarchy

```
Destructible  (abstract)
│   fields: maxHp, hp, defense, corpseName, xp
│   methods: takeDamage, heal, die (resets actor to corpse)
│   enum DestructibleType { MONSTER=0, PLAYER }
│
├── MonsterDestructible
│     die: log xp gain, award xp to player, call Destructible::die
│
└── PlayerDestructible
      die: log "You died!", set gameStatus = DEFEAT, call Destructible::die
```

### Effect Hierarchy

```
Effect  (abstract: applyTo(Actor*), create(TCODZip&))
│   enum EffectType { HEALTH=0, CHANGE_AI }
│
├── HealthEffect
│     fields: amount, message, textCol
│     applyTo: creates sol::state, loads Effects.lua, heals/damages actor
│
└── AiChangeEffect
      fields: newAi : unique_ptr<TemporaryAi>, message, textCol
      applyTo: calls TemporaryAi::applyToActor
```

**Extension point**: add a new `EffectType` enum value, subclass `Effect`, add to `Effect::create`.

---

## Data Models

### Tile

```cpp
struct Tile {
    bool         explored;      // has ever been in FOV
    unsigned int scent;         // last currentScentValue when tile was in FOV (minus distance)
};
// Index: tiles[x + y * width]
```

Scent degrades implicitly: a tile's scent is "fresh" if
`tile.scent > map.currentScentValue - SCENT_THRESHOLD` (SCENT_THRESHOLD = 20).

### Actor Fields

| Field         | Type                       | Notes                                  |
|---------------|----------------------------|----------------------------------------|
| `x`, `y`      | `int`                      | World position                         |
| `ch`          | `int`                      | Glyph (ASCII codepoint or TCOD char)   |
| `col`         | `TCODColor`                | Foreground colour                      |
| `name`        | `std::string`              | Display name and corpse name           |
| `blocks`      | `bool`                     | Blocks movement (true for live actors) |
| `fovOnly`     | `bool`                     | Only render when tile is in FOV        |
| `attacker`    | `shared_ptr<Attacker>`     | null if cannot attack                  |
| `destructible`| `shared_ptr<Destructible>` | null if indestructible                 |
| `ai`          | `shared_ptr<Ai>`           | null if no autonomous behaviour        |
| `pickable`    | `shared_ptr<Pickable>`     | null if cannot be picked up            |
| `container`   | `shared_ptr<Container>`    | null if cannot hold inventory          |

### Attacker Fields

| Field   | Type    | Notes                        |
|---------|---------|------------------------------|
| `power` | `float` | Raw damage before defence    |

Damage formula: `effective_damage = power - target.destructible.defense`.

### Destructible Fields

| Field       | Type          | Notes                               |
|-------------|---------------|-------------------------------------|
| `maxHp`     | `float`       | Maximum HP                          |
| `hp`        | `float`       | Current HP                          |
| `defense`   | `float`       | Damage reduction                    |
| `corpseName`| `std::string` | Name after death                    |
| `xp`        | `int`         | XP granted on kill (or player's XP) |

### TargetSelector Fields

| Field   | Type           | Notes                                    |
|---------|----------------|------------------------------------------|
| `type`  | `SelectorType` | SELF / CLOSEST_MONSTER / SELECTED_MONSTER / WEARER_RANGE / SELECTED_RANGE |
| `range` | `float`        | Max range in tiles (0 = unlimited)       |

### Camera Fields

| Field       | Type  | Notes                               |
|-------------|-------|-------------------------------------|
| `x`, `y`    | `int` | Signed offset: screen = world + offset |
| `width`     | `int` | Viewport width in tiles             |
| `height`    | `int` | Viewport height in tiles            |
| `mapWidth`  | `int` | Current map width (for clamping)    |
| `mapHeight` | `int` | Current map height (for clamping)   |

Camera transform: `screen_x = world_x + x`, `world_x = screen_x - x`  
Update: `x = -(player.x) + width/2`, `y = -(player.y) + height/2`.

### Map Fields

| Field               | Type                        | Notes                           |
|---------------------|-----------------------------|---------------------------------|
| `width`, `height`   | `int`                       | Map dimensions                  |
| `currentScentValue` | `unsigned int`              | Monotonically increasing counter |
| `tiles`             | `std::vector<Tile>`         | Flat array, `width × height`    |
| `map`               | `unique_ptr<TCODMap>`       | libtcod FOV / walkability map   |
| `rng`               | `unique_ptr<TCODRandom>`    | Seeded RNG for deterministic gen|
| `seed`              | `long`                      | BSP seed (saved/loaded)         |

---

## Serialisation Stream Layout

The save file `game.sav` is a **TCODZip binary archive**. Fields are written in a strict order;
loading must mirror this order exactly.

```
─── game.sav ────────────────────────────────────────────────────────────────
 1. int       map.width
 2. int       map.height
 3. Map::save
      int     seed
      foreach tile (width*height):
        int   tile.explored
        int   tile.scent
      int     currentScentValue
 4. Camera::save
      int  x, y, width, height, mapWidth, mapHeight
 5. Actor::save  [player]
      int     x, y, ch
      color   col
      string  name
      int     blocks, fovOnly
      int     hasAttacker, hasDestructible, hasAi, hasPickable, hasContainer
      [Attacker::save   → float power]
      [Destructible::save → int type, float maxHp/hp/defense, string corpseName, int xp]
      [Ai::save          → int type, ...]
      [Pickable::save    → Effect::save (int type, ...), int hasSelector, TargetSelector::save]
      [Container::save   → int size, int count, foreach Actor::save]
 6. Actor::save  [stairs]   (same layout as above, minimal fields)
 7. int       actorCount  (= actors.size() - 2, excludes player and stairs)
 8. foreach remaining actor: Actor::save
 9. Gui::save
      int   logSize
      foreach message: string text, color col

─── Polymorphic discriminators ───────────────────────────────────────────────
  Destructible type: MONSTER=0, PLAYER=1
  Ai type:           PLAYER=0, MONSTER=1, CONFUSED_MONSTER=2
  Effect type:       HEALTH=0, CHANGE_AI=1

  ConfusedMonsterAi payload:
    int   type  (CONFUSED_MONSTER=2)
    int   nbTurns
    int   hasOldAi
    [Ai::save  for oldAi — recursive, with its own type discriminator]
```

**Load invariant**: `Engine::load` calls `engine.term()` before constructing new objects to
prevent double-ownership. Player and stairs are emplaced first so that raw-pointer aliases
(`engine.player`, `engine.stairs`) are valid before other actors are loaded.

---

## Rendering and Camera

### Viewport Mapping

The map is 160×86 tiles; the screen is 80×50 with a 7-row HUD panel at the bottom, giving a
viewport of 80×43 for the dungeon. Camera keeps the player centred:

```
screen_x = world_x + camera.x     where camera.x = -(player.world_x) + 40
screen_y = world_y + camera.y     where camera.y = -(player.world_y) + 21
```

Tiles with `screen_x` or `screen_y` outside `[0, viewport_w)` / `[0, viewport_h)` are silently
skipped — libtcod clips out-of-bounds `setChar` calls.

### Wall Rendering

Walls are rendered using libtcod double-line box-drawing characters. The character is chosen by
testing the four cardinal neighbours with `isWall(x, y±1)` and `isExplorable(x±1, y)` to produce
the correct T-junction, corner, or straight-line glyph. Unexplored tiles are never rendered.

### GUI Layout

```
Screen (80 wide × 50 tall)
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│                        Dungeon viewport (80 × 43)                          │
│                                                                             │
├──────────────────┬──────────────────────────────────────────────────────────┤  row 43
│ HP bar  (col 1)  │  XP bar  (col 1, row 5)                                 │
│ Dungeon level    │  Message log (MSG_X = BAR_WIDTH+2, rows 1..5)           │
│                  │  Mouse-look  (col 1, row 0)                             │
└──────────────────┴──────────────────────────────────────────────────────────┘
                    PANEL_HEIGHT = 7 rows
```

Constants (from `constants.h`):

| Constant           | Value         |
|--------------------|---------------|
| `PANEL_HEIGHT`     | 7             |
| `BAR_WIDTH`        | 20            |
| `MSG_X`            | 22 (BAR_WIDTH + 2) |
| `MSG_HEIGHT`       | 5 (PANEL_HEIGHT - 2) |
| `PAUSE_MENU_WIDTH` | 30            |
| `PAUSE_MENU_HEIGHT`| 15            |

---

## Lua Scripting Boundary

### Current Usage

`HealthEffect::applyTo` is the only current call-site. It creates a `sol::state` per invocation,
registers two usertypes, sets two globals, and executes `Scripts/Effects.lua`.

### Exposed API (as of current build)

```lua
-- Globals injected by HealthEffect::applyTo
act      -- Actor usertype instance (the target)
amount   -- float, the heal amount configured on the item

-- Actor usertype fields / methods visible to Lua
act.name           -- string (read/write)
act.destructible   -- Destructible usertype instance
act.ai             -- shared_ptr<Ai> (accessible but no methods bound)

-- Destructible usertype methods
act.destructible:heal(amount)  -- returns float: actual HP restored
```

`Effects.lua` current body:
```lua
local healAmount = act.destructible:heal(amount)
return healAmount
```

### Error Handling Contract

`HealthEffect::applyTo` does not yet wrap `lua.script_file` in a try/catch. The planned
contract (Requirement 7.3) is:

```cpp
try {
    float result = lua.script_file("Scripts/Effects.lua");
    // use result
} catch (const sol::error& e) {
    engine.gui->message(TCOD_red, "Lua error: #", e.what());
    // return false — item not consumed
}
```

This must be added before Lua scripting is considered stable.

### Planned Lua API Extensions

| System                | Script file           | Data passed in                          |
|-----------------------|-----------------------|-----------------------------------------|
| Enemy definitions     | `Scripts/Enemies.lua` | Actor factory callback                  |
| Item definitions      | `Scripts/Items.lua`   | Item factory callback                   |
| Map generation        | `Scripts/Map.lua`     | Map primitives: createRoom, addMonster  |
| AI behaviours         | `Scripts/AiXxx.lua`   | Actor owner, engine query functions     |

When these systems are added, the Lua state should be initialised once in `Engine::init()` and
reused, not recreated per item use.

---

## Extension Points for Planned Systems

### 1. Guns / Ranged Combat

Add a `Ranged` component (`shared_ptr<Ranged>` on `Actor`) with fields: `weaponRange`, `damage`,
`ammo`. Add `EffectType::PROJECTILE` and a `ProjectileEffect` subclass. `TargetSelector` already
supports `SELECTED_MONSTER` and `SELECTED_RANGE` — these cover most ranged use-cases. Register
`RANGED` in the `AiType` or extend `MonsterAi` to prefer ranged attack when distance allows.

### 2. Equipment / Wearable Items

Add an `Equippable` component with an equipment slot enum (head, body, weapon, offhand). Add a
`worn` slot map to the player actor (or a dedicated `Equipment` component). `Container` already
holds inventory items as `unique_ptr<Actor>` — a separate equipped-items map can alias the same
objects via raw pointer or a parallel list. Serialise with a new `ComponentType` discriminator.

### 3. Character Generation

At `Engine::init`, before `map->init`, display a character-generation `Menu` collecting name,
background, and starting stats. The chosen values feed directly into `PlayerDestructible` and
`Attacker` parameters. No structural change to any existing class is needed.

### 4. NPC Dialogue

Add a `Dialogue` component holding a Lua script path and a conversation-state integer. `PlayerAi`
checks `target->dialogue` before attacking — if non-null, trigger dialogue instead of combat.
Dialogue trees can be fully defined in Lua using the existing sol2 binding.

### 5. Lua AI

Subclass `Ai` as `LuaAi` with a `std::string scriptPath` field. `LuaAi::update` calls the script,
passing the owner Actor via the existing `act` global pattern. Register `AiType::LUA` and add to
`Ai::create`. No change to `Actor` or `Engine` is needed.

### 6. Lua Map / Actor / Item Generation

Replace the hard-coded `BspListener::createRoom` population calls with a Lua callback that
receives `x1, y1, x2, y2` and calls back into `Map::addMonster` and `Map::addItem` C++ functions
exposed to Lua. The primitives already exist — only the wiring to Lua needs adding.

### 7. World Generation / Outdoor Regions

Add an `outdoor` flag to `Map` and a separate `WorldMap` class (a coarser grid of region
descriptors). `Engine::nextLevel` checks this flag and either generates a dungeon floor (current
path) or delegates to the world-map transition logic.

### 8. Stairs Up / Multiple Levels

`Engine` currently tracks `level` (int). Add `Engine::prevLevel()` mirroring `nextLevel()`. Store
the player's entry position when descending so ascending restores it. `Map` load/save is unchanged.

---

## Error Handling

| Scenario                              | Current behaviour                      | Required behaviour (per requirements)     |
|---------------------------------------|----------------------------------------|-------------------------------------------|
| Lua script file not found             | Uncaught exception propagates          | Catch `sol::error`, log via Gui, return false |
| Lua script runtime error              | Uncaught exception propagates          | Same as above                             |
| Save file missing at load             | "Continue" item not shown in menu      | ✓ Already correct                         |
| Player dead at save time              | `remove("game.sav")` before write      | ✓ Already correct                         |
| Out-of-bounds tile query              | `isInFOV`: returns false; `isWall`: delegate to TCODMap (may crash) | Add bounds check to `isWall` |
| Item with no valid targets            | Gui message "No enemy is close enough" | ✓ Already correct (Req 5.6)               |
| Iterator invalidation during erase    | `erase` returns new iterator; used correctly in most loops | Review all actor-list loops |
| ConfusedMonsterAi turns reach zero    | `owner->ai = move(oldAi)` in TemporaryAi::update | ✓ Correct                  |

---

## Testing Strategy

### PBT Applicability Assessment

This feature is primarily an **architecture documentation spec** covering an existing C++ codebase.
The codebase contains significant business logic in pure or near-pure functions — particularly the
serialisation round-trip, the combat damage formula, the scent-tracking logic, and the item
targeting system — which are all amenable to property-based testing. UI rendering (libtcod output),
infrastructure (file I/O, Lua VM initialisation), and BSP map generation (seed-determinism, not
transformation logic) are not.

The chosen property-based testing library for C++ is **[RapidCheck](https://github.com/emil-e/rapidcheck)** — a well-maintained, header-friendly C++ PBT library that integrates cleanly with most test runners (catch2, gtest, doctest). Each property test must run a minimum of 100 iterations.

### Unit Tests

Unit tests should cover:

- `Attacker::attack` — correct damage reduction, zero-damage case, dead-target no-op.
- `Destructible::takeDamage` — damage clamped to zero when `defense >= power`.
- `Destructible::heal` — HP capped at `maxHp`; returned value is actual amount healed.
- `MonsterAi::moveOrAttack` — attacks at distance < 2, moves at distance ≥ 2 when in FOV.
- `TemporaryAi` — `nbTurns` countdown and `oldAi` restoration.
- `TargetSelector::selectTargets` — each of the five selection modes with mock actors.
- `Camera::apply` / `Camera::getWorldLocation` — round-trip.
- `Map::isInFOV` / `canWalk` — out-of-bounds returns safe defaults.
- `Gui::message` — log size bounded by `MSG_HEIGHT`, oldest entry discarded.
- `Engine::save` / `Engine::load` — integration test with a temp file.

### Property-Based Tests

See **Correctness Properties** section for full property statements. Tag format:
`// Feature: game-architecture, Property N: <property text>`

Property tests run 100+ iterations via RapidCheck `rc::check(...)`.

### Integration Tests

- Full save/load cycle: `engine.init()` → populate → `engine.save()` → `engine.term()` →
  `engine.load()` → verify actor count, player position, map seed, log size.
- Lua effect execution: verify `Effects.lua` heals correct amount for a range of `amount` values.

---


## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a
system — essentially, a formal statement about what the system should do. Properties serve as the
bridge between human-readable specifications and machine-verifiable correctness guarantees.*

---

### Property 1: Scent counter increments by exactly one per NEW_TURN

*For any* starting value of `map.currentScentValue`, when `Engine::update` is called with
`gameStatus == NEW_TURN`, `map.currentScentValue` SHALL equal its previous value plus exactly 1.

**Validates: Requirements 1.3, 3.7**

---

### Property 2: Actor update delegates to Ai component

*For any* Actor that has a non-null `ai` component, calling `actor.update()` SHALL invoke
`ai->update(actor)` exactly once; calling `actor.update()` on an Actor with a null `ai` SHALL
produce no side effects.

**Validates: Requirements 2.2, 2.3**

---

### Property 3: Camera transform is a bijection

*For any* world coordinate `(wx, wy)` and any camera offset `(cx, cy)`,
`camera.apply(wx, wy)` SHALL return `(wx + cx, wy + cy)`, and
`camera.getWorldLocation(wx + cx, wy + cy)` SHALL return `(wx, wy)`.

This property covers both the Actor render path (which calls `camera.apply`) and the tile render
path.

**Validates: Requirements 2.5, 8.1**

---

### Property 4: Camera centres on the player after update

*For any* player world position `(px, py)` and camera with dimensions `(width, height)`,
after `camera.update(player)`:
- `camera.x == -(px) + width/2`
- `camera.y == -(py) + height/2`

**Validates: Requirements 8.2**

---

### Property 5: BSP map generation is deterministic

*For any* seed value, creating two `Map` instances with the same seed and calling `init(false)` on
both SHALL produce tile vectors that are element-wise identical (same `explored` flags, same
`isWall` results from the underlying `TCODMap`).

**Validates: Requirements 3.1**

---

### Property 6: Map tile vector size invariant

*For any* valid map dimensions `(width, height)`, after `Map::init()`, `tiles.size()` SHALL equal
`width * height` exactly.

**Validates: Requirements 3.2**

---

### Property 7: canWalk returns false for walls and blocked cells

*For any* map coordinate `(x, y)`:
- If `map.isWall(x, y)` is true, then `map.canWalk(x, y)` SHALL return false.
- If any `Actor` with `blocks == true` occupies `(x, y)`, then `map.canWalk(x, y)` SHALL return
  false.
- If neither condition holds, `map.canWalk(x, y)` SHALL return true.

**Validates: Requirements 3.3**

---

### Property 8: computeFOV scent is monotonically non-decreasing for in-FOV tiles

*For any* in-FOV tile at `(x, y)` after `map.computeFOV()` is called, the new scent value SHALL
satisfy:
```
new_scent == max(old_scent, currentScentValue - distance(player, x, y))
```
i.e. scent on a tile never decreases as a result of FOV computation.

**Validates: Requirements 3.4**

---

### Property 9: Out-of-bounds map queries return safe defaults

*For any* coordinate `(x, y)` where `x < 0`, `x >= width`, `y < 0`, or `y >= height`:
- `map.isInFOV(x, y)` SHALL return `false`
- `map.isExplored(x, y)` SHALL return `false`
- `map.isWall(x, y)` SHALL not crash (and shall return a wall/impassable result)

**Validates: Requirements 3.6**

---

### Property 10: MonsterAi moves toward player using normalised integer deltas when player is in FOV

*For any* monster position `(mx, my)` and player position `(px, py)` where the player is in FOV
and the Euclidean distance is ≥ 2, after `MonsterAi::moveOrAttack`, the monster's new position
SHALL equal:
```
(mx + round(dx/dist), my + round(dy/dist))
```
where `dx = px - mx`, `dy = py - my`, `dist = sqrt(dx²+dy²)`, provided the target cell is
walkable.

**Validates: Requirements 4.2**

---

### Property 11: MonsterAi follows the highest-scent walkable neighbour when player is out of FOV

*For any* monster position `(mx, my)` with a set of 8 neighbours having arbitrary walkability and
scent values, when the player is not in FOV, `MonsterAi::moveOrAttack` SHALL move the monster to
the walkable neighbour `n` that maximises `scent(n)` subject to
`scent(n) > currentScentValue - SCENT_THRESHOLD`; if no such neighbour exists, the monster SHALL
not move.

**Validates: Requirements 4.3**

---

### Property 12: TemporaryAi restores oldAi after exactly nbTurns updates

*For any* initial `nbTurns` value `N > 0`, after calling `TemporaryAi::update(owner)` exactly `N`
times, `owner->ai` SHALL equal the `oldAi` that was captured by `applyToActor`, and the
`TemporaryAi` instance SHALL no longer own itself (i.e., `owner->ai` is the restored original Ai).

**Validates: Requirements 4.5**

---

### Property 13: Pickable::use applies Effect to every selected target

*For any* `Pickable` item with a given `TargetSelector` and `Effect`, and *for any* non-empty set
of targets returned by `selectTargets`, `Pickable::use` SHALL call `effect->applyTo(actor)` for
each target in the returned list.

**Validates: Requirements 5.2**

---

### Property 14: Successful use removes item from inventory

*For any* inventory containing a `Pickable` item, when `Pickable::use` returns true (at least one
target successfully affected), the item SHALL no longer appear in the wearer's `container->inventory`
afterwards, and the inventory size SHALL be exactly one less than before.

**Validates: Requirements 5.3**

---

### Property 15: Pickable::drop transfers Actor from inventory to Engine::actors

*For any* inventory state where a specific `Actor` `item` is present, after calling
`item->pickable->drop(item, wearer)`:
- `item` SHALL appear in `Engine::actors`
- `item` SHALL no longer appear in `wearer->container->inventory`

**Validates: Requirements 5.7**

---

### Property 16: Serialisation round-trip preserves all game state

*For any* valid game state (any collection of actors with any combination of components, any map
seed, any camera offset, any message log), calling `Engine::save()` followed by `Engine::load()`
(selecting "Continue") SHALL reproduce:
- All actors with identical field values and component presence flags
- The map seed (and therefore tile layout after `init`)
- The camera offset
- The message log contents and colours

**Validates: Requirements 6.2, 6.3, 8.3**

---

### Property 17: Polymorphic components write their type discriminator as the first integer

*For any* instance of a polymorphic component (`MonsterDestructible`, `PlayerDestructible`,
`PlayerAi`, `MonsterAi`, `ConfusedMonsterAi`, `HealthEffect`, `AiChangeEffect`), the first `int`
written by `save(zip)` SHALL equal the corresponding enum value in `DestructibleType`, `AiType`,
or `EffectType`.

**Validates: Requirements 6.4**

---

### Property 18: Lua HealthEffect heal amount is capped at remaining HP

*For any* actor with current HP `h` and maximum HP `m` (where `h <= m`), and *for any* positive
heal `amount`, the value returned by `act.destructible:heal(amount)` (as executed through
`Effects.lua`) SHALL equal `min(amount, m - h)`, and the actor's HP after the call SHALL equal
`min(h + amount, m)`.

**Validates: Requirements 7.2**

---

### Property 19: Mouse-look includes all Actor names at cursor position

*For any* set of Actors whose world position equals the cursor's world position (when the cursor
tile is in FOV), the string produced by `Gui::renderMouseLook` SHALL contain each of those
Actors' `name` fields.

**Validates: Requirements 8.5**

---

### Property 20: Message log bounded by MSG_HEIGHT

*For any* sequence of `N` messages sent to `Gui::message` where `N > MSG_HEIGHT`, after all
messages are processed `log.size()` SHALL equal `MSG_HEIGHT`, and the log SHALL contain exactly
the last `MSG_HEIGHT` messages in the order they were sent (oldest discarded from the front).

**Validates: Requirements 8.6**

---
