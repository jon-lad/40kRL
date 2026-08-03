#include "main.h"
#include "InjuryTracker.h"
#include "InjuryDebuffs.h"

InjuryTracker::InjuryTracker() {
    magnitudes_.fill(0);
}

bool InjuryTracker::applyInjury(Actor* owner, HitLocation loc, int magnitude) {
    // STUB: does nothing — to be implemented in task 3.5
    return false;
}

void InjuryTracker::clearAll(Actor* owner) {
    // STUB: does nothing — to be implemented in task 3.5
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
    // STUB: to be implemented in task 5.3
}

void InjuryTracker::load(TCODZip& zip) {
    // STUB: to be implemented in task 5.3
}

void InjuryTracker::reapplyDebuffs(Actor* owner) {
    // STUB: to be implemented in task 3.5
}

void InjuryTracker::removeDebuff(Actor* owner, HitLocation loc, int magnitude) {
    // STUB: to be implemented in task 3.5
}

void InjuryTracker::applyDebuff(Actor* owner, HitLocation loc, int magnitude) {
    // STUB: to be implemented in task 3.5
}
