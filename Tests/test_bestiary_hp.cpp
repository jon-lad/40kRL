#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "BestiaryTestHarness.hpp"

#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — Property 7: Wounds map to positive hp (Task 10.3)
//
// Loads Scripts/Enemies.lua through the engine-isolated BestiaryTestHarness (a
// private sol::state, no global Engine) and asserts that every Enemy_Entry's `hp`
// is at least 1. Each entry's `hp` equals its Troop_Profile Wounds value; profiles
// that cite Wounds as 0 or omitted are floored to hp >= 1 (Requirement 3.5). This
// property guarantees no entry is spawned with non-positive hit points.
//
// TDD-RED: written before the faction-data tasks (2.3, 3.x–9.x) populate
// Enemies.lua. The roster is guarded with SKIP so this test is inert until the data
// lands, then verifies the invariant across the whole roster.
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: bestiary-npcs, Property 7: Wounds map to positive hp
//
// For every Enemy_Entry in the Bestiary, hp >= 1.
//
// **Validates: Requirements 3.1, 3.5**
TEST_CASE("Property 7: every bestiary entry has positive hp",
          "[bestiary-npcs][property][Feature: bestiary-npcs, Property 7]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this property.");
    }

    const std::vector<bestiary::EnemyEntry>& roster = harness.enemies();
    if (roster.empty()) {
        SKIP("Bestiary roster is empty; faction-data tasks have not populated "
             "Enemies.lua yet. Property 7 is exercised once the roster lands.");
    }

    rc::check("every entry's hp is >= 1", [&] {
        // Pick a random entry from the roster (inclusive index bounds).
        const int idx = *rc::gen::inRange(0, static_cast<int>(roster.size()) - 1);
        const bestiary::EnemyEntry& entry = roster[static_cast<std::size_t>(idx)];

        // `hp` is stored as a Lua number (float in the schema). Read defensively:
        // a missing/malformed hp reads as 0.0 and must fail the property.
        const double hp = entry.fieldOr<double>("hp", 0.0);

        RC_ASSERT(hp >= 1.0);
    });
}
