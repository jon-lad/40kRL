# Test Isolation from Engine

The test project (`Tests/40kRL_Tests.vcxproj`) links all game source files but does NOT initialize the global `Engine` object (no SDL window, no libtcod context, no Gui, no Map). This means:

## Rules

1. **Never call `engine.gui->message(...)` from within component code** that may be exercised by unit tests. The global `engine.gui` is a `std::unique_ptr<Gui>` that is **null** in the test binary — or worse, may point to a partially constructed object if another test initialized it.

2. **Never call `engine.map->isInFOV(...)`, `engine.player`, or any other engine member** from low-level component methods (StatusEffectTracker, InjuryTracker, Destructible, Characteristics, etc.) without a null guard.

3. **GUI message logging must be opt-in, not automatic.** Component methods like `apply()`, `remove()`, `tickStartOfTurn()`, `tickEndOfTurn()` must NOT call engine.gui internally. Instead:
   - Provide a separate `logApplication()` / `logExpiry()` method that callers (Engine turn loop, Attacker pipeline, PlayerAi) invoke explicitly when they know the engine is initialized.
   - Or accept a callback/flag parameter that enables logging.

4. **Guard `die()` calls in tick methods.** `Destructible::die(owner)` calls `engine.gui->message(...)` internally. When a tick method (e.g., Burning/Bleeding damage) can kill an actor, wrap the `die()` call with `if (engine.gui)` to prevent crashes in tests.

5. **RapidCheck `rc::gen::inRange(a, b)` uses INCLUSIVE bounds `[a, b]`** in this project's custom header. When generating indices into a container of size N, use `inRange(0, N - 1)`, NOT `inRange(0, N)`. For enum values with COUNT, use `inRange(0, static_cast<int>(Enum::COUNT) - 1)`.

## Pattern for Engine-Safe Components

```cpp
// GOOD: Component method is test-safe (no engine access)
void StatusEffectTracker::apply(Actor* owner, StatusType type, int duration, const std::string& source) {
    effects_.push_back(StatusEffect{ type, duration, source });
    if (owner) applyModifiers(owner, type);
    // NOTE: logging is caller's responsibility
}

// GOOD: Caller handles logging when engine is known to be available
void Engine::applyStatusFromCrit(Actor* target, DamageType dt, HitLocation loc, int mag) {
    StatusTrigger::fromCritical(target, dt, loc, mag);
    if (gui && target->statusTracker) {
        target->statusTracker->logApplication(target, ...);
    }
}
```

## Summary

Keep components (StatusEffectTracker, InjuryTracker, Attacker logic) free of direct `engine.*` access. Push all GUI/rendering/FOV calls to the game-loop layer (Engine, PlayerAi, MonsterAi) where the engine is guaranteed to be initialized.
