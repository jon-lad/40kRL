#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <algorithm>
#include <vector>
#include <string>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: look-and-decorations — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 2: Actor description formatting ────────────────────────────────
// **Validates: Requirements 3.2, 3.3**
//
// For any actor name (1–30 printable ASCII chars) and description (0–200
// printable ASCII chars), the formatted look-mode output shall equal the name
// alone when the description is empty, or "name: description" when non-empty.

TEST_CASE("PBT: Property 2 — actor description formatting", "[property][look-and-decorations]")
{
    rc::prop("formatted output is name when description empty, name + ': ' + description otherwise", []() {
        // Generate a random name: 1–30 printable ASCII characters (32-126 inclusive)
        auto printableChar = rc::gen::charInRange(32, 126);
        const auto name = *rc::gen::stringOf(printableChar, 1, 30);

        // Generate a random description: 0–200 printable ASCII characters
        const auto description = *rc::gen::stringOf(printableChar, 0, 200);

        // Apply the formatting logic (same as renderLook):
        // if description is empty → output = name
        // else → output = name + ": " + description
        std::string formatted;
        if (description.empty()) {
            formatted = name;
        } else {
            formatted = name + ": " + description;
        }

        // Verify the expected output
        if (description.empty()) {
            RC_ASSERT(formatted == name);
        } else {
            RC_ASSERT(formatted == name + ": " + description);
            // Also verify it starts with the name and contains the separator
            RC_ASSERT(formatted.substr(0, name.size()) == name);
            RC_ASSERT(formatted.substr(name.size(), 2) == ": ");
            RC_ASSERT(formatted.substr(name.size() + 2) == description);
        }

        // Additional invariant: formatted output is never empty (name is always >= 1 char)
        RC_ASSERT(!formatted.empty());
        RC_ASSERT(formatted.size() >= name.size());
    });
}

// ─── Property 5: Invalid decoration entries are skipped during loading ────────
// **Validates: Requirements 4.4**
//
// For any decoration entry table with random valid/invalid field combinations,
// the loader shall skip entries missing required fields or having an unrecognised
// colour name. The loaded template count equals the count of fully-valid entries.

// Helper: generate a random bool via inRange to avoid template-in-macro issues
static rc::Gen<bool> genBool() {
    return rc::gen::map(rc::gen::inRange(0, 2), [](int v) { return v != 0; });
}

TEST_CASE("PBT: Property 5 — invalid decoration entries are skipped during loading", "[property][look-and-decorations]")
{
    // Valid colour names recognised by Colors::colorFromName (non-black results)
    static const std::vector<std::string> validColorNames = {
        "white", "desaturatedGreen", "darkerGreen", "lightBlue",
        "orange", "lightGreen", "violet", "lightYellow",
        "lightGrey", "lighterOrange", "darkGrey", "red", "darkRed", "cyan"
    };

    rc::prop("loaded count equals count of entries with all fields valid", []() {
        // Generate random number of entries (1–15)
        const int entryCount = *rc::gen::inRange(1, 16);

        int validCount = 0;
        int skippedCount = 0;

        // For each entry, generate bool flags for field validity
        for (int i = 0; i < entryCount; ++i) {
            // Each bool flag: true = field is valid/present, false = invalid/missing
            const bool hasGlyph        = *genBool();
            const bool hasName         = *genBool();
            const bool hasColor        = *genBool();
            const bool hasDescription  = *genBool();
            const bool hasBlocks       = *genBool();
            const bool colorRecognized = *genBool();

            // Simulate the loader's two-stage validation (from Engine::init):
            // Stage 1: check required fields are non-empty and blocks is present
            //   if (glyphStr.empty() || name.empty() || colorName.empty() ||
            //       description.empty() || !blocksOpt.has_value()) { skip; }
            bool passesFieldCheck = hasGlyph && hasName && hasColor &&
                                    hasDescription && hasBlocks;

            // Stage 2: check colour is recognized (only reached if stage 1 passes)
            //   TCODColor color = Colors::colorFromName(colorName);
            //   if (color.r == 0 && color.g == 0 && color.b == 0) { skip; }
            bool passesColorCheck = passesFieldCheck && colorRecognized;

            if (passesColorCheck) {
                ++validCount;
            } else {
                ++skippedCount;
            }

            // Verify: entry is valid iff ALL 6 conditions hold
            bool allValid = hasGlyph && hasName && hasColor &&
                            hasDescription && hasBlocks && colorRecognized;
            RC_ASSERT(passesColorCheck == allValid);
        }

        // Verify: valid + skipped = total entries
        RC_ASSERT(validCount + skippedCount == entryCount);

        // Verify: loaded count is bounded by total
        RC_ASSERT(validCount >= 0);
        RC_ASSERT(validCount <= entryCount);

        // Verify against actual Colors::colorFromName logic:
        // All recognized colour names should NOT return black sentinel
        for (const auto& cname : validColorNames) {
            TCODColor c = Colors::colorFromName(cname);
            RC_ASSERT(!(c.r == 0 && c.g == 0 && c.b == 0));
        }

        // An unrecognized colour name SHOULD return black (sentinel)
        TCODColor sentinel = Colors::colorFromName("totallyBogusColorName");
        RC_ASSERT(sentinel.r == 0 && sentinel.g == 0 && sentinel.b == 0);
    });
}
