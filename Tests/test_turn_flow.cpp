#include "lib/catch_amalgamated.hpp"

#include "TurnFlow.hpp"
#include "Engine.hpp"

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: death-screen-not-showing — Bug-Condition Exploration Test (Task 1)
// ═══════════════════════════════════════════════════════════════════════════════
//
// Bug summary: when the player dies, PlayerDestructible::die() sets
// gameStatus = DEFEAT, but Engine::update() unconditionally reassigns
// gameStatus = IDLE at several end-of-turn sites within the same frame,
// clobbering DEFEAT before Engine::render() is reached — so the death screen
// (leaderboard overlay) never appears.
//
// The turn loop cannot be exercised directly in the headless test binary (no
// initialized Engine/GUI/Map — see .kiro/steering/test-isolation.md), so the
// fix is verified against the pure predicate seam
//   bool shouldEndTurnToIdle(bool playerDead, Engine::GameStatus current);
// which encodes the "should this end-of-turn site reset to IDLE?" decision.
//
// EXPLORATION NOTE: On UNFIXED code the seam (Headers/TurnFlow.hpp /
// Source/TurnFlow.cpp) does not exist yet, so this test fails to BUILD/LINK
// (unresolved external symbol, LNK2001). That build/link failure is the
// documented exploration signal that confirms the bug — the pre-fix turn loop
// maps a dead-player input to a reset-to-IDLE (the wrong decision). Once the
// seam is introduced (Task 3.1), these assertions encode and validate the fix:
// a dead player must never be reset to IDLE, and DEFEAT must be preserved.
//
// **Validates: Requirements 1.1, 1.2, 1.3, 2.1, 2.2, 2.3, 2.6**

TEST_CASE("Bug condition: a dead player is never reset to IDLE (DEFEAT preserved)", "[turn-flow]")
{
    // Dead player at an IDLE-assignment site: the loop must NOT reset to IDLE.
    // (Encodes the tick-death handler / trailing end-of-turn reset cases.)
    REQUIRE(shouldEndTurnToIdle(true, Engine::IDLE) == false);

    // Dead player with DEFEAT already set by die(): DEFEAT must be preserved.
    REQUIRE(shouldEndTurnToIdle(true, Engine::DEFEAT) == false);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: death-screen-not-showing — Preservation Property Tests (Task 2)
// ═══════════════════════════════════════════════════════════════════════════════
//
// Property 2: Preservation — a LIVING player still transitions to IDLE at the
// end-of-turn sites exactly as before the fix, so no living-player turn flow is
// regressed. These assertions target the same pure predicate seam used by the
// Task 1 exploration test:
//   bool shouldEndTurnToIdle(bool playerDead, Engine::GameStatus current);
//
// Observation-first methodology: they capture the behavior the fix must
// preserve for NON-buggy inputs (isBugCondition == false, i.e. playerDead ==
// false). On UNFIXED code they fail to build/link (same missing seam as Task 1);
// once the seam is added (Task 3.1) they PASS immediately, locking in the
// living-player transition and the DEFEAT fixed point.
//
// **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.5, 3.6**

#include "lib/rapidcheck_catch.h"

TEST_CASE("Preservation: a living player at a non-DEFEAT status still transitions to IDLE", "[turn-flow]")
{
    // Living player, AP exhausted after player->update() / runEnemyTurns()
    // (PLAYER_TURN path): still resets to IDLE as before (Req 3.1, 3.2).
    REQUIRE(shouldEndTurnToIdle(false, Engine::PLAYER_TURN) == true);

    // Living player in the IDLE branch: still resets to IDLE as before (Req 3.1).
    REQUIRE(shouldEndTurnToIdle(false, Engine::IDLE) == true);

    // Living player in the legacy NEW_TURN path: still resets to IDLE as before
    // (Req 3.2). Also covers the stunned-skip path, which ends the same way (Req 3.3).
    REQUIRE(shouldEndTurnToIdle(false, Engine::NEW_TURN) == true);
}

TEST_CASE("PBT: Property 2 — living player resets to IDLE; dead player never does", "[pbt][property][turn-flow]")
{
    // rc::check runs >= 100 iterations. Generators use the project's inclusive
    // rc::gen::inRange convention (see .kiro/steering/test-isolation.md).
    rc::check("shouldEndTurnToIdle: dead => false; living & non-DEFEAT => true", []() {
        // Generate a GameStatus across the FULL enum domain. inRange is INCLUSIVE
        // in this project's stub, so [0, HIGH_SCORES] covers STARTUP..HIGH_SCORES.
        const Engine::GameStatus status =
            static_cast<Engine::GameStatus>(*rc::gen::inRange(0, static_cast<int>(Engine::HIGH_SCORES)));

        // Generate a dead/alive flag. This project's rapidcheck stub exposes
        // arbitrary_bool() (there is no arbitrary<bool>() template).
        const bool playerDead = *rc::gen::arbitrary_bool();

        if (playerDead) {
            // Property 1 (fix): a dead player is NEVER reset to IDLE, for any
            // status — DEFEAT set by die() must persist to render().
            RC_ASSERT(shouldEndTurnToIdle(playerDead, status) == false);
        }

        if (!playerDead && status != Engine::DEFEAT) {
            // Property 2 (preservation): a living player at a non-DEFEAT status
            // still transitions to IDLE — identical to the original
            // unconditional-IDLE behavior for all living-player turn flow.
            RC_ASSERT(shouldEndTurnToIdle(playerDead, status) == true);
        }
    });
}
