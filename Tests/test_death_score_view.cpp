#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "HighScore.hpp"

#include <optional>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: death-screen-highscore-jump — Unit & Property-Based Tests for the
// pure view-decision seam (computeDeathScoreView / placementNumber / DeathScoreView).
//
// The seam is engine-free (per test-isolation.md): it never touches
// engine.gui/map/player, so these tests need no Engine initialization.
//
// NOTE (TDD): These tests are written BEFORE the seam exists (task 2.1 adds the
// declarations to Headers/HighScore.hpp and definitions to Source/HighScore.cpp).
// Until then the test project is EXPECTED to fail to build/link on the missing
// computeDeathScoreView / placementNumber / DeathScoreView symbols — this is the
// intended "red" TDD state.
//
// RapidCheck rc::gen::inRange uses INCLUSIVE bounds [a, b] in this project's
// custom header (see test-isolation.md). All bounds below are inclusive; indices
// use inRange(0, N - 1).
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Mirror of Paginator::totalPages(): ceil(totalItems / pageSize), minimum 1.
// Kept local so the property test does not depend on constructing a Paginator.
int totalPagesLike(int totalItems, int pageSize)
{
    const int ps = (pageSize < 1) ? 1 : pageSize;
    const int pages = (totalItems + ps - 1) / ps;
    return (pages < 1) ? 1 : pages;
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Task 1.1 — Unit (example) tests for placementNumber and computeDeathScoreView
// **Validates: Requirements 2.1, 2.2, 2.3, 3.1, 3.2, 3.3, 3.4**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("placementNumber is 1-based for representative indices", "[death-screen-highscore-jump]")
{
    // The rendered rank is entry index + 1 (Req 2.3).
    REQUIRE(placementNumber(0) == 1);
    REQUIRE(placementNumber(9) == 10);
    REQUIRE(placementNumber(99) == 100);
}

TEST_CASE("computeDeathScoreView: ranked entry on the first page", "[death-screen-highscore-jump]")
{
    // v = 3 with pageSize 10 sits on page 0, and a ranked run highlights (Req 2.1, 2.2).
    const DeathScoreView v = computeDeathScoreView(3, 50, 10);
    REQUIRE(v.initialPage == 0);
    REQUIRE(v.highlight == true);
}

TEST_CASE("computeDeathScoreView: ranked entry on a later page", "[death-screen-highscore-jump]")
{
    // v = 42 with pageSize 10 sits on page 4 (42 / 10), still highlighted (Req 2.1, 2.2).
    const DeathScoreView v = computeDeathScoreView(42, 100, 10);
    REQUIRE(v.initialPage == 4);
    REQUIRE(v.highlight == true);
}

TEST_CASE("computeDeathScoreView: unranked opens on page 0 without highlight", "[death-screen-highscore-jump]")
{
    // An unranked run (nullopt) opens at the first page and never highlights (Req 3.1).
    const DeathScoreView v = computeDeathScoreView(std::nullopt, 50, 10);
    REQUIRE(v.initialPage == 0);
    REQUIRE(v.highlight == false);
}

TEST_CASE("computeDeathScoreView: empty board unranked opens on page 0", "[death-screen-highscore-jump]")
{
    // Empty board (totalItems 0) with no earned entry: page 0, no highlight (Req 3.4).
    const DeathScoreView v = computeDeathScoreView(std::nullopt, 0, 10);
    REQUIRE(v.initialPage == 0);
    REQUIRE(v.highlight == false);
}

TEST_CASE("computeDeathScoreView: small board (1-9 entries) unranked stays on page 0", "[death-screen-highscore-jump]")
{
    // For a small board the first page shows all entries (Req 3.3).
    for (int total = 1; total <= 9; ++total) {
        const DeathScoreView v = computeDeathScoreView(std::nullopt, total, 10);
        REQUIRE(v.initialPage == 0);
        REQUIRE(v.highlight == false);
    }
}

TEST_CASE("computeDeathScoreView: degenerate pageSize 0 clamps to 1 without crashing", "[death-screen-highscore-jump]")
{
    // pageSize 0 must be clamped to 1 internally (division-by-zero guard).
    // With clamp, initialPage == v / 1 == v.
    const DeathScoreView v = computeDeathScoreView(3, 50, 0);
    REQUIRE(v.initialPage == 3);
    REQUIRE(v.highlight == true);
}

TEST_CASE("computeDeathScoreView: out-of-range lastEntryIndex treated as unranked", "[death-screen-highscore-jump]")
{
    // Index 50 with only 10 items is out of [0, totalItems): treat as unranked.
    const DeathScoreView v = computeDeathScoreView(50, 10, 10);
    REQUIRE(v.initialPage == 0);
    REQUIRE(v.highlight == false);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 1.2 — Property 1: Ranked opens on the containing page
// **Validates: Requirements 2.1**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 1 — ranked opens on the containing page", "[pbt][property][death-screen-highscore-jump]")
{
    rc::check("ranked initialPage == v / pageSize and v lies within the page window", []() {
        const int pageSize = *rc::gen::inRange(1, 50);
        const int totalItems = *rc::gen::inRange(1, 100);
        const int v = *rc::gen::inRange(0, totalItems - 1); // in-range index

        const DeathScoreView view = computeDeathScoreView(v, totalItems, pageSize);

        // initialPage is the page containing v.
        RC_ASSERT(view.initialPage == v / pageSize);

        // The earned index lies within that page's index window
        // [initialPage*pageSize, initialPage*pageSize + pageSize).
        RC_ASSERT(view.initialPage * pageSize <= v);
        RC_ASSERT(v < view.initialPage * pageSize + pageSize);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 1.3 — Property 2: Ranked always highlights; unranked never does
// **Validates: Requirements 2.2, 3.1**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 2 — highlight iff ranked in-range", "[pbt][property][death-screen-highscore-jump]")
{
    rc::check("in-range v highlights; nullopt never highlights", []() {
        const int pageSize = *rc::gen::inRange(1, 50);
        const int totalItems = *rc::gen::inRange(1, 100);
        const int v = *rc::gen::inRange(0, totalItems - 1); // in-range index

        // A valid in-range earned entry is always highlighted (Req 2.2).
        RC_ASSERT(computeDeathScoreView(v, totalItems, pageSize).highlight == true);

        // An unranked run is never highlighted (Req 3.1).
        RC_ASSERT(computeDeathScoreView(std::nullopt, totalItems, pageSize).highlight == false);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 1.4 — Property 3: Unranked opens at the first page
// **Validates: Requirements 3.1, 3.2, 3.3**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 3 — unranked opens at the first page", "[pbt][property][death-screen-highscore-jump]")
{
    rc::check("computeDeathScoreView(nullopt, ...).initialPage == 0 for all inputs", []() {
        const int totalItems = *rc::gen::inRange(0, 100);
        const int pageSize = *rc::gen::inRange(1, 50);

        RC_ASSERT(computeDeathScoreView(std::nullopt, totalItems, pageSize).initialPage == 0);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 1.5 — Property 4: Initial page is always a valid page index
// **Validates: Requirements 2.1, 4.1, 4.2**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 4 — initial page is always a valid page index", "[pbt][property][death-screen-highscore-jump]")
{
    rc::check("0 <= initialPage < totalPages for ranked and unranked inputs", []() {
        const int totalItems = *rc::gen::inRange(1, 100);
        const int pageSize = *rc::gen::inRange(1, 50);

        // Optionally supply an in-range earned index (bool + value), mirroring
        // how test_high_score.cpp builds optional inputs from a bool + value.
        const bool ranked = *rc::gen::arbitrary_bool();
        std::optional<int> lastEntryIndex;
        if (ranked) {
            lastEntryIndex = *rc::gen::inRange(0, totalItems - 1);
        }

        const DeathScoreView view = computeDeathScoreView(lastEntryIndex, totalItems, pageSize);
        const int totalPages = totalPagesLike(totalItems, pageSize);

        RC_ASSERT(view.initialPage >= 0);
        RC_ASSERT(view.initialPage < totalPages);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 1.6 — Property 5: Placement is 1-based and matches the rendered rank
// **Validates: Requirements 2.3**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 5 — placement number is 1-based", "[pbt][property][death-screen-highscore-jump]")
{
    rc::check("placementNumber(entryIndex) == entryIndex + 1", []() {
        const int entryIndex = *rc::gen::inRange(0, 99);
        RC_ASSERT(placementNumber(entryIndex) == entryIndex + 1);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 1.7 — Property 6: Determinism and no side effects
// **Validates: Requirements 2.1, 3.1**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 6 — determinism / no side effects", "[pbt][property][death-screen-highscore-jump]")
{
    rc::check("two calls with identical args return equal results", []() {
        const int totalItems = *rc::gen::inRange(1, 100);
        const int pageSize = *rc::gen::inRange(1, 50);

        const bool ranked = *rc::gen::arbitrary_bool();
        std::optional<int> lastEntryIndex;
        if (ranked) {
            lastEntryIndex = *rc::gen::inRange(0, totalItems - 1);
        }

        const DeathScoreView a = computeDeathScoreView(lastEntryIndex, totalItems, pageSize);
        const DeathScoreView b = computeDeathScoreView(lastEntryIndex, totalItems, pageSize);

        // Repeated calls with the same arguments yield equal results.
        RC_ASSERT(a.initialPage == b.initialPage);
        RC_ASSERT(a.highlight == b.highlight);
    });
}
