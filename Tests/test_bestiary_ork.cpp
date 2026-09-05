#include "lib/catch_amalgamated.hpp"

#include "BestiaryTestHarness.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — Ork reconciliation example test (Task 6.1)
//
// EXAMPLE-based unit tests that pin the finite Ork-reconciliation mapping from the
// design's "Ork reconciliation (Requirement 8)" section. Loads the real
// Scripts/Enemies.lua through the BestiaryTestHarness (engine-isolated sol::state,
// no global Engine) and asserts:
//
//   1. No legacy `Ork` / `Shoota Boy` / `Nob` names survive         (Req 8.1, 8.3)
//   2. Gretchin aligns to its IV.7 profile                          (Req 8.2)
//   3. Ork Boy aligns to its IV.7 profile                           (Req 8.2)
//   4. The `Ork` column is strictly ascending, terminates at 100    (Req 8.4)
//
// Reference profiles (Reference/RT-Bestiary.md §IV.7):
//   Gretchin  — WS 15 BS 35 S 20 T 20 Ag 45 Int 35 Per 35 WP 20 Fel 25; Wounds 6
//   Ork Boy   — WS 35 BS 25 S 45 T 45 Ag 30 Int 20 Per 30 WP 25 Fel 20; Wounds 18
//   (parenthetical Unnatural multipliers are excluded from the stored base integer)
//
// TDD-RED: this test is written BEFORE the Ork column rebuild (Task 6.2). Until the
// legacy four-entry flat distribution (Gretchin, Ork, Shoota Boy, Nob) is replaced
// with the bestiary IV.7 Troop set + Ork Freebooter, the legacy-name and
// profile-alignment assertions are EXPECTED TO FAIL.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Convenience: read one of the nine characteristics off an entry (defaults absent
// characteristics to 20, matching addActor's default — but every Ork profile here
// cites all nine, so a mismatch surfaces a real misalignment rather than a default).
int charOf(const bestiary::EnemyEntry& e, const char* key) {
    return e.fieldOr<int>(key, -1);
}

} // namespace

TEST_CASE("Ork reconciliation: no legacy Ork/Shoota Boy/Nob names survive",
          "[bestiary-npcs][ork-reconciliation]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua not found from test CWD");
    }

    // The retired legacy Ork names (design's reconciliation table): the bare `Ork`
    // is renamed to `Ork Boy`, `Shoota Boy` folds into `Ork Boy`, and `Nob` is Elite
    // (out of scope). None of these names may remain as an Enemy_Entry (Req 8.1, 8.3).
    for (const char* legacy : { "Ork", "Shoota Boy", "Nob" }) {
        INFO("legacy Ork entry that must not survive: " << legacy);
        CHECK_FALSE(harness.enemyByName(legacy).has_value());
    }
}

TEST_CASE("Ork reconciliation: Gretchin aligns to its IV.7 profile",
          "[bestiary-npcs][ork-reconciliation]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua not found from test CWD");
    }

    std::optional<bestiary::EnemyEntry> gretchin = harness.enemyByName("Gretchin");
    REQUIRE(gretchin.has_value());

    // Characteristics (base values; Unnatural multiplier excluded) — Req 8.2.
    CHECK(charOf(*gretchin, "ws") == 15);
    CHECK(charOf(*gretchin, "bs") == 35);
    CHECK(charOf(*gretchin, "s") == 20);
    CHECK(charOf(*gretchin, "t") == 20);
    CHECK(charOf(*gretchin, "ag") == 45);
    CHECK(charOf(*gretchin, "int") == 35);
    CHECK(charOf(*gretchin, "per") == 35);
    CHECK(charOf(*gretchin, "wp") == 20);
    CHECK(charOf(*gretchin, "fel") == 25);

    // Wounds 6 -> hp (Req 8.2 / 3.1).
    CHECK(gretchin->fieldOr<double>("hp", -1.0) == Catch::Approx(6.0));

    // Belongs to the Ork faction column (Req 8.2).
    CHECK(gretchin->chanceFor("Ork").has_value());
}

TEST_CASE("Ork reconciliation: Ork Boy aligns to its IV.7 profile",
          "[bestiary-npcs][ork-reconciliation]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua not found from test CWD");
    }

    std::optional<bestiary::EnemyEntry> orkBoy = harness.enemyByName("Ork Boy");
    REQUIRE(orkBoy.has_value());

    // Characteristics (base values; T 45 (6) stores 45) — Req 8.2.
    CHECK(charOf(*orkBoy, "ws") == 35);
    CHECK(charOf(*orkBoy, "bs") == 25);
    CHECK(charOf(*orkBoy, "s") == 45);
    CHECK(charOf(*orkBoy, "t") == 45);
    CHECK(charOf(*orkBoy, "ag") == 30);
    CHECK(charOf(*orkBoy, "int") == 20);
    CHECK(charOf(*orkBoy, "per") == 30);
    CHECK(charOf(*orkBoy, "wp") == 25);
    CHECK(charOf(*orkBoy, "fel") == 20);

    // Wounds 18 -> hp (Req 8.2 / 3.1).
    CHECK(orkBoy->fieldOr<double>("hp", -1.0) == Catch::Approx(18.0));

    // Belongs to the Ork faction column (Req 8.2).
    CHECK(orkBoy->chanceFor("Ork").has_value());
}

TEST_CASE("Ork reconciliation: Ork column is strictly ascending and terminates at 100",
          "[bestiary-npcs][ork-reconciliation]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua not found from test CWD");
    }

    std::vector<bestiary::EnemyEntry> column = harness.columnFor("Ork");
    REQUIRE_FALSE(column.empty());

    // The rebuilt Ork column should be the 13 IV.7 Troops + Ork Freebooter (14),
    // replacing the legacy four-entry flat distribution (Req 8.1, 8.4).
    CHECK(column.size() >= 5); // rebuilt column is far larger than the legacy four

    // Strictly ascending cumulative chance values in declaration order (Req 8.4).
    int previous = -1;
    for (const auto& e : column) {
        std::optional<int> c = e.chanceFor("Ork");
        REQUIRE(c.has_value());
        INFO("Ork column entry '" << e.name << "' cumulative = " << *c
                                  << " (previous = " << previous << ")");
        CHECK(*c > previous);
        previous = *c;
    }

    // Final cumulative value terminates at exactly 100 (Req 8.4).
    std::optional<int> last = column.back().chanceFor("Ork");
    REQUIRE(last.has_value());
    CHECK(*last == 100);
}
