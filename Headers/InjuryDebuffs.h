#pragma once

#include <array>
#include "HitLocation.h"
#include "Characteristics.h"

namespace InjuryDebuffs {
    static constexpr int MAX_MODIFIERS_PER_INJURY = 3;

    struct Modifier {
        CharId stat;
        int penalty; // negative value (e.g., -5, -10, -20)
    };

    struct DebuffEntry {
        std::array<Modifier, MAX_MODIFIERS_PER_INJURY> modifiers;
        int count; // how many modifiers are active (0-3)
    };

    // Returns the debuff entry for a given location and magnitude.
    // magnitude must be in [1, 4]; location must be valid.
    // STUB: Task 2.2 will provide the real implementation.
    constexpr DebuffEntry lookup(HitLocation loc, int magnitude) {
        (void)loc;
        (void)magnitude;
        return DebuffEntry{ {{{CharId::WS, 0}, {CharId::WS, 0}, {CharId::WS, 0}}}, 0 };
    }
}
