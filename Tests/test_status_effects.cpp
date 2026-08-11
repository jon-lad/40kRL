#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"
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


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects, Property 3: Duration Tick and Expiry
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 3: Duration Tick and Expiry ────────────────────────────────────
// **Validates: Requirements 1.4, 4.1, 4.2**
//
// For any temporary status effect (duration > 0) applied to an actor, calling
// tickStartOfTurn SHALL decrement the duration by exactly 1, and when the
// duration reaches 0 after decrement, the effect SHALL be removed from the
// active effects list and its associated characteristic modifiers SHALL be
// reversed.

TEST_CASE("PBT: Property 3 — Duration tick and expiry", "[pbt][property][status-effects]")
{
    rc::prop("duration decrements by 1 each tick and effect removed at 0", []() {
        // Generate random positive duration in [1, 20]
        const int duration = *rc::gen::inRange(1, 21);

        // Pick a random status type for the test
        const int typeInt = *rc::gen::inRange(0, static_cast<int>(StatusType::COUNT));
        const StatusType type = static_cast<StatusType>(typeInt);

        StatusEffectTracker tracker;
        tracker.apply(nullptr, type, duration, "test-source");

        // Tick D-1 times: effect should still exist with decreasing duration
        for (int tick = 1; tick < duration; ++tick) {
            tracker.tickStartOfTurn(nullptr);

            // Effect must still be present
            RC_ASSERT(tracker.has(type));

            // Duration must have decremented by 1 per tick
            const int expected = duration - tick;
            RC_ASSERT(tracker.getRemainingDuration(type) == expected);
        }

        // The D-th tick should bring duration to 0 and remove the effect
        tracker.tickStartOfTurn(nullptr);

        // Effect must be removed
        RC_ASSERT(!tracker.has(type));
        RC_ASSERT(tracker.getRemainingDuration(type) == -1);
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects — Unit Tests: Default Durations
// ═══════════════════════════════════════════════════════════════════════════════

// These tests document the expected default durations for each status effect
// when triggered via the StatusTrigger system (weapon qualities / critical hits).
// Since StatusTrigger is not yet implemented, we directly apply effects with the
// expected durations and verify the tracker stores them correctly.
//
// **Validates: Requirements 4.3, 4.4, 4.5, 4.6, 4.7**

TEST_CASE("Default duration: Stunned lasts 1 turn", "[status-effects]")
{
    StatusEffectTracker tracker;
    constexpr int STUNNED_DEFAULT_DURATION = 1;

    tracker.apply(nullptr, StatusType::Stunned, STUNNED_DEFAULT_DURATION, "Impact Head crit 1");

    REQUIRE(tracker.has(StatusType::Stunned));
    CHECK(tracker.getRemainingDuration(StatusType::Stunned) == 1);
    CHECK_FALSE(tracker.getActiveEffects().front().isPermanent());
}

TEST_CASE("Default duration: Burning lasts 3 turns", "[status-effects]")
{
    StatusEffectTracker tracker;
    constexpr int BURNING_DEFAULT_DURATION = 3;

    tracker.apply(nullptr, StatusType::Burning, BURNING_DEFAULT_DURATION, "Flame weapon quality");

    REQUIRE(tracker.has(StatusType::Burning));
    CHECK(tracker.getRemainingDuration(StatusType::Burning) == 3);
    CHECK_FALSE(tracker.getActiveEffects().front().isPermanent());
}

TEST_CASE("Default duration: Poisoned lasts 5 turns", "[status-effects]")
{
    StatusEffectTracker tracker;
    constexpr int POISONED_DEFAULT_DURATION = 5;

    tracker.apply(nullptr, StatusType::Poisoned, POISONED_DEFAULT_DURATION, "Toxic weapon quality");

    REQUIRE(tracker.has(StatusType::Poisoned));
    CHECK(tracker.getRemainingDuration(StatusType::Poisoned) == 5);
    CHECK_FALSE(tracker.getActiveEffects().front().isPermanent());
}

TEST_CASE("Default duration: Blinded duration is in [1,5] range (1d5)", "[status-effects]")
{
    // Blinded is triggered by Energy Head crit magnitude 3 with a duration of 1d5 turns.
    // Any value in [1, 5] is valid. We verify the tracker accepts all values in this range.
    for (int roll = 1; roll <= 5; ++roll) {
        StatusEffectTracker tracker;
        tracker.apply(nullptr, StatusType::Blinded, roll, "Energy Head crit 3");

        REQUIRE(tracker.has(StatusType::Blinded));
        CHECK(tracker.getRemainingDuration(StatusType::Blinded) == roll);
        CHECK_FALSE(tracker.getActiveEffects().front().isPermanent());
    }

    // Values outside [1,5] should not occur for Blinded's default trigger,
    // but we verify the boundary values are stored correctly.
    SECTION("Minimum valid roll (1)") {
        StatusEffectTracker tracker;
        tracker.apply(nullptr, StatusType::Blinded, 1, "Energy Head crit 3");
        CHECK(tracker.getRemainingDuration(StatusType::Blinded) == 1);
    }

    SECTION("Maximum valid roll (5)") {
        StatusEffectTracker tracker;
        tracker.apply(nullptr, StatusType::Blinded, 5, "Energy Head crit 3");
        CHECK(tracker.getRemainingDuration(StatusType::Blinded) == 5);
    }
}

TEST_CASE("Default duration: Bleeding never auto-expires (permanent)", "[status-effects]")
{
    StatusEffectTracker tracker;
    constexpr int BLEEDING_DURATION_PERMANENT = 0;

    tracker.apply(nullptr, StatusType::Bleeding, BLEEDING_DURATION_PERMANENT, "Rending Head crit 1");

    REQUIRE(tracker.has(StatusType::Bleeding));
    CHECK(tracker.getRemainingDuration(StatusType::Bleeding) == 0);
    CHECK(tracker.getActiveEffects().front().isPermanent());
}

// ═══════════════════════════════════════════════════════════════════════════════
// StatusEffectTracker — Modifier Application and Removal Symmetry
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 4: Modifier Application and Removal Symmetry ───────────────────
// **Validates: Requirements 5.2, 5.4, 5.9, 5.11, 10.3**
//
// For any status effect type that modifies characteristics (Prone: WS -10;
// Blinded: WS -30, BS -30; Poisoned: S -10, T -10, Ag -10), applying the status
// SHALL add the specified modifier to the actor's characteristics, and removing
// the status SHALL subtract the same modifier, resulting in the original
// characteristic values being restored.
//
// Note: Stunned's Evasion -20 does not map to a CharId field; it is verified
// via canAct() returning false (action prevention), not via Characteristics modifiers.

namespace {
    // Helper: creates a minimal Actor with characteristics initialized to a given base value.
    Actor makeTestActor(int baseStats = 40) {
        Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
        actor.characteristics = std::make_shared<Characteristics>(baseStats);
        return actor;
    }

    // The subset of StatusTypes that produce characteristic modifiers.
    constexpr StatusType kModifierTypes[] = {
        StatusType::Prone,
        StatusType::Blinded,
        StatusType::Poisoned
    };
    constexpr int kModifierTypeCount = 3;
}

TEST_CASE("PBT: Property 4 — Modifier application and removal symmetry", "[pbt][property][status-effects]")
{
    rc::prop("applying a modifier-producing status changes characteristics by expected delta", []() {
        // Pick a random modifier-producing status type (0=Prone, 1=Blinded, 2=Poisoned)
        const int idx = *rc::gen::inRange(0, 3);
        const StatusType type = kModifierTypes[idx];

        // Generate a random base stat value in [20, 80] to avoid clamping issues
        const int baseStat = *rc::gen::inRange(20, 80);

        // Create actor with known base stats
        Actor actor = makeTestActor(baseStat);
        StatusEffectTracker tracker;

        // Record baseline modifiers (should all be 0 initially)
        const int wsModBefore = actor.characteristics->getModifier(CharId::WS);
        const int bsModBefore = actor.characteristics->getModifier(CharId::BS);
        const int sModBefore  = actor.characteristics->getModifier(CharId::S);
        const int tModBefore  = actor.characteristics->getModifier(CharId::T);
        const int agModBefore = actor.characteristics->getModifier(CharId::Ag);

        // Apply the status (with a random duration > 0)
        const int duration = *rc::gen::inRange(1, 20);
        tracker.apply(&actor, type, duration, "test-modifier");
        tracker.applyModifiers(&actor, type);

        // Verify the expected modifier delta was applied
        if (type == StatusType::Prone) {
            RC_ASSERT(actor.characteristics->getModifier(CharId::WS) == wsModBefore + (-10));
            RC_ASSERT(actor.characteristics->getModifier(CharId::BS) == bsModBefore);
            RC_ASSERT(actor.characteristics->getModifier(CharId::S) == sModBefore);
            RC_ASSERT(actor.characteristics->getModifier(CharId::T) == tModBefore);
            RC_ASSERT(actor.characteristics->getModifier(CharId::Ag) == agModBefore);
        } else if (type == StatusType::Blinded) {
            RC_ASSERT(actor.characteristics->getModifier(CharId::WS) == wsModBefore + (-30));
            RC_ASSERT(actor.characteristics->getModifier(CharId::BS) == bsModBefore + (-30));
            RC_ASSERT(actor.characteristics->getModifier(CharId::S) == sModBefore);
            RC_ASSERT(actor.characteristics->getModifier(CharId::T) == tModBefore);
            RC_ASSERT(actor.characteristics->getModifier(CharId::Ag) == agModBefore);
        } else if (type == StatusType::Poisoned) {
            RC_ASSERT(actor.characteristics->getModifier(CharId::S) == sModBefore + (-10));
            RC_ASSERT(actor.characteristics->getModifier(CharId::T) == tModBefore + (-10));
            RC_ASSERT(actor.characteristics->getModifier(CharId::Ag) == agModBefore + (-10));
            RC_ASSERT(actor.characteristics->getModifier(CharId::WS) == wsModBefore);
            RC_ASSERT(actor.characteristics->getModifier(CharId::BS) == bsModBefore);
        }
    });

    rc::prop("removing a modifier-producing status restores characteristics to baseline", []() {
        // Pick a random modifier-producing status type
        const int idx = *rc::gen::inRange(0, 3);
        const StatusType type = kModifierTypes[idx];

        // Generate a random base stat value in [20, 80]
        const int baseStat = *rc::gen::inRange(20, 80);

        // Create actor with known base stats
        Actor actor = makeTestActor(baseStat);
        StatusEffectTracker tracker;

        // Record baseline modifiers
        const int wsModBefore = actor.characteristics->getModifier(CharId::WS);
        const int bsModBefore = actor.characteristics->getModifier(CharId::BS);
        const int sModBefore  = actor.characteristics->getModifier(CharId::S);
        const int tModBefore  = actor.characteristics->getModifier(CharId::T);
        const int agModBefore = actor.characteristics->getModifier(CharId::Ag);

        // Apply the status
        const int duration = *rc::gen::inRange(1, 20);
        tracker.apply(&actor, type, duration, "test-modifier");
        tracker.applyModifiers(&actor, type);

        // Now remove the status
        tracker.removeModifiers(&actor, type);
        tracker.remove(&actor, type);

        // Verify ALL modifiers are restored to baseline
        RC_ASSERT(actor.characteristics->getModifier(CharId::WS) == wsModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::BS) == bsModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::S) == sModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::T) == tModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::Ag) == agModBefore);
    });

    rc::prop("Stunned status prevents action but does not modify any CharId-based characteristic", []() {
        const int baseStat = *rc::gen::inRange(20, 80);
        Actor actor = makeTestActor(baseStat);
        StatusEffectTracker tracker;

        // Record all modifier baselines
        const int wsModBefore = actor.characteristics->getModifier(CharId::WS);
        const int bsModBefore = actor.characteristics->getModifier(CharId::BS);
        const int sModBefore  = actor.characteristics->getModifier(CharId::S);
        const int tModBefore  = actor.characteristics->getModifier(CharId::T);
        const int agModBefore = actor.characteristics->getModifier(CharId::Ag);

        // Apply Stunned
        const int duration = *rc::gen::inRange(1, 5);
        tracker.apply(&actor, StatusType::Stunned, duration, "test-stun");
        tracker.applyModifiers(&actor, StatusType::Stunned);

        // Stunned should prevent action
        RC_ASSERT(!tracker.canAct());

        // No CharId characteristics should be modified (Evasion is not a CharId field)
        RC_ASSERT(actor.characteristics->getModifier(CharId::WS) == wsModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::BS) == bsModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::S) == sModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::T) == tModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::Ag) == agModBefore);

        // After removal, canAct returns true
        tracker.removeModifiers(&actor, StatusType::Stunned);
        tracker.remove(&actor, StatusType::Stunned);
        RC_ASSERT(tracker.canAct());
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects — Unit Tests for Mechanical Effects
// Task 4.2: Validates Requirements 5.2, 5.3, 5.4, 5.6, 5.7, 5.8, 5.9, 5.10, 5.11
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Mechanical effects: Prone applies WS -10 penalty", "[status-effects]")
{
    StatusEffectTracker tracker;

    SECTION("No Prone — WS modifier is 0") {
        CHECK(tracker.getWSModifier() == 0);
    }

    SECTION("Prone active — WS modifier includes -10") {
        tracker.apply(nullptr, StatusType::Prone, 0, "test");
        CHECK(tracker.getWSModifier() == -10);
    }

    SECTION("Prone removed — WS modifier returns to 0") {
        tracker.apply(nullptr, StatusType::Prone, 0, "test");
        tracker.remove(nullptr, StatusType::Prone);
        CHECK(tracker.getWSModifier() == 0);
    }
}

TEST_CASE("Mechanical effects: Prone -10 ranged targeting modifier", "[status-effects]")
{
    StatusEffectTracker tracker;

    SECTION("No Prone — ranged targeting modifier is 0") {
        CHECK(tracker.getRangedTargetingModifier() == 0);
    }

    SECTION("Prone active — ranged targeting modifier is -10") {
        tracker.apply(nullptr, StatusType::Prone, 0, "test");
        CHECK(tracker.getRangedTargetingModifier() == -10);
    }

    SECTION("Prone removed — ranged targeting modifier returns to 0") {
        tracker.apply(nullptr, StatusType::Prone, 0, "test");
        tracker.remove(nullptr, StatusType::Prone);
        CHECK(tracker.getRangedTargetingModifier() == 0);
    }
}

TEST_CASE("Mechanical effects: Stunned prevents actions and Evasion -20", "[status-effects]")
{
    StatusEffectTracker tracker;

    SECTION("No Stunned — canAct is true and evasion modifier is 0") {
        CHECK(tracker.canAct() == true);
        CHECK(tracker.getEvasionModifier() == 0);
    }

    SECTION("Stunned active — canAct is false") {
        tracker.apply(nullptr, StatusType::Stunned, 1, "test");
        CHECK(tracker.canAct() == false);
    }

    SECTION("Stunned active — evasion modifier is -20") {
        tracker.apply(nullptr, StatusType::Stunned, 1, "test");
        CHECK(tracker.getEvasionModifier() == -20);
    }

    SECTION("Stunned removed — canAct returns to true and evasion restored") {
        tracker.apply(nullptr, StatusType::Stunned, 1, "test");
        tracker.remove(nullptr, StatusType::Stunned);
        CHECK(tracker.canAct() == true);
        CHECK(tracker.getEvasionModifier() == 0);
    }
}

TEST_CASE("Mechanical effects: Blinded applies WS -30 and BS -30", "[status-effects]")
{
    StatusEffectTracker tracker;

    SECTION("No Blinded — WS and BS modifiers are 0") {
        CHECK(tracker.getWSModifier() == 0);
        CHECK(tracker.getBSModifier() == 0);
    }

    SECTION("Blinded active — WS modifier is -30") {
        tracker.apply(nullptr, StatusType::Blinded, 3, "test");
        CHECK(tracker.getWSModifier() == -30);
    }

    SECTION("Blinded active — BS modifier is -30") {
        tracker.apply(nullptr, StatusType::Blinded, 3, "test");
        CHECK(tracker.getBSModifier() == -30);
    }

    SECTION("Blinded removed — modifiers return to 0") {
        tracker.apply(nullptr, StatusType::Blinded, 3, "test");
        tracker.remove(nullptr, StatusType::Blinded);
        CHECK(tracker.getWSModifier() == 0);
        CHECK(tracker.getBSModifier() == 0);
    }

    SECTION("Blinded + Prone stack WS penalties: -30 + -10 = -40") {
        tracker.apply(nullptr, StatusType::Blinded, 3, "test");
        tracker.apply(nullptr, StatusType::Prone, 0, "test");
        CHECK(tracker.getWSModifier() == -40);
    }
}

TEST_CASE("Mechanical effects: Blinded +30 attacker bonus", "[status-effects]")
{
    StatusEffectTracker tracker;

    SECTION("No Blinded — attacker bonus is 0") {
        CHECK(tracker.getAttackerBonusAgainstMe() == 0);
    }

    SECTION("Blinded active — attacker bonus is +30") {
        tracker.apply(nullptr, StatusType::Blinded, 3, "test");
        CHECK(tracker.getAttackerBonusAgainstMe() == 30);
    }

    SECTION("Blinded removed — attacker bonus returns to 0") {
        tracker.apply(nullptr, StatusType::Blinded, 3, "test");
        tracker.remove(nullptr, StatusType::Blinded);
        CHECK(tracker.getAttackerBonusAgainstMe() == 0);
    }
}

TEST_CASE("Mechanical effects: Poisoned applies S -10, T -10, Ag -10", "[status-effects]")
{
    StatusEffectTracker tracker;

    SECTION("No Poisoned — all modifiers are 0") {
        CHECK(tracker.getSModifier() == 0);
        CHECK(tracker.getTModifier() == 0);
        CHECK(tracker.getAgModifier() == 0);
    }

    SECTION("Poisoned active — S modifier is -10") {
        tracker.apply(nullptr, StatusType::Poisoned, 5, "test");
        CHECK(tracker.getSModifier() == -10);
    }

    SECTION("Poisoned active — T modifier is -10") {
        tracker.apply(nullptr, StatusType::Poisoned, 5, "test");
        CHECK(tracker.getTModifier() == -10);
    }

    SECTION("Poisoned active — Ag modifier is -10") {
        tracker.apply(nullptr, StatusType::Poisoned, 5, "test");
        CHECK(tracker.getAgModifier() == -10);
    }

    SECTION("Poisoned removed — all modifiers return to 0") {
        tracker.apply(nullptr, StatusType::Poisoned, 5, "test");
        tracker.remove(nullptr, StatusType::Poisoned);
        CHECK(tracker.getSModifier() == 0);
        CHECK(tracker.getTModifier() == 0);
        CHECK(tracker.getAgModifier() == 0);
    }
}

TEST_CASE("Mechanical effects: Missing_Arm prevents weapon equip in corresponding slot", "[status-effects]")
{
    StatusEffectTracker tracker;

    SECTION("No Missing_Arm — all slots enabled") {
        CHECK(tracker.isSlotDisabled(0) == false); // WEAPON
        CHECK(tracker.isSlotDisabled(1) == false); // OFFHAND
    }

    SECTION("Missing_Right_Arm — WEAPON slot (0) is disabled") {
        tracker.apply(nullptr, StatusType::Missing_Right_Arm, 0, "crit");
        CHECK(tracker.isSlotDisabled(0) == true);
        CHECK(tracker.isSlotDisabled(1) == false); // OFFHAND still enabled
    }

    SECTION("Missing_Left_Arm — OFFHAND slot (1) is disabled") {
        tracker.apply(nullptr, StatusType::Missing_Left_Arm, 0, "crit");
        CHECK(tracker.isSlotDisabled(0) == false); // WEAPON still enabled
        CHECK(tracker.isSlotDisabled(1) == true);
    }

    SECTION("Both arms missing — both weapon slots disabled") {
        tracker.apply(nullptr, StatusType::Missing_Right_Arm, 0, "crit-R");
        tracker.apply(nullptr, StatusType::Missing_Left_Arm, 0, "crit-L");
        CHECK(tracker.isSlotDisabled(0) == true);
        CHECK(tracker.isSlotDisabled(1) == true);
    }

    SECTION("HEAD and BODY slots are never disabled by Missing_Arm") {
        tracker.apply(nullptr, StatusType::Missing_Right_Arm, 0, "crit");
        tracker.apply(nullptr, StatusType::Missing_Left_Arm, 0, "crit");
        CHECK(tracker.isSlotDisabled(2) == false); // HEAD
        CHECK(tracker.isSlotDisabled(3) == false); // BODY
    }
}

TEST_CASE("Mechanical effects: Missing_Arm prevents two-handed weapons", "[status-effects]")
{
    StatusEffectTracker tracker;

    SECTION("No Missing_Arm — two-handed not blocked") {
        CHECK(tracker.isTwoHandedBlocked() == false);
    }

    SECTION("Missing_Right_Arm — two-handed blocked") {
        tracker.apply(nullptr, StatusType::Missing_Right_Arm, 0, "crit");
        CHECK(tracker.isTwoHandedBlocked() == true);
    }

    SECTION("Missing_Left_Arm — two-handed blocked") {
        tracker.apply(nullptr, StatusType::Missing_Left_Arm, 0, "crit");
        CHECK(tracker.isTwoHandedBlocked() == true);
    }

    SECTION("Both arms missing — two-handed still blocked") {
        tracker.apply(nullptr, StatusType::Missing_Right_Arm, 0, "crit-R");
        tracker.apply(nullptr, StatusType::Missing_Left_Arm, 0, "crit-L");
        CHECK(tracker.isTwoHandedBlocked() == true);
    }

    SECTION("After removing Missing_Arm — two-handed unblocked") {
        tracker.apply(nullptr, StatusType::Missing_Right_Arm, 0, "crit");
        tracker.remove(nullptr, StatusType::Missing_Right_Arm);
        CHECK(tracker.isTwoHandedBlocked() == false);
    }
}

TEST_CASE("Mechanical effects: Missing_Leg halves movement (2 AP/tile)", "[status-effects]")
{
    StatusEffectTracker tracker;

    SECTION("No Missing_Leg — movement not halved") {
        CHECK(tracker.isMovementHalved() == false);
    }

    SECTION("Missing_Right_Leg — movement halved") {
        tracker.apply(nullptr, StatusType::Missing_Right_Leg, 0, "crit");
        CHECK(tracker.isMovementHalved() == true);
    }

    SECTION("Missing_Left_Leg — movement halved") {
        tracker.apply(nullptr, StatusType::Missing_Left_Leg, 0, "crit");
        CHECK(tracker.isMovementHalved() == true);
    }

    SECTION("Both legs missing — movement still halved (no double-halving)") {
        tracker.apply(nullptr, StatusType::Missing_Right_Leg, 0, "crit-R");
        tracker.apply(nullptr, StatusType::Missing_Left_Leg, 0, "crit-L");
        CHECK(tracker.isMovementHalved() == true);
    }

    SECTION("After removing one Missing_Leg with other remaining — still halved") {
        tracker.apply(nullptr, StatusType::Missing_Right_Leg, 0, "crit-R");
        tracker.apply(nullptr, StatusType::Missing_Left_Leg, 0, "crit-L");
        tracker.remove(nullptr, StatusType::Missing_Right_Leg);
        CHECK(tracker.isMovementHalved() == true);
    }

    SECTION("After removing all Missing_Leg — movement not halved") {
        tracker.apply(nullptr, StatusType::Missing_Right_Leg, 0, "crit");
        tracker.remove(nullptr, StatusType::Missing_Right_Leg);
        CHECK(tracker.isMovementHalved() == false);
    }
}
