#pragma once
// StatusEffects.h — Status effect data model
// STUB: Minimal declarations for TDD. Implementation in task 1.3.

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
