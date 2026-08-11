#pragma once
// StatusEffectTracker.h — Stub header for TDD (tests written before implementation)
// This provides minimal declarations so property tests compile.
// Full implementation will follow in task 2.3.

#include "StatusEffects.h"
#include <vector>
#include <string>

class Actor; // forward declaration

class StatusEffectTracker {
public:
    StatusEffectTracker() = default;

    // Apply a new effect. Handles stacking (refresh if new > existing, keep permanent).
    void apply(Actor* owner, StatusType type, int duration, const std::string& source);

    // Remove a specific effect type.
    void remove(Actor* owner, StatusType type);

    // Query interface
    bool has(StatusType type) const;
    int getRemainingDuration(StatusType type) const;
    const std::vector<StatusEffect>& getActiveEffects() const;

private:
    std::vector<StatusEffect> effects_;
};
