# Design Document: Equipment System

## Overview

The equipment system extends the existing component-based Actor architecture with an `Equippable` component that enables items to be worn or wielded in designated body slots. When equipped, items apply stat modifiers (power, defense, maxHp) to the player. The system integrates with the existing `Container` (inventory), `Attacker` (damage), `Destructible` (defense/HP), and `Persistent` (save/load) subsystems.

Phase 1 targets melee weapons that increase attack power. The architecture accommodates future armor slots and a shop system by storing item weight and gold value on all items from the outset. Equipment can also modify the wielder's hit chance via the `Attacker` modifier system — positive skill modifiers (e.g., weapon optics) improve accuracy, while negative modifiers (e.g., heavy armor) reduce it.

### Design Goals

- Minimal coupling: the Equippable component is optional — existing items remain unchanged
- Data-driven: weapon definitions live in Lua, matching the existing `Items.lua` pattern
- Consistent serialization: equipment state integrates into the existing TCODZip save/load order
- Weight system as a strategic constraint without adding tedious micromanagement

## Architecture

```mermaid
graph TD
    subgraph Actor Components
        A[Actor] --> P[Pickable]
        A --> E[Equippable]
        A --> D[Destructible]
        A --> AT[Attacker]
        A --> C[Container]
    end

    subgraph Equipment System
        ES[EquipmentSlots] --> |weapon| SLOT_W[slot: Actor*]
        ES --> |offhand| SLOT_O[slot: Actor*]
        ES --> |head| SLOT_H[slot: Actor*]
        ES --> |body| SLOT_B[slot: Actor*]
    end

    subgraph Stat Flow
        AT -->|base power| EPC[EffectivePowerCalc]
        E -->|power modifier| EPC
        EPC -->|effective power| DMG[Damage Formula]
        E -->|skill modifier| ATTMOD[Attacker::addModifier/removeModifier]
        ATTMOD -->|modifiers vector| THR[computeThreshold]
        THR -->|effective skill| HIT[Hit Check d100]
    end

    C -->|inventory items| ES
    ES -->|unequipped items| C
    A -->|player owns| ES
```

The player Actor gains an `EquipmentSlots` structure (not a separate component — it lives inside `Container` or as a peer pointer set on the player). Each slot holds a raw pointer to an Actor that is **also** stored in the player's inventory list (equipped items remain owned by the Container). This avoids ownership ambiguity — the Container continues to own all item Actors; the EquipmentSlots merely track which inventory item occupies each slot.

**Rationale:** Keeping equipped items in the Container's `inventory` list simplifies save/load (they serialize with the container) and weight calculations (all carried items are in one place). The slot pointers are lightweight metadata serialized alongside.

### Skill Modifier Integration with Attacker

When an item is equipped that has a non-zero `skill` modifier, the `Equipment::equip()` method calls `owner->attacker->addModifier(skill)` to push the value onto the Attacker's transient `modifiers` vector. This modifier is then included in `computeThreshold()` (which sums all modifiers and clamps to [1, 99]) for all subsequent hit checks.

When that item is unequipped, `Equipment::unequip()` calls `owner->attacker->removeModifier(skill)` to erase the first occurrence of that value from the vector.

**Key behaviors:**
- If `skill == 0`, no modifier is added or removed (avoids polluting the vector with zero-value entries)
- Multiple equipped items with skill modifiers each add their own entry independently
- Positive values (e.g., weapon optics: +10) increase hit chance
- Negative values (e.g., heavy armor: -5) decrease hit chance
- The Attacker's `computeThreshold()` handles clamping, so equipment cannot push effective skill below 1 or above 99

## Components and Interfaces

### Equippable Component

```cpp
// Headers/Equippable.h
#pragma once
#include "Persistent.h"

enum class EquipmentSlot : int {
    WEAPON  = 0,
    OFFHAND = 1,
    HEAD    = 2,
    BODY    = 3,
    COUNT   = 4  // sentinel for iteration
};

struct StatModifiers {
    float power   = 0.0f;
    float defense = 0.0f;
    float maxHp   = 0.0f;
    int   skill   = 0;      // hit chance modifier (signed), applied to Attacker::modifiers
};

class Equippable : public Persistent {
public:
    EquipmentSlot slot;
    StatModifiers modifiers;
    float weight = 0.0f;
    int   value  = 0;       // gold value (non-negative)

    Equippable(EquipmentSlot slot, StatModifiers modifiers, float weight, int value);

    void save(TCODZip& zip) override;
    void load(TCODZip& zip) override;
};
```

### Actor Extension

The `Actor` class gains one new optional component pointer:

```cpp
// In Actor.h — add alongside existing components:
std::shared_ptr<Equippable> equippable;
```

An item is equippable when `actor->pickable != nullptr && actor->equippable != nullptr`.

### Equipment Slots (on Player)

```cpp
// Headers/Equipment.h
#pragma once
#include <array>

class Actor;
enum class EquipmentSlot : int;

class Equipment {
public:
    // Returns the item in the given slot, or nullptr if empty.
    Actor* getSlot(EquipmentSlot slot) const;

    // Equips item into its target slot. Returns the previously equipped item
    // (which has been moved back to inventory), or nullptr if slot was empty.
    // Returns nullptr and does nothing if item has no equippable component.
    // If the item has a non-zero skill modifier, calls owner->attacker->addModifier(skill).
    // If swapping out an old item with a non-zero skill modifier, calls
    // owner->attacker->removeModifier(oldSkill) before adding the new one.
    Actor* equip(Actor* item, class Container& inventory, class Attacker* attacker);

    // Unequips the item in the given slot, moving it back to inventory.
    // If the item has a non-zero skill modifier, calls owner->attacker->removeModifier(skill).
    // Returns false if inventory is full or slot is already empty.
    bool unequip(EquipmentSlot slot, Container& inventory, Attacker* attacker);

    // Returns the sum of all equipped modifiers for a given stat.
    float getTotalPowerModifier() const;
    float getTotalDefenseModifier() const;
    float getTotalMaxHpModifier() const;
    int   getTotalSkillModifier() const;

    // Returns the total weight of all equipped items.
    float getEquippedWeight() const;

    // Returns true if the given actor is currently equipped in any slot.
    bool isEquipped(const Actor* item) const;

    void save(TCODZip& zip);
    void load(TCODZip& zip, Container& inventory);

private:
    std::array<Actor*, static_cast<int>(EquipmentSlot::COUNT)> slots = {};
};
```

### Weight System Extension

All items (equippable or not) gain weight and value. For non-equippable items, these are stored directly on the `Pickable` component or as fields on `Actor`. The cleanest approach: add `weight` and `value` fields to `Pickable`:

```cpp
// Added to Pickable class:
float weight = 0.0f;  // item weight (0 if unspecified)
int   value  = 0;     // gold value (0 if unspecified)
```

For equippable items, `Equippable::weight` and `Equippable::value` are the authoritative source. A helper on Actor resolves the weight:

```cpp
// Actor helper:
float Actor::getWeight() const {
    if (equippable) return equippable->weight;
    if (pickable)   return pickable->weight;
    return 0.0f;
}

int Actor::getValue() const {
    if (equippable) return equippable->value;
    if (pickable)   return pickable->value;
    return 0;
}
```

### Carrying Capacity

A configurable value loaded from `Config.lua`:

```lua
-- In Scripts/Config.lua:
carryingCapacity = 50.0  -- maximum weight units the player can carry
```

The pickup logic in `Pickable::pick` is extended to check total carried weight (inventory + equipped) against the configured capacity before accepting the item.

### Effective Power Calculation

The `Attacker::attack` method is modified to compute effective power:

```cpp
float effectivePower = power + owner->equipment->getTotalPowerModifier();
```

This replaces the raw `power` value in the damage formula. Defense modifiers from equipped items are similarly added to `Destructible::defense` as a computed effective defense (or applied as a transient bonus tracked by Equipment).

**Design Decision:** Stat modifiers are computed on-the-fly from currently-equipped items rather than cached. The equipment set is at most 4 items — iterating them each attack is negligible. This avoids stale-cache bugs.

### Skill Modifier Application Flow

Unlike power/defense/maxHp modifiers (which are summed on-the-fly during combat), skill modifiers integrate with the existing `Attacker::modifiers` vector via push/erase semantics. This is because `computeThreshold()` already sums that vector — equipment simply participates as another modifier source.

```cpp
// In Equipment::equip():
Actor* Equipment::equip(Actor* item, Container& inventory, Attacker* attacker) {
    if (!item || !item->equippable) return nullptr;

    EquipmentSlot targetSlot = item->equippable->slot;
    Actor* previous = slots[static_cast<int>(targetSlot)];

    // Remove old item's skill modifier if present
    if (previous && previous->equippable->modifiers.skill != 0 && attacker) {
        attacker->removeModifier(previous->equippable->modifiers.skill);
    }

    // ... swap logic (return previous to inventory) ...

    // Apply new item's skill modifier if non-zero
    if (item->equippable->modifiers.skill != 0 && attacker) {
        attacker->addModifier(item->equippable->modifiers.skill);
    }

    slots[static_cast<int>(targetSlot)] = item;
    return previous;
}

// In Equipment::unequip():
bool Equipment::unequip(EquipmentSlot slot, Container& inventory, Attacker* attacker) {
    Actor* item = slots[static_cast<int>(slot)];
    if (!item) return false;
    // ... inventory full check ...

    // Remove skill modifier if non-zero
    if (item->equippable->modifiers.skill != 0 && attacker) {
        attacker->removeModifier(item->equippable->modifiers.skill);
    }

    slots[static_cast<int>(slot)] = nullptr;
    // ... move item back to inventory ...
    return true;
}
```

### Equipment Menu

A new UI state in the game loop, triggered by a configurable key (default: `e`). The menu displays:

```
┌─ Equipment ──────────────────┐
│ [Weapon]  Chainsword (+3 pow)│
│ [Offhand] empty              │
│ [Head]    empty              │
│ [Body]    Flak Armor (+2 def)│
│                              │
│ Select slot to unequip       │
│ [ESC] Close                  │
└──────────────────────────────┘
```

The player navigates with up/down arrows and presses Enter to unequip the selected slot's item. ESC closes the menu. This reuses the existing `Menu` pattern from `Gui.h`.

### Lua Equipment Definitions

A new script file `Scripts/Equipment.lua` defines weapons and armor:

```lua
-- Scripts/Equipment.lua
equipment = {
    {
        name    = "Chainsword",
        glyph   = "/",
        color   = "lightBlue",
        slot    = "weapon",
        weight  = 3.5,
        value   = 50,
        power   = 3.0,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = 0,      -- optional, defaults to 0
    },
    {
        name    = "Scoped Bolter",
        glyph   = "}",
        color   = "lightGrey",
        slot    = "weapon",
        weight  = 5.0,
        value   = 80,
        power   = 4.0,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = 10,     -- weapon optics improve hit chance
    },
    {
        name    = "Flak Armor",
        glyph   = "[",
        color   = "grey",
        slot    = "body",
        weight  = 8.0,
        value   = 30,
        power   = 0.0,
        defense = 2.0,
        maxHp   = 0.0,
        skill   = -5,     -- heavy armor reduces accuracy
    },
}
```

At initialization, the engine loads this file via sol2, iterates the `equipment` table, validates each entry (required fields: name, glyph, color, slot, weight; optional: value defaults to 0, power/defense/maxHp default to 0.0, skill defaults to 0), and registers each as a spawnable equipment template.

**Validation rules:**
- `slot` must be one of: "weapon", "offhand", "head", "body"
- `weight` must be >= 0
- `name` and `glyph` must be non-empty
- `skill` must be an integer (if present); defaults to 0 if absent
- Invalid entries log a warning and are skipped

### Persistence

Equipment save/load extends the existing Actor serialization:

1. **Actor::save/load** gains a presence flag for the `equippable` component (same pattern as existing components)
2. **Equipment::save** writes slot assignments as indices into the container's inventory list
3. **Equipment::load** restores slot pointers by reading indices and resolving them against the loaded inventory

The save order within Actor becomes:
```
[existing fields]
hasAttacker, hasDestructible, hasAi, hasPickable, hasContainer, hasEquippable  // 6 flags
[component payloads in order]
```

The `Equipment` structure is serialized after the player's Container (since it references items within the container):
```
// In Engine::save, after player->save:
player_equipment->save(zip);
```

## Data Models

### EquipmentSlot Enum

| Value    | Int | Description              |
|----------|-----|--------------------------|
| WEAPON   | 0   | Melee/ranged weapon      |
| OFFHAND  | 1   | Shield or secondary item |
| HEAD     | 2   | Helmet or headgear       |
| BODY     | 3   | Chest armor              |

### StatModifiers Struct

| Field   | Type  | Default | Description                                    |
|---------|-------|---------|------------------------------------------------|
| power   | float | 0.0     | Added to attack power                          |
| defense | float | 0.0     | Added to damage reduction                      |
| maxHp   | float | 0.0     | Added to maximum hit points                    |
| skill   | int   | 0       | Added to Attacker modifiers vector (hit chance)|

### Equippable Component Fields

| Field     | Type           | Constraints      | Description                          |
|-----------|----------------|------------------|--------------------------------------|
| slot      | EquipmentSlot  | one of 4 values  | Target body slot                     |
| modifiers | StatModifiers  | any values       | Stat bonuses when equipped           |
| weight    | float          | >= 0.0           | Item weight in abstract units        |
| value     | int            | >= 0             | Gold worth                           |

Note: `modifiers.skill` is an int (positive or negative) that integrates with `Attacker::addModifier`/`removeModifier` on equip/unequip.

### Carrying Capacity Model

| Property        | Type  | Source       | Description                           |
|-----------------|-------|--------------|---------------------------------------|
| carryingCapacity| float | Config.lua   | Max total weight player can carry     |
| currentWeight   | float | computed     | Sum of all inventory + equipped weight|

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system — essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Equippable component field storage

*For any* valid EquipmentSlot, StatModifiers (power, defense, maxHp, skill), non-negative weight, and non-negative integer value, constructing an Equippable component and reading back its fields SHALL yield the same values.

**Validates: Requirements 1.1, 1.3, 8.1, 11.5**

### Property 2: Equippable item identification

*For any* Actor, the system identifies it as equippable if and only if the Actor has both a non-null Pickable component and a non-null Equippable component.

**Validates: Requirements 1.2**

### Property 3: Single item per slot invariant

*For any* sequence of equip and unequip operations on the player's Equipment, each Equipment_Slot SHALL contain at most one item at all times.

**Validates: Requirements 2.2, 2.3**

### Property 4: Equip/unequip round-trip preserves inventory

*For any* equippable item in the player's inventory, equipping it and then immediately unequipping it SHALL result in the item being back in the inventory and the slot being empty, with the inventory contents identical to the original state.

**Validates: Requirements 3.1, 4.1**

### Property 5: Stat modifier round-trip

*For any* equippable item with arbitrary stat modifiers (power, defense, maxHp, skill), equipping and then unequipping that item SHALL return the player's effective stats (power, defense, maxHp) and the Attacker's modifiers vector to their pre-equip values.

**Validates: Requirements 3.2, 4.2, 5.4, 5.5, 11.1, 11.2**

### Property 6: Effective power formula

*For any* base power value and any set of equipped items (0 to 4), the player's effective power SHALL equal the base power plus the sum of all power stat modifiers from currently equipped items.

**Validates: Requirements 5.1, 5.2, 5.3**

### Property 7: Slot mismatch rejection

*For any* equippable item targeting slot S, attempting to equip it into a different slot T (where T ≠ S) SHALL be rejected, leaving the equipment state unchanged.

**Validates: Requirements 3.3**

### Property 8: Inventory-full unequip rejection

*For any* equipment state where the player's inventory is at maximum capacity and all slots are occupied by items not in the inventory, attempting to unequip SHALL be rejected, leaving the equipped item in its slot.

**Validates: Requirements 4.3**

### Property 9: Carrying capacity enforcement

*For any* inventory state with total carried weight W and any item with weight w, attempting to pick up the item SHALL succeed if and only if W + w <= carryingCapacity.

**Validates: Requirements 7.1, 7.3**

### Property 10: Equipment menu data completeness

*For any* equipment state (any combination of filled/empty slots), the equipment menu data model SHALL contain exactly 4 entries — one for each slot — with the equipped item's name or "empty" for unoccupied slots.

**Validates: Requirements 6.1**

### Property 11: Lua equipment definition loading round-trip

*For any* valid Lua equipment table with name, glyph, color, slot, weight, value, and stat modifiers (power, defense, maxHp, skill), loading that definition SHALL produce an Actor whose fields (name, glyph, color, equippable.slot, equippable.weight, equippable.value, equippable.modifiers including skill) match the Lua source values. If `skill` is absent from the Lua table, it SHALL default to 0.

**Validates: Requirements 9.2, 9.3**

### Property 12: Invalid Lua definition rejection

*For any* Lua equipment table that is missing a required field (name, glyph, slot, weight), has an unknown slot name, or has a negative weight, the loader SHALL skip that definition without creating an Actor and SHALL log a warning.

**Validates: Requirements 9.4**

### Property 13: Save/load equipment round-trip

*For any* equipment state (items in slots with various modifiers, weights, values), saving and then loading SHALL produce an equivalent equipment state where all slot assignments, stat modifiers (including skill), item weights, and item values match the pre-save state.

**Validates: Requirements 10.1, 10.2, 10.3**

### Property 14: Skill modifier application round-trip

*For any* equippable item with a non-zero skill modifier value (positive or negative), equipping the item SHALL add that value to the Attacker's modifiers vector (increasing its length by one), and subsequently unequipping the same item SHALL remove that value (restoring the vector to its previous state), such that `computeThreshold()` returns the same value before equip and after unequip.

**Validates: Requirements 11.1, 11.2, 11.5**

### Property 15: Multiple skill modifiers are additive

*For any* set of 1–4 equippable items each with non-zero skill modifiers, equipping all items SHALL result in the Attacker's `computeThreshold()` returning `clamp(baseSkill + sum(allSkillModifiers), 1, 99)`, and the modifiers vector SHALL contain one entry per equipped item's skill value.

**Validates: Requirements 11.3**

### Property 16: Zero skill modifier is a no-op

*For any* equippable item with skill modifier equal to zero, equipping or unequipping that item SHALL NOT change the Attacker's modifiers vector length or contents.

**Validates: Requirements 11.4**

## Error Handling

| Scenario                          | Response                                              |
|-----------------------------------|-------------------------------------------------------|
| Equip to wrong slot               | Reject silently (GUI message: "Cannot equip here")    |
| Unequip with full inventory       | Reject (GUI message: "Inventory is full")             |
| Pickup exceeds carrying capacity  | Reject (GUI message: "Too heavy to carry")            |
| Invalid Lua equipment definition  | Log warning to console, skip entry, continue loading  |
| Corrupt save data for equipment   | Fall back to empty equipment state, log warning       |
| Equip non-equippable item         | No-op (function returns immediately)                  |
| Negative weight in Lua def        | Treated as invalid, entry skipped                     |
| Negative value in Lua def         | Clamped to 0 with warning                             |

All error messages use the existing `engine.gui->message()` system for player-facing feedback. Internal errors (Lua loading, save corruption) log to stderr or a debug log.

## Testing Strategy

### Unit Tests (Catch2)

Specific examples and edge cases:
- Equip a weapon, verify slot contains it
- Unequip from empty slot (no-op)
- Equip to occupied slot swaps correctly
- Weight exactly at capacity (boundary)
- Weight exactly 0.01 over capacity (boundary)
- Default item value is 0 when unspecified
- All four slot types can be equipped independently
- Equipment menu shows "empty" for unoccupied slots
- Equip item with skill=0, verify Attacker modifiers unchanged
- Equip item with skill=+10, verify Attacker modifiers contains 10
- Equip item with skill=-5, verify computeThreshold decreased by 5
- Swap weapon with different skill modifier, verify old removed and new added
- Unequip all skill items, verify modifiers vector is empty

### Property-Based Tests (RapidCheck stub)

Each correctness property maps to a property-based test using the project's existing `rc::prop()` / `rc::check()` API with the RapidCheck-compatible stub in `Tests/lib/rapidcheck.h`.

**Configuration:**
- Minimum 100 iterations per property test (default in the stub)
- Tag format in comments: `// Feature: equipment-system, Property N: <title>`

**Generators needed:**
- `genEquipmentSlot()` — random slot from {WEAPON, OFFHAND, HEAD, BODY}
- `genStatModifiers()` — random power/defense/maxHp in range [0, 20], skill in [-20, 20]
- `genWeight()` — random float in [0.0, 25.0]
- `genValue()` — random int in [0, 500]
- `genEquippableActor()` — actor with Pickable + Equippable using above generators
- `genEquipmentState()` — random subset of slots filled with valid items
- `genSkillModifier()` — random int in [-50, 50] (non-zero variants for modifier tests)
- `genAttacker()` — Attacker with random base skillValue in [1, 99]

**Property test file:** `Tests/test_equipment.cpp`

### Integration Tests

- Load `Scripts/Equipment.lua` with real sol2, verify items are created correctly (including skill field)
- Save game with equipment, reload, verify equipment state matches
- Full equip → attack → verify effective damage includes modifier
- Equip weapon with skill=+10, verify computeThreshold returns baseSkill + 10
- Equip weapon with skill=+10 and armor with skill=-5, verify computeThreshold returns baseSkill + 5

### Test Dependencies

Tests for the Equipment system should be runnable without the rendering engine (no TCODConsole initialization). The `Equipment`, `Equippable`, `StatModifiers`, and weight calculation logic are pure data operations testable in isolation. Tests that require serialization use `TCODZip` directly without initializing the display.
