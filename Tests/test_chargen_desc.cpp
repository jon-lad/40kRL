#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"

#include <string>
#include <array>

// ═══════════════════════════════════════════════════════════════════════════════
// Test-local model for character generation description display.
// This mirrors the interface defined in the design document and will be replaced
// by production code once tasks 13.2 and 13.3 are complete.
// ═══════════════════════════════════════════════════════════════════════════════

namespace ChargenDescModel {

// Simplified template for testing — name + description
struct Template {
    std::string name;
    std::string description;
};

// Model function: given a highlighted template, produce "rendered output" string.
// When description is non-empty, output must contain the description text.
// When description is empty, output should contain "No description available."
std::string renderHighlightedDescription(const Template& tmpl) {
    std::string output;
    output += "[" + tmpl.name + "]\n";
    if (tmpl.description.empty()) {
        output += "No description available.";
    } else {
        output += tmpl.description;
    }
    return output;
}

} // namespace ChargenDescModel

using ChargenDescModel::Template;
using ChargenDescModel::renderHighlightedDescription;

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: ui-rework, Property 10: Character generation description display
// **Validates: Requirements 9.1, 9.2, 9.5**
//
// For any homeworld or career template with a non-empty description field,
// when that option is highlighted in the character generation menu, the
// rendered output SHALL contain the template's description text.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Chargen description — non-empty description appears in output",
          "[pbt][property][ui-rework][chargen-desc]")
{
    // Feature: ui-rework, Property 10: Character generation description display
    rc::check("Non-empty description is contained in rendered output", []() {
        // Generate a non-empty name
        const std::string name = *rc::gen::string(1, 30);

        // Generate a non-empty description (the core property condition)
        const std::string description = *rc::gen::string(1, 100);

        Template tmpl{name, description};
        const std::string output = renderHighlightedDescription(tmpl);

        // The rendered output SHALL contain the template's description text
        RC_ASSERT(output.find(description) != std::string::npos);
    });
}

TEST_CASE("PBT: Chargen description — empty description shows placeholder",
          "[pbt][property][ui-rework][chargen-desc]")
{
    // Feature: ui-rework, Property 10: Character generation description display
    rc::check("Empty description produces 'No description available.' in output", []() {
        // Generate a non-empty name but empty description
        const std::string name = *rc::gen::string(1, 30);

        Template tmpl{name, ""};
        const std::string output = renderHighlightedDescription(tmpl);

        // When description is empty, output should contain placeholder text
        RC_ASSERT(output.find("No description available.") != std::string::npos);
    });
}

TEST_CASE("PBT: Chargen description — output updates when highlighted option changes",
          "[pbt][property][ui-rework][chargen-desc]")
{
    // Feature: ui-rework, Property 10: Character generation description display
    // Validates Requirement 9.5: description updates to match currently highlighted option
    rc::check("Different highlighted templates produce output containing their own description", []() {
        // Generate two distinct non-empty descriptions
        const std::string name1 = *rc::gen::string(1, 30);
        const std::string desc1 = *rc::gen::string(1, 100);
        const std::string name2 = *rc::gen::string(1, 30);
        const std::string desc2 = *rc::gen::string(1, 100);

        // Precondition: descriptions must be different to meaningfully test switching
        RC_PRE(desc1 != desc2);

        Template tmpl1{name1, desc1};
        Template tmpl2{name2, desc2};

        const std::string output1 = renderHighlightedDescription(tmpl1);
        const std::string output2 = renderHighlightedDescription(tmpl2);

        // Each output contains its own template's description
        RC_ASSERT(output1.find(desc1) != std::string::npos);
        RC_ASSERT(output2.find(desc2) != std::string::npos);

        // And they are different outputs (since descriptions differ)
        RC_ASSERT(output1 != output2);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Unit tests — specific examples for chargen description display
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Chargen description: homeworld with lore text", "[unit][ui-rework][chargen-desc]")
{
    Template tmpl{"Hive World", "Born in the underhive, you learned to survive among the teeming masses."};
    const std::string output = renderHighlightedDescription(tmpl);

    CHECK(output.find("Born in the underhive, you learned to survive among the teeming masses.") != std::string::npos);
    CHECK(output.find("Hive World") != std::string::npos);
}

TEST_CASE("Chargen description: career with lore text", "[unit][ui-rework][chargen-desc]")
{
    Template tmpl{"Void-Master", "A seasoned navigator of the void, commanding ships through the warp."};
    const std::string output = renderHighlightedDescription(tmpl);

    CHECK(output.find("A seasoned navigator of the void, commanding ships through the warp.") != std::string::npos);
    CHECK(output.find("Void-Master") != std::string::npos);
}

TEST_CASE("Chargen description: empty description shows placeholder", "[unit][ui-rework][chargen-desc]")
{
    Template tmpl{"Unknown World", ""};
    const std::string output = renderHighlightedDescription(tmpl);

    CHECK(output.find("No description available.") != std::string::npos);
}

TEST_CASE("Chargen description: switching between options updates output", "[unit][ui-rework][chargen-desc]")
{
    Template tmpl1{"Feral World", "A world of savage beasts and tribal warriors."};
    Template tmpl2{"Forge World", "The domain of the Adeptus Mechanicus, filled with industry and machines."};

    const std::string output1 = renderHighlightedDescription(tmpl1);
    const std::string output2 = renderHighlightedDescription(tmpl2);

    CHECK(output1.find("savage beasts") != std::string::npos);
    CHECK(output2.find("Adeptus Mechanicus") != std::string::npos);

    // Ensure the outputs are distinct
    CHECK(output1 != output2);
}
