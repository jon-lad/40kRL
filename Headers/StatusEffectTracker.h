#pragma once
// StatusEffectTracker.h — Tracks active status effects on an actor.
// Implements stacking (refresh if new > existing, keep permanent), idempotent removal,
// and query interface for active effects.

#include "StatusEffects.h"
#include <vector>
#include <string>
#include <algorithm>

class Actor; // forward declaration

class StatusEffectTracker {
public:
    StatusEffectTracker() = default;

    // Apply a new effect. Handles stacking:
    //   - If same type already permanent (duration 0): keep permanent, ignore new
    //   - If same type exists and new duration > existing: update to new duration
    //   - If same type exists and existing >= new: no change
    //   - If type is new: add to effects vector
    void apply(Actor* owner, StatusType type, int duration, const std::string& source);

    // Remove a specific effect type. Idempotent (no-op if not present).
    void remove(Actor* owner, StatusType type);

    // Query interface
    bool has(StatusType type) const;
    int getRemainingDuration(StatusType type) const;
    const std::vector<StatusEffect>& getActiveEffects() const;

private:
    std::vector<StatusEffect> effects_;
};
