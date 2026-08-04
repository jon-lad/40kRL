// ═══════════════════════════════════════════════════════════════════════════════
// Feature: action-system — Property-Based Tests for ActionBudget & ActionRegistry
// ═══════════════════════════════════════════════════════════════════════════════
//
// TDD: These tests are written BEFORE the implementation exists.
// They will not compile until ActionBudget.h and ActionRegistry.h are created.

#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include "ActionBudget.h"
#include "ActionRegistry.h"

// ─── Property 1: AP budget conservation ──────────────────────────────────────
// For any sequence of valid spend() calls with costs in {0,1,2}, the sum of
// deducted costs equals MAX_AP minus final getAP().
// **Validates: Requirements 1.1, 1.2, 1.3, 1.4**

TEST_CASE("PBT: Property 1 — AP budget conservation", "[property][action-system]")
{
    rc::prop("sum of spent costs equals MAX_AP minus final AP", []() {
        ActionBudget budget;
        budget.beginTurn();

        // Generate a random sequence of costs (0, 1, or 2)
        auto costs = *rc::gen::container<int>(1, 6, rc::gen::inRange(0, 2));

        int totalSpent = 0;
        for (int cost : costs) {
            if (budget.canAfford(cost)) {
                bool ok = budget.spend(cost);
                if (ok) {
                    totalSpent += cost;
                }
            }
        }

        // Conservation: initial AP - final AP == total spent
        RC_ASSERT(ActionBudget::MAX_AP - budget.getAP() == totalSpent);
    });
}

// ─── Property 2: Action rejection preserves AP ───────────────────────────────
// For any (cost, currentAP) pair where cost > currentAP, spend() returns false
// and getAP() is unchanged.
// **Validates: Requirements 1.5, 1.6**

TEST_CASE("PBT: Property 2 — Action rejection preserves AP", "[property][action-system]")
{
    rc::prop("spend() with insufficient AP returns false and leaves AP unchanged", []() {
        ActionBudget budget;
        budget.beginTurn();

        // Generate a current AP in [0, 1] so we can always find a cost that exceeds it
        int currentAP = *rc::gen::inRange(0, 1);
        budget.setAP(currentAP);

        // Generate a cost that exceeds currentAP
        int cost = *rc::gen::inRange(currentAP + 1, 3);

        int apBefore = budget.getAP();
        bool result = budget.spend(cost);

        RC_ASSERT(result == false);
        RC_ASSERT(budget.getAP() == apBefore);
    });
}

// ─── Property 3: Turn termination at zero AP ─────────────────────────────────
// After action sequences that exhaust AP to 0, canAfford(1) and canAfford(2)
// both return false.
// **Validates: Requirements 1.7**

TEST_CASE("PBT: Property 3 — Turn termination at zero AP", "[property][action-system]")
{
    rc::prop("at zero AP, canAfford(1) and canAfford(2) both return false", []() {
        ActionBudget budget;
        budget.beginTurn();

        // Generate a sequence of costs that will exhaust AP
        auto costs = *rc::gen::container<int>(1, 4, rc::gen::inRange(1, 2));

        for (int cost : costs) {
            if (budget.canAfford(cost)) {
                budget.spend(cost);
            }
        }

        // Force AP to 0 if not already exhausted
        budget.setAP(0);

        RC_ASSERT(budget.getAP() == 0);
        RC_ASSERT(budget.canAfford(1) == false);
        RC_ASSERT(budget.canAfford(2) == false);
        // Free actions (cost 0) should still be allowed
        RC_ASSERT(budget.canAfford(0) == true);
    });
}

// ─── Property 4: Reaction refresh round-trip ─────────────────────────────────
// Generate random beginTurn/useReaction sequences; verify hasReaction() is true
// after beginTurn() and false after useReaction().
// **Validates: Requirements 4.1, 4.2, 4.3**

TEST_CASE("PBT: Property 4 — Reaction refresh round-trip", "[property][action-system]")
{
    rc::prop("hasReaction() is true after beginTurn, false after useReaction", []() {
        ActionBudget budget;

        // Generate random number of turns to simulate
        int numTurns = *rc::gen::inRange(1, 5);

        for (int turn = 0; turn < numTurns; ++turn) {
            budget.beginTurn();
            RC_ASSERT(budget.hasReaction() == true);

            // Randomly decide whether to use the reaction this turn
            bool useIt = *rc::gen::arbitrary_bool();
            if (useIt) {
                budget.useReaction();
                RC_ASSERT(budget.hasReaction() == false);

                // Using reaction again should not crash, still false
                // (defensive — no double-spend)
            }
        }

        // After the last beginTurn, reaction should always be refreshed
        budget.beginTurn();
        RC_ASSERT(budget.hasReaction() == true);
    });
}

// ─── Property 5: Aim bonus lifecycle ─────────────────────────────────────────
// Generate random aim/consume/endTurn sequences; verify bonus equals
// min(N×10, 20) and resets to 0 after consumeAimBonus() or clearAimBonus().
// **Validates: Requirements 12.1, 12.2, 12.3, 12.4**

TEST_CASE("PBT: Property 5 — Aim bonus lifecycle", "[property][action-system]")
{
    rc::prop("aim bonus equals min(N*10, 20) and resets after consume or clear", []() {
        ActionBudget budget;
        budget.beginTurn();

        // Generate a random number of aim actions [0, 4] (more than 2 tests capping)
        int numAims = *rc::gen::inRange(0, 4);

        for (int i = 0; i < numAims; ++i) {
            budget.addAimBonus();
        }

        // Verify aim bonus is min(N * AIM_PER_ACTION, MAX_AIM_BONUS)
        int expectedBonus = std::min(numAims * ActionBudget::AIM_PER_ACTION,
                                     ActionBudget::MAX_AIM_BONUS);
        RC_ASSERT(budget.getAimBonus() == expectedBonus);

        // Randomly choose how to reset: consume or clear via endTurn
        int resetMethod = *rc::gen::inRange(0, 1);
        if (resetMethod == 0) {
            budget.consumeAimBonus();
        } else {
            budget.clearAimBonus();
        }

        RC_ASSERT(budget.getAimBonus() == 0);
    });
}

// ─── Property 6: Action registry consistency ─────────────────────────────────
// Iterate all ActionId entries; verify Half=1 AP, Full=2 AP, Free=0 AP,
// Reaction=0 AP.
// **Validates: Requirements 11.2**

TEST_CASE("PBT: Property 6 — Action registry consistency", "[property][action-system]")
{
    rc::prop("all registry entries have cost matching their declared type", []() {
        // Generate a random ActionId index to check (covers all entries over many iterations)
        int idx = *rc::gen::inRange(0, static_cast<int>(ActionId::COUNT) - 1);
        ActionId id = static_cast<ActionId>(idx);

        const ActionMeta& meta = ActionRegistry::get(id);

        switch (meta.type) {
            case ActionType::HALF:
                RC_ASSERT(meta.apCost == 1);
                break;
            case ActionType::FULL:
                RC_ASSERT(meta.apCost == 2);
                break;
            case ActionType::FREE:
                RC_ASSERT(meta.apCost == 0);
                break;
            case ActionType::REACTION:
                RC_ASSERT(meta.apCost == 0);
                break;
        }

        // Also verify the id field matches the index
        RC_ASSERT(meta.id == id);
    });
}

// ─── Property 11: End Turn is always available ───────────────────────────────
// Generate random AP values [0..2]; verify End_Turn (cost 0) is always accepted.
// **Validates: Requirements 9.1, 9.2**

TEST_CASE("PBT: Property 11 — End Turn is always available", "[property][action-system]")
{
    rc::prop("End_Turn (free action, cost 0) is always accepted regardless of AP", []() {
        ActionBudget budget;
        budget.beginTurn();

        // Set AP to a random value in [0, 2]
        int ap = *rc::gen::inRange(0, 2);
        budget.setAP(ap);

        // End_Turn is a free action (cost 0) — should always be affordable
        RC_ASSERT(budget.canAfford(0) == true);

        // Spending 0 AP should succeed and not change AP
        int apBefore = budget.getAP();
        bool result = budget.spend(0);
        RC_ASSERT(result == true);
        RC_ASSERT(budget.getAP() == apBefore);

        // Also verify via the registry that End_Turn has cost 0
        const ActionMeta& endTurnMeta = ActionRegistry::get(ActionId::END_TURN);
        RC_ASSERT(endTurnMeta.apCost == 0);
        RC_ASSERT(endTurnMeta.type == ActionType::FREE);
        RC_ASSERT(ActionRegistry::canAfford(ActionId::END_TURN, ap));
    });
}
