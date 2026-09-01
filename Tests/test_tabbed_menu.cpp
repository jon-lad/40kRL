#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"
#include "../Headers/Paginator.hpp"

#include <array>
#include <algorithm>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Test-local stub for TabbedMenuState (production code not yet implemented).
// This mirrors the interface defined in the design document and will be replaced
// by #include "../Headers/TabbedMenuState.hpp" once task 11.3 is complete.
// ═══════════════════════════════════════════════════════════════════════════════

namespace TabbedMenuStub {

enum class Tab { INVENTORY = 0, EQUIPMENT, SKILLS, COUNT };

struct TabbedMenuState {
    Tab activeTab = Tab::INVENTORY;
    std::array<Paginator, static_cast<int>(Tab::COUNT)> paginators;

    void cycleTab() {
        int next = (static_cast<int>(activeTab) + 1) % static_cast<int>(Tab::COUNT);
        activeTab = static_cast<Tab>(next);
    }

    Paginator& activePaginator() {
        return paginators[static_cast<int>(activeTab)];
    }
};

} // namespace TabbedMenuStub

using TabbedMenuStub::TabbedMenuState;
using TabbedMenuStub::Tab;

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 7: Tab cycling is modular arithmetic
// **Validates: Requirements 7.2**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Tab cycling — modular arithmetic",
          "[pbt][property][ui-rework][tabbed-menu]")
{
    // Feature: ui-rework, Property 7: Tab cycling is modular arithmetic
    rc::check("cycleTab() produces (current + 1) % Tab::COUNT", []() {
        constexpr int tabCount = static_cast<int>(Tab::COUNT);
        const int startTab = *rc::gen::inRange(0, tabCount);

        TabbedMenuState state;
        state.activeTab = static_cast<Tab>(startTab);

        state.cycleTab();

        const int expected = (startTab + 1) % tabCount;
        RC_ASSERT(static_cast<int>(state.activeTab) == expected);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 8: Tab scroll position round-trip preservation
// **Validates: Requirements 7.4**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Tab scroll position — round-trip preservation",
          "[pbt][property][ui-rework][tabbed-menu]")
{
    // Feature: ui-rework, Property 8: Tab scroll position round-trip preservation
    rc::check("Switching away and back to a tab preserves its paginator currentPage", []() {
        constexpr int tabCount = static_cast<int>(Tab::COUNT);

        // Generate random total items and page size for each tab's paginator
        TabbedMenuState state;
        std::array<int, 3> targetPages{};

        for (int i = 0; i < tabCount; ++i) {
            const int totalItems = *rc::gen::inRange(1, 200);
            const int pageSize = *rc::gen::inRange(1, 50);
            state.paginators[i].totalItems = totalItems;
            state.paginators[i].pageSize = pageSize;

            // Advance to a random valid page
            const int maxPage = std::max(1, (totalItems + pageSize - 1) / pageSize);
            const int page = *rc::gen::inRange(0, maxPage);
            state.paginators[i].currentPage = page;
            targetPages[i] = page;
        }

        // Generate a sequence of tab switches (at least 2 full cycles to ensure round-trips)
        const int numSwitches = *rc::gen::inRange(tabCount, tabCount * 5);

        for (int s = 0; s < numSwitches; ++s) {
            state.cycleTab();
        }

        // After arbitrary switches, verify ALL tab paginators still hold their original page
        for (int i = 0; i < tabCount; ++i) {
            RC_ASSERT(state.paginators[i].currentPage == targetPages[i]);
        }
    });
}

TEST_CASE("PBT: Tab scroll position — modify then return preserves page",
          "[pbt][property][ui-rework][tabbed-menu]")
{
    // Feature: ui-rework, Property 8: Tab scroll position round-trip preservation
    rc::check("Modifying one tab's paginator, switching away, then returning preserves page", []() {
        constexpr int tabCount = static_cast<int>(Tab::COUNT);

        TabbedMenuState state;

        // Set up all paginators with reasonable data
        for (int i = 0; i < tabCount; ++i) {
            state.paginators[i].totalItems = 100;
            state.paginators[i].pageSize = 10;
            state.paginators[i].currentPage = 0;
        }

        // Start on tab 0, advance its paginator
        state.activeTab = Tab::INVENTORY;
        const int advancesA = *rc::gen::inRange(0, 10); // 10 total pages, advance up to all
        for (int i = 0; i < advancesA; ++i) {
            state.activePaginator().nextPage();
        }
        const int savedPageA = state.activePaginator().currentPage;

        // Switch to tab 1, advance its paginator differently
        state.cycleTab(); // now on EQUIPMENT
        const int advancesB = *rc::gen::inRange(0, 10);
        for (int i = 0; i < advancesB; ++i) {
            state.activePaginator().nextPage();
        }
        const int savedPageB = state.activePaginator().currentPage;

        // Switch to tab 2, advance its paginator differently
        state.cycleTab(); // now on SKILLS
        const int advancesC = *rc::gen::inRange(0, 10);
        for (int i = 0; i < advancesC; ++i) {
            state.activePaginator().nextPage();
        }
        const int savedPageC = state.activePaginator().currentPage;

        // Cycle back to tab 0 (one more cycleTab from SKILLS)
        state.cycleTab(); // back to INVENTORY

        // Verify all pages are preserved
        RC_ASSERT(state.paginators[0].currentPage == savedPageA);
        RC_ASSERT(state.paginators[1].currentPage == savedPageB);
        RC_ASSERT(state.paginators[2].currentPage == savedPageC);
        RC_ASSERT(static_cast<int>(state.activeTab) == 0);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Unit tests — specific examples for tab scroll preservation
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Tab scroll preservation: basic round-trip", "[unit][ui-rework][tabbed-menu]")
{
    TabbedMenuState state;
    for (auto& p : state.paginators) {
        p.totalItems = 50;
        p.pageSize = 10;
        p.currentPage = 0;
    }

    // Advance inventory paginator to page 3
    state.activePaginator().nextPage();
    state.activePaginator().nextPage();
    state.activePaginator().nextPage();
    CHECK(state.activePaginator().currentPage == 3);

    // Switch to equipment tab
    state.cycleTab();
    CHECK(static_cast<int>(state.activeTab) == 1);

    // Advance equipment paginator to page 1
    state.activePaginator().nextPage();
    CHECK(state.activePaginator().currentPage == 1);

    // Switch to skills tab
    state.cycleTab();
    CHECK(static_cast<int>(state.activeTab) == 2);

    // Switch back to inventory tab
    state.cycleTab();
    CHECK(static_cast<int>(state.activeTab) == 0);

    // Inventory paginator should still be at page 3
    CHECK(state.activePaginator().currentPage == 3);

    // Switch to equipment — should still be at page 1
    state.cycleTab();
    CHECK(state.activePaginator().currentPage == 1);
}

TEST_CASE("Tab scroll preservation: no cross-contamination", "[unit][ui-rework][tabbed-menu]")
{
    TabbedMenuState state;
    for (auto& p : state.paginators) {
        p.totalItems = 100;
        p.pageSize = 20;
        p.currentPage = 0;
    }

    // Set each tab to a distinct page
    state.activeTab = Tab::INVENTORY;
    state.activePaginator().currentPage = 2;

    state.activeTab = Tab::EQUIPMENT;
    state.activePaginator().currentPage = 4;

    state.activeTab = Tab::SKILLS;
    state.activePaginator().currentPage = 0;

    // Verify each independently
    CHECK(state.paginators[0].currentPage == 2);
    CHECK(state.paginators[1].currentPage == 4);
    CHECK(state.paginators[2].currentPage == 0);

    // Cycle through all tabs — nothing should change
    state.activeTab = Tab::INVENTORY;
    state.cycleTab(); // -> EQUIPMENT
    state.cycleTab(); // -> SKILLS
    state.cycleTab(); // -> INVENTORY

    CHECK(state.paginators[0].currentPage == 2);
    CHECK(state.paginators[1].currentPage == 4);
    CHECK(state.paginators[2].currentPage == 0);
}
