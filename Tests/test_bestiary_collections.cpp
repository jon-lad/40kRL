#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "BestiaryTestHarness.hpp"

#include <sol/sol.hpp>

#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — Property 8 over the loaded bestiary data.
//
// Loads Scripts/Enemies.lua and Scripts/Equipment.lua through the engine-isolated
// BestiaryTestHarness (a private sol::state, no global Engine) and asserts that the
// optional collection fields are always PRESENT as Lua tables — never omitted —
// even when a profile cites no entries for them (in which case they are empty
// tables `{}`), per Requirements 3.4 and 5.3.
//
// TDD-RED: written before the faction-data tasks (2.3, 3.x–9.x) populate Enemies.lua
// with the ten faction columns. The roster is surfaced by the harness by driving
// the real spawnEnemy across every faction column; against an unpopulated roster the
// harness yields no entries, so the property is guarded with SKIP.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// True when the Enemy_Entry carries `key` as a Lua table (empty or not). A missing
// field or a non-table value both fail this check — the property demands the field
// be present specifically as a table (a possibly-empty collection).
bool hasTableField(const bestiary::EnemyEntry& entry, const char* key) {
    sol::object field = entry.table[key];
    return field.valid() && field.get_type() == sol::type::table;
}

} // namespace

// Feature: bestiary-npcs, Property 8: Optional collections are present, never omitted
//
// For every Enemy_Entry in the Bestiary, the `skills`, `talents`, and `traits`
// fields are each present as a Lua table (possibly empty). A profile that cites no
// entries for one of these fields must still declare it as an empty table `{}`
// rather than omitting the field entirely.
//
// Validates: Requirements 3.4, 5.3
TEST_CASE("Property 8: optional collections skills/talents/traits are present as tables",
          "[bestiary-npcs][property][Feature: bestiary-npcs, Property 8]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test "
             "CWD; run from the repo root to exercise this property.");
    }

    const std::vector<bestiary::EnemyEntry>& roster = harness.enemies();
    if (roster.empty()) {
        SKIP("Bestiary roster is empty; the faction-data tasks have not populated "
             "Enemies.lua yet. This property is exercised once the roster lands.");
    }

    rc::check("every entry declares skills, talents, and traits as tables", [&] {
        // Pick an arbitrary entry from the surfaced roster.
        const bestiary::EnemyEntry& entry = *rc::gen::elementOf(roster);

        // Each optional collection field must be present specifically as a table.
        RC_ASSERT(hasTableField(entry, "skills"));
        RC_ASSERT(hasTableField(entry, "talents"));
        RC_ASSERT(hasTableField(entry, "traits"));
    });
}
