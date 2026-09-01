#pragma once
// StatusTrigger.h — Maps critical injuries and weapon qualities to status effects.
// Stub: fromCritical and fromWeaponQualities are no-ops until task 7.3 / 8.3 implement the tables.

#include "WeaponTypes.hpp"
#include "HitLocation.hpp"
#include "StatusEffects.hpp"
#include <vector>
#include <string>

class Actor;

namespace StatusTrigger {

    struct TriggerEntry {
        DamageType damageType;
        HitLocation location;
        int minMagnitude;       // minimum crit magnitude to trigger
        StatusType status;
        int duration;           // 0 = permanent, >0 = turns
    };

    // Returns the constexpr crit trigger table. Empty until task 7.3 populates it.
    const std::vector<TriggerEntry>& getCritTriggerTable();

    // Apply all matching status effects for this crit event.
    // No-op stub until task 7.3 implements the trigger table lookup.
    void fromCritical(Actor* target, DamageType dmgType, HitLocation loc, int magnitude);

    // Apply status effects from weapon qualities (called after damage dealt).
    // No-op stub until task 8.3 implements the quality mapping.
    void fromWeaponQualities(Actor* target, const std::vector<std::string>& qualities);
}
