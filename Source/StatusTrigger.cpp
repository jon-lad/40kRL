#include "main.h"
#include "StatusTrigger.h"
#include "StatusEffectTracker.h"
#include "Actor.h"

namespace StatusTrigger {

    // The crit trigger table — empty stub until task 7.3 populates it.
    static const std::vector<TriggerEntry> s_critTriggerTable = {};

    const std::vector<TriggerEntry>& getCritTriggerTable() {
        return s_critTriggerTable;
    }

    void fromCritical(Actor* target, DamageType dmgType, HitLocation loc, int magnitude) {
        // No-op stub — will iterate getCritTriggerTable() and apply matching entries
        // once task 7.3 implements the full trigger table.
        (void)target;
        (void)dmgType;
        (void)loc;
        (void)magnitude;
    }

    void fromWeaponQualities(Actor* target, const std::vector<std::string>& qualities) {
        // No-op stub — will map quality strings to status effects
        // once task 8.3 implements the quality mapping.
        (void)target;
        (void)qualities;
    }

} // namespace StatusTrigger
