#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <array>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 2: Right sidebar content completeness
// **Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5**
//
// For any player state with arbitrary equipment loadout (including empty slots),
// arbitrary characteristic values, and arbitrary skill set, the right sidebar
// content function SHALL produce output containing:
// (a) the name of every equipped item grouped by its slot
// (b) a placeholder label for every empty slot
// (c) current ammo and max capacity for any equipped ranged weapon
// (d) all nine characteristic abbreviations with their numeric values
// (e) the name of every passive skill
// ═══════════════════════════════════════════════════════════════════════════════

namespace test_sidebar {

// ─── Equipment slot names (matches EquipmentSlot enum: WEAPON, OFFHAND, HEAD, BODY) ───
static const std::array<std::string, 4> SLOT_NAMES = {
    "Weapon", "Offhand", "Head", "Body"
};

// ─── The nine characteristics (matches CharId enum order) ───
static const std::array<std::string, 9> CHAR_ABBREVIATIONS = {
    "WS", "BS", "S", "T", "Ag", "Int", "Per", "WP", "Fel"
};

static constexpr const char* EMPTY_PLACEHOLDER = "--empty--";

// ─── Ammo info for a ranged weapon ───
struct AmmoInfo {
    int currentAmmo;
    int maxAmmo;
};

// ─── Sidebar input model ───
struct SidebarInput {
    // Each slot: empty string means unoccupied
    std::array<std::string, 4> equippedItems;

    // Ammo info for slots that have ranged weapons
    std::array<std::optional<AmmoInfo>, 4> ammoInfo;

    // The nine characteristic values (index matches CharId order)
    std::array<int, 9> characteristics;

    // Passive skill names
    std::vector<std::string> passiveSkills;
};

// ─── Sidebar content generation function (test-local model) ───
// This models the expected behavior of the right sidebar rendering logic.
// The production renderRightSidebar() (task 8.2) must produce equivalent output.
std::vector<std::string> generateSidebarContent(const SidebarInput& input) {
    std::vector<std::string> lines;

    // Equipment section
    lines.push_back("-- Equipment --");
    for (int i = 0; i < 4; ++i) {
        if (input.equippedItems[i].empty()) {
            lines.push_back(SLOT_NAMES[i] + ": " + EMPTY_PLACEHOLDER);
        } else {
            lines.push_back(SLOT_NAMES[i] + ": " + input.equippedItems[i]);
        }
    }

    // Ammo section (only for slots with ranged weapons)
    lines.push_back("-- Ammo --");
    for (int i = 0; i < 4; ++i) {
        if (input.ammoInfo[i].has_value()) {
            const auto& ammo = input.ammoInfo[i].value();
            lines.push_back(input.equippedItems[i] + ": "
                + std::to_string(ammo.currentAmmo) + "/"
                + std::to_string(ammo.maxAmmo));
        }
    }

    // Characteristics section
    lines.push_back("-- Characteristics --");
    for (int i = 0; i < 9; ++i) {
        lines.push_back(CHAR_ABBREVIATIONS[i] + " " + std::to_string(input.characteristics[i]));
    }

    // Skills section
    lines.push_back("-- Skills --");
    for (const auto& skill : input.passiveSkills) {
        lines.push_back(skill);
    }

    return lines;
}

// ─── Helper: join lines into a single string for searching ───
std::string joinLines(const std::vector<std::string>& lines) {
    std::string result;
    for (const auto& line : lines) {
        result += line + "\n";
    }
    return result;
}

// ─── Helper: check if a string appears in the joined content ───
bool contentContains(const std::string& content, const std::string& needle) {
    return content.find(needle) != std::string::npos;
}

} // namespace test_sidebar

// ═══════════════════════════════════════════════════════════════════════════════
// Property 2a: All equipped item names appear in the sidebar output
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 2a — All equipped item names appear in sidebar output",
          "[pbt][property][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    // Feature: ui-rework, Property 2: Right sidebar content completeness
    rc::prop("every equipped item name appears in sidebar content", []() {
        SidebarInput input{};
        for (int i = 0; i < 4; ++i) {
            bool occupied = *rc::gen::arbitrary_bool();
            if (occupied) {
                input.equippedItems[i] = *rc::gen::string(1, 20);
                // Possibly add ranged stats
                bool isRanged = *rc::gen::arbitrary_bool();
                if (isRanged) {
                    int maxAmmo = *rc::gen::inRange(1, 30);
                    int curAmmo = *rc::gen::inRange(0, 30);
                    if (curAmmo > maxAmmo) curAmmo = maxAmmo;
                    input.ammoInfo[i] = AmmoInfo{curAmmo, maxAmmo};
                }
            }
        }
        // Fill characteristics with valid values
        for (int i = 0; i < 9; ++i) {
            input.characteristics[i] = *rc::gen::inRange(1, 100);
        }

        auto content = joinLines(generateSidebarContent(input));

        // Verify: every equipped item name appears in the output
        for (int i = 0; i < 4; ++i) {
            if (!input.equippedItems[i].empty()) {
                RC_ASSERT(contentContains(content, input.equippedItems[i]));
            }
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Property 2b: Empty slots show placeholder label
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 2b — Empty slots show placeholder label",
          "[pbt][property][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    // Feature: ui-rework, Property 2: Right sidebar content completeness
    rc::prop("every empty slot shows '--empty--' placeholder", []() {
        SidebarInput input{};
        int emptyCount = 0;
        for (int i = 0; i < 4; ++i) {
            bool occupied = *rc::gen::arbitrary_bool();
            if (occupied) {
                input.equippedItems[i] = *rc::gen::string(1, 20);
            } else {
                input.equippedItems[i] = "";
                ++emptyCount;
            }
        }
        // Fill characteristics
        for (int i = 0; i < 9; ++i) {
            input.characteristics[i] = *rc::gen::inRange(1, 100);
        }

        auto lines = generateSidebarContent(input);
        auto content = joinLines(lines);

        // Count occurrences of placeholder in the output
        int placeholderCount = 0;
        for (const auto& line : lines) {
            if (contentContains(line, EMPTY_PLACEHOLDER)) {
                ++placeholderCount;
            }
        }

        // There should be exactly one placeholder per empty slot
        RC_ASSERT(placeholderCount == emptyCount);

        // Each empty slot's name should appear with the placeholder
        for (int i = 0; i < 4; ++i) {
            if (input.equippedItems[i].empty()) {
                std::string expected = SLOT_NAMES[i] + ": " + EMPTY_PLACEHOLDER;
                RC_ASSERT(contentContains(content, expected));
            }
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Property 2c: Ranged weapon ammo shows "current/max" format
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 2c — Ranged weapon ammo shows current/max format",
          "[pbt][property][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    // Feature: ui-rework, Property 2: Right sidebar content completeness
    rc::prop("equipped ranged weapons display ammo as 'current/max'", []() {
        SidebarInput input{};

        // Ensure at least one ranged weapon exists
        int rangedSlot = *rc::gen::inRange(0, 3);
        input.equippedItems[rangedSlot] = *rc::gen::string(1, 20);
        int maxAmmo = *rc::gen::inRange(1, 30);
        int curAmmo = *rc::gen::inRange(0, 30);
        // Ensure curAmmo <= maxAmmo (clamp rather than depend on maxAmmo for generation)
        if (curAmmo > maxAmmo) curAmmo = maxAmmo;
        input.ammoInfo[rangedSlot] = AmmoInfo{curAmmo, maxAmmo};

        // Fill other slots randomly
        for (int i = 0; i < 4; ++i) {
            if (i == rangedSlot) continue;
            bool occupied = *rc::gen::arbitrary_bool();
            if (occupied) {
                input.equippedItems[i] = *rc::gen::string(1, 20);
            }
        }

        // Fill characteristics
        for (int i = 0; i < 9; ++i) {
            input.characteristics[i] = *rc::gen::inRange(1, 100);
        }

        auto content = joinLines(generateSidebarContent(input));

        // Verify ammo display format: "current/max"
        std::string expectedAmmo = std::to_string(curAmmo) + "/" + std::to_string(maxAmmo);
        RC_ASSERT(contentContains(content, expectedAmmo));

        // Verify the weapon name is associated with the ammo
        std::string expectedLine = input.equippedItems[rangedSlot] + ": " + expectedAmmo;
        RC_ASSERT(contentContains(content, expectedLine));
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Property 2d: All 9 characteristic abbreviations with values appear
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 2d — All 9 characteristic abbreviations with values appear",
          "[pbt][property][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    // Feature: ui-rework, Property 2: Right sidebar content completeness
    rc::prop("all 9 characteristics appear with correct abbreviation and value", []() {
        SidebarInput input{};
        // Generate arbitrary characteristics in valid range [1, 99]
        for (int i = 0; i < 9; ++i) {
            input.characteristics[i] = *rc::gen::inRange(1, 100);
        }
        // Empty equipment (don't care for this property)
        for (int i = 0; i < 4; ++i) {
            input.equippedItems[i] = "";
        }

        auto content = joinLines(generateSidebarContent(input));

        // Verify each characteristic abbreviation + value appears
        for (int i = 0; i < 9; ++i) {
            std::string expected = CHAR_ABBREVIATIONS[i] + " "
                + std::to_string(input.characteristics[i]);
            RC_ASSERT(contentContains(content, expected));
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Property 2e: All passive skill names appear in the output
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 2e — All passive skill names appear in sidebar output",
          "[pbt][property][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    // Feature: ui-rework, Property 2: Right sidebar content completeness
    rc::prop("every passive skill name appears in sidebar content", []() {
        SidebarInput input{};
        // Generate arbitrary characteristics
        for (int i = 0; i < 9; ++i) {
            input.characteristics[i] = *rc::gen::inRange(1, 100);
        }
        // Empty equipment
        for (int i = 0; i < 4; ++i) {
            input.equippedItems[i] = "";
        }
        // Generate a random number of passive skills (0 to 10)
        int numSkills = *rc::gen::inRange(0, 11);
        for (int i = 0; i < numSkills; ++i) {
            std::string skill = *rc::gen::string(1, 15);
            input.passiveSkills.push_back(skill);
        }

        auto content = joinLines(generateSidebarContent(input));

        // Verify every skill name appears
        for (const auto& skill : input.passiveSkills) {
            RC_ASSERT(contentContains(content, skill));
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Combined property: Full sidebar completeness across all aspects
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 2 — Full right sidebar content completeness",
          "[pbt][property][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    // Feature: ui-rework, Property 2: Right sidebar content completeness
    rc::prop("sidebar contains all equipment, placeholders, ammo, stats, and skills", []() {
        SidebarInput input{};

        // Generate random equipment loadout
        for (int i = 0; i < 4; ++i) {
            bool occupied = *rc::gen::arbitrary_bool();
            if (occupied) {
                input.equippedItems[i] = *rc::gen::string(1, 15);
                // Possibly ranged
                bool isRanged = *rc::gen::arbitrary_bool();
                if (isRanged) {
                    int maxAmmo = *rc::gen::inRange(1, 30);
                    int curAmmo = *rc::gen::inRange(0, 30);
                    if (curAmmo > maxAmmo) curAmmo = maxAmmo;
                    input.ammoInfo[i] = AmmoInfo{curAmmo, maxAmmo};
                }
            } else {
                input.equippedItems[i] = "";
            }
        }

        // Generate characteristics
        for (int i = 0; i < 9; ++i) {
            input.characteristics[i] = *rc::gen::inRange(1, 100);
        }

        // Generate passive skills
        int numSkills = *rc::gen::inRange(0, 8);
        for (int i = 0; i < numSkills; ++i) {
            std::string skill = *rc::gen::string(1, 15);
            input.passiveSkills.push_back(skill);
        }

        auto lines = generateSidebarContent(input);
        auto content = joinLines(lines);

        // (a) Every equipped item name appears
        for (int i = 0; i < 4; ++i) {
            if (!input.equippedItems[i].empty()) {
                RC_ASSERT(contentContains(content, input.equippedItems[i]));
            }
        }

        // (b) Every empty slot shows placeholder
        for (int i = 0; i < 4; ++i) {
            if (input.equippedItems[i].empty()) {
                std::string expected = SLOT_NAMES[i] + ": " + EMPTY_PLACEHOLDER;
                RC_ASSERT(contentContains(content, expected));
            }
        }

        // (c) Ammo info for ranged weapons
        for (int i = 0; i < 4; ++i) {
            if (input.ammoInfo[i].has_value()) {
                const auto& ammo = input.ammoInfo[i].value();
                std::string expectedAmmo = input.equippedItems[i] + ": "
                    + std::to_string(ammo.currentAmmo) + "/"
                    + std::to_string(ammo.maxAmmo);
                RC_ASSERT(contentContains(content, expectedAmmo));
            }
        }

        // (d) All 9 characteristics
        for (int i = 0; i < 9; ++i) {
            std::string expected = CHAR_ABBREVIATIONS[i] + " "
                + std::to_string(input.characteristics[i]);
            RC_ASSERT(contentContains(content, expected));
        }

        // (e) All passive skills
        for (const auto& skill : input.passiveSkills) {
            RC_ASSERT(contentContains(content, skill));
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Edge Cases — unit tests
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Sidebar: all slots empty shows 4 placeholders", "[unit][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    SidebarInput input{};
    for (int i = 0; i < 4; ++i) input.equippedItems[i] = "";
    for (int i = 0; i < 9; ++i) input.characteristics[i] = 30;

    auto content = joinLines(generateSidebarContent(input));

    for (int i = 0; i < 4; ++i) {
        std::string expected = SLOT_NAMES[i] + ": " + EMPTY_PLACEHOLDER;
        CHECK(contentContains(content, expected));
    }
}

TEST_CASE("Sidebar: all slots equipped shows no placeholders", "[unit][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    SidebarInput input{};
    input.equippedItems[0] = "Laspistol";
    input.equippedItems[1] = "Combat Knife";
    input.equippedItems[2] = "Flak Helmet";
    input.equippedItems[3] = "Flak Armour";
    for (int i = 0; i < 9; ++i) input.characteristics[i] = 35;

    auto content = joinLines(generateSidebarContent(input));

    CHECK_FALSE(contentContains(content, EMPTY_PLACEHOLDER));
    CHECK(contentContains(content, "Laspistol"));
    CHECK(contentContains(content, "Combat Knife"));
    CHECK(contentContains(content, "Flak Helmet"));
    CHECK(contentContains(content, "Flak Armour"));
}

TEST_CASE("Sidebar: ranged weapon shows ammo format", "[unit][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    SidebarInput input{};
    input.equippedItems[0] = "Laspistol";
    input.equippedItems[1] = "";
    input.equippedItems[2] = "";
    input.equippedItems[3] = "";
    input.ammoInfo[0] = AmmoInfo{4, 6};
    for (int i = 0; i < 9; ++i) input.characteristics[i] = 30;

    auto content = joinLines(generateSidebarContent(input));

    CHECK(contentContains(content, "Laspistol: 4/6"));
}

TEST_CASE("Sidebar: all characteristics displayed", "[unit][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    SidebarInput input{};
    for (int i = 0; i < 4; ++i) input.equippedItems[i] = "";
    input.characteristics = {35, 42, 30, 28, 40, 32, 38, 30, 25};

    auto content = joinLines(generateSidebarContent(input));

    CHECK(contentContains(content, "WS 35"));
    CHECK(contentContains(content, "BS 42"));
    CHECK(contentContains(content, "S 30"));
    CHECK(contentContains(content, "T 28"));
    CHECK(contentContains(content, "Ag 40"));
    CHECK(contentContains(content, "Int 32"));
    CHECK(contentContains(content, "Per 38"));
    CHECK(contentContains(content, "WP 30"));
    CHECK(contentContains(content, "Fel 25"));
}

TEST_CASE("Sidebar: passive skills displayed", "[unit][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    SidebarInput input{};
    for (int i = 0; i < 4; ++i) input.equippedItems[i] = "";
    for (int i = 0; i < 9; ++i) input.characteristics[i] = 30;
    input.passiveSkills = {"Dodge", "Awareness", "Parry"};

    auto content = joinLines(generateSidebarContent(input));

    CHECK(contentContains(content, "Dodge"));
    CHECK(contentContains(content, "Awareness"));
    CHECK(contentContains(content, "Parry"));
}

TEST_CASE("Sidebar: no passive skills shows empty skills section", "[unit][ui-rework][sidebar]")
{
    using namespace test_sidebar;

    SidebarInput input{};
    for (int i = 0; i < 4; ++i) input.equippedItems[i] = "";
    for (int i = 0; i < 9; ++i) input.characteristics[i] = 30;
    // No passive skills

    auto lines = generateSidebarContent(input);
    auto content = joinLines(lines);

    // The skills section header should still appear
    CHECK(contentContains(content, "-- Skills --"));
    // But no skill names after it (last line should be the section header)
    auto it = std::find(lines.begin(), lines.end(), "-- Skills --");
    REQUIRE(it != lines.end());
    // Nothing after the skills header
    CHECK(std::next(it) == lines.end());
}
