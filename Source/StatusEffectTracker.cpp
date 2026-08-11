#include "StatusEffectTracker.h"

// Stub implementations for TDD — real logic in task 2.3

void StatusEffectTracker::apply(Actor* /*owner*/, StatusType type, int duration, const std::string& source) {
    // Stub: just push for now (no stacking logic yet)
    effects_.push_back(StatusEffect{ type, duration, source });
}

void StatusEffectTracker::remove(Actor* /*owner*/, StatusType type) {
    effects_.erase(
        std::remove_if(effects_.begin(), effects_.end(),
            [type](const StatusEffect& e) { return e.type == type; }),
        effects_.end());
}

bool StatusEffectTracker::has(StatusType type) const {
    for (const auto& e : effects_) {
        if (e.type == type) return true;
    }
    return false;
}

int StatusEffectTracker::getRemainingDuration(StatusType type) const {
    for (const auto& e : effects_) {
        if (e.type == type) return e.duration;
    }
    return -1;
}

const std::vector<StatusEffect>& StatusEffectTracker::getActiveEffects() const {
    return effects_;
}
