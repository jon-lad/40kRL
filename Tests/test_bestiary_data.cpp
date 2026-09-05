#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "BestiaryTestHarness.hpp"

#include <optional>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — property tests over the loaded bestiary data.
//
// These tests load Scripts/Enemies.lua and Scripts/Equipment.lua through the
// engine-isolated BestiaryTestHarness (a private sol::state, no global Engine) and
// assert the design's Correctness Properties over the real spawnEnemy selection
// function and the parsed Enemy_Entry tables.
//
// TDD-RED: written before the faction-data tasks (2.3, 3.x–9.x) populate Enemies.lua
// with the ten faction columns. Against the current shipped script (a single flat
// "Ork" column with legacy Gretchin/Ork/Shoota Boy/Nob entries) the property still
// holds for the "Ork" region and vacuously for the other nine (empty) columns, but
// full faction coverage is only exercised once the data lands.
//
// Bounds convention: rc::gen::inRange(a, b) is INCLUSIVE at both ends in this
// project's RapidCheck header. The roll domain per the design is the integer range
// [0, 100].
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Reference cumulative-selection model (the specification of correct behaviour):
// iterate the region column in declaration order and select the first entry whose
// region-scoped cumulative threshold is strictly greater than the roll. An entry
// without a key for the region is not part of the column and can never be selected.
// If no entry qualifies, there is no selection.
std::optional<std::string> referenceSelect(
    const std::vector<bestiary::EnemyEntry>& column,
    const std::string& region, int roll) {
    for (const auto& e : column) {
        std::optional<int> threshold = e.chanceFor(region);
        if (threshold.has_value() && roll < threshold.value()) {
            return e.name;
        }
    }
    return std::nullopt;
}

} // namespace

// Feature: bestiary-npcs, Property 2: Selection is total and faction-pure
//
// For any Faction_Region R that is a defined column and any roll integer in
// [0, 100], spawnEnemy(roll, x, y, R) selects at most one entry, and any selected
// entry belongs to R (its `chance` table contains key R); an entry is selected
// whenever roll is strictly less than the column's terminal value (100), and none
// is selected only when no cumulative value strictly exceeds roll.
//
// Validates: Requirements 6.4, 7.4, 7.5, 10.2, 10.3
TEST_CASE("Property 2: spawnEnemy selection is total and faction-pure",
          "[bestiary-npcs][property][Feature: bestiary-npcs, Property 2]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this property.");
    }

    rc::check("selection is total and faction-pure across rolls and regions", [&] {
        // A defined Faction_Region column.
        const std::vector<std::string>& regions = bestiary::factionRegions();
        const std::string region =
            *rc::gen::elementOf(regions);

        // A roll in the inclusive integer domain [0, 100].
        const int roll = *rc::gen::inRange(0, 100);

        // The declaration-ordered column for this region (entries whose `chance`
        // table carries the region key).
        const std::vector<bestiary::EnemyEntry> column = harness.columnFor(region);

        // Reference expectation from the cumulative-selection model.
        const std::optional<std::string> expected =
            referenceSelect(column, region, roll);

        // Drive the real spawnEnemy through the stub addActor.
        bestiary::BestiaryTestHarness::SpawnResult result =
            harness.spawn(roll, 3, 7, region);

        // spawnEnemy must not error for any defined region / roll.
        RC_ASSERT(result.ok);

        if (expected.has_value()) {
            // An entry is selected: at most one (exactly one here), and it belongs
            // to R — matching the reference model's first-in-order choice.
            RC_ASSERT(result.called);
            RC_ASSERT(result.name == expected.value());

            // Faction purity: the selected entry carries the region's chance key.
            const std::optional<bestiary::EnemyEntry> picked =
                harness.enemyByName(result.name);
            RC_ASSERT(picked.has_value());
            RC_ASSERT(picked->chanceFor(region).has_value());

            // Coordinates flow through unchanged (spawn-call fidelity).
            RC_ASSERT(result.x == 3);
            RC_ASSERT(result.y == 7);
        } else {
            // No cumulative value strictly exceeds the roll => nothing selected.
            RC_ASSERT(!result.called);
        }

        // Totality: whenever the column is non-empty and terminates at 100, a roll
        // strictly less than 100 must select an entry (some cumulative value > roll
        // exists). This ties selection totality to the column's terminal-100
        // invariant (Property 1) without re-deriving it.
        if (!column.empty()) {
            std::optional<int> terminal = column.back().chanceFor(region);
            if (terminal.has_value() && terminal.value() == 100 && roll < 100) {
                RC_ASSERT(result.called);
            }
        }
    });
}

// ─── Property 1: Cumulative columns are strictly ascending and terminate at 100 ──
// Feature: bestiary-npcs, Property 1
// **Validates: Requirements 7.2, 7.3, 8.4**
//
// For any defined Faction_Region column, the cumulative `chance` values are strictly
// ascending in declaration order and the final entry's value is exactly 100. This
// was authored by Task 2.1; it lives alongside Property 2 in this file (both were
// dispatched concurrently and target the same test file per the tasks.md plan).
TEST_CASE("Property 1: cumulative faction columns are strictly ascending, terminate at 100",
          "[bestiary-npcs][property][Feature: bestiary-npcs, Property 1]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this property.");
    }

    rc::check("each defined column is strictly ascending and terminates at 100", [&] {
        const std::vector<std::string>& regions = bestiary::factionRegions();
        const std::string region = *rc::gen::elementOf(regions);

        const std::vector<bestiary::EnemyEntry> column = harness.columnFor(region);

        // The property is quantified over non-empty (defined) columns.
        RC_PRE(!column.empty());

        int previous = -1;
        for (const auto& e : column) {
            std::optional<int> c = e.chanceFor(region);
            RC_ASSERT(c.has_value());
            RC_ASSERT(*c >= 0);
            RC_ASSERT(*c <= 100);
            // Strictly ascending in declaration order.
            RC_ASSERT(*c > previous);
            previous = *c;
        }

        // Terminal cumulative value is exactly 100.
        std::optional<int> last = column.back().chanceFor(region);
        RC_ASSERT(last.has_value());
        RC_ASSERT(last.value() == 100);
    });
}
