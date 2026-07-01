# Requirements Document

## Introduction

This document captures the architectural requirements for 40kRL — a Warhammer 40K themed roguelike
written in C++ with libtcod for rendering and sol2/Lua for data-driven scripting. The architecture
must describe how the current system is structured, why the major design decisions were made, and
what constraints must hold when the codebase is extended. It serves both as a reference for the
existing design and as a blueprint for future contributors.

The scope covers the core game loop, the component-based Actor system, the Map/FOV/scent subsystem,
the UI layer (Gui + Camera), the serialization layer (Persistent/TCODZip), the Lua scripting
boundary, and the roadmap of planned systems.

## Glossary

- **Actor**: The central game object. Any entity that can exist on the map — player, monster, item,
  corpse, stairs — is an Actor. Actors have optional components attached via `shared_ptr`.
- **Component**: An optional capability object attached to an Actor (`Attacker`, `Destructible`,
  `Ai`, `Pickable`, `Container`). An Actor's set of non-null components defines its role.
- **Engine**: The global singleton that owns the game loop, the actor list, the map, the camera, the
  GUI, and the current input state.
- **Map**: The dungeon floor, generated via BSP, containing tile data, the libtcod FOV map, and per-
  tile scent values.
- **Gui**: The HUD panel (HP/XP bars, message log, mouse-look tooltip) and the `Menu` overlay for
  main menu and level-up choices.
- **Camera**: A viewport offset that maps world coordinates to screen coordinates, keeping the
  player centred.
- **Persistent**: The abstract base class for every object that participates in save/load, exposing
  `save(TCODZip&)` and `load(TCODZip&)`.
- **TCODZip**: The libtcod binary archive used for serialisation.
- **TemporaryAi**: An Ai subclass that wraps another Ai and reverts to it after a fixed number of
  turns.
- **TargetSelector**: Encapsulates item targeting logic (self, closest monster, selected monster,
  range around wearer, selected-tile range).
- **Effect**: Encapsulates the outcome of using a `Pickable` item (`HealthEffect`, `AiChangeEffect`).
- **Lua / sol2**: Lua 5.x scripts loaded at runtime via the sol2 C++ binding; currently used for
  the heal-effect calculation in `Effects.lua`.
- **BSP**: Binary Space Partitioning tree used by libtcod to procedurally generate rooms and
  corridors.
- **FOV**: Field-of-View, computed by libtcod's `TCODMap::computeFov`. Determines what the player
  can see each turn.
- **Scent**: An unsigned integer stamped on each in-FOV tile each turn; monsters follow the gradient
  when the player is out of sight.
- **GameStatus**: Engine state machine value — `STARTUP`, `IDLE`, `NEW_TURN`, `VICTORY`, `DEFEAT`.

---

## Requirements

### Requirement 1: Single-Pass Game Loop

**User Story:** As a developer maintaining 40kRL, I want a clear, single-pass game loop, so that
I can reason about update order, rendering order, and input handling without ambiguity.

#### Acceptance Criteria

1. THE Engine SHALL process one frame per iteration of the main loop: read input, update game state,
   render, flush.
2. WHEN `gameStatus` is `STARTUP`, THE Engine SHALL recompute FOV before the first rendered frame
   and before processing any player input.
3. WHEN `gameStatus` is `NEW_TURN`, THE Engine SHALL increment the map scent counter and update all
   non-player Actors in list order.
4. WHEN `gameStatus` is `IDLE`, THE Engine SHALL not update any non-player Actor.
5. WHILE `gameStatus` is `NEW_TURN` or `IDLE`, THE Engine SHALL update the player Actor each frame;
   THE Engine SHALL not update the player Actor while `gameStatus` is `STARTUP`, `VICTORY`, or
   `DEFEAT`.
6. WHEN the player presses `ESCAPE`, THE Engine SHALL save the current game state and display the
   main menu before resuming or starting a new game.
7. WHEN the window is closed normally, THE Engine SHALL save the current game state before exit.

---

### Requirement 2: Component-Based Actor System

**User Story:** As a developer adding new entity types, I want a composable component system, so
that I can create new actors by combining existing components without writing new classes.

#### Acceptance Criteria

1. THE Actor SHALL hold optional shared_ptr components for `Attacker`, `Destructible`, `Ai`,
   `Pickable`, and `Container`; a null shared_ptr indicates the component is absent.
2. WHEN an Actor's `Ai` component is non-null, THE Actor SHALL delegate `update()` to that
   component.
3. WHEN an Actor's `Ai` component is null, THE Actor's `update()` SHALL be a no-op.
4. THE Actor SHALL store position (`x`, `y`), display glyph (`ch`), colour (`col`), name, `blocks`
   flag, and `fovOnly` flag as first-class fields, not as components.
5. WHEN an Actor is rendered, THE Actor SHALL apply the Camera transform to convert world
   coordinates to screen coordinates before drawing.
6. WHEN an Actor is added to `Engine::actors`, THE Engine SHALL own it via `unique_ptr`; the Actor
   itself MAY be referenced by raw pointer elsewhere (e.g., `engine.player`, `engine.stairs`).

---

### Requirement 3: Map Generation and Spatial Queries

**User Story:** As a developer extending level generation, I want the Map system to be the sole
authority on spatial data, so that all passability, FOV, and scent queries go through a single
interface.

#### Acceptance Criteria

1. THE Map SHALL generate dungeon layouts using a BSP tree; each run with the same seed SHALL
   produce the same layout.
2. THE Map SHALL maintain a flat `vector<Tile>` of exactly `width * height` elements, indexed as
   `x + y * width`; the vector size SHALL remain `width * height` regardless of coordinate
   validity.
3. WHEN `Map::canWalk(x, y)` is called, THE Map SHALL return false if the tile is a wall or if any
   blocking Actor occupies that coordinate.
4. WHEN `Map::computeFOV()` is called, THE Map SHALL recompute the libtcod FOV from the player's
   current position and update per-tile scent values for all in-FOV tiles.
5. THE Map SHALL render walls using box-drawing characters derived from the explored/explorable
   state of adjacent wall tiles.
6. IF a requested tile coordinate is outside map bounds, THEN THE Map SHALL return safe defaults
   (wall for `isWall`, false for `isInFOV`, false for `isExplored`).
7. THE Map SHALL expose `currentScentValue` as an unsigned integer counter incremented once per
   `NEW_TURN`; scent on a tile degrades implicitly as `currentScentValue` grows away from the
   stored tile value.

---

### Requirement 4: AI and Movement

**User Story:** As a developer adding new enemy behaviours, I want a well-defined Ai interface, so
that I can add new behaviour types by subclassing Ai without changing Actor or Engine.

#### Acceptance Criteria

1. THE Ai base class SHALL declare a pure virtual `update(Actor* owner)` method that each subclass
   implements.
2. WHEN a `MonsterAi` is updated and the player is within FOV, THE MonsterAi SHALL move one step
   directly towards the player using normalised integer deltas.
3. WHEN a `MonsterAi` is updated and the player is not within FOV, THE MonsterAi SHALL follow the
   scent gradient by moving to the walkable neighbour with the highest scent value above
   `currentScentValue - SCENT_THRESHOLD`.
4. WHEN a monster is at distance >= 1 and distance < 2 (i.e., strictly adjacent) to its target,
   THE MonsterAi SHALL attack instead of moving; WHEN distance is 0 (same tile), THE MonsterAi
   SHALL not attack and SHALL not move.
5. WHEN `TemporaryAi::update` is called and `nbTurns` reaches zero, THE TemporaryAi SHALL restore
   the previous Ai by assigning `oldAi` back to `owner->ai`.
6. WHEN `TemporaryAi::applyToActor` is called, THE TemporaryAi SHALL take ownership of the Actor's
   existing Ai into `oldAi` before assigning itself to `owner->ai`.
7. THE `PlayerAi` SHALL handle level-up by displaying the level-up menu and applying the chosen
   stat bonus before processing movement input on the same frame.

---

### Requirement 5: Item System (Pickable, TargetSelector, Effect)

**User Story:** As a developer adding new item types, I want a composable item system, so that new
items can be defined by pairing a TargetSelector with an Effect without writing new Pickable
subclasses.

#### Acceptance Criteria

1. THE `Pickable` component SHALL own a `TargetSelector` and an `Effect` via `unique_ptr`.
2. WHEN `Pickable::use` is called, THE Pickable SHALL delegate target selection to its
   `TargetSelector`, then apply its `Effect` to each selected target.
3. WHEN an Effect is successfully applied to at least one target, THE Pickable SHALL remove itself
   from the wearer's inventory.
4. THE `TargetSelector` SHALL support five selection modes: `SELF`, `CLOSEST_MONSTER`,
   `SELECTED_MONSTER`, `WEARER_RANGE`, and `SELECTED_RANGE`.
5. WHEN a `TargetSelector` of mode `SELECTED_MONSTER` or `SELECTED_RANGE` is active, THE Engine
   SHALL enter tile-pick mode, highlighting in-range tiles and waiting for a mouse click before
   returning targets.
6. IF no valid targets are found by a `TargetSelector` AND the Effect fails to apply to any target,
   THEN THE Gui SHALL display a "no enemy close enough" message and THE Pickable SHALL not consume
   the item.
7. WHEN `Pickable::drop` is called, THE Pickable SHALL transfer the Actor from the wearer's
   inventory to `Engine::actors` at the wearer's current position.

---

### Requirement 6: Serialisation Contract

**User Story:** As a player, I want save and load to work reliably, so that my progress is
preserved between sessions.

#### Acceptance Criteria

1. THE `Persistent` interface SHALL require every participating class to implement both `save` and
   `load` with a `TCODZip&` parameter.
2. WHEN saving, THE Engine SHALL write data in this fixed order: map dimensions, map data, camera
   data, player actor, stairs actor, count of remaining actors, remaining actors, GUI message log.
3. WHEN loading, THE Engine SHALL read data in the same order as it was saved, reconstructing
   objects in the reverse of destruction order.
4. WHEN a polymorphic component (Destructible, Ai, Effect) is saved, THE component SHALL write a
   type discriminator integer as the first byte of its payload so that the static `create` factory
   can reconstruct the correct subclass on load.
5. IF the save file does not exist, THEN THE Engine SHALL not offer "Continue" in the main menu.
6. WHEN saving is requested, THE Engine SHALL check whether the player is dead before writing any
   data; IF the player is dead, THE Engine SHALL delete any existing save file and SHALL not begin
   writing a new save file.
7. WHEN a `ConfusedMonsterAi` is saved, THE serializer SHALL write the remaining turn count and
   optionally serialize the stored `oldAi` with its own type discriminator.

---

### Requirement 7: Lua Scripting Boundary

**User Story:** As a designer configuring item effects, I want item effects to be expressible in
Lua, so that I can tune values without recompiling the game.

#### Acceptance Criteria

1. THE Engine SHALL initialise a `sol::state` Lua context on demand (currently per-item-use) and
   expose only the types and values required for the current effect script.
2. WHEN a `HealthEffect` with a positive amount is applied, THE HealthEffect SHALL load and execute
   `Scripts/Effects.lua`, passing the target Actor and the heal amount, and SHALL use the returned
   heal amount as the actual heal value.
3. WHEN a Lua script file cannot be loaded or contains a runtime error, THE HealthEffect SHALL
   handle the exception and log an error message via Gui rather than propagating the exception to
   the caller.
4. THE Lua API surface SHALL be documented in this architecture document so that script authors
   know which C++ types and methods are available.
5. WHERE future Lua scripts define enemies or map generation, THE Engine SHALL load those scripts
   during `init()` or `Map::init()` rather than at individual use sites.

---

### Requirement 8: Rendering and Camera

**User Story:** As a player, I want the viewport to follow the player smoothly, so that I can always
see the area around my character.

#### Acceptance Criteria

1. THE Camera SHALL convert world coordinates to screen coordinates by applying a signed offset
   centred on the player's world position.
2. WHEN the player moves, THE Camera SHALL update its offset so that the player remains at the
   centre of the viewport.
3. THE Camera SHALL be saved and restored as part of the session save file.
4. THE Gui SHALL render a HUD panel at the bottom of the screen containing: HP bar, XP bar,
   dungeon-level indicator, message log, and mouse-look tooltip.
5. WHEN the mouse cursor is within the FOV, THE Gui SHALL display the names of all Actors at the
   cursor's world position in the mouse-look area.
6. THE message log SHALL hold at most `constants::MSG_HEIGHT` messages; older messages SHALL be
   discarded when the log is full.
7. THE Map render function SHALL use the Camera offset to determine which world tiles map to which
   screen columns and rows.

---

### Requirement 9: Memory Ownership Model

**User Story:** As a developer maintaining or extending 40kRL, I want a consistent ownership
model, so that I can avoid use-after-free bugs and double-frees when actors are created, moved, or
destroyed.

#### Acceptance Criteria

1. THE Engine SHALL own all live Actors via `std::unique_ptr<Actor>` stored in
   `Engine::actors` (a `std::list`).
2. WHEN a raw pointer alias to an Actor is stored (e.g., `engine.player`, `engine.stairs`), THE
   corresponding `unique_ptr` in `Engine::actors` SHALL remain valid for the lifetime of the alias.
3. WHEN an Actor is moved between `Engine::actors` and a `Container::inventory`, THE move SHALL
   transfer the `unique_ptr` ownership; no raw pointer alias to that Actor SHALL be retained across
   the move.
4. THE Actor components SHALL be stored as `shared_ptr` to permit safe reference-sharing; a
   component SHALL be released by resetting its `shared_ptr` (e.g., `owner->ai.reset()`).
5. WHEN `TemporaryAi` stores a previous Ai, IT SHALL use `shared_ptr<Ai>` for `oldAi`; the
   transition back to `oldAi` SHALL be effected by move-assigning `oldAi` into `owner->ai`.
6. IF an Actor's `unique_ptr` in `Engine::actors` is erased during iteration, THE iterator SHALL
   be updated to the value returned by `erase` before continuing.

---

### Requirement 10: Extensibility and Future Systems

**User Story:** As a developer implementing v1.0 features, I want the architecture to define clear
extension points, so that I can add guns, equipment, character generation, NPC AI, stairs up, and
Lua-driven map/actor/AI generation without restructuring the existing codebase.

#### Acceptance Criteria

1. THE Actor component model SHALL support new component types (e.g., `Equippable`, `Ranged`,
   `Dialogue`) by adding new `shared_ptr` fields to Actor and new `Persistent` subclasses, without
   modifying existing components.
2. THE Ai subclass hierarchy SHALL support new behaviour types by subclassing `Ai` (or
   `TemporaryAi`) and registering a new `AiType` enum value, without modifying existing Ai
   subclasses.
3. WHERE Lua map generation is added, THE Map SHALL delegate room/actor placement to a Lua script
   instead of the hard-coded `BspListener`, using the same `Map::createRoom` and `Map::addMonster`
   primitives.
4. WHERE Lua actor generation is added, THE Engine SHALL replace the hard-coded `addMonster` /
   `addItem` calls with Lua-driven factory calls that return fully-initialised Actor configurations.
5. WHEN a new item Effect type is added, THE developer SHALL add a new `EffectType` enum value and
   a new subclass of `Effect` with corresponding `save`/`load` support in `Effect::create` before
   the new Effect type is made available to the item system; partial implementations where the
   subclass exists but is not yet registered in `Effect::create` are permitted during development.
6. THE architecture document SHALL list all currently planned systems (guns, equipment, character
   generation, NPC dialogue, Lua Ai, world generation, outdoor regions) and describe how each
   maps to extension points in the existing design.
