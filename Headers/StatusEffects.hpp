#pragma once
// StatusEffects.h — Status effect data model

#include <string>

enum class StatusType : int {
    Burning = 0,
    Prone,
    Stunned,
    Bleeding,
    Missing_Right_Arm,
    Missing_Left_Arm,
    Missing_Right_Leg,
    Missing_Left_Leg,
    Blinded,
    Poisoned,
    COUNT
};

struct StatusEffect {
    StatusType type;
    int duration;           // turns remaining; 0 = permanent
    std::string source;     // human-readable origin

    bool isPermanent() const { return duration == 0; }
};

// Returns a short (≤5 char) abbreviation for the given status type for UI display.
inline std::string statusAbbreviation(StatusType type) {
    switch (type) {
        case StatusType::Burning:           return "BRN";
        case StatusType::Prone:             return "PRN";
        case StatusType::Stunned:           return "STN";
        case StatusType::Bleeding:          return "BLD";
        case StatusType::Poisoned:          return "PSN";
        case StatusType::Missing_Right_Arm: return "ARM-R";
        case StatusType::Missing_Left_Arm:  return "ARM-L";
        case StatusType::Missing_Right_Leg: return "LEG-R";
        case StatusType::Missing_Left_Leg:  return "LEG-L";
        case StatusType::Blinded:           return "BLND";
        default:                            return "";
    }
}
