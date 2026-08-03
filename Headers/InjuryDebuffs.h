#pragma once

#include <array>
#include "HitLocation.h"
#include "Characteristics.h"

namespace InjuryDebuffs {
    static constexpr int MAX_MODIFIERS_PER_INJURY = 3;
    static constexpr int MAX_MAGNITUDE = 9;
    static constexpr int NUM_LOCATIONS = static_cast<int>(HitLocation::COUNT);

    struct Modifier {
        CharId stat;
        int penalty; // negative value (e.g., -5, -10, -20)
    };

    struct DebuffEntry {
        std::array<Modifier, MAX_MODIFIERS_PER_INJURY> modifiers;
        int count; // how many modifiers are active (0-3)
    };

    // Constexpr 2D lookup table indexed by [HitLocation][magnitude-1].
    // Each entry defines the debuff penalties for that location/magnitude combo.
    // Magnitudes 1-4 are minor injuries, 5-9 are severe.
    static constexpr DebuffEntry TABLE[NUM_LOCATIONS][MAX_MAGNITUDE] = {
        // HEAD (index 0)
        {
            DebuffEntry{ {{{ CharId::Per, -5  }, { CharId::WS, 0 }, { CharId::WS, 0 }}}, 1 },  // mag 1
            DebuffEntry{ {{{ CharId::Per, -10 }, { CharId::BS, -5 }, { CharId::WS, 0 }}}, 2 },  // mag 2
            DebuffEntry{ {{{ CharId::WS, -10  }, { CharId::BS, -10 }, { CharId::WS, 0 }}}, 2 }, // mag 3
            DebuffEntry{ {{{ CharId::WS, -20  }, { CharId::BS, -20 }, { CharId::Int, -10 }}}, 3 }, // mag 4
            DebuffEntry{ {{{ CharId::WS, -20  }, { CharId::BS, -20 }, { CharId::Per, -15 }}}, 3 }, // mag 5
            DebuffEntry{ {{{ CharId::WS, -25  }, { CharId::BS, -25 }, { CharId::Int, -15 }}}, 3 }, // mag 6
            DebuffEntry{ {{{ CharId::WS, -30  }, { CharId::BS, -30 }, { CharId::Int, -20 }}}, 3 }, // mag 7
            DebuffEntry{ {{{ CharId::WS, -30  }, { CharId::BS, -30 }, { CharId::Per, -25 }}}, 3 }, // mag 8
            DebuffEntry{ {{{ CharId::WS, -35  }, { CharId::BS, -35 }, { CharId::Int, -30 }}}, 3 }  // mag 9
        },
        // RIGHT_ARM (index 1)
        {
            DebuffEntry{ {{{ CharId::WS, -5  }, { CharId::WS, 0 }, { CharId::WS, 0 }}}, 1 },  // mag 1
            DebuffEntry{ {{{ CharId::WS, -10 }, { CharId::BS, -5 }, { CharId::WS, 0 }}}, 2 },  // mag 2
            DebuffEntry{ {{{ CharId::WS, -10 }, { CharId::BS, -10 }, { CharId::WS, 0 }}}, 2 }, // mag 3
            DebuffEntry{ {{{ CharId::WS, -20 }, { CharId::BS, -20 }, { CharId::WS, 0 }}}, 2 }, // mag 4
            DebuffEntry{ {{{ CharId::WS, -25 }, { CharId::BS, -25 }, { CharId::WS, 0 }}}, 2 }, // mag 5
            DebuffEntry{ {{{ CharId::WS, -30 }, { CharId::BS, -30 }, { CharId::WS, 0 }}}, 2 }, // mag 6
            DebuffEntry{ {{{ CharId::WS, -30 }, { CharId::BS, -30 }, { CharId::S, -10 }}}, 3 }, // mag 7
            DebuffEntry{ {{{ CharId::WS, -35 }, { CharId::BS, -35 }, { CharId::S, -15 }}}, 3 }, // mag 8
            DebuffEntry{ {{{ CharId::WS, -40 }, { CharId::BS, -40 }, { CharId::S, -20 }}}, 3 }  // mag 9
        },
        // LEFT_ARM (index 2)
        {
            DebuffEntry{ {{{ CharId::WS, -5  }, { CharId::WS, 0 }, { CharId::WS, 0 }}}, 1 },  // mag 1
            DebuffEntry{ {{{ CharId::WS, -10 }, { CharId::BS, -5 }, { CharId::WS, 0 }}}, 2 },  // mag 2
            DebuffEntry{ {{{ CharId::WS, -10 }, { CharId::BS, -10 }, { CharId::WS, 0 }}}, 2 }, // mag 3
            DebuffEntry{ {{{ CharId::WS, -20 }, { CharId::BS, -20 }, { CharId::WS, 0 }}}, 2 }, // mag 4
            DebuffEntry{ {{{ CharId::WS, -25 }, { CharId::BS, -25 }, { CharId::WS, 0 }}}, 2 }, // mag 5
            DebuffEntry{ {{{ CharId::WS, -30 }, { CharId::BS, -30 }, { CharId::WS, 0 }}}, 2 }, // mag 6
            DebuffEntry{ {{{ CharId::WS, -30 }, { CharId::BS, -30 }, { CharId::S, -10 }}}, 3 }, // mag 7
            DebuffEntry{ {{{ CharId::WS, -35 }, { CharId::BS, -35 }, { CharId::S, -15 }}}, 3 }, // mag 8
            DebuffEntry{ {{{ CharId::WS, -40 }, { CharId::BS, -40 }, { CharId::S, -20 }}}, 3 }  // mag 9
        },
        // BODY (index 3)
        {
            DebuffEntry{ {{{ CharId::T, -5  }, { CharId::WS, 0 }, { CharId::WS, 0 }}}, 1 },  // mag 1
            DebuffEntry{ {{{ CharId::T, -10 }, { CharId::WS, 0 }, { CharId::WS, 0 }}}, 1 },  // mag 2
            DebuffEntry{ {{{ CharId::T, -10 }, { CharId::S, -5 }, { CharId::WS, 0 }}}, 2 },   // mag 3
            DebuffEntry{ {{{ CharId::T, -20 }, { CharId::S, -10 }, { CharId::WS, 0 }}}, 2 },  // mag 4
            DebuffEntry{ {{{ CharId::T, -25 }, { CharId::S, -15 }, { CharId::WS, 0 }}}, 2 },  // mag 5
            DebuffEntry{ {{{ CharId::T, -30 }, { CharId::S, -20 }, { CharId::WS, 0 }}}, 2 },  // mag 6
            DebuffEntry{ {{{ CharId::T, -30 }, { CharId::S, -20 }, { CharId::Ag, -10 }}}, 3 }, // mag 7
            DebuffEntry{ {{{ CharId::T, -35 }, { CharId::S, -25 }, { CharId::Ag, -15 }}}, 3 }, // mag 8
            DebuffEntry{ {{{ CharId::T, -40 }, { CharId::S, -30 }, { CharId::Ag, -20 }}}, 3 }  // mag 9
        },
        // RIGHT_LEG (index 4)
        {
            DebuffEntry{ {{{ CharId::Ag, -5  }, { CharId::WS, 0 }, { CharId::WS, 0 }}}, 1 },  // mag 1
            DebuffEntry{ {{{ CharId::Ag, -10 }, { CharId::WS, 0 }, { CharId::WS, 0 }}}, 1 },  // mag 2
            DebuffEntry{ {{{ CharId::Ag, -15 }, { CharId::WS, -5 }, { CharId::WS, 0 }}}, 2 }, // mag 3
            DebuffEntry{ {{{ CharId::Ag, -20 }, { CharId::WS, -10 }, { CharId::WS, 0 }}}, 2 }, // mag 4
            DebuffEntry{ {{{ CharId::Ag, -25 }, { CharId::WS, -15 }, { CharId::WS, 0 }}}, 2 }, // mag 5
            DebuffEntry{ {{{ CharId::Ag, -30 }, { CharId::WS, -20 }, { CharId::WS, 0 }}}, 2 }, // mag 6
            DebuffEntry{ {{{ CharId::Ag, -30 }, { CharId::WS, -20 }, { CharId::S, -10 }}}, 3 }, // mag 7
            DebuffEntry{ {{{ CharId::Ag, -35 }, { CharId::WS, -25 }, { CharId::S, -15 }}}, 3 }, // mag 8
            DebuffEntry{ {{{ CharId::Ag, -40 }, { CharId::WS, -30 }, { CharId::S, -20 }}}, 3 }  // mag 9
        },
        // LEFT_LEG (index 5)
        {
            DebuffEntry{ {{{ CharId::Ag, -5  }, { CharId::WS, 0 }, { CharId::WS, 0 }}}, 1 },  // mag 1
            DebuffEntry{ {{{ CharId::Ag, -10 }, { CharId::WS, 0 }, { CharId::WS, 0 }}}, 1 },  // mag 2
            DebuffEntry{ {{{ CharId::Ag, -15 }, { CharId::WS, -5 }, { CharId::WS, 0 }}}, 2 }, // mag 3
            DebuffEntry{ {{{ CharId::Ag, -20 }, { CharId::WS, -10 }, { CharId::WS, 0 }}}, 2 }, // mag 4
            DebuffEntry{ {{{ CharId::Ag, -25 }, { CharId::WS, -15 }, { CharId::WS, 0 }}}, 2 }, // mag 5
            DebuffEntry{ {{{ CharId::Ag, -30 }, { CharId::WS, -20 }, { CharId::WS, 0 }}}, 2 }, // mag 6
            DebuffEntry{ {{{ CharId::Ag, -30 }, { CharId::WS, -20 }, { CharId::S, -10 }}}, 3 }, // mag 7
            DebuffEntry{ {{{ CharId::Ag, -35 }, { CharId::WS, -25 }, { CharId::S, -15 }}}, 3 }, // mag 8
            DebuffEntry{ {{{ CharId::Ag, -40 }, { CharId::WS, -30 }, { CharId::S, -20 }}}, 3 }  // mag 9
        }
    };

    // Returns the debuff entry for a given location and magnitude.
    // magnitude must be in [1, 9]; location must be valid.
    constexpr DebuffEntry lookup(HitLocation loc, int magnitude) {
        int locIdx = static_cast<int>(loc);
        // Clamp to valid range
        if (locIdx < 0) locIdx = 0;
        if (locIdx >= NUM_LOCATIONS) locIdx = NUM_LOCATIONS - 1;
        int magIdx = magnitude - 1;
        if (magIdx < 0) magIdx = 0;
        if (magIdx >= MAX_MAGNITUDE) magIdx = MAX_MAGNITUDE - 1;
        return TABLE[locIdx][magIdx];
    }
}
