#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "BestiaryTestHarness.hpp"

#include <optional>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — Property 3: Every equipment reference resolves.
//
// For every Enemy_Entry and every name in that entry's `equipment` list, exactly
// one Equipment.lua entry has a byte-equal `name`. This ties the enemy roster's
// gear references to the equipment catalogue so the C++ loader never encounters a
// dangling reference (Requirements 9.1, 9.6).
//
// The test loads Scripts/Enemies.lua and Scripts/Equipment.lua through the engine-
// isolated BestiaryTestHarness (a private sol::state, no global Engine) and asserts
// the property over the parsed Enemy_Entry / Equipment_Entry tables.
//
// TDD-RED: written before the equipment additions (Tasks 12.3 / 12.4) reconcile
// every enemy `equipment` reference against Equipment.lua. Because the enemy roster
// may not yet be populated with faction data (Tasks 3.x–9.x), an empty roster is
// guarded with SKIP so the property is only exercised once entries exist.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Collect the string entries of an Enemy_Entry's `equipment` list. Returns an empty
// vector when the field is absent or not a table (Requirement 3.4 keeps it present
// as `{}` for equipped-less NPCs, but tolerate omission here for robustness).
std::vector<std::string> equipmentNames(const bestiary::EnemyEntry& entry) {
    std::vector<std::string> names;
    sol::object equipObj = entry.table["equipment"];
    if (equipObj.get_type() != sol::type::table) {
        return names;
    }
    sol::table equipTbl = equipObj.as<sol::table>();
    for (std::size_t i = 1; i <= equipTbl.size(); ++i) {
        sol::optional<std::string> ref = equipTbl[i];
        if (ref) {
            names.push_back(*ref);
        }
    }
    return names;
}

// Count Equipment.lua entries whose `name` is byte-for-byte equal to `ref`.
int countEquipmentByName(const bestiary::BestiaryTestHarness& harness,
                         const std::string& ref) {
    int count = 0;
    for (const auto& e : harness.equipment()) {
        if (e.name == ref) {
            ++count;
        }
    }
    return count;
}

} // namespace

// Feature: bestiary-npcs, Property 3: Every equipment reference resolves
//
// For every entry and every name in its `equipment` list, exactly one Equipment.lua
// entry has a byte-equal `name`.
//
// Validates: Requirements 9.1, 9.6
TEST_CASE("Property 3: every enemy equipment reference resolves to exactly one equipment entry",
          "[bestiary-npcs][property][Feature: bestiary-npcs, Property 3]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this property.");
    }

    const std::vector<bestiary::EnemyEntry>& roster = harness.enemies();
    if (roster.empty()) {
        SKIP("Enemy roster is empty; faction-data tasks (3.x-9.x) have not yet "
             "populated Enemies.lua. Property 3 is exercised once entries exist.");
    }

    rc::check("each equipment reference resolves to exactly one equipment entry", [&] {
        // Pick a random Enemy_Entry from the roster.
        const bestiary::EnemyEntry& entry = *rc::gen::elementOf(roster);

        const std::vector<std::string> refs = equipmentNames(entry);

        // Entries with no equipment vacuously satisfy the property.
        RC_PRE(!refs.empty());

        // Pick a random reference from this entry's equipment list.
        const std::string ref = *rc::gen::elementOf(refs);

        // Exactly one Equipment.lua entry has a byte-equal name.
        const int matches = countEquipmentByName(harness, ref);
        RC_ASSERT(matches == 1);
    });
}
