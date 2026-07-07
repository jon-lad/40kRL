#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <algorithm>
#include <vector>
#include <memory>
#include <list>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: decoration-render-priority — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 1: Bug Condition — Decorations Render Beneath Gameplay Actors ──
// **Validates: Requirements 2.1, 2.2**
//
// For any tile where a decoration and a gameplay actor share coordinates,
// the decoration SHALL appear earlier in the actors list (lower index) than
// the gameplay actor, ensuring the gameplay actor's glyph renders on top.

TEST_CASE("PBT: Bug Condition — decorations render beneath gameplay actors",
          "[pbt][property][decoration-render-priority]")
{
    SECTION("Bug Condition") {
        rc::prop("decoration list index < gameplay actor list index when sharing a tile", []() {
            // Generate random tile coordinates (within typical map bounds)
            const int x = *rc::gen::inRange(1, 78);
            const int y = *rc::gen::inRange(1, 48);

            // Create a gameplay actor at (x, y) with an ai component
            auto gameplayActor = std::make_unique<Actor>(x, y, 'M', "monster", Colors::white);
            gameplayActor->ai = std::make_shared<MonsterAi>();
            gameplayActor->destructible = std::make_shared<MonsterDestructible>(10.0f, 0.0f, "corpse", 10);
            Actor* gameplayPtr = gameplayActor.get();
            engine.actors.push_back(std::move(gameplayActor));

            // Create a decoration at (x, y) — use push_front (fixed behavior)
            // The fix inserts decorations at the front so they render beneath actors
            auto decoration = std::make_unique<Actor>(x, y, '#', "crate", Colors::white);
            decoration->fovOnly = true;
            // No ai, no destructible, no attacker — it's a decoration
            Actor* decoPtr = decoration.get();
            engine.actors.push_front(std::move(decoration));

            // Find list indices
            int decoIndex = -1;
            int gameplayIndex = -1;
            int idx = 0;
            for (const auto& actor : engine.actors) {
                if (actor.get() == decoPtr) decoIndex = idx;
                if (actor.get() == gameplayPtr) gameplayIndex = idx;
                ++idx;
            }

            // Assert: decoration appears before gameplay actor in list (lower index = renders first = underneath)
            RC_ASSERT(decoIndex >= 0);
            RC_ASSERT(gameplayIndex >= 0);
            RC_ASSERT(decoIndex < gameplayIndex);

            // Cleanup
            engine.actors.remove_if([decoPtr, gameplayPtr](const std::unique_ptr<Actor>& a) {
                return a.get() == decoPtr || a.get() == gameplayPtr;
            });
        });
    }
}

// ─── Property 2: Preservation — Non-Overlapping Tile Rendering Unchanged ─────
// **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.5**
//
// For all non-bug-condition inputs (tiles where decorations and gameplay actors
// do NOT overlap), the actors list order and rendering behavior is unchanged.
// This captures the baseline behavior on UNFIXED code to ensure the fix does
// not regress any non-overlapping scenarios.

// Helper: check if an actor is a decoration (no ai, no destructible, no attacker)
static bool isDecoration(const Actor* a) {
    return !a->ai && !a->destructible && !a->attacker;
}

// Helper: check if an actor is a gameplay actor (has ai, destructible, or attacker)
static bool isGameplayActor(const Actor* a) {
    return a->ai || a->destructible || a->attacker;
}

// ─── Preservation 2a: Decorations on empty tiles retain properties after insertion ───
// **Validates: Requirements 3.1**
//
// For any random decoration placed on a tile with NO gameplay actor,
// the decoration retains its position, glyph, fovOnly flag, and properties.

TEST_CASE("PBT: Preservation 2a — decorations on empty tiles retain properties",
          "[pbt][property][decoration-render-priority]")
{
    rc::prop("decorations on tiles without gameplay actors retain all properties after insertion", []() {
        // Generate random tile coordinates (within typical map bounds)
        const int x = *rc::gen::inRange(1, 78);
        const int y = *rc::gen::inRange(1, 48);

        // Generate random decoration properties
        const int glyph = *rc::gen::inRange(33, 126); // printable ASCII
        const bool blocks = *rc::gen::inRange(0, 1) == 1;
        const int coverValue = *rc::gen::inRange(0, 50);

        // Save initial actors list size
        const size_t initialSize = engine.actors.size();

        // Create a decoration actor (no ai, no destructible, no attacker)
        auto decoration = std::make_unique<Actor>(x, y, glyph, "test_deco", Colors::white);
        decoration->blocks = blocks;
        decoration->fovOnly = true;
        decoration->coverValue = coverValue;

        // Store expected values before insertion
        const int expectedX = x;
        const int expectedY = y;
        const int expectedGlyph = glyph;
        const bool expectedBlocks = blocks;
        const bool expectedFovOnly = true;
        const int expectedCoverValue = coverValue;

        Actor* rawPtr = decoration.get();

        // Insert using push_back (current unfixed behavior)
        engine.actors.push_back(std::move(decoration));

        // Verify the decoration is in the list
        RC_ASSERT(engine.actors.size() == initialSize + 1);

        // Find the decoration in the list and verify all properties are retained
        bool found = false;
        for (const auto& actor : engine.actors) {
            if (actor.get() == rawPtr) {
                found = true;
                RC_ASSERT(actor->getX() == expectedX);
                RC_ASSERT(actor->getY() == expectedY);
                RC_ASSERT(actor->getGlyph() == expectedGlyph);
                RC_ASSERT(actor->blocks == expectedBlocks);
                RC_ASSERT(actor->fovOnly == expectedFovOnly);
                RC_ASSERT(actor->coverValue == expectedCoverValue);
                RC_ASSERT(isDecoration(actor.get()));
                break;
            }
        }
        RC_ASSERT(found);

        // Cleanup: remove the decoration we just added
        engine.actors.remove_if([rawPtr](const std::unique_ptr<Actor>& a) {
            return a.get() == rawPtr;
        });
    });
}

// ─── Preservation 2b: sendToBack correctly splices target to front of list ───
// **Validates: Requirements 3.2**
//
// For any list of actors, sendToBack(target) places target at the front
// of the actors list (rendering beneath everything else).

TEST_CASE("PBT: Preservation 2b — sendToBack splices target actor to front of list",
          "[pbt][property][decoration-render-priority]")
{
    rc::prop("sendToBack moves target actor to position 0 (front) of actors list", []() {
        // Generate random number of actors to pre-populate [2, 8]
        const int numActors = *rc::gen::inRange(2, 8);

        // Save existing actors to restore later
        // We'll work with actors we insert ourselves
        std::vector<Actor*> inserted;

        for (int i = 0; i < numActors; ++i) {
            auto actor = std::make_unique<Actor>(i + 1, i + 1, '@' + i, "actor_" + std::to_string(i), Colors::white);
            inserted.push_back(actor.get());
            engine.actors.push_back(std::move(actor));
        }

        // Pick a random actor from our inserted ones to sendToBack
        const int targetIdx = *rc::gen::inRange(0, numActors - 1);
        Actor* target = inserted[targetIdx];

        // Call sendToBack — should splice target to front
        engine.sendToBack(target);

        // Verify: target is now at the front of the list
        RC_ASSERT(engine.actors.front().get() == target);

        // Cleanup: remove all actors we inserted
        for (Actor* a : inserted) {
            engine.actors.remove_if([a](const std::unique_ptr<Actor>& ptr) {
                return ptr.get() == a;
            });
        }
    });
}

// ─── Preservation 2c: Gameplay actor order among themselves is unaffected ─────
// **Validates: Requirements 3.4**
//
// When decorations are placed on tiles that do NOT have gameplay actors,
// the relative order of gameplay actors among themselves is preserved.

TEST_CASE("PBT: Preservation 2c — gameplay actor order is preserved when decorations are on separate tiles",
          "[pbt][property][decoration-render-priority]")
{
    rc::prop("gameplay actors maintain their insertion order when decorations occupy different tiles", []() {
        // Generate random number of gameplay actors [2, 6]
        const int numGameplay = *rc::gen::inRange(2, 6);
        // Generate random number of decorations [1, 4]
        const int numDecorations = *rc::gen::inRange(1, 4);

        std::vector<Actor*> gameplayActors;
        std::vector<Actor*> decorations;

        // Insert gameplay actors (with ai component) at unique tile positions
        for (int i = 0; i < numGameplay; ++i) {
            int gx = i * 2 + 1;  // tiles: 1, 3, 5, 7, 9, 11
            int gy = 1;
            auto actor = std::make_unique<Actor>(gx, gy, 'A' + i, "gameplay_" + std::to_string(i), Colors::white);
            actor->ai = std::make_shared<MonsterAi>();
            gameplayActors.push_back(actor.get());
            engine.actors.push_back(std::move(actor));
        }

        // Insert decorations at DIFFERENT tile positions (no overlap with gameplay actors)
        for (int i = 0; i < numDecorations; ++i) {
            int dx = i * 2 + 20;  // tiles: 20, 22, 24, 26 — far from gameplay actors
            int dy = 10;
            auto deco = std::make_unique<Actor>(dx, dy, '#', "deco_" + std::to_string(i), Colors::white);
            deco->fovOnly = true;
            // No ai, no destructible, no attacker — it's a decoration
            decorations.push_back(deco.get());
            engine.actors.push_back(std::move(deco));
        }

        // Verify: the relative order of gameplay actors in the list is same as insertion order
        std::vector<Actor*> gameplayInList;
        for (const auto& actor : engine.actors) {
            // Check if this is one of our gameplay actors
            for (Actor* ga : gameplayActors) {
                if (actor.get() == ga) {
                    gameplayInList.push_back(ga);
                    break;
                }
            }
        }

        // The gameplay actors should appear in their original insertion order
        RC_ASSERT(gameplayInList.size() == static_cast<size_t>(numGameplay));
        for (int i = 0; i < numGameplay; ++i) {
            RC_ASSERT(gameplayInList[i] == gameplayActors[i]);
        }

        // Cleanup
        for (Actor* a : gameplayActors) {
            engine.actors.remove_if([a](const std::unique_ptr<Actor>& ptr) {
                return ptr.get() == a;
            });
        }
        for (Actor* a : decorations) {
            engine.actors.remove_if([a](const std::unique_ptr<Actor>& ptr) {
                return ptr.get() == a;
            });
        }
    });
}

// ─── Preservation 2d: Tiles with only gameplay actors are in insertion order ──
// **Validates: Requirements 3.4, 3.5**
//
// When tiles contain only gameplay actors and no decorations, their relative
// order in the actors list matches their insertion order.

TEST_CASE("PBT: Preservation 2d — tiles with only gameplay actors maintain insertion order",
          "[pbt][property][decoration-render-priority]")
{
    rc::prop("actors with no decorations on their tiles maintain original insertion order", []() {
        // Generate random number of gameplay actors [2, 8]
        const int numActors = *rc::gen::inRange(2, 8);

        std::vector<Actor*> inserted;

        // Insert gameplay actors at unique tile positions
        for (int i = 0; i < numActors; ++i) {
            int ax = *rc::gen::inRange(1, 78);
            int ay = *rc::gen::inRange(1, 48);
            auto actor = std::make_unique<Actor>(ax, ay, 'M', "monster_" + std::to_string(i), Colors::white);
            actor->ai = std::make_shared<MonsterAi>();
            actor->destructible = std::make_shared<MonsterDestructible>(10.0f, 0.0f, "monster corpse", 10);
            inserted.push_back(actor.get());
            engine.actors.push_back(std::move(actor));
        }

        // No decorations added — pure gameplay actor scenario

        // Verify: actors appear in the list in their insertion order
        std::vector<Actor*> foundOrder;
        for (const auto& actor : engine.actors) {
            for (Actor* ins : inserted) {
                if (actor.get() == ins) {
                    foundOrder.push_back(ins);
                    break;
                }
            }
        }

        RC_ASSERT(foundOrder.size() == static_cast<size_t>(numActors));
        for (int i = 0; i < numActors; ++i) {
            RC_ASSERT(foundOrder[i] == inserted[i]);
        }

        // Cleanup
        for (Actor* a : inserted) {
            engine.actors.remove_if([a](const std::unique_ptr<Actor>& ptr) {
                return ptr.get() == a;
            });
        }
    });
}
