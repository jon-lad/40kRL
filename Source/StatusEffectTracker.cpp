#include "StatusEffectTracker.h"

void StatusEffectTracker::apply(Actor* /*owner*/, StatusType type, int duration, const std::string& source) {
    // Clamp negative durations to 0 (treat as permanent)
    if (duration < 0) {
        duration = 0;
    }

    // Check if this status type already exists
    for (auto& effect : effects_) {
        if (effect.type == type) {
            // Existing is permanent (duration 0): keep it, ignore new
            if (effect.isPermanent()) {
                return;
            }
            // New duration is greater than existing: refresh to new duration
            if (duration > effect.duration) {
                effect.duration = duration;
                effect.source = source;
            }
            // Otherwise existing >= new: no change
            return;
        }
    }

    // Type is new: add to effects vector
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
