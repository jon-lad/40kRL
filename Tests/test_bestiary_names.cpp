#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "BestiaryTestHarness.hpp"

#include <set>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — entry-name uniqueness property test (Task 10.2)
//
// Loads Scripts/Enemies.lua / Scripts/Equipment.lua through the sol2-based
// BestiaryTestHarness (no global Engine — test-isolation steering) and asserts that
// every Enemy_Entry carries a `name` that is unique across the entire Bestiary.
//
// TDD-RED: written BEFORE the faction columns are populated (Tasks 2.3, 3–9). Until
// the roster exists the property has nothing to check, so an empty roster is
// guarded with SKIP to keep the suite green. Once entries land, any duplicate
// `name` value across the roster fails the property. The file is expected to
// COMPILE.
//
// RapidCheck rc::gen::inRange uses INCLUSIVE bounds [a, b] in this project's custom
// header (see test-isolation.md); all bounds below are inclusive.
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 5: Entry names are unique ──────────────────────────────────────────
// **Validates: Requirements 1.11**
//
// The Enemy_Script gives each Enemy_Entry a unique `name` value across the entire
// Bestiary (Requirement 1.11). Equivalently: the `name` values are pairwise
// distinct. This is verified by a full-roster scan (the definitive check) plus a
// property-based pairwise probe that samples entry pairs and asserts distinct names.
TEST_CASE("PBT: Property 5 — entry names are unique across the Bestiary",
          "[pbt][property][Feature: bestiary-npcs][Property 5]")
{
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua could not be loaded");
    }

    const std::vector<bestiary::EnemyEntry>& roster = harness.enemies();

    // Nothing to check on an empty roster; the property is quantified over entries.
    // (Roster completeness is covered separately by Task 10.5.)
    if (roster.empty()) {
        SKIP("Bestiary roster is empty — no entries to validate yet (TDD-RED)");
    }

    // Definitive check: the set of names has the same cardinality as the roster,
    // i.e. no two entries share a `name` (pairwise distinct).
    std::set<std::string> uniqueNames;
    for (const auto& entry : roster) {
        uniqueNames.insert(entry.name);
    }
    RC_ASSERT(uniqueNames.size() == roster.size());

    // Property-based pairwise probe: for any two distinct entry indices, the names
    // must differ. rc::check runs >= 100 iterations by default; inclusive bounds.
    rc::check("any two distinct entries have distinct names", [&]() {
        const int n = static_cast<int>(roster.size());
        const int i = *rc::gen::inRange(0, n - 1);
        const int j = *rc::gen::inRange(0, n - 1);
        RC_PRE(i != j);

        const std::string& nameI = roster[static_cast<std::size_t>(i)].name;
        const std::string& nameJ = roster[static_cast<std::size_t>(j)].name;
        RC_ASSERT(nameI != nameJ);
    });
}
