#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"

#include <string>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework — Skill Bar Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Local test-only definitions ─────────────────────────────────────────────
// These mirror the design doc structs. Once the production SkillBarEntry and
// color selection function are implemented (task 7.2), these can be replaced
// with includes to the production headers.

namespace test_skill_bar {

// TCODColor stand-in for testing (avoids libtcod dependency in pure logic test)
struct Color {
    int r, g, b;
    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
    bool operator!=(const Color& other) const {
        return !(*this == other);
    }
};

// Skill bar colors matching the design doc
namespace SkillBarColors {
    // Normal skill color (white — matches Colors::white in production)
    inline constexpr Color normal{255, 255, 255};
    // Dimmed color for unavailable skills (grey — matches a darker variant)
    inline constexpr Color dimmed{128, 128, 128};
}

// Mirrors design doc SkillBarEntry struct
struct SkillBarEntry {
    std::string name;
    char keybinding;       // '1' through '9'
    bool available;        // false if on cooldown or prerequisites unmet
};

// Pure color selection function implementing the design doc logic:
// - If available == true  → return normal skill color
// - If available == false → return dimmed color
Color getSkillBarColor(const SkillBarEntry& entry) {
    return entry.available ? SkillBarColors::normal : SkillBarColors::dimmed;
}

} // namespace test_skill_bar

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 4: Skill bar availability color selection
// ═══════════════════════════════════════════════════════════════════════════════

// **Validates: Requirements 5.1, 5.3**
//
// For any skill bar entry, the color selection function SHALL return a dimmed
// color variant when `available == false` and the normal skill color when
// `available == true`.

TEST_CASE("PBT: Property 4 — Skill bar availability color selection", "[pbt][property][ui-rework][skill-bar]")
{
    using namespace test_skill_bar;

    rc::prop("available=true returns normal color for any skill entry", []() {
        // Generate arbitrary skill name (1-20 chars)
        const auto name = *rc::gen::string(1, 20);
        // Generate keybinding '1' through '9'
        const char keybinding = static_cast<char>(*rc::gen::inRange('1', '9'));

        SkillBarEntry entry{name, keybinding, true};
        Color result = getSkillBarColor(entry);

        RC_ASSERT(result == SkillBarColors::normal);
    });

    rc::prop("available=false returns dimmed color for any skill entry", []() {
        // Generate arbitrary skill name (1-20 chars)
        const auto name = *rc::gen::string(1, 20);
        // Generate keybinding '1' through '9'
        const char keybinding = static_cast<char>(*rc::gen::inRange('1', '9'));

        SkillBarEntry entry{name, keybinding, false};
        Color result = getSkillBarColor(entry);

        RC_ASSERT(result == SkillBarColors::dimmed);
    });

    rc::prop("color selection depends only on availability flag, not on name or keybinding", []() {
        // Generate two different names and keybindings but same availability
        const auto name1 = *rc::gen::string(1, 20);
        const auto name2 = *rc::gen::string(1, 20);
        const char key1 = static_cast<char>(*rc::gen::inRange('1', '9'));
        const char key2 = static_cast<char>(*rc::gen::inRange('1', '9'));
        const bool available = *rc::gen::arbitrary_bool();

        SkillBarEntry entry1{name1, key1, available};
        SkillBarEntry entry2{name2, key2, available};

        // Same availability → same color regardless of other fields
        RC_ASSERT(getSkillBarColor(entry1) == getSkillBarColor(entry2));
    });

    rc::prop("normal and dimmed colors are always distinct", []() {
        const auto name = *rc::gen::string(1, 20);
        const char keybinding = static_cast<char>(*rc::gen::inRange('1', '9'));

        SkillBarEntry availableEntry{name, keybinding, true};
        SkillBarEntry unavailableEntry{name, keybinding, false};

        Color availColor = getSkillBarColor(availableEntry);
        Color unavailColor = getSkillBarColor(unavailableEntry);

        // The two color states must always be different
        RC_ASSERT(availColor != unavailColor);
    });
}
