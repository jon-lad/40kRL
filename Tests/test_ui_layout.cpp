#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"

#include <vector>
#include <string>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Local test-only definitions ─────────────────────────────────────────────
// These mirror the design doc structs. Once Headers/LayoutRect.h and
// Headers/ScreenLayout.h are implemented (task 1.2), these can be replaced
// with #include "LayoutRect.h" / #include "ScreenLayout.h".

namespace test_layout {

struct LayoutRect {
    int x, y, width, height;

    bool intersects(const LayoutRect& other) const {
        return x < other.x + other.width && x + width > other.x
            && y < other.y + other.height && y + height > other.y;
    }
};

// Compute a ScreenLayout from arbitrary valid constants (parameterized for PBT)
struct LayoutConfig {
    int screenWidth;
    int screenHeight;
    int rightSidebarWidth;
    int leftSidebarWidth;
    bool leftSidebarEnabled;
    int hudHeight;
    int msgLogHeight;
    int skillBarHeight;
};

struct ScreenLayout {
    LayoutRect viewport;
    LayoutRect rightSidebar;
    LayoutRect leftSidebar;   // only valid when leftSidebarEnabled
    LayoutRect messageLog;
    LayoutRect skillBar;
    LayoutRect hudBars;
    bool leftSidebarEnabled;

    static ScreenLayout compute(const LayoutConfig& cfg) {
        ScreenLayout layout{};
        layout.leftSidebarEnabled = cfg.leftSidebarEnabled;

        int viewportX = cfg.leftSidebarEnabled ? cfg.leftSidebarWidth : 0;
        int viewportWidth = cfg.screenWidth - cfg.rightSidebarWidth
                            - (cfg.leftSidebarEnabled ? cfg.leftSidebarWidth : 0);
        int viewportHeight = cfg.screenHeight - cfg.hudHeight;

        layout.viewport = { viewportX, 0, viewportWidth, viewportHeight };
        layout.rightSidebar = { cfg.screenWidth - cfg.rightSidebarWidth, 0,
                                cfg.rightSidebarWidth, cfg.screenHeight };
        layout.leftSidebar = { 0, 0, cfg.leftSidebarWidth, cfg.screenHeight };
        layout.messageLog = { viewportX, viewportHeight, viewportWidth, cfg.msgLogHeight };
        layout.skillBar = { viewportX, viewportHeight + cfg.msgLogHeight + 1,
                            viewportWidth, cfg.skillBarHeight };
        layout.hudBars = { 0, viewportHeight, viewportX > 0 ? viewportX : 20, cfg.hudHeight };

        return layout;
    }
};

} // namespace test_layout

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 1: Layout geometry non-overlap invariant
// ═══════════════════════════════════════════════════════════════════════════════

// **Validates: Requirements 1.4, 4.1**
//
// For any valid layout configuration (with or without left sidebar enabled),
// the Map_Viewport, Right_Sidebar, Left_Sidebar (when enabled), Message_Log,
// and Skill_Bar rectangles SHALL NOT overlap — i.e., no two layout rectangles
// intersect.

TEST_CASE("PBT: Property 1 — Layout geometry non-overlap invariant", "[pbt][property][ui-rework]")
{
    using namespace test_layout;

    rc::prop("no layout rectangles overlap (left sidebar disabled)", []() {
        // Generate valid layout constants — screen must be large enough to fit all panels
        const int rightSidebarWidth = *rc::gen::inRange(20, 30);
        const int screenWidth = *rc::gen::inRange(rightSidebarWidth + 40, 160);
        const int hudHeight = *rc::gen::inRange(5, 12);
        const int screenHeight = *rc::gen::inRange(hudHeight + 20, 60);
        const int msgLogHeight = *rc::gen::inRange(3, hudHeight - 2);
        const int skillBarHeight = 1;
        const int leftSidebarWidth = *rc::gen::inRange(15, 25);

        LayoutConfig cfg{
            screenWidth, screenHeight,
            rightSidebarWidth, leftSidebarWidth,
            false, // leftSidebarEnabled = false
            hudHeight, msgLogHeight, skillBarHeight
        };

        ScreenLayout layout = ScreenLayout::compute(cfg);

        // Collect all active rectangles
        std::vector<std::pair<std::string, LayoutRect>> rects;
        rects.push_back({"viewport", layout.viewport});
        rects.push_back({"rightSidebar", layout.rightSidebar});
        rects.push_back({"messageLog", layout.messageLog});
        rects.push_back({"skillBar", layout.skillBar});

        // Check no two rectangles overlap
        for (size_t i = 0; i < rects.size(); ++i) {
            for (size_t j = i + 1; j < rects.size(); ++j) {
                RC_ASSERT(!rects[i].second.intersects(rects[j].second));
            }
        }
    });

    rc::prop("no layout rectangles overlap (left sidebar enabled)", []() {
        // Generate valid layout constants — screen must be large enough for left + right sidebars + viewport
        const int rightSidebarWidth = *rc::gen::inRange(20, 30);
        const int leftSidebarWidth = *rc::gen::inRange(15, 25);
        const int screenWidth = *rc::gen::inRange(leftSidebarWidth + rightSidebarWidth + 30, 160);
        const int hudHeight = *rc::gen::inRange(5, 12);
        const int screenHeight = *rc::gen::inRange(hudHeight + 20, 60);
        const int msgLogHeight = *rc::gen::inRange(3, hudHeight - 2);
        const int skillBarHeight = 1;

        LayoutConfig cfg{
            screenWidth, screenHeight,
            rightSidebarWidth, leftSidebarWidth,
            true, // leftSidebarEnabled = true
            hudHeight, msgLogHeight, skillBarHeight
        };

        ScreenLayout layout = ScreenLayout::compute(cfg);

        // Collect all active rectangles (left sidebar included)
        std::vector<std::pair<std::string, LayoutRect>> rects;
        rects.push_back({"viewport", layout.viewport});
        rects.push_back({"rightSidebar", layout.rightSidebar});
        rects.push_back({"leftSidebar", layout.leftSidebar});
        rects.push_back({"messageLog", layout.messageLog});
        rects.push_back({"skillBar", layout.skillBar});

        // Check no two rectangles overlap
        for (size_t i = 0; i < rects.size(); ++i) {
            for (size_t j = i + 1; j < rects.size(); ++j) {
                RC_ASSERT(!rects[i].second.intersects(rects[j].second));
            }
        }
    });

    rc::prop("all rectangles have positive dimensions", []() {
        const int rightSidebarWidth = *rc::gen::inRange(20, 30);
        const int leftSidebarWidth = *rc::gen::inRange(15, 25);
        const bool leftSidebarEnabled = *rc::gen::arbitrary_bool();
        const int minWidth = (leftSidebarEnabled ? leftSidebarWidth : 0) + rightSidebarWidth + 30;
        const int screenWidth = *rc::gen::inRange(minWidth, 160);
        const int hudHeight = *rc::gen::inRange(5, 12);
        const int screenHeight = *rc::gen::inRange(hudHeight + 20, 60);
        const int msgLogHeight = *rc::gen::inRange(3, hudHeight - 2);
        const int skillBarHeight = 1;

        LayoutConfig cfg{
            screenWidth, screenHeight,
            rightSidebarWidth, leftSidebarWidth,
            leftSidebarEnabled,
            hudHeight, msgLogHeight, skillBarHeight
        };

        ScreenLayout layout = ScreenLayout::compute(cfg);

        // All rectangles must have positive width and height
        RC_ASSERT(layout.viewport.width > 0);
        RC_ASSERT(layout.viewport.height > 0);
        RC_ASSERT(layout.rightSidebar.width > 0);
        RC_ASSERT(layout.rightSidebar.height > 0);
        RC_ASSERT(layout.messageLog.width > 0);
        RC_ASSERT(layout.messageLog.height > 0);
        RC_ASSERT(layout.skillBar.width > 0);
        RC_ASSERT(layout.skillBar.height > 0);

        if (leftSidebarEnabled) {
            RC_ASSERT(layout.leftSidebar.width > 0);
            RC_ASSERT(layout.leftSidebar.height > 0);
        }
    });
}
