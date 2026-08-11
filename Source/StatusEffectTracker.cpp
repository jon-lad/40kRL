#include "main.h"
#include "StatusEffectTracker.h"

void StatusEffectTracker::apply(Actor* owner, StatusType type, int duration, const std::string& source) {
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

    // Apply characteristic modifiers for the newly added effect
    if (owner) {
        applyModifiers(owner, type);
    }

    // NOTE: logApplication() is NOT called here — callers (Attacker, Engine)
    // are responsible for invoking logApplication() when the engine/GUI is initialized.
    // This keeps StatusEffectTracker unit-testable without engine dependencies.
}

bool StatusEffectTracker::tickStartOfTurn(Actor* owner) {
    // 1. Check if Stunned — actor cannot act this turn
    const bool stunned = has(StatusType::Stunned);

    // 2. Deal Burning tick damage (1d10 Energy, ignoring armour)
    if (has(StatusType::Burning) && owner && owner->destructible && !owner->destructible->isDead()) {
        int damage = TCODRandom::getInstance()->getInt(1, 10);
        owner->destructible->hp -= static_cast<float>(damage);
        if (owner->destructible->isDead()) {
            owner->destructible->die(owner);
            // Actor died — decrement durations before returning, then halt
            // Actually: halt processing immediately per spec
            return !stunned;
        }
    }

    // 3. Decrement durations of all temporary effects and remove expired ones
    for (auto it = effects_.begin(); it != effects_.end(); ) {
        if (!it->isPermanent()) {
            it->duration -= 1;
            if (it->duration <= 0) {
                StatusType expiredType = it->type;
                // Remove modifiers before erasing
                removeModifiers(owner, expiredType);
                it = effects_.erase(it);
                // NOTE: logExpiry() is NOT called here — callers (Engine turn loop)
                // are responsible for logging expiry when the GUI is initialized.
                continue;
            }
        }
        ++it;
    }

    return !stunned;
}

void StatusEffectTracker::tickEndOfTurn(Actor* owner) {
    // Deal Bleeding damage: 1 wound per turn
    if (has(StatusType::Bleeding) && owner && owner->destructible && !owner->destructible->isDead()) {
        owner->destructible->hp -= 1.0f;
        if (owner->destructible->isDead()) {
            owner->destructible->die(owner);
        }
    }
}

void StatusEffectTracker::remove(Actor* owner, StatusType type) {
    // Reverse characteristic modifiers before removing the effect
    if (owner && has(type)) {
        removeModifiers(owner, type);
    }

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

void StatusEffectTracker::save(TCODZip& zip) {
    zip.putInt(static_cast<int>(effects_.size()));
    for (const auto& effect : effects_) {
        zip.putInt(static_cast<int>(effect.type));
        zip.putInt(effect.duration);
        zip.putString(effect.source.c_str());
    }
}

void StatusEffectTracker::load(TCODZip& zip) {
    effects_.clear();
    const int count = zip.getInt();
    for (int i = 0; i < count; ++i) {
        StatusEffect effect;
        effect.type = static_cast<StatusType>(zip.getInt());
        effect.duration = zip.getInt();
        const char* src = zip.getString();
        effect.source = src ? src : "";
        effects_.push_back(std::move(effect));
    }
}

void StatusEffectTracker::reapplyModifiers(Actor* owner) {
    if (!owner) return;
    for (const auto& effect : effects_) {
        applyModifiers(owner, effect.type);
    }
}

void StatusEffectTracker::logApplication(Actor* owner, StatusType type) {
    if (!owner || !engine.gui) return;

    bool isPlayer = (owner == engine.player);
    if (isPlayer) {
        switch (type) {
            case StatusType::Burning:           engine.gui->message(Colors::damage, "You are on fire!"); break;
            case StatusType::Stunned:           engine.gui->message(Colors::damage, "You are stunned."); break;
            case StatusType::Prone:             engine.gui->message(Colors::damage, "You are knocked prone!"); break;
            case StatusType::Bleeding:          engine.gui->message(Colors::damage, "You are bleeding!"); break;
            case StatusType::Blinded:           engine.gui->message(Colors::damage, "You are blinded!"); break;
            case StatusType::Poisoned:          engine.gui->message(Colors::damage, "You are poisoned!"); break;
            case StatusType::Missing_Right_Arm: engine.gui->message(Colors::damage, "Your right arm is destroyed!"); break;
            case StatusType::Missing_Left_Arm:  engine.gui->message(Colors::damage, "Your left arm is destroyed!"); break;
            case StatusType::Missing_Right_Leg: engine.gui->message(Colors::damage, "Your right leg is destroyed!"); break;
            case StatusType::Missing_Left_Leg:  engine.gui->message(Colors::damage, "Your left leg is destroyed!"); break;
            default: break;
        }
    } else {
        // Only log if the actor is visible to the player
        bool visible = engine.map && engine.map->isInFOV(owner->getX(), owner->getY());
        if (visible) {
            switch (type) {
                case StatusType::Burning:           engine.gui->message(Colors::damage, "The # is on fire!", owner->name); break;
                case StatusType::Stunned:           engine.gui->message(Colors::damage, "The # is stunned.", owner->name); break;
                case StatusType::Prone:             engine.gui->message(Colors::damage, "The # is knocked prone!", owner->name); break;
                case StatusType::Bleeding:          engine.gui->message(Colors::damage, "The # is bleeding!", owner->name); break;
                case StatusType::Blinded:           engine.gui->message(Colors::damage, "The # is blinded!", owner->name); break;
                case StatusType::Poisoned:          engine.gui->message(Colors::damage, "The # is poisoned!", owner->name); break;
                case StatusType::Missing_Right_Arm: engine.gui->message(Colors::damage, "The #'s right arm is destroyed!", owner->name); break;
                case StatusType::Missing_Left_Arm:  engine.gui->message(Colors::damage, "The #'s left arm is destroyed!", owner->name); break;
                case StatusType::Missing_Right_Leg: engine.gui->message(Colors::damage, "The #'s right leg is destroyed!", owner->name); break;
                case StatusType::Missing_Left_Leg:  engine.gui->message(Colors::damage, "The #'s left leg is destroyed!", owner->name); break;
                default: break;
            }
        }
    }
}

void StatusEffectTracker::logExpiry(Actor* owner, StatusType type) {
    if (!owner || !engine.gui) return;

    bool isPlayer = (owner == engine.player);
    if (isPlayer) {
        switch (type) {
            case StatusType::Burning:  engine.gui->message(Colors::uiText, "The flames die out."); break;
            case StatusType::Stunned:  engine.gui->message(Colors::uiText, "You are no longer stunned."); break;
            case StatusType::Blinded:  engine.gui->message(Colors::uiText, "Your vision clears."); break;
            case StatusType::Poisoned: engine.gui->message(Colors::uiText, "The poison wears off."); break;
            case StatusType::Bleeding: engine.gui->message(Colors::uiText, "The bleeding stops."); break;
            default: break;
        }
    } else {
        // Only log if the actor is visible to the player
        bool visible = engine.map && engine.map->isInFOV(owner->getX(), owner->getY());
        if (visible) {
            switch (type) {
                case StatusType::Burning:  engine.gui->message(Colors::uiText, "The flames on # die out.", owner->name); break;
                case StatusType::Stunned:  engine.gui->message(Colors::uiText, "The # is no longer stunned.", owner->name); break;
                case StatusType::Blinded:  engine.gui->message(Colors::uiText, "The #'s vision clears.", owner->name); break;
                case StatusType::Poisoned: engine.gui->message(Colors::uiText, "The poison on # wears off.", owner->name); break;
                case StatusType::Bleeding: engine.gui->message(Colors::uiText, "The # stops bleeding.", owner->name); break;
                default: break;
            }
        }
    }
}
