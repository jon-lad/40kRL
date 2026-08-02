#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"
#include "../Headers/Paginator.h"

#include <algorithm>
#include <string>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 9: Pagination state machine correctness
// **Validates: Requirements 8.1, 8.2, 8.3, 8.4**
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 9a: displayCount equals min(pageSize, totalItems - startIndex) ─
TEST_CASE("PBT: Pagination — displayCount correctness",
          "[pbt][property][ui-rework][paginator]")
{
    // Feature: ui-rework, Property 9: Pagination state machine correctness
    rc::check("displayCount == min(pageSize, totalItems - startIndex())", []() {
        const int totalItems = *rc::gen::inRange(0, 500);
        const int pageSize = *rc::gen::inRange(1, 50);
        const int maxPage = std::max(1, (totalItems + pageSize - 1) / pageSize);
        const int currentPage = *rc::gen::inRange(0, maxPage - 1);

        Paginator p{totalItems, pageSize, currentPage};

        const int expectedStart = currentPage * pageSize;
        const int expectedDisplay = std::min(pageSize, totalItems - expectedStart);

        RC_ASSERT(p.displayCount() == expectedDisplay);
    });
}

// ─── Property 9b: totalPages equals ceil(totalItems / pageSize), minimum 1 ──
TEST_CASE("PBT: Pagination — totalPages correctness",
          "[pbt][property][ui-rework][paginator]")
{
    // Feature: ui-rework, Property 9: Pagination state machine correctness
    rc::check("totalPages == max(1, ceil(totalItems / pageSize))", []() {
        const int totalItems = *rc::gen::inRange(0, 500);
        const int pageSize = *rc::gen::inRange(1, 50);

        Paginator p{totalItems, pageSize, 0};

        const int expected = std::max(1, (totalItems + pageSize - 1) / pageSize);

        RC_ASSERT(p.totalPages() == expected);
    });
}

// ─── Property 9c: nextPage increments currentPage iff canAdvance ─────────────
TEST_CASE("PBT: Pagination — nextPage increments iff canAdvance",
          "[pbt][property][ui-rework][paginator]")
{
    // Feature: ui-rework, Property 9: Pagination state machine correctness
    rc::check("nextPage increments currentPage by 1 iff canAdvance()", []() {
        const int totalItems = *rc::gen::inRange(0, 500);
        const int pageSize = *rc::gen::inRange(1, 50);
        const int maxPage = std::max(1, (totalItems + pageSize - 1) / pageSize);
        const int currentPage = *rc::gen::inRange(0, maxPage - 1);

        Paginator p{totalItems, pageSize, currentPage};

        const bool couldAdvance = p.canAdvance();
        const int pageBefore = p.currentPage;

        p.nextPage();

        if (couldAdvance) {
            RC_ASSERT(p.currentPage == pageBefore + 1);
        } else {
            RC_ASSERT(p.currentPage == pageBefore);
        }
    });
}

// ─── Property 9d: prevPage decrements currentPage iff canRetreat ─────────────
TEST_CASE("PBT: Pagination — prevPage decrements iff canRetreat",
          "[pbt][property][ui-rework][paginator]")
{
    // Feature: ui-rework, Property 9: Pagination state machine correctness
    rc::check("prevPage decrements currentPage by 1 iff canRetreat()", []() {
        const int totalItems = *rc::gen::inRange(0, 500);
        const int pageSize = *rc::gen::inRange(1, 50);
        const int maxPage = std::max(1, (totalItems + pageSize - 1) / pageSize);
        const int currentPage = *rc::gen::inRange(0, maxPage - 1);

        Paginator p{totalItems, pageSize, currentPage};

        const bool couldRetreat = p.canRetreat();
        const int pageBefore = p.currentPage;

        p.prevPage();

        if (couldRetreat) {
            RC_ASSERT(p.currentPage == pageBefore - 1);
        } else {
            RC_ASSERT(p.currentPage == pageBefore);
        }
    });
}

// ─── Property 9e: indicator contains correct 1-indexed page and total ────────
TEST_CASE("PBT: Pagination — indicator string correctness",
          "[pbt][property][ui-rework][paginator]")
{
    // Feature: ui-rework, Property 9: Pagination state machine correctness
    rc::check("indicator() contains correct 1-indexed current page and total pages", []() {
        const int totalItems = *rc::gen::inRange(0, 500);
        const int pageSize = *rc::gen::inRange(1, 50);
        const int maxPage = std::max(1, (totalItems + pageSize - 1) / pageSize);
        const int currentPage = *rc::gen::inRange(0, maxPage - 1);

        Paginator p{totalItems, pageSize, currentPage};

        const std::string ind = p.indicator();
        const int expectedPage = currentPage + 1; // 1-indexed
        const int expectedTotal = maxPage;

        // indicator should contain "Page X/Y" format
        const std::string expectedStr = "Page " + std::to_string(expectedPage) + "/" + std::to_string(expectedTotal);
        RC_ASSERT(ind.find(expectedStr) != std::string::npos);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Edge Cases — unit tests for boundary conditions
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Paginator edge case: totalItems=0", "[unit][ui-rework][paginator]")
{
    Paginator p{0, 20, 0};

    CHECK(p.totalPages() == 1);
    CHECK(p.displayCount() == 0);
    CHECK(p.startIndex() == 0);
    CHECK(p.endIndex() == 0);
    CHECK(p.canAdvance() == false);
    CHECK(p.canRetreat() == false);
    CHECK(p.indicator() == "Page 1/1");
}

TEST_CASE("Paginator edge case: pageSize=1", "[unit][ui-rework][paginator]")
{
    Paginator p{5, 1, 0};

    CHECK(p.totalPages() == 5);
    CHECK(p.displayCount() == 1);
    CHECK(p.canAdvance() == true);
    CHECK(p.canRetreat() == false);

    p.nextPage();
    CHECK(p.currentPage == 1);
    CHECK(p.displayCount() == 1);
    CHECK(p.canRetreat() == true);

    // Advance to last page
    p.currentPage = 4;
    CHECK(p.canAdvance() == false);
    CHECK(p.displayCount() == 1);
    CHECK(p.indicator() == "Page 5/5");
}

TEST_CASE("Paginator edge case: single page (totalItems <= pageSize)", "[unit][ui-rework][paginator]")
{
    Paginator p{10, 20, 0};

    CHECK(p.totalPages() == 1);
    CHECK(p.displayCount() == 10);
    CHECK(p.canAdvance() == false);
    CHECK(p.canRetreat() == false);
    CHECK(p.startIndex() == 0);
    CHECK(p.endIndex() == 10);
    CHECK(p.indicator() == "Page 1/1");
}

TEST_CASE("Paginator edge case: negative totalItems clamps to 0", "[unit][ui-rework][paginator]")
{
    // Per design: "If totalItems is negative, clamp to 0; totalPages returns 1."
    Paginator p{-5, 10, 0};

    CHECK(p.totalPages() == 1);
    CHECK(p.displayCount() == 0);
    CHECK(p.canAdvance() == false);
    CHECK(p.canRetreat() == false);
    CHECK(p.indicator() == "Page 1/1");
}

TEST_CASE("Paginator edge case: nextPage clamped at last page", "[unit][ui-rework][paginator]")
{
    Paginator p{25, 10, 2}; // last page (3 total pages: 0,1,2)

    CHECK(p.canAdvance() == false);
    p.nextPage();
    CHECK(p.currentPage == 2); // unchanged
}

TEST_CASE("Paginator edge case: prevPage clamped at page 0", "[unit][ui-rework][paginator]")
{
    Paginator p{25, 10, 0};

    CHECK(p.canRetreat() == false);
    p.prevPage();
    CHECK(p.currentPage == 0); // unchanged
}
