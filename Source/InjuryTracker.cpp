#include "main.h"
#include "InjuryTracker.h"
#include "InjuryDebuffs.h"

#include <algorithm>

InjuryTracker::InjuryTracker() {
    magnitudes_.fill(0);
}

bool InjuryTracker::applyInjury(Actor* owner, HitLocation loc, int magnitude) {
    // Clamp magnitude to [1, 4]
    magnitude = std::clamp(magnitude, 1, MAX_MAGNITUDE);

    int idx = static_cast<int>(loc);
    if (idx < 0 || idx >= MAX_LOCATIONS) return false;

    int existing = magnitudes_[idx];
    if (existing == 0) {
        // Fresh injury — apply at the given magnitude
        magnitudes_[idx] = magnitude;
        applyDebuff(owner, loc, magnitudes_[idx]);
        return true;
    } else {
        // Escalation: increment existing magnitude by 1
        int escalated = existing + 1;
        if (escalated > MAX_MAGNITUDE) {
            return false; // caller triggers fatal effect at magnitude 5
        }
        removeDebuff(owner, loc, existing);
        magnitudes_[idx] = escalated;
        applyDebuff(owner, loc, escalated);
        return true;
    }
}

void InjuryTracker::clearAll(Actor* owner) {
    for (int i = 0; i < MAX_LOCATIONS; ++i) {
        if (magnitudes_[i] > 0) {
            removeDebuff(owner, static_cast<HitLocation>(i), magnitudes_[i]);
            magnitudes_[i] = 0;
        }
    }
}

int InjuryTracker::getMagnitude(HitLocation loc) const {
    int idx = static_cast<int>(loc);
    if (idx < 0 || idx >= MAX_LOCATIONS) return 0;
    return magnitudes_[idx];
}

bool InjuryTracker::hasInjuries() const {
    for (int m : magnitudes_) {
        if (m > 0) return true;
    }
    return false;
}

int InjuryTracker::activeCount() const {
    int count = 0;
    for (int m : magnitudes_) {
        if (m > 0) ++count;
    }
    return count;
}

void InjuryTracker::save(TCODZip& zip) {
    static constexpr int SENTINEL = 0x494E4A52; // "INJR"
    zip.putInt(SENTINEL);
    for (int i = 0; i < MAX_LOCATIONS; ++i) {
        zip.putInt(magnitudes_[i]);
    }
}

void InjuryTracker::load(TCODZip& zip) {
    static constexpr int SENTINEL = 0x494E4A52; // "INJR"
    int firstInt = zip.getInt();
    if (firstInt != SENTINEL) {
        // Backward compatibility: no valid injury data, stay empty
        magnitudes_.fill(0);
        return;
    }
    for (int i = 0; i < MAX_LOCATIONS; ++i) {
        magnitudes_[i] = std::clamp(zip.getInt(), 0, MAX_MAGNITUDE);
    }
}

void InjuryTracker::reapplyDebuffs(Actor* owner) {
    if (!owner || !owner->characteristics) return;
    for (int i = 0; i < MAX_LOCATIONS; ++i) {
        if (magnitudes_[i] > 0) {
            applyDebuff(owner, static_cast<HitLocation>(i), magnitudes_[i]);
        }
    }
}

void InjuryTracker::removeDebuff(Actor* owner, HitLocation loc, int magnitude) {
    if (!owner || !owner->characteristics) return;
    auto entry = InjuryDebuffs::lookup(loc, magnitude);
    for (int i = 0; i < entry.count; ++i) {
        owner->characteristics->removeModifier(entry.modifiers[i].stat, entry.modifiers[i].penalty);
    }
}

void InjuryTracker::applyDebuff(Actor* owner, HitLocation loc, int magnitude) {
    if (!owner || !owner->characteristics) return;
    auto entry = InjuryDebuffs::lookup(loc, magnitude);
    for (int i = 0; i < entry.count; ++i) {
        owner->characteristics->addModifier(entry.modifiers[i].stat, entry.modifiers[i].penalty);
    }
}
