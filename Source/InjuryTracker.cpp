#include "main.hpp"
#include "InjuryTracker.hpp"
#include "InjuryDebuffs.hpp"

#include <algorithm>

InjuryTracker::InjuryTracker()
    : magnitude_(0)
{
}

int InjuryTracker::getMagnitude() const {
    return magnitude_;
}

bool InjuryTracker::applyCrit(Actor* owner, HitLocation loc, int critMagnitude) {
    // Clamp incoming crit magnitude to at least 1
    if (critMagnitude < 1) critMagnitude = 1;

    int newTotal = magnitude_ + critMagnitude;
    if (newTotal >= FATAL_MAGNITUDE) {
        return false; // caller triggers death
    }

    magnitude_ = newTotal;

    // Apply the debuff for this hit location at the new total magnitude
    applyDebuff(owner, loc, newTotal);
    records_.push_back(InjuryRecord{ loc, newTotal });
    return true;
}

void InjuryTracker::recordFatalCrit(HitLocation loc, int magnitude) {
    // Clamp to [1, MAX_MAGNITUDE] so the record round-trips symmetrically through
    // save/load (load clamps magnitude to [1, MAX_MAGNITUDE]).
    int clampedMagnitude = std::clamp(magnitude, 1, MAX_MAGNITUDE);

    // Set cumulative magnitude to reflect the killing blow. Kept within
    // [1, MAX_MAGNITUDE] for save/load symmetry (load clamps magnitude_ to
    // [0, MAX_MAGNITUDE]).
    magnitude_ = clampedMagnitude;

    // Record the fatal blow's location for display. Do NOT apply debuffs — the
    // actor is dying/dead, so there are no living characteristics to modify.
    records_.push_back(InjuryRecord{ loc, clampedMagnitude });
}

int InjuryTracker::healMagnitude(int amount) {
    if (amount <= 0) return 0;
    if (magnitude_ <= 0) return amount; // no crit damage, all goes to HP

    int reduction = std::min(amount, magnitude_);
    magnitude_ -= reduction;
    return amount - reduction; // excess for HP healing
}

bool InjuryTracker::hasInjuries() const {
    return !records_.empty();
}

int InjuryTracker::activeCount() const {
    return static_cast<int>(records_.size());
}

const std::vector<InjuryRecord>& InjuryTracker::getRecords() const {
    return records_;
}

void InjuryTracker::clearAll(Actor* owner) {
    // Remove all active debuffs
    for (const auto& record : records_) {
        removeDebuff(owner, record.location, record.magnitude);
    }
    records_.clear();
    magnitude_ = 0;
}

void InjuryTracker::save(TCODZip& zip) {
    static constexpr int SENTINEL = 0x494E4A52; // "INJR"
    zip.putInt(SENTINEL);
    zip.putInt(magnitude_);
    zip.putInt(static_cast<int>(records_.size()));
    for (const auto& record : records_) {
        zip.putInt(static_cast<int>(record.location));
        zip.putInt(record.magnitude);
    }
}

void InjuryTracker::load(TCODZip& zip) {
    static constexpr int SENTINEL = 0x494E4A52; // "INJR"
    int firstInt = zip.getInt();
    if (firstInt != SENTINEL) {
        // Backward compatibility: no valid injury data, stay empty
        magnitude_ = 0;
        records_.clear();
        return;
    }
    magnitude_ = std::clamp(zip.getInt(), 0, MAX_MAGNITUDE);
    int count = zip.getInt();
    records_.clear();
    for (int i = 0; i < count; ++i) {
        int locInt = std::clamp(zip.getInt(), 0, static_cast<int>(HitLocation::COUNT) - 1);
        int mag = std::clamp(zip.getInt(), 1, MAX_MAGNITUDE);
        records_.push_back(InjuryRecord{ static_cast<HitLocation>(locInt), mag });
    }
}

void InjuryTracker::reapplyDebuffs(Actor* owner) {
    if (!owner || !owner->characteristics) return;
    for (const auto& record : records_) {
        applyDebuff(owner, record.location, record.magnitude);
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
