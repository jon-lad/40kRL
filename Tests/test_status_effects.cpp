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
        // apply() now automatically calls applyModifiers() internally
        const int duration = *rc::gen::inRange(1, 20);
        tracker.apply(&actor, type, duration, "test-modifier");

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

        // Apply the status — apply() now calls applyModifiers() internally
        const int duration = *rc::gen::inRange(1, 20);
        tracker.apply(&actor, type, duration, "test-modifier");

        // Now remove the status — remove() now calls removeModifiers() internally
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

        // Apply Stunned — apply() now calls applyModifiers() internally
        const int duration = *rc::gen::inRange(1, 5);
        tracker.apply(&actor, StatusType::Stunned, duration, "test-stun");

        // Stunned should prevent action
        RC_ASSERT(!tracker.canAct());

        // No CharId characteristics should be modified (Evasion is not a CharId field)
        RC_ASSERT(actor.characteristics->getModifier(CharId::WS) == wsModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::BS) == bsModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::S) == sModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::T) == tModBefore);
        RC_ASSERT(actor.characteristics->getModifier(CharId::Ag) == agModBefore);

        // After removal, canAct returns true — remove() now calls removeModifiers() internally
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


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects, Property 5: Burning Tick Damage
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 5: Burning Tick Damage ─────────────────────────────────────────
// **Validates: Requirements 5.1, 9.3**
//
// For any actor with an active Burning status, calling tickStartOfTurn SHALL
// deal damage in the range [1, 10] (1d10 Energy, ignoring armour) to the
// actor's HP.

TEST_CASE("PBT: Property 5 — Burning tick damage", "[pbt][property][status-effects]")
{
    rc::prop("Burning tick deals damage in [1,10] ignoring armour", []() {
        // Generate a random defense value in [0, 50] to verify armour is ignored
        const float defense = static_cast<float>(*rc::gen::inRange(0, 51));

        // Generate random maxHp high enough to survive one tick [50, 200]
        const float maxHp = static_cast<float>(*rc::gen::inRange(50, 201));

        // Create an actor with a destructible component
        Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
        actor.destructible = std::make_shared<MonsterDestructible>(maxHp, defense, "corpse", 0);

        // Record HP before tick
        const float hpBefore = actor.destructible->hp;

        // Apply Burning status (duration 3 is typical)
        StatusEffectTracker tracker;
        tracker.apply(&actor, StatusType::Burning, 3, "test-burning");

        // Tick start of turn — should deal burning damage
        tracker.tickStartOfTurn(&actor);

        // Calculate damage dealt
        const float hpAfter = actor.destructible->hp;
        const float damageTaken = hpBefore - hpAfter;

        // Damage must be in [1, 10] (1d10 Energy)
        RC_ASSERT(damageTaken >= 1.0f);
        RC_ASSERT(damageTaken <= 10.0f);

        // Damage must be a whole number (integer roll)
        RC_ASSERT(damageTaken == static_cast<float>(static_cast<int>(damageTaken)));
    });

    rc::prop("Burning damage ignores armour — damage dealt regardless of defense value", []() {
        // Use a very high defense value to prove damage bypasses it
        const float highDefense = static_cast<float>(*rc::gen::inRange(20, 100));
        const float maxHp = 100.0f;

        Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
        actor.destructible = std::make_shared<MonsterDestructible>(maxHp, highDefense, "corpse", 0);

        const float hpBefore = actor.destructible->hp;

        StatusEffectTracker tracker;
        tracker.apply(&actor, StatusType::Burning, 3, "test-burning");

        tracker.tickStartOfTurn(&actor);

        const float hpAfter = actor.destructible->hp;
        const float damageTaken = hpBefore - hpAfter;

        // Even with high defense, damage MUST be dealt (>= 1)
        // If armour were applied, high defense could reduce damage to 0
        RC_ASSERT(damageTaken >= 1.0f);
        RC_ASSERT(damageTaken <= 10.0f);
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects, Property 6: Bleeding Tick Damage
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 6: Bleeding Tick Damage ────────────────────────────────────────
// **Validates: Requirements 5.5, 9.4**
//
// For any actor with an active Bleeding status, calling tickEndOfTurn SHALL deal
// exactly 1 wound of damage to the actor's HP.

TEST_CASE("PBT: Property 6 — Bleeding tick damage", "[pbt][property][status-effects]")
{
    rc::prop("tickEndOfTurn deals exactly 1 wound when Bleeding is active", []() {
        // Generate random HP in range [2, 100] to ensure actor doesn't die from 1 wound
        const int hpValue = *rc::gen::inRange(2, 101);
        const float hp = static_cast<float>(hpValue);

        // Create an Actor with a Destructible component
        Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
        actor.destructible = std::make_shared<MonsterDestructible>(hp, 0.0f, "corpse", 0);

        // MonsterDestructible constructor adds CRIT_BUFFER to hp internally.
        // Record actual internal HP before tick.
        const float previousHp = actor.destructible->hp;

        // Apply Bleeding (permanent, duration 0)
        StatusEffectTracker tracker;
        tracker.apply(&actor, StatusType::Bleeding, 0, "test-bleed");

        // Call tickEndOfTurn — should deal exactly 1 wound
        tracker.tickEndOfTurn(&actor);

        // Verify HP decreased by exactly 1
        RC_ASSERT(previousHp - actor.destructible->hp == 1.0f);
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects, Property 2: Weapon Quality Trigger Correctness
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 2: Weapon Quality Trigger Correctness ──────────────────────────
// **Validates: Requirements 3.1, 3.2, 3.3**
//
// For any weapon qualities list containing a status-triggering quality string
// ("Flame", "Shocking", or "Toxic"), calling StatusTrigger::fromWeaponQualities
// SHALL apply the corresponding StatusType with the correct default duration to
// the target actor. Non-triggering quality strings SHALL produce no status effects.

#include "StatusTrigger.h"

TEST_CASE("PBT: Property 2 — Weapon quality trigger correctness", "[pbt][property][status-effects]")
{
    // Known triggering qualities and their expected outcomes
    // "Flame"    → Burning  (duration 3)
    // "Shocking" → Stunned  (duration 1)
    // "Toxic"    → Poisoned (duration 5)

    // Known noise strings that should NOT trigger any status
    const std::vector<std::string> noisePool = {
        "Balanced", "Accurate", "Reliable", "Tearing"
    };

    const std::vector<std::string> triggerPool = {
        "Flame", "Shocking", "Toxic"
    };

    rc::prop("triggering qualities apply correct statuses with correct durations", []() {
        // Build a random qualities list mixing triggers and noise
        const std::vector<std::string> triggerOptions = { "Flame", "Shocking", "Toxic" };
        const std::vector<std::string> noiseOptions = { "Balanced", "Accurate", "Reliable", "Tearing" };

        std::vector<std::string> qualities;

        // Randomly include each trigger quality (0 or 1 times)
        const bool includeFlame = *rc::gen::arbitrary_bool();
        const bool includeShocking = *rc::gen::arbitrary_bool();
        const bool includeToxic = *rc::gen::arbitrary_bool();

        if (includeFlame) qualities.push_back("Flame");
        if (includeShocking) qualities.push_back("Shocking");
        if (includeToxic) qualities.push_back("Toxic");

        // Add random noise qualities (0 to 3 noise strings)
        const int noiseCount = *rc::gen::inRange(0, 4);
        for (int i = 0; i < noiseCount; ++i) {
            const int noiseIdx = *rc::gen::inRange(0, static_cast<int>(noiseOptions.size()));
            qualities.push_back(noiseOptions[noiseIdx]);
        }

        // Create an actor with a status tracker
        Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
        StatusEffectTracker tracker;

        // Call fromWeaponQualities — applies effects to the tracker via the actor
        // The actor needs a statusTracker pointer for the function to use
        actor.statusTracker = std::make_unique<StatusEffectTracker>();

        StatusTrigger::fromWeaponQualities(&actor, qualities);

        // Verify: if "Flame" was in the list → actor has Burning with duration 3
        if (includeFlame) {
            RC_ASSERT(actor.statusTracker->has(StatusType::Burning));
            RC_ASSERT(actor.statusTracker->getRemainingDuration(StatusType::Burning) == 3);
        } else {
            RC_ASSERT(!actor.statusTracker->has(StatusType::Burning));
        }

        // Verify: if "Shocking" was in the list → actor has Stunned with duration 1
        if (includeShocking) {
            RC_ASSERT(actor.statusTracker->has(StatusType::Stunned));
            RC_ASSERT(actor.statusTracker->getRemainingDuration(StatusType::Stunned) == 1);
        } else {
            RC_ASSERT(!actor.statusTracker->has(StatusType::Stunned));
        }

        // Verify: if "Toxic" was in the list → actor has Poisoned with duration 5
        if (includeToxic) {
            RC_ASSERT(actor.statusTracker->has(StatusType::Poisoned));
            RC_ASSERT(actor.statusTracker->getRemainingDuration(StatusType::Poisoned) == 5);
        } else {
            RC_ASSERT(!actor.statusTracker->has(StatusType::Poisoned));
        }
    });

    rc::prop("noise-only qualities produce no status effects", []() {
        const std::vector<std::string> noiseOptions = { "Balanced", "Accurate", "Reliable", "Tearing" };

        // Build a qualities list containing ONLY noise strings (1 to 4)
        const int noiseCount = *rc::gen::inRange(1, 5);
        std::vector<std::string> qualities;
        for (int i = 0; i < noiseCount; ++i) {
            const int noiseIdx = *rc::gen::inRange(0, static_cast<int>(noiseOptions.size()));
            qualities.push_back(noiseOptions[noiseIdx]);
        }

        // Create an actor with status tracker
        Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
        actor.statusTracker = std::make_unique<StatusEffectTracker>();

        StatusTrigger::fromWeaponQualities(&actor, qualities);

        // No status effects should be applied
        RC_ASSERT(!actor.statusTracker->has(StatusType::Burning));
        RC_ASSERT(!actor.statusTracker->has(StatusType::Stunned));
        RC_ASSERT(!actor.statusTracker->has(StatusType::Poisoned));

        // Active effects list should be empty
        RC_ASSERT(actor.statusTracker->getActiveEffects().empty());
    });

    rc::prop("empty qualities list produces no status effects", []() {
        std::vector<std::string> qualities; // empty

        Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
        actor.statusTracker = std::make_unique<StatusEffectTracker>();

        StatusTrigger::fromWeaponQualities(&actor, qualities);

        // No status effects should be applied
        RC_ASSERT(actor.statusTracker->getActiveEffects().empty());
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects, Property 1: Critical Trigger Table Correctness
// ═══════════════════════════════════════════════════════════════════════════════

#include "StatusTrigger.h"
#include "HitLocation.h"
#include "WeaponTypes.h"

// ─── Property 1: Critical Trigger Table Correctness ──────────────────────────
// **Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 2.10, 2.11, 2.12, 2.13, 2.14, 2.15, 2.16, 2.17, 2.18**
//
// For any valid combination of (DamageType, HitLocation, magnitude) where
// magnitude meets or exceeds a trigger threshold defined in the crit trigger
// table, calling StatusTrigger::fromCritical SHALL apply exactly the
// StatusType(s) specified by the table to the target actor.

namespace {
    // Reference crit trigger table — hardcoded from the design document.
    // Duration of 3 is used for "1d5" entries (deterministic test value).
    struct RefTrigger {
        DamageType dmg;
        HitLocation loc;
        int minMag;
        StatusType status;
        int duration;
    };

    const std::vector<RefTrigger>& getReferenceTriggerTable() {
        static const std::vector<RefTrigger> table = {
            // Energy
            { DamageType::E, HitLocation::HEAD,      3, StatusType::Blinded,           3 },
            { DamageType::E, HitLocation::HEAD,      5, StatusType::Burning,            3 },
            { DamageType::E, HitLocation::RIGHT_ARM, 5, StatusType::Missing_Right_Arm,  0 },
            { DamageType::E, HitLocation::LEFT_ARM,  5, StatusType::Missing_Left_Arm,   0 },
            { DamageType::E, HitLocation::BODY,      3, StatusType::Burning,            3 },
            { DamageType::E, HitLocation::RIGHT_LEG, 5, StatusType::Missing_Right_Leg,  0 },
            { DamageType::E, HitLocation::LEFT_LEG,  5, StatusType::Missing_Left_Leg,   0 },
            // Impact
            { DamageType::I, HitLocation::HEAD,      1, StatusType::Stunned,            1 },
            { DamageType::I, HitLocation::BODY,      5, StatusType::Prone,              0 },
            { DamageType::I, HitLocation::RIGHT_ARM, 6, StatusType::Missing_Right_Arm,  0 },
            { DamageType::I, HitLocation::LEFT_ARM,  6, StatusType::Missing_Left_Arm,   0 },
            { DamageType::I, HitLocation::RIGHT_LEG, 6, StatusType::Missing_Right_Leg,  0 },
            { DamageType::I, HitLocation::LEFT_LEG,  6, StatusType::Missing_Left_Leg,   0 },
            // Rending
            { DamageType::R, HitLocation::HEAD,      1, StatusType::Bleeding,           0 },
            { DamageType::R, HitLocation::HEAD,      5, StatusType::Blinded,            3 },
            { DamageType::R, HitLocation::RIGHT_ARM, 6, StatusType::Missing_Right_Arm,  0 },
            { DamageType::R, HitLocation::LEFT_ARM,  6, StatusType::Missing_Left_Arm,   0 },
            { DamageType::R, HitLocation::RIGHT_LEG, 5, StatusType::Missing_Right_Leg,  0 },
            { DamageType::R, HitLocation::LEFT_LEG,  5, StatusType::Missing_Left_Leg,   0 },
            { DamageType::R, HitLocation::BODY,      4, StatusType::Bleeding,           0 },
            { DamageType::R, HitLocation::BODY,      4, StatusType::Prone,              0 },
            // Explosive
            { DamageType::X, HitLocation::HEAD,      1, StatusType::Stunned,            1 },
            { DamageType::X, HitLocation::RIGHT_ARM, 4, StatusType::Missing_Right_Arm,  0 },
            { DamageType::X, HitLocation::LEFT_ARM,  4, StatusType::Missing_Left_Arm,   0 },
            { DamageType::X, HitLocation::RIGHT_LEG, 4, StatusType::Missing_Right_Leg,  0 },
            { DamageType::X, HitLocation::LEFT_LEG,  4, StatusType::Missing_Left_Leg,   0 },
            { DamageType::X, HitLocation::BODY,      4, StatusType::Bleeding,           0 },
            { DamageType::X, HitLocation::BODY,      4, StatusType::Prone,              0 },
        };
        return table;
    }

    // Given a (DamageType, HitLocation, magnitude), compute expected set of triggered statuses.
    std::vector<StatusType> expectedStatuses(DamageType dmg, HitLocation loc, int magnitude) {
        std::vector<StatusType> result;
        for (const auto& entry : getReferenceTriggerTable()) {
            if (entry.dmg == dmg && entry.loc == loc && magnitude >= entry.minMag) {
                result.push_back(entry.status);
            }
        }
        return result;
    }
}

TEST_CASE("PBT: Property 1 — Critical trigger table correctness", "[pbt][property][status-effects]")
{
    rc::prop("fromCritical applies exactly the correct statuses for any (DamageType, HitLocation, magnitude)", []() {
        // Generate random DamageType: E=0, X=1, I=2, R=3
        const int dmgInt = *rc::gen::inRange(0, 4);
        const DamageType dmgType = static_cast<DamageType>(dmgInt);

        // Generate random HitLocation: HEAD=0, RIGHT_ARM=1, LEFT_ARM=2, BODY=3, RIGHT_LEG=4, LEFT_LEG=5
        const int locInt = *rc::gen::inRange(0, 6);
        const HitLocation loc = static_cast<HitLocation>(locInt);

        // Generate random magnitude in [1, 9]
        const int magnitude = *rc::gen::inRange(1, 10);

        // Build expected set of triggered statuses from the reference table
        std::vector<StatusType> expected = expectedStatuses(dmgType, loc, magnitude);

        // Create an actor with statusTracker
        Actor actor(0, 0, '@', "TestActor", TCODColor(255, 255, 255));
        actor.statusTracker = std::make_unique<StatusEffectTracker>();

        // Call fromCritical
        StatusTrigger::fromCritical(&actor, dmgType, loc, magnitude);

        // Collect actual applied statuses
        const auto& effects = actor.statusTracker->getActiveEffects();
        std::vector<StatusType> actual;
        for (const auto& e : effects) {
            actual.push_back(e.type);
        }

        // Sort both for comparison
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());

        // Verify exactly the correct statuses were applied (no more, no less)
        RC_ASSERT(actual.size() == expected.size());
        RC_ASSERT(actual == expected);
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects — Unit Tests: Weapon Quality Parsing (fromWeaponQualities)
// Task 8.2: Validates Requirements 3.1, 3.2, 3.3, 3.4
// ═══════════════════════════════════════════════════════════════════════════════

#include "StatusTrigger.h"

namespace {
    // Helper: creates an Actor with statusTracker initialized for weapon quality tests.
    Actor makeActorWithTracker() {
        Actor actor(0, 0, '@', "TestTarget", TCODColor(255, 255, 255));
        actor.statusTracker = std::make_unique<StatusEffectTracker>();
        return actor;
    }
}

TEST_CASE("Weapon quality: Flame applies Burning with duration 3", "[status-effects]")
{
    // Requirement 3.1: Flame quality → Burning status
    Actor actor = makeActorWithTracker();
    std::vector<std::string> qualities = { "Flame" };

    StatusTrigger::fromWeaponQualities(&actor, qualities);

    REQUIRE(actor.statusTracker->has(StatusType::Burning));
    CHECK(actor.statusTracker->getRemainingDuration(StatusType::Burning) == 3);
}

TEST_CASE("Weapon quality: Shocking applies Stunned with duration 1", "[status-effects]")
{
    // Requirement 3.2: Shocking quality → Stunned with duration 1
    Actor actor = makeActorWithTracker();
    std::vector<std::string> qualities = { "Shocking" };

    StatusTrigger::fromWeaponQualities(&actor, qualities);

    REQUIRE(actor.statusTracker->has(StatusType::Stunned));
    CHECK(actor.statusTracker->getRemainingDuration(StatusType::Stunned) == 1);
}

TEST_CASE("Weapon quality: Toxic applies Poisoned with duration 5", "[status-effects]")
{
    // Requirement 3.3: Toxic quality → Poisoned with duration 5
    Actor actor = makeActorWithTracker();
    std::vector<std::string> qualities = { "Toxic" };

    StatusTrigger::fromWeaponQualities(&actor, qualities);

    REQUIRE(actor.statusTracker->has(StatusType::Poisoned));
    CHECK(actor.statusTracker->getRemainingDuration(StatusType::Poisoned) == 5);
}

TEST_CASE("Weapon quality: Multiple qualities apply all corresponding effects", "[status-effects]")
{
    // Requirement 3.4: Qualities read from Lua table structure (vector<string> interface)
    // Both "Flame" and "Toxic" should produce their respective statuses
    Actor actor = makeActorWithTracker();
    std::vector<std::string> qualities = { "Flame", "Toxic" };

    StatusTrigger::fromWeaponQualities(&actor, qualities);

    REQUIRE(actor.statusTracker->has(StatusType::Burning));
    CHECK(actor.statusTracker->getRemainingDuration(StatusType::Burning) == 3);

    REQUIRE(actor.statusTracker->has(StatusType::Poisoned));
    CHECK(actor.statusTracker->getRemainingDuration(StatusType::Poisoned) == 5);
}

TEST_CASE("Weapon quality: Unknown quality does not apply any status effect", "[status-effects]")
{
    // Non-status-triggering qualities like "Balanced" should be silently ignored
    Actor actor = makeActorWithTracker();
    std::vector<std::string> qualities = { "Balanced" };

    StatusTrigger::fromWeaponQualities(&actor, qualities);

    CHECK_FALSE(actor.statusTracker->has(StatusType::Burning));
    CHECK_FALSE(actor.statusTracker->has(StatusType::Stunned));
    CHECK_FALSE(actor.statusTracker->has(StatusType::Poisoned));
    CHECK(actor.statusTracker->getActiveEffects().empty());
}

TEST_CASE("Weapon quality: Empty qualities list applies no status effects", "[status-effects]")
{
    // An empty qualities vector (e.g., Combat Knife) should produce no effects
    Actor actor = makeActorWithTracker();
    std::vector<std::string> qualities = {};

    StatusTrigger::fromWeaponQualities(&actor, qualities);

    CHECK(actor.statusTracker->getActiveEffects().empty());
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects — Unit Tests: Specific Crit Scenarios
// Task 7.2: Validates Requirements 2.1, 2.2, 2.3, 2.6, 2.7, 2.10, 2.17, 2.18
// ═══════════════════════════════════════════════════════════════════════════════

#include "StatusTrigger.h"

TEST_CASE("Crit scenario: Energy Head magnitude 3 applies Blinded", "[status-effects]")
{
    Actor actor(0, 0, '@', "Test", TCODColor(255, 255, 255));
    actor.statusTracker = std::make_unique<StatusEffectTracker>();

    StatusTrigger::fromCritical(&actor, DamageType::E, HitLocation::HEAD, 3);

    CHECK(actor.statusTracker->has(StatusType::Blinded));
}

TEST_CASE("Crit scenario: Energy Head magnitude 5 applies Burning", "[status-effects]")
{
    Actor actor(0, 0, '@', "Test", TCODColor(255, 255, 255));
    actor.statusTracker = std::make_unique<StatusEffectTracker>();

    StatusTrigger::fromCritical(&actor, DamageType::E, HitLocation::HEAD, 5);

    CHECK(actor.statusTracker->has(StatusType::Burning));
}

TEST_CASE("Crit scenario: Energy Right Arm magnitude 5 applies Missing_Right_Arm", "[status-effects]")
{
    Actor actor(0, 0, '@', "Test", TCODColor(255, 255, 255));
    actor.statusTracker = std::make_unique<StatusEffectTracker>();

    StatusTrigger::fromCritical(&actor, DamageType::E, HitLocation::RIGHT_ARM, 5);

    CHECK(actor.statusTracker->has(StatusType::Missing_Right_Arm));
}

TEST_CASE("Crit scenario: Energy Left Arm magnitude 5 applies Missing_Left_Arm", "[status-effects]")
{
    Actor actor(0, 0, '@', "Test", TCODColor(255, 255, 255));
    actor.statusTracker = std::make_unique<StatusEffectTracker>();

    StatusTrigger::fromCritical(&actor, DamageType::E, HitLocation::LEFT_ARM, 5);

    CHECK(actor.statusTracker->has(StatusType::Missing_Left_Arm));
}

TEST_CASE("Crit scenario: Impact Head magnitude 1 applies Stunned", "[status-effects]")
{
    Actor actor(0, 0, '@', "Test", TCODColor(255, 255, 255));
    actor.statusTracker = std::make_unique<StatusEffectTracker>();

    StatusTrigger::fromCritical(&actor, DamageType::I, HitLocation::HEAD, 1);

    CHECK(actor.statusTracker->has(StatusType::Stunned));
}

TEST_CASE("Crit scenario: Impact Body magnitude 5 applies Prone", "[status-effects]")
{
    Actor actor(0, 0, '@', "Test", TCODColor(255, 255, 255));
    actor.statusTracker = std::make_unique<StatusEffectTracker>();

    StatusTrigger::fromCritical(&actor, DamageType::I, HitLocation::BODY, 5);

    CHECK(actor.statusTracker->has(StatusType::Prone));
}

TEST_CASE("Crit scenario: Rending Head magnitude 1 applies Bleeding", "[status-effects]")
{
    Actor actor(0, 0, '@', "Test", TCODColor(255, 255, 255));
    actor.statusTracker = std::make_unique<StatusEffectTracker>();

    StatusTrigger::fromCritical(&actor, DamageType::R, HitLocation::HEAD, 1);

    CHECK(actor.statusTracker->has(StatusType::Bleeding));
}

TEST_CASE("Crit scenario: Rending Body magnitude 4 applies Bleeding and Prone (dual)", "[status-effects]")
{
    Actor actor(0, 0, '@', "Test", TCODColor(255, 255, 255));
    actor.statusTracker = std::make_unique<StatusEffectTracker>();

    StatusTrigger::fromCritical(&actor, DamageType::R, HitLocation::BODY, 4);

    CHECK(actor.statusTracker->has(StatusType::Bleeding));
    CHECK(actor.statusTracker->has(StatusType::Prone));
}

TEST_CASE("Crit scenario: Explosive Body magnitude 4 applies Bleeding and Prone (dual)", "[status-effects]")
{
    Actor actor(0, 0, '@', "Test", TCODColor(255, 255, 255));
    actor.statusTracker = std::make_unique<StatusEffectTracker>();

    StatusTrigger::fromCritical(&actor, DamageType::X, HitLocation::BODY, 4);

    CHECK(actor.statusTracker->has(StatusType::Bleeding));
    CHECK(actor.statusTracker->has(StatusType::Prone));
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: status-effects — Unit Tests: Attacker Integration
// Task 9.1: Validates Requirements 2.1, 3.1
//
// These tests verify that the StatusTrigger functions (fromCritical and
// fromWeaponQualities) correctly apply status effects to target actors when
// called in the same sequence as the combat pipeline in
// Attacker::resolveCharacterAttack. Since the actual pipeline requires a full
// engine/gui/map setup, we test the integration points directly:
//   1. fromCritical triggered by a crit event → target gains status
//   2. fromWeaponQualities triggered by weapon qualities → target gains status
//   3. Combined scenario simulating the full combat pipeline sequence
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Attacker integration: crit triggers StatusTrigger::fromCritical and applies status to target", "[status-effects]")
{
    // Simulates what happens in Attacker::resolveCharacterAttack when a crit occurs.
    // After InjuryTracker::applyCrit, the pipeline calls:
    //   StatusTrigger::fromCritical(target, dmgType, loc, critMagnitude)
    // The target should receive the appropriate status effects.

    // Set up target actor — needs a statusTracker (auto-created by fromCritical if missing)
    Actor target(5, 5, 'o', "Ork Boy", TCODColor(0, 255, 0));
    target.destructible = std::make_shared<MonsterDestructible>(20.0f, 0.0f, "ork corpse", 10);
    target.characteristics = std::make_shared<Characteristics>(30);

    SECTION("Impact Head crit magnitude 1 applies Stunned to target") {
        // This is the same call the Attacker pipeline makes after a crit is resolved
        StatusTrigger::fromCritical(&target, DamageType::I, HitLocation::HEAD, 1);

        REQUIRE(target.statusTracker != nullptr);
        CHECK(target.statusTracker->has(StatusType::Stunned));
        CHECK(target.statusTracker->getRemainingDuration(StatusType::Stunned) == 1);
    }

    SECTION("Energy Body crit magnitude 3 applies Burning to target") {
        StatusTrigger::fromCritical(&target, DamageType::E, HitLocation::BODY, 3);

        REQUIRE(target.statusTracker != nullptr);
        CHECK(target.statusTracker->has(StatusType::Burning));
        CHECK(target.statusTracker->getRemainingDuration(StatusType::Burning) == 3);
    }

    SECTION("Rending Body crit magnitude 4 applies both Bleeding and Prone to target") {
        StatusTrigger::fromCritical(&target, DamageType::R, HitLocation::BODY, 4);

        REQUIRE(target.statusTracker != nullptr);
        CHECK(target.statusTracker->has(StatusType::Bleeding));
        CHECK(target.statusTracker->has(StatusType::Prone));
    }

    SECTION("fromCritical auto-creates statusTracker on target if missing") {
        // Target starts without a statusTracker
        CHECK(target.statusTracker == nullptr);

        StatusTrigger::fromCritical(&target, DamageType::I, HitLocation::HEAD, 1);

        // After the call, statusTracker exists and has the effect
        REQUIRE(target.statusTracker != nullptr);
        CHECK(target.statusTracker->has(StatusType::Stunned));
    }
}

TEST_CASE("Attacker integration: successful hit with weapon qualities triggers StatusTrigger::fromWeaponQualities", "[status-effects]")
{
    // Simulates the post-hit weapon quality processing in Attacker::resolveCharacterAttack.
    // After any successful hit (whether crit or normal damage), the pipeline calls:
    //   StatusTrigger::fromWeaponQualities(target, weaponItem->equippable->meleeStats->qualities)
    // The target should receive status effects for each triggering quality.

    Actor target(5, 5, 'o', "Ork Boy", TCODColor(0, 255, 0));
    target.destructible = std::make_shared<MonsterDestructible>(20.0f, 0.0f, "ork corpse", 10);
    target.characteristics = std::make_shared<Characteristics>(30);

    SECTION("Flame quality applies Burning to target after hit") {
        std::vector<std::string> qualities = { "Flame" };
        StatusTrigger::fromWeaponQualities(&target, qualities);

        REQUIRE(target.statusTracker != nullptr);
        CHECK(target.statusTracker->has(StatusType::Burning));
        CHECK(target.statusTracker->getRemainingDuration(StatusType::Burning) == 3);
    }

    SECTION("Shocking quality applies Stunned to target after hit") {
        std::vector<std::string> qualities = { "Shocking" };
        StatusTrigger::fromWeaponQualities(&target, qualities);

        REQUIRE(target.statusTracker != nullptr);
        CHECK(target.statusTracker->has(StatusType::Stunned));
        CHECK(target.statusTracker->getRemainingDuration(StatusType::Stunned) == 1);
    }

    SECTION("Toxic quality applies Poisoned to target after hit") {
        std::vector<std::string> qualities = { "Toxic" };
        StatusTrigger::fromWeaponQualities(&target, qualities);

        REQUIRE(target.statusTracker != nullptr);
        CHECK(target.statusTracker->has(StatusType::Poisoned));
        CHECK(target.statusTracker->getRemainingDuration(StatusType::Poisoned) == 5);
    }

    SECTION("Weapon with multiple triggering qualities applies all effects") {
        std::vector<std::string> qualities = { "Flame", "Toxic", "Balanced" };
        StatusTrigger::fromWeaponQualities(&target, qualities);

        REQUIRE(target.statusTracker != nullptr);
        CHECK(target.statusTracker->has(StatusType::Burning));
        CHECK(target.statusTracker->has(StatusType::Poisoned));
        // "Balanced" is not a trigger quality — no extra effects
        CHECK_FALSE(target.statusTracker->has(StatusType::Stunned));
    }

    SECTION("fromWeaponQualities auto-creates statusTracker on target if missing") {
        CHECK(target.statusTracker == nullptr);

        std::vector<std::string> qualities = { "Shocking" };
        StatusTrigger::fromWeaponQualities(&target, qualities);

        REQUIRE(target.statusTracker != nullptr);
        CHECK(target.statusTracker->has(StatusType::Stunned));
    }
}

TEST_CASE("Attacker integration: combined crit + weapon qualities pipeline applies all statuses to target", "[status-effects]")
{
    // Simulates the FULL combat pipeline integration path:
    // 1. Attacker hits target and deals a crit (magnitude 1 Impact to Head → Stunned)
    // 2. Then weapon qualities are processed (Flame → Burning)
    // Both statuses should coexist on the target after the pipeline completes.

    // Set up target actor
    Actor target(5, 5, 'o', "Ork Boy", TCODColor(0, 255, 0));
    target.destructible = std::make_shared<MonsterDestructible>(20.0f, 0.0f, "ork corpse", 10);
    target.characteristics = std::make_shared<Characteristics>(30);

    // Step 1: Crit pipeline — Impact Head magnitude 1 triggers Stunned
    StatusTrigger::fromCritical(&target, DamageType::I, HitLocation::HEAD, 1);

    // Step 2: Weapon quality pipeline — Flame quality triggers Burning
    std::vector<std::string> qualities = { "Flame" };
    StatusTrigger::fromWeaponQualities(&target, qualities);

    // Both statuses should be active simultaneously on the target
    REQUIRE(target.statusTracker != nullptr);
    CHECK(target.statusTracker->has(StatusType::Stunned));
    CHECK(target.statusTracker->has(StatusType::Burning));

    // Verify correct durations
    CHECK(target.statusTracker->getRemainingDuration(StatusType::Stunned) == 1);
    CHECK(target.statusTracker->getRemainingDuration(StatusType::Burning) == 3);

    // Verify total effect count (exactly 2)
    CHECK(target.statusTracker->getActiveEffects().size() == 2);
}

TEST_CASE("Attacker integration: status effects persist on target after combat resolution", "[status-effects]")
{
    // Verifies that after the combat pipeline applies status effects, they remain
    // queryable on the target actor and affect subsequent turns (tick processing).

    Actor target(5, 5, 'o', "Ork Boy", TCODColor(0, 255, 0));
    target.destructible = std::make_shared<MonsterDestructible>(50.0f, 0.0f, "ork corpse", 10);
    target.characteristics = std::make_shared<Characteristics>(40);

    // Simulate crit pipeline: Energy Head magnitude 3 → Blinded (duration 1d5, random)
    StatusTrigger::fromCritical(&target, DamageType::E, HitLocation::HEAD, 3);

    REQUIRE(target.statusTracker != nullptr);
    REQUIRE(target.statusTracker->has(StatusType::Blinded));

    // The Blinded status should apply mechanical modifiers
    // WS -30, BS -30 from Blinded
    CHECK(target.statusTracker->getWSModifier() == -30);
    CHECK(target.statusTracker->getBSModifier() == -30);

    // Attackers get +30 against this blinded target
    CHECK(target.statusTracker->getAttackerBonusAgainstMe() == 30);

    // Status persists across tick (duration > 0, decrements by 1 per tick)
    const int durationBefore = target.statusTracker->getRemainingDuration(StatusType::Blinded);
    REQUIRE(durationBefore >= 1);
    REQUIRE(durationBefore <= 5);

    // Tick once — duration should decrement
    target.statusTracker->tickStartOfTurn(&target);

    if (durationBefore > 1) {
        CHECK(target.statusTracker->has(StatusType::Blinded));
        CHECK(target.statusTracker->getRemainingDuration(StatusType::Blinded) == durationBefore - 1);
    } else {
        // Duration was 1, so after tick it expires
        CHECK_FALSE(target.statusTracker->has(StatusType::Blinded));
    }
}
