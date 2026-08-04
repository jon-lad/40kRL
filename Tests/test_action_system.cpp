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


// ═══════════════════════════════════════════════════════════════════════════════
// Engine State Transition Unit Tests — Task 3.1
// ═══════════════════════════════════════════════════════════════════════════════
//
// TDD: These tests verify the ActionBudget conditions that drive Engine state
// transitions (IDLE → PLAYER_TURN → ENEMY_TURN → IDLE). The actual Engine
// states (PLAYER_TURN, ENEMY_TURN) will be added in task 3.2; these tests
// validate the transition CONDITIONS via ActionBudget.
//
// Requirements validated: 2.1, 2.2, 2.3, 3.1

// ─── Test: IDLE → PLAYER_TURN condition (beginTurn gives full AP) ────────────
// Requirement 2.1: While the player has AP remaining, the Engine SHALL keep the
// game in the player's turn state and accept further input.
// After beginTurn(), AP is full — this is the condition that enables PLAYER_TURN.

TEST_CASE("Engine transition: IDLE to PLAYER_TURN on beginTurn", "[action-system]")
{
    ActionBudget budget;
    budget.beginTurn();

    // After beginTurn, AP should be full — this enables PLAYER_TURN state
    REQUIRE(budget.getAP() == ActionBudget::MAX_AP);
    REQUIRE(budget.getAP() == 2);

    // Player should be able to afford actions (turn is active)
    REQUIRE(budget.canAfford(1) == true);
    REQUIRE(budget.canAfford(2) == true);

    // Reaction should also be refreshed at turn start
    REQUIRE(budget.hasReaction() == true);
}

// ─── Test: PLAYER_TURN → ENEMY_TURN condition (AP reaches 0) ────────────────
// Requirement 2.2: When the player's AP reaches 0, the Engine SHALL transition
// to the enemy turn phase.
// Spending all AP (two half actions) causes AP to reach 0 — triggers transition.

TEST_CASE("Engine transition: PLAYER_TURN to ENEMY_TURN when AP reaches 0", "[action-system]")
{
    ActionBudget budget;
    budget.beginTurn();

    // Simulate player spending AP via two half actions (1 AP each)
    REQUIRE(budget.spend(1) == true);
    REQUIRE(budget.getAP() == 1);

    // Still in PLAYER_TURN — AP > 0
    REQUIRE(budget.canAfford(1) == true);

    REQUIRE(budget.spend(1) == true);
    REQUIRE(budget.getAP() == 0);

    // AP has reached 0 — this is the condition that triggers ENEMY_TURN transition
    REQUIRE(budget.canAfford(1) == false);
    REQUIRE(budget.canAfford(2) == false);

    // Free actions are still allowed (cost 0), but turn should end
    REQUIRE(budget.canAfford(0) == true);
}

// ─── Test: PLAYER_TURN → ENEMY_TURN via Full Action (2 AP at once) ──────────
// Validates the same transition but through a single full action spend.

TEST_CASE("Engine transition: PLAYER_TURN to ENEMY_TURN via Full Action", "[action-system]")
{
    ActionBudget budget;
    budget.beginTurn();

    // Full action costs 2 AP — exhausts budget immediately
    REQUIRE(budget.spend(2) == true);
    REQUIRE(budget.getAP() == 0);

    // Transition condition met: AP == 0
    REQUIRE(budget.canAfford(1) == false);
    REQUIRE(budget.canAfford(2) == false);
}

// ─── Test: ENEMY_TURN → IDLE condition (enemy spends full budget) ────────────
// Requirement 3.1: When an enemy's turn begins, the MonsterAi SHALL spend its
// AP budget by selecting valid actions until AP reaches 0.
// After enemy exhausts AP, system returns to IDLE for next player turn.

TEST_CASE("Engine transition: ENEMY_TURN to IDLE after enemy spends AP", "[action-system]")
{
    // Simulate an enemy's turn budget
    ActionBudget enemyBudget;
    enemyBudget.beginTurn();

    REQUIRE(enemyBudget.getAP() == ActionBudget::MAX_AP);

    // Enemy performs two half actions (move + attack pattern)
    REQUIRE(enemyBudget.spend(1) == true);  // move toward player
    REQUIRE(enemyBudget.getAP() == 1);

    REQUIRE(enemyBudget.spend(1) == true);  // attack player
    REQUIRE(enemyBudget.getAP() == 0);

    // Enemy budget exhausted — condition for transitioning back to IDLE
    REQUIRE(enemyBudget.canAfford(1) == false);
    REQUIRE(enemyBudget.canAfford(2) == false);
}

// ─── Test: Multiple enemies all exhaust AP before returning to IDLE ──────────
// Requirement 2.2/3.1: All enemies must finish before returning to IDLE.

TEST_CASE("Engine transition: All enemies exhaust AP before IDLE", "[action-system]")
{
    // Simulate three enemies each spending their budget
    ActionBudget enemy1, enemy2, enemy3;
    enemy1.beginTurn();
    enemy2.beginTurn();
    enemy3.beginTurn();

    // Enemy 1: two half actions
    enemy1.spend(1);
    enemy1.spend(1);
    REQUIRE(enemy1.getAP() == 0);

    // Enemy 2: one full action
    enemy2.spend(2);
    REQUIRE(enemy2.getAP() == 0);

    // Enemy 3: can't do anything useful, ends immediately
    enemy3.setAP(0);
    REQUIRE(enemy3.getAP() == 0);

    // All enemies at 0 AP — transition to IDLE is valid
    REQUIRE(enemy1.canAfford(1) == false);
    REQUIRE(enemy2.canAfford(1) == false);
    REQUIRE(enemy3.canAfford(1) == false);
}

// ─── Test: End_Turn free action sets AP to 0 and triggers enemy turn ─────────
// Requirement 2.3: When the player performs End_Turn, the Engine SHALL set
// the player's remaining AP to 0 and transition to the enemy turn phase.

TEST_CASE("Engine transition: End_Turn sets AP to 0", "[action-system]")
{
    ActionBudget budget;
    budget.beginTurn();

    // Player has full AP but chooses to end turn early
    REQUIRE(budget.getAP() == 2);

    // End_Turn is a free action: sets AP to 0 directly
    budget.setAP(0);

    REQUIRE(budget.getAP() == 0);
    // This triggers the ENEMY_TURN transition condition
    REQUIRE(budget.canAfford(1) == false);
    REQUIRE(budget.canAfford(2) == false);
}

// ─── Test: End_Turn works even with partial AP spent ─────────────────────────
// Player has 1 AP left and decides to end turn rather than use it.

TEST_CASE("Engine transition: End_Turn with partial AP remaining", "[action-system]")
{
    ActionBudget budget;
    budget.beginTurn();

    // Spend 1 AP on a half action (e.g., move)
    REQUIRE(budget.spend(1) == true);
    REQUIRE(budget.getAP() == 1);

    // Player decides to end turn with 1 AP remaining
    budget.setAP(0);

    REQUIRE(budget.getAP() == 0);
    REQUIRE(budget.canAfford(1) == false);
}

// ─── Test: End_Turn is always valid (even at 0 AP) ───────────────────────────
// Requirement 9.1/9.2: Free actions are allowed regardless of remaining AP.

TEST_CASE("Engine transition: End_Turn valid at 0 AP", "[action-system]")
{
    ActionBudget budget;
    budget.beginTurn();

    // Exhaust AP first
    budget.spend(2);
    REQUIRE(budget.getAP() == 0);

    // setAP(0) on already-zero AP should be safe (idempotent)
    budget.setAP(0);
    REQUIRE(budget.getAP() == 0);
}

// ─── Test: beginTurn resets state for next round (IDLE → PLAYER_TURN again) ──
// After enemy turns complete and we return to IDLE, the next beginTurn()
// must reset AP for the player's new turn.

TEST_CASE("Engine transition: beginTurn resets for new round", "[action-system]")
{
    ActionBudget budget;

    // First turn
    budget.beginTurn();
    budget.spend(1);
    budget.spend(1);
    REQUIRE(budget.getAP() == 0);

    // Simulate: enemy turns run, then IDLE → new PLAYER_TURN
    budget.beginTurn();
    REQUIRE(budget.getAP() == ActionBudget::MAX_AP);
    REQUIRE(budget.canAfford(1) == true);
    REQUIRE(budget.canAfford(2) == true);
    REQUIRE(budget.hasReaction() == true);
}
