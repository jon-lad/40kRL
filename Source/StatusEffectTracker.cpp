#include "main.h"
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

bool StatusEffectTracker::tickStartOfTurn(Actor* /*owner*/) {
    // Stub - real implementation in task 3.3
    return true;
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

bool StatusEffectTracker::canAct() const {
    return !has(StatusType::Stunned);
}

bool StatusEffectTracker::isMovementHalved() const {
    return has(StatusType::Missing_Right_Leg) || has(StatusType::Missing_Left_Leg);
}

int StatusEffectTracker::getWSModifier() const {
    int mod = 0;
    if (has(StatusType::Prone)) mod -= 10;
    if (has(StatusType::Blinded)) mod -= 30;
    return mod;
}

int StatusEffectTracker::getBSModifier() const {
    int mod = 0;
    if (has(StatusType::Blinded)) mod -= 30;
    return mod;
}

int StatusEffectTracker::getEvasionModifier() const {
    int mod = 0;
    if (has(StatusType::Stunned)) mod -= 20;
    return mod;
}

int StatusEffectTracker::getSModifier() const {
    int mod = 0;
    if (has(StatusType::Poisoned)) mod -= 10;
    return mod;
}

int StatusEffectTracker::getTModifier() const {
    int mod = 0;
    if (has(StatusType::Poisoned)) mod -= 10;
    return mod;
}

int StatusEffectTracker::getAgModifier() const {
    int mod = 0;
    if (has(StatusType::Poisoned)) mod -= 10;
    return mod;
}

int StatusEffectTracker::getRangedTargetingModifier() const {
    int mod = 0;
    if (has(StatusType::Prone)) mod -= 10;
    return mod;
}

int StatusEffectTracker::getAttackerBonusAgainstMe() const {
    int mod = 0;
    if (has(StatusType::Blinded)) mod += 30;
    return mod;
}

bool StatusEffectTracker::isSlotDisabled(int slot) const {
    // WEAPON slot (0) disabled by Missing_Right_Arm
    // OFFHAND slot (1) disabled by Missing_Left_Arm
    if (slot == 0 && has(StatusType::Missing_Right_Arm)) return true;
    if (slot == 1 && has(StatusType::Missing_Left_Arm)) return true;
    return false;
}

bool StatusEffectTracker::isTwoHandedBlocked() const {
    return has(StatusType::Missing_Right_Arm) || has(StatusType::Missing_Left_Arm);
}

void StatusEffectTracker::applyModifiers(Actor* owner, StatusType type) {
    if (!owner || !owner->characteristics) return;
    switch (type) {
        case StatusType::Prone:
            owner->characteristics->addModifier(CharId::WS, -10);
            break;
        case StatusType::Blinded:
            owner->characteristics->addModifier(CharId::WS, -30);
            owner->characteristics->addModifier(CharId::BS, -30);
            break;
        case StatusType::Poisoned:
            owner->characteristics->addModifier(CharId::S, -10);
            owner->characteristics->addModifier(CharId::T, -10);
            owner->characteristics->addModifier(CharId::Ag, -10);
            break;
        default:
            break;
    }
}

void StatusEffectTracker::removeModifiers(Actor* owner, StatusType type) {
    if (!owner || !owner->characteristics) return;
    switch (type) {
        case StatusType::Prone:
            owner->characteristics->removeModifier(CharId::WS, -10);
            break;
        case StatusType::Blinded:
            owner->characteristics->removeModifier(CharId::WS, -30);
            owner->characteristics->removeModifier(CharId::BS, -30);
            break;
        case StatusType::Poisoned:
            owner->characteristics->removeModifier(CharId::S, -10);
            owner->characteristics->removeModifier(CharId::T, -10);
            owner->characteristics->removeModifier(CharId::Ag, -10);
            break;
        default:
            break;
    }
}
