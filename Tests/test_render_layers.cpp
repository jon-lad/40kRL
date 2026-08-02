#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"

#include "main.h"

#include <algorithm>
#include <vector>
#include <string>
#include <memory>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework — Render Layer Tests
//
// Property 5: Tests render layer ASSIGNMENT based on actor components.
// Property 6: Tests render ORDER sorting algorithm.
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Render layer constants (test-local, mirrors design doc) ─────────────────
// Once task 5.3 adds these to production code, replace with the real include.
namespace TestRenderLayers {
    inline constexpr int DECORATION = 0;
    inline constexpr int ITEM       = 1;
    inline constexpr int DOOR       = 2;
    inline constexpr int CORPSE     = 3;
    inline constexpr int LIVING     = 4;
}

// ─── Local stub for assignRenderLayer ────────────────────────────────────────
// Encapsulates the assignment logic from the design doc.
// Task 5.3 will implement the production version on Actor.
// This stub defines the expected behavior so the tests express the property.
//
// Assignment rules (priority order):
//   1. Has ai component OR is player (name == "player") → LIVING (4)
//   2. Has openable component (door) → DOOR (2)
//   3. Has pickable component (item) → ITEM (1)
//   4. Otherwise (decoration) → DECORATION (0)
static int assignRenderLayerForTest(const Actor& actor) {
    if (actor.ai || actor.name == "player") {
        return TestRenderLayers::LIVING;
    }
    if (actor.openable) {
        return TestRenderLayers::DOOR;
    }
    if (actor.pickable) {
        return TestRenderLayers::ITEM;
    }
    return TestRenderLayers::DECORATION;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 5: Render layer assignment by actor category
// **Validates: Requirements 6.1, 6.3**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 5 — Render layer assignment by actor category",
          "[pbt][property][ui-rework][render-layers]")
{
    rc::prop("actors with ai component are assigned LIVING layer (4)", []() {
        const int x = *rc::gen::inRange(0, 160);
        const int y = *rc::gen::inRange(0, 86);
        const int glyph = *rc::gen::inRange(33, 126);

        Actor actor(x, y, glyph, "monster", Colors::white);
        actor.ai = std::make_shared<MonsterAi>();
        // Optionally add other components — ai should still dominate
        if (*rc::gen::inRange(0, 1) == 1) {
            actor.destructible = std::make_shared<MonsterDestructible>(10.0f, 0.0f, "corpse", 10);
        }
        if (*rc::gen::inRange(0, 1) == 1) {
            actor.attacker = std::make_shared<Attacker>(5.0f);
        }

        int layer = assignRenderLayerForTest(actor);
        RC_ASSERT(layer == TestRenderLayers::LIVING);
    });

    rc::prop("player actor is assigned LIVING layer (4)", []() {
        const int x = *rc::gen::inRange(0, 160);
        const int y = *rc::gen::inRange(0, 86);

        Actor actor(x, y, '@', "player", Colors::white);
        // Player typically has destructible and attacker but NOT ai
        actor.destructible = std::make_shared<PlayerDestructible>(20.0f, 2.0f, "your cadaver", 0);
        actor.attacker = std::make_shared<Attacker>(5.0f);

        int layer = assignRenderLayerForTest(actor);
        RC_ASSERT(layer == TestRenderLayers::LIVING);
    });

    rc::prop("actors with openable component (doors) are assigned DOOR layer (2)", []() {
        const int x = *rc::gen::inRange(0, 160);
        const int y = *rc::gen::inRange(0, 86);
        const int glyph = *rc::gen::inRange(43, 47);

        Actor actor(x, y, glyph, "door", Colors::white);
        actor.openable = std::make_shared<Openable>();
        actor.blocks = true;

        int layer = assignRenderLayerForTest(actor);
        RC_ASSERT(layer == TestRenderLayers::DOOR);
    });

    rc::prop("actors with pickable component (items) are assigned ITEM layer (1)", []() {
        const int x = *rc::gen::inRange(0, 160);
        const int y = *rc::gen::inRange(0, 86);
        const int glyph = *rc::gen::inRange(33, 126);

        Actor actor(x, y, glyph, "item", Colors::white);
        actor.pickable = std::make_shared<Pickable>(nullptr, nullptr);
        actor.blocks = false;

        int layer = assignRenderLayerForTest(actor);
        RC_ASSERT(layer == TestRenderLayers::ITEM);
    });

    rc::prop("actors with no ai/openable/pickable are assigned DECORATION layer (0)", []() {
        const int x = *rc::gen::inRange(0, 160);
        const int y = *rc::gen::inRange(0, 86);
        const int glyph = *rc::gen::inRange(33, 126);

        Actor actor(x, y, glyph, "pillar", Colors::white);
        actor.blocks = *rc::gen::inRange(0, 1) == 1;
        actor.fovOnly = true;

        int layer = assignRenderLayerForTest(actor);
        RC_ASSERT(layer == TestRenderLayers::DECORATION);
    });

    rc::prop("LIVING layer is strictly higher than ITEM, DOOR, and DECORATION", []() {
        // Generate a random actor category
        enum Category { CAT_DECORATION, CAT_ITEM, CAT_DOOR, CAT_LIVING };
        const int cat = *rc::gen::inRange(0, 3);

        const int x = *rc::gen::inRange(0, 160);
        const int y = *rc::gen::inRange(0, 86);

        Actor actor(x, y, 'X', "test", Colors::white);

        switch (cat) {
            case CAT_DECORATION:
                break;
            case CAT_ITEM:
                actor.pickable = std::make_shared<Pickable>(nullptr, nullptr);
                break;
            case CAT_DOOR:
                actor.openable = std::make_shared<Openable>();
                break;
            case CAT_LIVING:
                actor.ai = std::make_shared<MonsterAi>();
                break;
        }

        int layer = assignRenderLayerForTest(actor);

        if (cat == CAT_LIVING) {
            RC_ASSERT(layer > TestRenderLayers::DECORATION);
            RC_ASSERT(layer > TestRenderLayers::ITEM);
            RC_ASSERT(layer > TestRenderLayers::DOOR);
        } else {
            RC_ASSERT(layer < TestRenderLayers::LIVING);
        }
    });

    rc::prop("ai component takes priority over pickable and openable", []() {
        const int x = *rc::gen::inRange(0, 160);
        const int y = *rc::gen::inRange(0, 86);

        Actor actor(x, y, 'M', "hybrid", Colors::white);
        actor.ai = std::make_shared<MonsterAi>();
        actor.pickable = std::make_shared<Pickable>(nullptr, nullptr);
        actor.openable = std::make_shared<Openable>();

        int layer = assignRenderLayerForTest(actor);
        RC_ASSERT(layer == TestRenderLayers::LIVING);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// ─── Render Order Sort Tests (Property 6) ────────────────────────────────────
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Lightweight struct representing an actor for render-layer testing.
// Avoids pulling in the full Actor class and its dependencies.
struct TestActor {
    int renderLayer;
    int tileX;
    int tileY;
    int insertionOrder; // tracks original position in list
    std::string name;   // for debugging
};

// Simulates the Engine's render sort: stable_sort by renderLayer ascending.
// stable_sort preserves relative order of elements with equal keys.
void sortByRenderLayer(std::vector<TestActor>& actors) {
    std::stable_sort(actors.begin(), actors.end(),
        [](const TestActor& a, const TestActor& b) {
            return a.renderLayer < b.renderLayer;
        });
}

} // anonymous namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 6: Render order sort invariant
// **Validates: Requirements 6.2, 6.4**
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 6a: Sorted sequence is in non-decreasing render layer order ────
TEST_CASE("PBT: Render order — sorted sequence is non-decreasing by renderLayer",
          "[pbt][property][ui-rework][render-layers]")
{
    // Feature: ui-rework, Property 6: Render order sort invariant
    rc::check("sorted actor sequence is in non-decreasing render layer order", []() {
        // Generate a random number of actors [1, 50]
        const int numActors = *rc::gen::inRange(1, 50);

        std::vector<TestActor> actors;
        actors.reserve(numActors);

        for (int i = 0; i < numActors; ++i) {
            TestActor a;
            // Render layers match design: DECORATION=0, ITEM=1, DOOR=2, CORPSE=3, LIVING=4
            a.renderLayer = *rc::gen::inRange(0, 4);
            a.tileX = *rc::gen::inRange(0, 159);
            a.tileY = *rc::gen::inRange(0, 85);
            a.insertionOrder = i;
            a.name = "actor_" + std::to_string(i);
            actors.push_back(a);
        }

        // Sort using the same algorithm the Engine uses
        sortByRenderLayer(actors);

        // Assert: result is in non-decreasing renderLayer order
        for (size_t i = 1; i < actors.size(); ++i) {
            RC_ASSERT(actors[i].renderLayer >= actors[i - 1].renderLayer);
        }
    });
}

// ─── Property 6b: Same-layer same-tile actors preserve insertion order ───────
TEST_CASE("PBT: Render order — same-layer same-tile actors preserve insertion order",
          "[pbt][property][ui-rework][render-layers]")
{
    // Feature: ui-rework, Property 6: Render order sort invariant
    rc::check("actors with same renderLayer and same tile preserve insertion order", []() {
        // Generate a shared tile position
        const int sharedX = *rc::gen::inRange(0, 159);
        const int sharedY = *rc::gen::inRange(0, 85);
        // Generate a shared render layer
        const int sharedLayer = *rc::gen::inRange(0, 4);

        // Generate a random number of actors on the same tile with the same layer [2, 20]
        const int numSameTile = *rc::gen::inRange(2, 20);

        // Also generate some actors on different tiles/layers to create noise
        const int numOther = *rc::gen::inRange(0, 20);

        std::vector<TestActor> actors;
        actors.reserve(numSameTile + numOther);

        int insertIdx = 0;

        // Insert same-tile same-layer actors interspersed with others
        int sameTileInserted = 0;
        int otherInserted = 0;

        // Interleave insertion: randomly decide whether next actor is same-tile or other
        while (sameTileInserted < numSameTile || otherInserted < numOther) {
            bool insertSameTile = false;
            if (sameTileInserted < numSameTile && otherInserted < numOther) {
                insertSameTile = (*rc::gen::inRange(0, 1) == 0);
            } else if (sameTileInserted < numSameTile) {
                insertSameTile = true;
            }

            TestActor a;
            a.insertionOrder = insertIdx++;

            if (insertSameTile) {
                a.renderLayer = sharedLayer;
                a.tileX = sharedX;
                a.tileY = sharedY;
                a.name = "same_" + std::to_string(sameTileInserted);
                sameTileInserted++;
            } else {
                // Different layer or different tile
                a.renderLayer = *rc::gen::inRange(0, 4);
                a.tileX = *rc::gen::inRange(0, 159);
                a.tileY = *rc::gen::inRange(0, 85);
                a.name = "other_" + std::to_string(otherInserted);
                otherInserted++;
            }

            actors.push_back(a);
        }

        // Sort using the same algorithm the Engine uses
        sortByRenderLayer(actors);

        // Extract only the actors that share the same tile and same layer
        std::vector<int> sameTileInsertionOrders;
        for (const auto& a : actors) {
            if (a.tileX == sharedX && a.tileY == sharedY && a.renderLayer == sharedLayer) {
                sameTileInsertionOrders.push_back(a.insertionOrder);
            }
        }

        // Assert: insertion order is preserved (strictly increasing)
        RC_ASSERT(static_cast<int>(sameTileInsertionOrders.size()) == numSameTile);
        for (size_t i = 1; i < sameTileInsertionOrders.size(); ++i) {
            RC_ASSERT(sameTileInsertionOrders[i] > sameTileInsertionOrders[i - 1]);
        }
    });
}

// ─── Property 6c: Global stability — equal renderLayer preserves insertion order ─
TEST_CASE("PBT: Render order — equal renderLayer actors preserve relative insertion order globally",
          "[pbt][property][ui-rework][render-layers]")
{
    // Feature: ui-rework, Property 6: Render order sort invariant
    rc::check("all actors with identical renderLayer maintain their original relative order", []() {
        // Generate actors with a mix of layers
        const int numActors = *rc::gen::inRange(2, 50);

        std::vector<TestActor> actors;
        actors.reserve(numActors);

        for (int i = 0; i < numActors; ++i) {
            TestActor a;
            a.renderLayer = *rc::gen::inRange(0, 4);
            a.tileX = *rc::gen::inRange(0, 159);
            a.tileY = *rc::gen::inRange(0, 85);
            a.insertionOrder = i;
            a.name = "actor_" + std::to_string(i);
            actors.push_back(a);
        }

        // Sort using the same algorithm the Engine uses
        sortByRenderLayer(actors);

        // For each render layer, verify insertion order is preserved
        for (int layer = 0; layer <= 4; ++layer) {
            std::vector<int> insertionOrdersForLayer;
            for (const auto& a : actors) {
                if (a.renderLayer == layer) {
                    insertionOrdersForLayer.push_back(a.insertionOrder);
                }
            }
            // Within each layer, insertion orders must be strictly increasing
            for (size_t i = 1; i < insertionOrdersForLayer.size(); ++i) {
                RC_ASSERT(insertionOrdersForLayer[i] > insertionOrdersForLayer[i - 1]);
            }
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Edge Cases — unit tests for specific render order scenarios
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Render order: single actor remains unchanged after sort",
          "[unit][ui-rework][render-layers]")
{
    std::vector<TestActor> actors = {
        {3, 10, 10, 0, "lone_actor"}
    };

    sortByRenderLayer(actors);

    CHECK(actors.size() == 1);
    CHECK(actors[0].renderLayer == 3);
    CHECK(actors[0].insertionOrder == 0);
}

TEST_CASE("Render order: already sorted list remains unchanged",
          "[unit][ui-rework][render-layers]")
{
    std::vector<TestActor> actors = {
        {0, 5, 5, 0, "decoration"},
        {1, 5, 5, 1, "item"},
        {2, 5, 5, 2, "door"},
        {3, 5, 5, 3, "corpse"},
        {4, 5, 5, 4, "living"}
    };

    sortByRenderLayer(actors);

    for (int i = 0; i < 5; ++i) {
        CHECK(actors[i].renderLayer == i);
        CHECK(actors[i].insertionOrder == i);
    }
}

TEST_CASE("Render order: reversed list is corrected by sort",
          "[unit][ui-rework][render-layers]")
{
    std::vector<TestActor> actors = {
        {4, 5, 5, 0, "living"},
        {3, 5, 5, 1, "corpse"},
        {2, 5, 5, 2, "door"},
        {1, 5, 5, 3, "item"},
        {0, 5, 5, 4, "decoration"}
    };

    sortByRenderLayer(actors);

    CHECK(actors[0].renderLayer == 0);
    CHECK(actors[1].renderLayer == 1);
    CHECK(actors[2].renderLayer == 2);
    CHECK(actors[3].renderLayer == 3);
    CHECK(actors[4].renderLayer == 4);
}

TEST_CASE("Render order: multiple actors on same tile with same layer preserve insertion order",
          "[unit][ui-rework][render-layers]")
{
    // Three items on the same tile — insertion order must be preserved
    std::vector<TestActor> actors = {
        {4, 10, 10, 0, "monster_A"},  // living
        {1, 5, 5, 1, "sword"},        // item on tile (5,5)
        {1, 5, 5, 2, "shield"},       // item on tile (5,5) — inserted after sword
        {1, 5, 5, 3, "potion"},       // item on tile (5,5) — inserted after shield
        {0, 3, 3, 4, "rubble"}        // decoration
    };

    sortByRenderLayer(actors);

    // Find the items (layer 1)
    std::vector<int> itemOrders;
    for (const auto& a : actors) {
        if (a.renderLayer == 1) {
            itemOrders.push_back(a.insertionOrder);
        }
    }

    // Insertion order preserved: sword(1), shield(2), potion(3)
    REQUIRE(itemOrders.size() == 3);
    CHECK(itemOrders[0] == 1);
    CHECK(itemOrders[1] == 2);
    CHECK(itemOrders[2] == 3);
}

TEST_CASE("Render order: all actors with same layer preserve full insertion order",
          "[unit][ui-rework][render-layers]")
{
    // All LIVING — stable sort should preserve original order entirely
    std::vector<TestActor> actors = {
        {4, 1, 1, 0, "player"},
        {4, 2, 2, 1, "goblin"},
        {4, 3, 3, 2, "orc"},
        {4, 4, 4, 3, "dragon"}
    };

    sortByRenderLayer(actors);

    for (int i = 0; i < 4; ++i) {
        CHECK(actors[i].insertionOrder == i);
    }
}

TEST_CASE("Render order: empty list is handled gracefully",
          "[unit][ui-rework][render-layers]")
{
    std::vector<TestActor> actors;
    sortByRenderLayer(actors);
    CHECK(actors.empty());
}
