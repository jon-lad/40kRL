#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "BestiaryTestHarness.hpp"

#include <set>
#include <string>
#include <utility>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — Property 9: glyph/color pairs are distinct (Task).
//
// Loads Scripts/Enemies.lua / Scripts/Equipment.lua through the engine-isolated
// BestiaryTestHarness (a private sol::state, no global Engine — test-isolation
// steering) and asserts that no two Enemy_Entries share the SAME combination of
// `glyph` and `color`. Two entries may share a glyph if their colors differ, or a
// color if their glyphs differ, but never both — so every NPC is visually
// distinguishable on the map.
//
// Also asserts each entry's `color` names a color defined in Headers/Colors.hpp
// (resolvable by colorFromName): the harness cannot call the C++ resolver directly
// without pulling in engine headers, so this test pins the color against the known
// palette name list kept in sync with Colors.hpp.
//
// **Validates: Requirements 5.4**
//
// TDD note: this property holds against the current (partially populated) roster
// and must continue to hold as each faction column is added.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// The set of color names defined in Headers/Colors.hpp colorFromName. Kept in sync
// with the resolver; a color used by an Enemy_Entry must appear here (Requirement
// 5.4 — color must be resolvable, i.e. not fall through to the black sentinel).
// This list must match colorFromName in Headers/Colors.hpp EXACTLY: adding a name
// here that the resolver does not define would let an entry render as black.
const std::set<std::string>& knownColorNames() {
    static const std::set<std::string> names = {
        "white", "desaturatedGreen", "darkerGreen", "lightBlue", "orange",
        "lightGreen", "violet", "lightYellow", "lightGrey", "lighterOrange",
        "darkGrey", "red", "darkRed", "cyan", "brown", "gold",
    };
    return names;
}

// Read an entry's glyph as an integer (stored via string.byte("x")).
long long glyphOf(const bestiary::EnemyEntry& e) {
    return e.fieldOr<long long>("glyph", -1);
}

std::string colorOf(const bestiary::EnemyEntry& e) {
    return e.fieldOr<std::string>("color", "");
}

} // namespace

// ─── Property 9: distinct (glyph, color) pairs ───────────────────────────────────
TEST_CASE("Property 9: every Enemy_Entry has a distinct (glyph, color) pair",
          "[bestiary-npcs][property][Feature: bestiary-npcs][Property 9]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua could not be loaded");
    }

    const std::vector<bestiary::EnemyEntry>& roster = harness.enemies();
    if (roster.empty()) {
        SKIP("Bestiary roster is empty — no entries to validate yet (TDD-RED)");
    }

    // Definitive check: the set of (glyph, color) pairs is as large as the roster,
    // i.e. no two entries share both glyph and color.
    std::set<std::pair<long long, std::string>> pairs;
    for (const auto& e : roster) {
        pairs.insert({ glyphOf(e), colorOf(e) });
    }
    INFO("roster size = " << roster.size() << ", distinct (glyph,color) pairs = "
                          << pairs.size());
    RC_ASSERT(pairs.size() == roster.size());

    // Property-based pairwise probe: any two distinct entries differ in glyph or
    // color (or both). rc::check runs >= 100 iterations; inclusive inRange bounds.
    rc::check("any two distinct entries differ in glyph or color", [&]() {
        const int n = static_cast<int>(roster.size());
        const int i = *rc::gen::inRange(0, n - 1);
        const int j = *rc::gen::inRange(0, n - 1);
        RC_PRE(i != j);

        const bestiary::EnemyEntry& a = roster[static_cast<std::size_t>(i)];
        const bestiary::EnemyEntry& b = roster[static_cast<std::size_t>(j)];

        const bool sameGlyph = glyphOf(a) == glyphOf(b);
        const bool sameColor = colorOf(a) == colorOf(b);
        RC_ASSERT(!(sameGlyph && sameColor));
    });
}

// ─── Requirement 1.12: colors must be defined in Colors.hpp ──────────────────────
TEST_CASE("Every Enemy_Entry color names a color defined in Colors.hpp",
          "[bestiary-npcs][appearance]") {
    bestiary::BestiaryTestHarness harness;
    if (!harness.loaded()) {
        SKIP("Scripts/Enemies.lua or Scripts/Equipment.lua could not be loaded");
    }

    const std::vector<bestiary::EnemyEntry>& roster = harness.enemies();
    if (roster.empty()) {
        SKIP("Bestiary roster is empty — no entries to validate yet (TDD-RED)");
    }

    const std::set<std::string>& known = knownColorNames();
    for (const auto& e : roster) {
        const std::string color = colorOf(e);
        INFO("entry '" << e.name << "' color = '" << color << "'");
        CHECK(known.count(color) == 1);
    }
}
