#include "main.h"
#include "StatusTrigger.h"
#include "StatusEffectTracker.h"
#include "Actor.h"
#include "HitLocation.h"

namespace StatusTrigger {

    // The full crit trigger table — 27 entries mapping (DamageType, HitLocation, minMagnitude) → StatusType.
    static const std::vector<TriggerEntry> s_critTriggerTable = {
        // Energy
        { DamageType::E, HitLocation::HEAD,      3, StatusType::Blinded,           0 },  // duration rolled at apply time (1d5)
        { DamageType::E, HitLocation::HEAD,      5, StatusType::Burning,           3 },
        { DamageType::E, HitLocation::RIGHT_ARM, 5, StatusType::Missing_Right_Arm, 0 },
        { DamageType::E, HitLocation::LEFT_ARM,  5, StatusType::Missing_Left_Arm,  0 },
        { DamageType::E, HitLocation::BODY,      3, StatusType::Burning,           3 },
        { DamageType::E, HitLocation::RIGHT_LEG, 5, StatusType::Missing_Right_Leg, 0 },
        { DamageType::E, HitLocation::LEFT_LEG,  5, StatusType::Missing_Left_Leg,  0 },

        // Impact
        { DamageType::I, HitLocation::HEAD,      1, StatusType::Stunned,           1 },
        { DamageType::I, HitLocation::BODY,      5, StatusType::Prone,             0 },
        { DamageType::I, HitLocation::RIGHT_ARM, 6, StatusType::Missing_Right_Arm, 0 },
        { DamageType::I, HitLocation::LEFT_ARM,  6, StatusType::Missing_Left_Arm,  0 },
        { DamageType::I, HitLocation::RIGHT_LEG, 6, StatusType::Missing_Right_Leg, 0 },
        { DamageType::I, HitLocation::LEFT_LEG,  6, StatusType::Missing_Left_Leg,  0 },

        // Rending
        { DamageType::R, HitLocation::HEAD,      1, StatusType::Bleeding,          0 },
        { DamageType::R, HitLocation::HEAD,      5, StatusType::Blinded,           0 },  // duration rolled at apply time (1d5)
        { DamageType::R, HitLocation::RIGHT_ARM, 6, StatusType::Missing_Right_Arm, 0 },
        { DamageType::R, HitLocation::LEFT_ARM,  6, StatusType::Missing_Left_Arm,  0 },
        { DamageType::R, HitLocation::RIGHT_LEG, 5, StatusType::Missing_Right_Leg, 0 },
        { DamageType::R, HitLocation::LEFT_LEG,  5, StatusType::Missing_Left_Leg,  0 },
        { DamageType::R, HitLocation::BODY,      4, StatusType::Bleeding,          0 },
        { DamageType::R, HitLocation::BODY,      4, StatusType::Prone,             0 },

        // Explosive
        { DamageType::X, HitLocation::HEAD,      1, StatusType::Stunned,           1 },
        { DamageType::X, HitLocation::RIGHT_ARM, 4, StatusType::Missing_Right_Arm, 0 },
        { DamageType::X, HitLocation::LEFT_ARM,  4, StatusType::Missing_Left_Arm,  0 },
        { DamageType::X, HitLocation::RIGHT_LEG, 4, StatusType::Missing_Right_Leg, 0 },
        { DamageType::X, HitLocation::LEFT_LEG,  4, StatusType::Missing_Left_Leg,  0 },
        { DamageType::X, HitLocation::BODY,      4, StatusType::Bleeding,          0 },
        { DamageType::X, HitLocation::BODY,      4, StatusType::Prone,             0 },
    };

    const std::vector<TriggerEntry>& getCritTriggerTable() {
        return s_critTriggerTable;
    }

    void fromCritical(Actor* target, DamageType dmgType, HitLocation loc, int magnitude) {
        if (!target) return;

        // Clamp magnitude to valid range [1, 9]
        if (magnitude < 1) magnitude = 1;
        if (magnitude > 9) magnitude = 9;

        // Ensure target has a status tracker
        if (!target->statusTracker) {
            target->statusTracker = std::make_unique<StatusEffectTracker>();
        }

        // Build a source description string like "Energy Head crit 5"
        std::string source = std::string(damageTypeName(dmgType)) + " "
                           + HitLocationTable::name(loc) + " crit "
                           + std::to_string(magnitude);

        // Iterate the table and apply all matching entries
        for (const auto& entry : s_critTriggerTable) {
            if (entry.damageType == dmgType && entry.location == loc && magnitude >= entry.minMagnitude) {
                int duration = entry.duration;

                // Blinded entries have random 1d5 duration
                if (entry.status == StatusType::Blinded) {
                    duration = TCODRandom::getInstance()->getInt(1, 5);
                }

                target->statusTracker->apply(target, entry.status, duration, source);
            }
        }
    }

    void fromWeaponQualities(Actor* target, const std::vector<std::string>& qualities) {
        if (!target) return;
        if (!target->statusTracker) {
            target->statusTracker = std::make_unique<StatusEffectTracker>();
        }
        for (const auto& quality : qualities) {
            if (quality == "Flame") {
                target->statusTracker->apply(target, StatusType::Burning, 3, "Flame weapon quality");
            } else if (quality == "Shocking") {
                target->statusTracker->apply(target, StatusType::Stunned, 1, "Shocking weapon quality");
            } else if (quality == "Toxic") {
                target->statusTracker->apply(target, StatusType::Poisoned, 5, "Toxic weapon quality");
            }
        }
    }

} // namespace StatusTrigger
