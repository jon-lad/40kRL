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

        // Generate random source string (non-empty)
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

// ─── Property 12: Status Abbreviation Mapping ────────────────────────────────
// **Validates: Requirements 8.1**
//
// For any valid StatusType enum value, calling statusAbbreviation() SHALL return
// a non-empty string of at most 5 characters matching the expected abbreviation.

TEST_CASE("PBT: Property 12 — Status abbreviation mapping", "[pbt][property][status-effects]")
{
    rc::prop("statusAbbreviation returns non-empty string <= 5 chars for all valid types", []() {
        const int typeInt = *rc::gen::inRange(0, static_cast<int>(StatusType::COUNT) - 1);
        const StatusType type = static_cast<StatusType>(typeInt);

        const std::string abbr = statusAbbreviation(type);

        // Must be non-empty
        RC_ASSERT(!abbr.empty());
        // Must be at most 5 characters
        RC_ASSERT(abbr.size() <= 5);
    });

    // Verify exact expected abbreviations
    SECTION("Expected abbreviations match") {
        CHECK(statusAbbreviation(StatusType::Burning) == "BRN");
        CHECK(statusAbbreviation(StatusType::Prone) == "PRN");
        CHECK(statusAbbreviation(StatusType::Stunned) == "STN");
        CHECK(statusAbbreviation(StatusType::Bleeding) == "BLD");
        CHECK(statusAbbreviation(StatusType::Poisoned) == "PSN");
        CHECK(statusAbbreviation(StatusType::Missing_Right_Arm) == "ARM-R");
        CHECK(statusAbbreviation(StatusType::Missing_Left_Arm) == "ARM-L");
        CHECK(statusAbbreviation(StatusType::Missing_Right_Leg) == "LEG-R");
        CHECK(statusAbbreviation(StatusType::Missing_Left_Leg) == "LEG-L");
        CHECK(statusAbbreviation(StatusType::Blinded) == "BLND");
    }
}

#include "StatusEffectTracker.h"

#include <set>
#include <algorithm>

// ─── Property 9: Multiple Statuses Coexist ───────────────────────────────────
// **Validates: Requirements 6.3, 6.5, 6.6**
//
// For any set of distinct StatusType values applied to a single actor, all
// applied statuses SHALL be simultaneously active and independently queryable
// via has(). Missing_Right_Arm and Missing_Left_Arm coexist as distinct statuses.
// Missing_Right_Leg and Missing_Left_Leg coexist as distinct statuses.

TEST_CASE("PBT: Property 9 — Multiple statuses coexist", "[pbt][property][status-effects]")
{
    rc::prop("all applied distinct statuses are independently queryable via has()", []() {
        // Generate a random subset of StatusType values (at least 1, up to all)
        const int statusCount = static_cast<int>(StatusType::COUNT);
        std::set<StatusType> chosenTypes;

        // Generate a bitmask to select a random subset
        for (int i = 0; i < statusCount; ++i) {
            const int include = *rc::gen::inRange(0, 2);
            if (include == 1) {
                chosenTypes.insert(static_cast<StatusType>(i));
            }
        }

        // Ensure at least one status is chosen
        if (chosenTypes.empty()) {
            const int forced = *rc::gen::inRange(0, statusCount);
            chosenTypes.insert(static_cast<StatusType>(forced));
        }

        // Create a tracker and apply all chosen statuses
        StatusEffectTracker tracker;
        for (StatusType type : chosenTypes) {
            const int duration = *rc::gen::inRange(0, 20);
            tracker.apply(nullptr, type, duration, "test-source");
        }

        // Verify all applied statuses are independently queryable
        for (StatusType type : chosenTypes) {
            RC_ASSERT(tracker.has(type));
        }

        // Verify statuses NOT applied are not reported as active
        for (int i = 0; i < statusCount; ++i) {
            StatusType type = static_cast<StatusType>(i);
            if (chosenTypes.find(type) == chosenTypes.end()) {
                RC_ASSERT(!tracker.has(type));
            }
        }
    });

    rc::prop("Missing_Right_Arm and Missing_Left_Arm coexist as distinct statuses", []() {
        StatusEffectTracker tracker;

        const int durRight = *rc::gen::inRange(0, 10);
        const int durLeft = *rc::gen::inRange(0, 10);

        tracker.apply(nullptr, StatusType::Missing_Right_Arm, durRight, "crit-right");
        tracker.apply(nullptr, StatusType::Missing_Left_Arm, durLeft, "crit-left");

        // Both must be independently queryable
        RC_ASSERT(tracker.has(StatusType::Missing_Right_Arm));
        RC_ASSERT(tracker.has(StatusType::Missing_Left_Arm));

        // They are distinct — removing one does not affect the other
        // (This verifies they are stored as separate entries)
        const auto& effects = tracker.getActiveEffects();
        int armCount = 0;
        for (const auto& e : effects) {
            if (e.type == StatusType::Missing_Right_Arm || e.type == StatusType::Missing_Left_Arm) {
                armCount++;
            }
        }
        RC_ASSERT(armCount == 2);
    });

    rc::prop("Missing_Right_Leg and Missing_Left_Leg coexist as distinct statuses", []() {
        StatusEffectTracker tracker;

        const int durRight = *rc::gen::inRange(0, 10);
        const int durLeft = *rc::gen::inRange(0, 10);

        tracker.apply(nullptr, StatusType::Missing_Right_Leg, durRight, "crit-right");
        tracker.apply(nullptr, StatusType::Missing_Left_Leg, durLeft, "crit-left");

        // Both must be independently queryable
        RC_ASSERT(tracker.has(StatusType::Missing_Right_Leg));
        RC_ASSERT(tracker.has(StatusType::Missing_Left_Leg));

        // They are distinct entries
        const auto& effects = tracker.getActiveEffects();
        int legCount = 0;
        for (const auto& e : effects) {
            if (e.type == StatusType::Missing_Right_Leg || e.type == StatusType::Missing_Left_Leg) {
                legCount++;
            }
        }
        RC_ASSERT(legCount == 2);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// StatusEffectTracker — Stacking Rules Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 7: Stacking Rules — Duration Refresh ───────────────────────────
// **Validates: Requirements 6.1, 6.2, 6.4**
//
// For any status type already active on an actor with remaining duration D1,
// applying the same status type with duration D2 SHALL result in the active
// duration being max(D1, D2). Additionally, if the existing instance is permanent
// (duration 0), any temporary reapplication SHALL leave the permanent instance
// unchanged.

TEST_CASE("PBT: Property 7 — Stacking rules duration refresh", "[pbt][property][status-effects]")
{
    rc::prop("reapplying same status refreshes duration to max(D1, D2)", []() {
        // Generate a random StatusType
        const int typeInt = *rc::gen::inRange(0, static_cast<int>(StatusType::COUNT));
        const StatusType type = static_cast<StatusType>(typeInt);

        // Generate two positive durations (temporary effects: duration > 0)
        const int d1 = *rc::gen::inRange(1, 100);
        const int d2 = *rc::gen::inRange(1, 100);

        StatusEffectTracker tracker;

        // Apply first instance with duration D1
        tracker.apply(nullptr, type, d1, "test-source-1");

        // Apply second instance with duration D2 (same type)
        tracker.apply(nullptr, type, d2, "test-source-2");

        // Resulting duration should be max(D1, D2)
        const int expected = std::max(d1, d2);
        RC_ASSERT(tracker.getRemainingDuration(type) == expected);

        // Should still be only one instance of this status type
        int count = 0;
        for (const auto& effect : tracker.getActiveEffects()) {
            if (effect.type == type) count++;
        }
        RC_ASSERT(count == 1);
    });

    rc::prop("permanent status (duration 0) remains unchanged when temporary reapplication occurs", []() {
        // Generate a random StatusType
        const int typeInt = *rc::gen::inRange(0, static_cast<int>(StatusType::COUNT));
        const StatusType type = static_cast<StatusType>(typeInt);

        // Generate a positive duration for the temporary reapplication
        const int tempDuration = *rc::gen::inRange(1, 100);

        StatusEffectTracker tracker;

        // Apply permanent instance (duration 0)
        tracker.apply(nullptr, type, 0, "permanent-source");

        // Try to reapply with a temporary duration
        tracker.apply(nullptr, type, tempDuration, "temporary-source");

        // Permanent instance should remain unchanged (duration still 0)
        RC_ASSERT(tracker.getRemainingDuration(type) == 0);
        RC_ASSERT(tracker.has(type));

        // Should still be only one instance
        int count = 0;
        for (const auto& effect : tracker.getActiveEffects()) {
            if (effect.type == type) count++;
        }
        RC_ASSERT(count == 1);
    });
}
