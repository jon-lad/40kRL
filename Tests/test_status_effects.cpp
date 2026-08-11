#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "StatusEffects.h"

#include <string>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 11: StatusEffect Data Integrity ────────────────────────────────
// **Validates: Requirements 1.2, 1.3**
//
// For any StatusEffect constructed with a valid StatusType, non-negative duration,
// and source string, querying the struct's fields SHALL return the original
// construction values, and isPermanent() SHALL return true if and only if
// duration equals 0.

TEST_CASE("PBT: Property 11 — StatusEffect data integrity", "[pbt][property][status-effects]")
{
    rc::prop("fields return construction values and isPermanent iff duration == 0", []() {
        // Generate random StatusType in valid range [0, COUNT-1]
        const int typeInt = *rc::gen::inRange(0, static_cast<int>(StatusType::COUNT) - 1);
        const StatusType type = static_cast<StatusType>(typeInt);

        // Generate non-negative duration [0, 1000]
        const int duration = *rc::gen::inRange(0, 1000);

        // Generate random source string (1-20 lowercase chars)
        const std::string source = *rc::gen::string(1, 20);

        // Construct the StatusEffect
        StatusEffect effect{ type, duration, source };

        // Verify fields return original construction values
        RC_ASSERT(effect.type == type);
        RC_ASSERT(effect.duration == duration);
        RC_ASSERT(effect.source == source);

        // Verify isPermanent() returns true iff duration == 0
        if (duration == 0) {
            RC_ASSERT(effect.isPermanent());
        } else {
            RC_ASSERT(!effect.isPermanent());
        }
    });
}
