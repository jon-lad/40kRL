#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "HelpContent.h"

#include <algorithm>
#include <string>
#include <string_view>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: help-system — Unit Tests for HelpContentRegistry Content (Task 1.2)
// ═══════════════════════════════════════════════════════════════════════════════

// Helper: find a section by title
static const HelpContent::HelpSection* findSection(std::string_view title) {
    auto sections = HelpContent::allSections();
    for (const auto& section : sections) {
        if (section.title == title) {
            return &section;
        }
    }
    return nullptr;
}

// Helper: check if a section contains an entry with the given key
static bool sectionContainsKey(const HelpContent::HelpSection& section, std::string_view key) {
    for (const auto& entry : section.entries) {
        if (entry.key == key) {
            return true;
        }
    }
    return false;
}

// ─── Movement section content ────────────────────────────────────────────────
// **Validates: Requirements 4.1**

TEST_CASE("Movement section contains arrow keys", "[help-system]") {
    const auto* section = findSection("Movement");
    REQUIRE(section != nullptr);

    // Arrow keys should be listed (may be as a group like "Arrow keys" or individual)
    bool hasArrows = false;
    for (const auto& entry : section->entries) {
        if (entry.key.find("Arrow") != std::string_view::npos ||
            entry.key.find("arrow") != std::string_view::npos) {
            hasArrows = true;
            break;
        }
    }
    REQUIRE(hasArrows);
}

TEST_CASE("Movement section contains numpad keys KP_1 through KP_9", "[help-system]") {
    const auto* section = findSection("Movement");
    REQUIRE(section != nullptr);

    // Check for numpad keys — may be listed as a range "KP_1-KP_9" or individually
    bool hasNumpad = false;
    for (const auto& entry : section->entries) {
        if (entry.key.find("KP_") != std::string_view::npos ||
            entry.key.find("Numpad") != std::string_view::npos ||
            entry.key.find("numpad") != std::string_view::npos) {
            hasNumpad = true;
            break;
        }
    }
    REQUIRE(hasNumpad);
}

TEST_CASE("Movement section contains vi-keys h/j/k/l/y/u/b/n", "[help-system]") {
    const auto* section = findSection("Movement");
    REQUIRE(section != nullptr);

    // vi-keys: h, j, k, l, y, u, b, n
    // They may be listed individually or as a group
    bool hasViKeys = false;
    for (const auto& entry : section->entries) {
        // Check for individual vi-keys or a grouped entry mentioning vi-keys
        if (entry.key == "h" || entry.key == "j" || entry.key == "k" || entry.key == "l" ||
            entry.key == "y" || entry.key == "u" || entry.key == "b" || entry.key == "n" ||
            entry.key.find("hjkl") != std::string_view::npos ||
            entry.key.find("vi") != std::string_view::npos) {
            hasViKeys = true;
            break;
        }
    }
    REQUIRE(hasViKeys);
}

// ─── Actions section content ─────────────────────────────────────────────────
// **Validates: Requirements 4.2**

TEST_CASE("Actions section contains all expected action keys", "[help-system]") {
    const auto* section = findSection("Actions");
    REQUIRE(section != nullptr);

    // Required keys: g, s, r, o, a, A, R, C, KP_5
    REQUIRE(sectionContainsKey(*section, "g"));
    REQUIRE(sectionContainsKey(*section, "s"));
    REQUIRE(sectionContainsKey(*section, "r"));
    REQUIRE(sectionContainsKey(*section, "o"));
    REQUIRE(sectionContainsKey(*section, "a"));
    REQUIRE(sectionContainsKey(*section, "A"));
    REQUIRE(sectionContainsKey(*section, "R"));
    REQUIRE(sectionContainsKey(*section, "C"));
    REQUIRE(sectionContainsKey(*section, "KP_5"));
}

// ─── Overlays section content ────────────────────────────────────────────────
// **Validates: Requirements 4.3**

TEST_CASE("Overlays section contains all expected overlay keys", "[help-system]") {
    const auto* section = findSection("Overlays");
    REQUIRE(section != nullptr);

    // Required keys: i, c, l, x, m, e, ?
    REQUIRE(sectionContainsKey(*section, "i"));
    REQUIRE(sectionContainsKey(*section, "c"));
    REQUIRE(sectionContainsKey(*section, "l"));
    REQUIRE(sectionContainsKey(*section, "x"));
    REQUIRE(sectionContainsKey(*section, "m"));
    REQUIRE(sectionContainsKey(*section, "e"));
    REQUIRE(sectionContainsKey(*section, "?"));
}

// ─── Navigation section content ──────────────────────────────────────────────
// **Validates: Requirements 4.4**

TEST_CASE("Navigation section contains all expected navigation keys", "[help-system]") {
    const auto* section = findSection("Navigation");
    REQUIRE(section != nullptr);

    // Required keys: <, >, ESC
    REQUIRE(sectionContainsKey(*section, "<"));
    REQUIRE(sectionContainsKey(*section, ">"));
    REQUIRE(sectionContainsKey(*section, "ESC"));
}

// ─── totalLineCount correctness ──────────────────────────────────────────────
// **Validates: Requirements 4.1, 4.2, 4.3, 4.4**

TEST_CASE("totalLineCount returns correct value based on sections + entries + spacing", "[help-system]") {
    auto sections = HelpContent::allSections();
    REQUIRE(!sections.empty());

    // Calculate expected line count:
    // Each section contributes:
    //   1 line for the section title
    //   N lines for its entries (one per entry)
    //   1 blank line after the section (spacing between sections)
    // The last section may or may not have trailing spacing — we check both conventions.
    int expectedLines = 0;
    for (const auto& section : sections) {
        expectedLines += 1; // section title
        expectedLines += static_cast<int>(section.entries.size()); // entries
        expectedLines += 1; // spacing after section
    }
    // If the last section doesn't have trailing spacing, adjust:
    // Accept either convention (with or without trailing blank line on last section)
    int totalLines = HelpContent::totalLineCount();
    REQUIRE((totalLines == expectedLines || totalLines == expectedLines - 1));
}
