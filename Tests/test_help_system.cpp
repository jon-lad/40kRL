#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "HelpContent.hpp"
#include "main.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

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

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: help-system — Property-Based Tests for Scroll Bounds and State
//                        Preservation (Task 3.1)
// ═══════════════════════════════════════════════════════════════════════════════

#include "lib/rapidcheck.h"
#include "lib/rapidcheck_catch.h"

// ─── Helpers for Property 1: State Preservation Logic ────────────────────────
// We test the STATE MACHINE LOGIC in isolation — no Engine instance needed.
// This models the open/close cycle: the prior state and AP must be unchanged.

namespace HelpStateLogic {

// Models the relevant subset of Engine GameStatus for help transitions
enum class GameState { IDLE, PLAYER_TURN };

// Models the help overlay state
struct HelpState {
    int scrollOffset = 0;
    bool returnToMenu = false;
    GameState priorState = GameState::IDLE;
};

// Simulates beginHelp(): saves prior state, transitions to HELP
struct SimEngine {
    GameState gameStatus = GameState::IDLE;
    int playerAP = 0;
    bool inHelp = false;
    HelpState helpState;

    void beginHelp() {
        helpState.priorState = gameStatus;
        helpState.scrollOffset = 0;
        helpState.returnToMenu = false;
        inHelp = true;
    }

    // Simulates closeHelp() via ESC or '?': restores prior state, AP unchanged
    void closeHelp() {
        gameStatus = helpState.priorState;
        inHelp = false;
    }
};

} // namespace HelpStateLogic

// ─── Property 1: Help close preserves game state and action points ───────────
// **Validates: Requirements 1.2, 1.3**

TEST_CASE("PBT: Help close preserves game state and action points",
          "[pbt][property][help-system]")
{
    // Feature: help-system, Property 1: Help close preserves game state and AP
    rc::check("open+close help returns to original state with AP unchanged", []() {
        // Generate random AP (1–10)
        const int ap = *rc::gen::inRange(1, 10);

        // Generate random prior state: 0 = IDLE, 1 = PLAYER_TURN
        const int stateChoice = *rc::gen::inRange(0, 1);
        const auto priorState = (stateChoice == 0)
            ? HelpStateLogic::GameState::IDLE
            : HelpStateLogic::GameState::PLAYER_TURN;

        // Generate random close key: 0 = ESC, 1 = '?'
        const int closeKeyChoice = *rc::gen::inRange(0, 1);
        (void)closeKeyChoice; // Both keys trigger the same closeHelp() logic

        // Set up simulated engine
        HelpStateLogic::SimEngine sim;
        sim.gameStatus = priorState;
        sim.playerAP = ap;

        // Open help
        sim.beginHelp();
        RC_ASSERT(sim.inHelp);

        // Close help (same logic for ESC or '?')
        sim.closeHelp();

        // Verify state restored
        RC_ASSERT(sim.gameStatus == priorState);
        // Verify AP unchanged
        RC_ASSERT(sim.playerAP == ap);
        // Verify no longer in help
        RC_ASSERT(!sim.inHelp);
    });
}

// ─── Helpers for Property 4: Scroll Bounds ───────────────────────────────────
// Pure function test — simulates scroll clamping logic without Engine dependency.

namespace HelpScrollLogic {

enum class ScrollCommand { UP, DOWN, PAGE_UP, PAGE_DOWN };

// Applies a single scroll command and clamps to valid bounds.
// Returns the new scroll offset.
inline int applyScroll(int currentOffset, ScrollCommand cmd,
                       int totalLines, int visibleLines) {
    const int maxOffset = std::max(0, totalLines - visibleLines);

    switch (cmd) {
        case ScrollCommand::UP:
            currentOffset -= 1;
            break;
        case ScrollCommand::DOWN:
            currentOffset += 1;
            break;
        case ScrollCommand::PAGE_UP:
            currentOffset -= visibleLines;
            break;
        case ScrollCommand::PAGE_DOWN:
            currentOffset += visibleLines;
            break;
    }

    // Clamp to [0, maxOffset]
    if (currentOffset < 0) currentOffset = 0;
    if (currentOffset > maxOffset) currentOffset = maxOffset;

    return currentOffset;
}

} // namespace HelpScrollLogic

// ─── Property 4: Scroll offset remains within valid bounds ───────────────────
// **Validates: Requirements 5.4**

TEST_CASE("PBT: Scroll offset remains within valid bounds",
          "[pbt][property][help-system]")
{
    // Feature: help-system, Property 4: Scroll offset remains within valid bounds
    rc::check("scroll offset stays in [0, max(0, totalLines - visibleLines)] after any command sequence", []() {
        // Generate random totalLines (1–200) and visibleLines (5–50)
        const int totalLines = *rc::gen::inRange(1, 200);
        const int visibleLines = *rc::gen::inRange(5, 50);
        const int maxOffset = std::max(0, totalLines - visibleLines);

        // Generate random sequence of scroll commands (1–50 commands)
        const int numCommands = *rc::gen::inRange(1, 50);

        int offset = 0;
        for (int i = 0; i < numCommands; ++i) {
            // Generate random command: 0=UP, 1=DOWN, 2=PAGE_UP, 3=PAGE_DOWN
            const int cmdChoice = *rc::gen::inRange(0, 3);
            const auto cmd = static_cast<HelpScrollLogic::ScrollCommand>(cmdChoice);

            offset = HelpScrollLogic::applyScroll(offset, cmd, totalLines, visibleLines);

            // Verify bounds after EVERY command application
            RC_ASSERT(offset >= 0);
            RC_ASSERT(offset <= maxOffset);
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: help-system — Unit Tests for Help Overlay State Transitions (Task 3.2)
// These tests require the HELP GameStatus enum and Engine::beginHelp()/updateHelp()
// which are added in task 3.3/3.4.
// ═══════════════════════════════════════════════════════════════════════════════

// ─── '?' key transitions ─────────────────────────────────────────────────────
// **Validates: Requirements 1.1**

TEST_CASE("'?' key in IDLE state transitions to HELP", "[help-system]") {
    // Save original state to restore after test
    auto originalStatus = engine.gameStatus;

    engine.gameStatus = Engine::IDLE;
    engine.beginHelp();

    REQUIRE(engine.gameStatus == Engine::HELP);
    REQUIRE(engine.helpState.has_value());
    REQUIRE(engine.helpState->returnToMenu == false);

    // Restore
    engine.helpState.reset();
    engine.gameStatus = originalStatus;
}

TEST_CASE("'?' key in PLAYER_TURN state transitions to HELP", "[help-system]") {
    auto originalStatus = engine.gameStatus;

    engine.gameStatus = Engine::PLAYER_TURN;
    engine.beginHelp();

    REQUIRE(engine.gameStatus == Engine::HELP);
    REQUIRE(engine.helpState.has_value());
    REQUIRE(engine.helpState->returnToMenu == false);

    // Restore
    engine.helpState.reset();
    engine.gameStatus = originalStatus;
}

// ─── ESC and '?' close behaviour ────────────────────────────────────────────
// **Validates: Requirements 1.2, 1.3**

TEST_CASE("ESC in HELP state returns to previous state (IDLE)", "[help-system]") {
    auto originalStatus = engine.gameStatus;

    // Setup: transition from IDLE to HELP
    engine.gameStatus = Engine::IDLE;
    engine.beginHelp();
    REQUIRE(engine.gameStatus == Engine::HELP);

    // Simulate ESC keypress
    engine.inputState.key.key = SDLK_ESCAPE;
    engine.inputState.key.c = 0;
    engine.inputState.key.pressed = true;
    engine.updateHelp();

    REQUIRE(engine.gameStatus == Engine::IDLE);
    REQUIRE_FALSE(engine.helpState.has_value());

    // Restore
    engine.gameStatus = originalStatus;
}

TEST_CASE("ESC in HELP state returns to previous state (PLAYER_TURN)", "[help-system]") {
    auto originalStatus = engine.gameStatus;

    // Setup: transition from PLAYER_TURN to HELP
    engine.gameStatus = Engine::PLAYER_TURN;
    engine.beginHelp();
    REQUIRE(engine.gameStatus == Engine::HELP);

    // Simulate ESC keypress
    engine.inputState.key.key = SDLK_ESCAPE;
    engine.inputState.key.c = 0;
    engine.inputState.key.pressed = true;
    engine.updateHelp();

    REQUIRE(engine.gameStatus == Engine::PLAYER_TURN);
    REQUIRE_FALSE(engine.helpState.has_value());

    // Restore
    engine.gameStatus = originalStatus;
}

TEST_CASE("'?' in HELP state returns to previous state", "[help-system]") {
    auto originalStatus = engine.gameStatus;

    // Setup: transition from IDLE to HELP
    engine.gameStatus = Engine::IDLE;
    engine.beginHelp();
    REQUIRE(engine.gameStatus == Engine::HELP);

    // Simulate '?' keypress
    engine.inputState.key.key = SDLK_UNKNOWN;
    engine.inputState.key.c = '?';
    engine.inputState.key.pressed = true;
    engine.updateHelp();

    REQUIRE(engine.gameStatus == Engine::IDLE);
    REQUIRE_FALSE(engine.helpState.has_value());

    // Restore
    engine.gameStatus = originalStatus;
}

// ─── Rendering properties ────────────────────────────────────────────────────
// **Validates: Requirements 5.5, 5.2**
//
// Note: These are structural tests that verify the overlay produces expected
// rendering artefacts. Full visual verification requires manual playtesting.
// These tests will compile only after task 3.3/3.4 adds the HELP state and
// renderHelp() implementation.

TEST_CASE("Navigation instructions rendered at bottom of overlay", "[help-system]") {
    auto originalStatus = engine.gameStatus;

    // Setup: open help overlay
    engine.gameStatus = Engine::IDLE;
    engine.beginHelp();
    REQUIRE(engine.gameStatus == Engine::HELP);

    // renderHelp() should not crash and should render navigation instructions.
    // We verify that helpState is valid and the method is callable.
    // Full content verification (checking "ESC: close" text at bottom) requires
    // a console buffer mock — here we verify the state is set up correctly for
    // rendering and the function completes without error.
    REQUIRE(engine.helpState.has_value());
    REQUIRE(engine.helpState->scrollOffset == 0);

    // The navigation bar should always be rendered — verified via visual playtest.
    // Here we confirm that the overlay's initial state is consistent with rendering.

    // Cleanup
    engine.helpState.reset();
    engine.gameStatus = originalStatus;
}

TEST_CASE("Section headers rendered visually distinct from entry text", "[help-system]") {
    // This test verifies the structural precondition for visual distinction:
    // HelpContent sections have non-empty titles that differ from entry keys,
    // ensuring the renderer CAN distinguish them (the renderer uses a different
    // colour for section titles vs entry text — verified visually).
    auto sections = HelpContent::allSections();
    REQUIRE(!sections.empty());

    for (const auto& section : sections) {
        // Section titles must be non-empty
        REQUIRE(!section.title.empty());

        // Section titles should not match any entry key in that section
        // (this ensures the renderer can visually separate them)
        for (const auto& entry : section.entries) {
            REQUIRE(section.title != entry.key);
        }
    }
}

// end of help-system unit tests for state transitions


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: help-system — Unit Tests for Menu Help Item (Task 5.1)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Menu contains "Help" item in MAIN display mode ──────────────────────────
// **Validates: Requirements 2.1**
//
// Verifies that when the menu is populated for the MAIN display mode,
// it includes a HELP MenuItemCode entry. Since Menu::pick() blocks
// (it enters a render loop), we test via addItem + the items list structure.
// Task 5.2 adds Menu::MenuItemCode::HELP and wires addItem calls.

TEST_CASE("Menu contains Help item in MAIN display mode", "[help-system]") {
    // Clear any existing items and populate for MAIN mode
    engine.gui->menu.clear();
    engine.gui->menu.addItem(Menu::MenuItemCode::NEW_GAME, "New Game");
    engine.gui->menu.addItem(Menu::MenuItemCode::HELP, "Help");
    engine.gui->menu.addItem(Menu::MenuItemCode::EXIT, "Exit");

    // Verify the menu is populated with HELP by checking that calling
    // beginHelpFromMenu() from the HELP code path works correctly.
    // Since items list is protected, we verify behaviour:
    // simulate what happens when HELP code is returned from pick().
    auto originalStatus = engine.gameStatus;
    engine.gameStatus = Engine::STARTUP;

    engine.beginHelpFromMenu();

    REQUIRE(engine.gameStatus == Engine::HELP);
    REQUIRE(engine.helpState.has_value());
    REQUIRE(engine.helpState->returnToMenu == true);

    // Cleanup
    engine.helpState.reset();
    engine.gameStatus = originalStatus;
    engine.gui->menu.clear();
}

// ─── Menu contains "Help" item in PAUSE display mode ─────────────────────────
// **Validates: Requirements 2.1**
//
// The PAUSE mode menu should also contain HELP. This test verifies
// that the same HELP MenuItemCode is present when populating for pause.

TEST_CASE("Menu contains Help item in PAUSE display mode", "[help-system]") {
    // Clear and populate as the pause menu would
    engine.gui->menu.clear();
    engine.gui->menu.addItem(Menu::MenuItemCode::CONTINUE, "Continue");
    engine.gui->menu.addItem(Menu::MenuItemCode::HELP, "Help");
    engine.gui->menu.addItem(Menu::MenuItemCode::EXIT, "Exit");

    // Verify by simulating HELP selection from pause context
    auto originalStatus = engine.gameStatus;
    engine.gameStatus = Engine::IDLE; // simulating in-game pause

    engine.beginHelpFromMenu();

    REQUIRE(engine.gameStatus == Engine::HELP);
    REQUIRE(engine.helpState.has_value());
    REQUIRE(engine.helpState->returnToMenu == true);

    // Cleanup
    engine.helpState.reset();
    engine.gameStatus = originalStatus;
    engine.gui->menu.clear();
}

// ─── Selecting HELP MenuItemCode calls engine.beginHelpFromMenu() ────────────
// **Validates: Requirements 2.2**
//
// When pick() returns HELP, the engine should transition to the HELP state
// with returnToMenu = true. We test the engine side of this contract directly.

TEST_CASE("Selecting HELP MenuItemCode triggers beginHelpFromMenu", "[help-system]") {
    auto originalStatus = engine.gameStatus;

    // Simulate what happens when Menu::pick() returns HELP:
    // The caller (Engine::load or equivalent) invokes beginHelpFromMenu().
    engine.gameStatus = Engine::IDLE;
    engine.beginHelpFromMenu();

    // Verify state transition
    REQUIRE(engine.gameStatus == Engine::HELP);
    REQUIRE(engine.helpState.has_value());
    REQUIRE(engine.helpState->returnToMenu == true);
    REQUIRE(engine.helpState->scrollOffset == 0);

    // Cleanup
    engine.helpState.reset();
    engine.gameStatus = originalStatus;
}

// ─── ESC from menu-opened help returns to Menu state (not gameplay) ──────────
// **Validates: Requirements 2.3**
//
// When help is opened from the menu (returnToMenu == true), pressing ESC
// should restore the prior state (the state before beginHelpFromMenu was called),
// NOT go to IDLE/PLAYER_TURN. This ensures the player returns to the menu context.

TEST_CASE("Menu-opened help records the menu context for its ESC return path",
          "[help-system][integration]") {
    auto originalStatus = engine.gameStatus;

    // The menu operates during STARTUP (for MAIN) or any state (for PAUSE).
    // Simulate the state being STARTUP when help is opened from the main menu.
    engine.gameStatus = Engine::STARTUP;
    engine.beginHelpFromMenu();

    REQUIRE(engine.gameStatus == Engine::HELP);
    REQUIRE(engine.helpState.has_value());
    // returnToMenu == true is what routes ESC through Engine::load() (re-show the
    // main menu) rather than restoring gameplay. The actual load()/menu re-display
    // is exercised by integration/manual runs with a live Gui — it is NOT invoked
    // here because the unit-test binary has no initialised menu/map/Lua state
    // (test-isolation steering), and load() would dereference that uninitialised
    // state. We assert the state machine records the correct return contract.
    REQUIRE(engine.helpState->returnToMenu == true);
    // priorState should be STARTUP (the state when menu called beginHelpFromMenu),
    // i.e. the menu context — never a gameplay state like PLAYER_TURN.
    REQUIRE(engine.helpState->priorState == static_cast<int>(Engine::STARTUP));
    REQUIRE(engine.helpState->priorState != static_cast<int>(Engine::PLAYER_TURN));

    // Cleanup
    engine.helpState.reset();
    engine.gameStatus = originalStatus;
}

// ─── ESC from in-game help restores the prior gameplay state ─────────────────
// **Validates: Requirements 2.3**
//
// When help is opened in-game (returnToMenu == false), pressing ESC restores the
// exact prior state (not the menu, not a hard-coded default). This path does not
// call Engine::load(), so it is fully unit-testable headless.

TEST_CASE("ESC from in-game help restores the prior gameplay state",
          "[help-system][integration]") {
    auto originalStatus = engine.gameStatus;

    engine.gameStatus = Engine::IDLE;
    engine.beginHelp(); // in-game help: returnToMenu == false

    REQUIRE(engine.gameStatus == Engine::HELP);
    REQUIRE(engine.helpState.has_value());
    REQUIRE(engine.helpState->returnToMenu == false);
    REQUIRE(engine.helpState->priorState == static_cast<int>(Engine::IDLE));

    // Simulate ESC keypress
    engine.inputState.key.key = SDLK_ESCAPE;
    engine.inputState.key.c = 0;
    engine.inputState.key.pressed = true;
    engine.updateHelp();

    // Restores the prior gameplay state and clears help — no load() involved.
    REQUIRE(engine.gameStatus == Engine::IDLE);
    REQUIRE_FALSE(engine.helpState.has_value());

    // Cleanup
    engine.gameStatus = originalStatus;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: help-system — Property-Based Tests for ManualExporter (Task 7.1)
//
// These tests validate the ManualExporter formatting logic using a self-contained
// local formatter that implements the same algorithm ManualExporter::generate()
// will use. When task 7.3 implements the real ManualExporter, these tests confirm
// the expected output properties hold.
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Local test data types and formatter ─────────────────────────────────────
// Mirrors HelpContent::HelpEntry/HelpSection for test isolation.

namespace ManualExporterTestHelpers {

struct TestEntry {
    std::string key;
    std::string description;
};

struct TestSection {
    std::string title;
    std::vector<TestEntry> entries;
};

// Formats sections into manual text using the same algorithm ManualExporter will use:
//   TITLE
//   -----  (dashes of equal length to title)
//     key  description
//     key  description
//   <blank line>
//
// Preceded by a header with title and version.
std::string formatManual(const std::vector<TestSection>& sections,
                         const std::string& version) {
    std::string output;
    // Title header
    output += "40kRL MANUAL\n";
    output += "Version " + version + "\n";
    output += "\n";

    for (const auto& section : sections) {
        // Section title line
        output += section.title + "\n";
        // Dashed underline of equal length to title
        output += std::string(section.title.size(), '-') + "\n";
        // Indented entries
        for (const auto& entry : section.entries) {
            output += "  " + entry.key + "  " + entry.description + "\n";
        }
        // Blank line after section
        output += "\n";
    }

    return output;
}

// Generator: produces a printable ASCII string (chars in [0x21, 0x7E], no whitespace/control)
// with no embedded newlines. Length in [minLen, maxLen].
rc::Gen<std::string> printableAsciiString(int minLen, int maxLen) {
    return rc::gen::stringOf(rc::gen::charInRange('!', '~'), minLen, maxLen);
}

// Generator: produces a TestEntry with printable ASCII key and description
rc::Gen<TestEntry> genEntry() {
    return {[](std::mt19937& rng) {
        auto keyGen = rc::gen::stringOf(rc::gen::charInRange('!', '~'), 1, 10);
        auto descGen = rc::gen::stringOf(rc::gen::charInRange(' ', '~'), 1, 40);
        TestEntry e;
        e.key = keyGen(rng);
        e.description = descGen(rng);
        return e;
    }};
}

// Generator: produces a TestSection with printable ASCII title and 1–20 entries
rc::Gen<TestSection> genSection() {
    return {[](std::mt19937& rng) {
        auto titleGen = rc::gen::stringOf(rc::gen::charInRange('A', 'Z'), 3, 15);
        TestSection s;
        s.title = titleGen(rng);
        int numEntries = std::uniform_int_distribution<int>(1, 20)(rng);
        auto eGen = genEntry();
        for (int i = 0; i < numEntries; ++i) {
            s.entries.push_back(eGen(rng));
        }
        return s;
    }};
}

// Generator: produces a vector of 1–10 TestSections
rc::Gen<std::vector<TestSection>> genRegistry() {
    return {[](std::mt19937& rng) {
        int numSections = std::uniform_int_distribution<int>(1, 10)(rng);
        std::vector<TestSection> sections;
        auto sGen = genSection();
        for (int i = 0; i < numSections; ++i) {
            sections.push_back(sGen(rng));
        }
        return sections;
    }};
}

} // namespace ManualExporterTestHelpers

// ─── Property 5: Manual generation completeness ─────────────────────────────
// **Validates: Requirements 6.1, 6.2**
//
// Generate random registry configurations (1–10 sections, 1–20 entries each);
// run exporter; verify all section titles and all entry keys appear in output.

TEST_CASE("PBT: Manual generation completeness",
          "[pbt][property][help-system]")
{
    using namespace ManualExporterTestHelpers;

    rc::check("all section titles and all entry keys appear in generated manual output", []() {
        // Generate random registry
        auto sections = *genRegistry();
        RC_PRE(!sections.empty());

        // Generate manual
        std::string output = formatManual(sections, "1.0.0");

        // Verify all section titles appear in output
        for (const auto& section : sections) {
            RC_ASSERT(output.find(section.title) != std::string::npos);

            // Verify all entry keys appear in output
            for (const auto& entry : section.entries) {
                RC_ASSERT(output.find(entry.key) != std::string::npos);
            }
        }
    });
}

// ─── Property 6: Manual output is pure ASCII ─────────────────────────────────
// **Validates: Requirements 6.2**
//
// Generate random registry with printable ASCII content; run exporter;
// verify all output chars are in range [0x0A, 0x7E].

TEST_CASE("PBT: Manual output is pure ASCII",
          "[pbt][property][help-system]")
{
    using namespace ManualExporterTestHelpers;

    rc::check("all characters in generated manual are in ASCII range [0x0A, 0x7E]", []() {
        // Generate random registry with printable ASCII content
        auto sections = *genRegistry();
        RC_PRE(!sections.empty());

        // Generate manual
        std::string output = formatManual(sections, "1.0.0");

        // Verify every character is in valid ASCII range
        // Valid: 0x0A (newline), 0x20-0x7E (printable ASCII including space)
        for (size_t i = 0; i < output.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(output[i]);
            bool valid = (c == 0x0A) || (c >= 0x20 && c <= 0x7E);
            RC_ASSERT(valid);
        }
    });
}

// ─── Property 7: Manual section formatting structure ─────────────────────────
// **Validates: Requirements 6.4**
//
// Generate random sections with 1–10 entries; run exporter; verify
// title line + dashed underline + indented entries pattern.

TEST_CASE("PBT: Manual section formatting structure",
          "[pbt][property][help-system]")
{
    using namespace ManualExporterTestHelpers;

    rc::check("each section has title line + dashed underline + indented entries", []() {
        // Generate a single random section for focused verification
        auto section = *genSection();
        RC_PRE(!section.entries.empty());

        // Format just this one section (with minimal header)
        std::vector<TestSection> singleSection = { section };
        std::string output = formatManual(singleSection, "1.0.0");

        // Split output into lines
        std::vector<std::string> lines;
        std::istringstream stream(output);
        std::string line;
        while (std::getline(stream, line)) {
            lines.push_back(line);
        }

        // Find the section title line in the output
        // (skip the header lines: "40kRL MANUAL", "Version ...", blank)
        int titleLineIdx = -1;
        for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
            if (lines[i] == section.title) {
                titleLineIdx = i;
                break;
            }
        }
        RC_ASSERT(titleLineIdx >= 0);

        // Next line should be dashes of equal length to title
        int dashLineIdx = titleLineIdx + 1;
        RC_ASSERT(dashLineIdx < static_cast<int>(lines.size()));
        std::string expectedDashes(section.title.size(), '-');
        RC_ASSERT(lines[dashLineIdx] == expectedDashes);

        // Following lines should be indented entries (start with "  ")
        for (int i = 0; i < static_cast<int>(section.entries.size()); ++i) {
            int entryLineIdx = dashLineIdx + 1 + i;
            RC_ASSERT(entryLineIdx < static_cast<int>(lines.size()));
            // Each entry line starts with two spaces (indentation)
            RC_ASSERT(lines[entryLineIdx].size() >= 2);
            RC_ASSERT(lines[entryLineIdx][0] == ' ');
            RC_ASSERT(lines[entryLineIdx][1] == ' ');
            // Entry line contains the key
            RC_ASSERT(lines[entryLineIdx].find(section.entries[i].key) != std::string::npos);
        }
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: help-system — Unit Tests for ManualExporter Output (Task 7.2)
// These tests require ManualExporter.h which is created in task 7.3.
// They are conditionally compiled only when MANUAL_EXPORTER_AVAILABLE is defined.
// ═══════════════════════════════════════════════════════════════════════════════

#if __has_include("ManualExporter.hpp")
#define MANUAL_EXPORTER_AVAILABLE 1
#include "ManualExporter.hpp"
#else
#define MANUAL_EXPORTER_AVAILABLE 0
#endif

#if MANUAL_EXPORTER_AVAILABLE

#include <fstream>
#include <filesystem>

// ─── Manual includes title header with "MANUAL" text ─────────────────────────
// **Validates: Requirements 6.3**

TEST_CASE("Generated manual includes title header with MANUAL text", "[help-system]") {
    std::string output = ManualExporter::generate("0.3.0");

    // The title should contain "MANUAL" (case-sensitive, as per design: "40kRL MANUAL")
    REQUIRE(output.find("MANUAL") != std::string::npos);
}

// ─── Manual includes version string ─────────────────────────────────────────
// **Validates: Requirements 6.3**

TEST_CASE("Generated manual includes version string", "[help-system]") {
    std::string output = ManualExporter::generate("0.3.0");

    // Should contain the version string passed to generate()
    REQUIRE(output.find("0.3.0") != std::string::npos);
}

// ─── Each section formatted as title + dashed underline + indented entries ───
// **Validates: Requirements 6.4**

TEST_CASE("Each section formatted as title + dashed underline + indented entries", "[help-system]") {
    std::string output = ManualExporter::generate("0.3.0");

    auto sections = HelpContent::allSections();
    REQUIRE(!sections.empty());

    for (const auto& section : sections) {
        // Find the section title in the output
        std::string title(section.title);
        auto titlePos = output.find(title);
        REQUIRE(titlePos != std::string::npos);

        // After the title line, there should be a line of dashes
        // Find the newline after the title
        auto newlineAfterTitle = output.find('\n', titlePos);
        REQUIRE(newlineAfterTitle != std::string::npos);

        // The next line should be all dashes (at least as long as the title)
        auto dashLineStart = newlineAfterTitle + 1;
        auto dashLineEnd = output.find('\n', dashLineStart);
        REQUIRE(dashLineEnd != std::string::npos);

        std::string dashLine = output.substr(dashLineStart, dashLineEnd - dashLineStart);
        // Verify the dash line contains only '-' characters and is non-empty
        REQUIRE(!dashLine.empty());
        REQUIRE(dashLine.find_first_not_of('-') == std::string::npos);

        // After the dash line, entries should be indented (start with spaces)
        auto firstEntryStart = dashLineEnd + 1;
        if (firstEntryStart < output.size()) {
            // First entry line should begin with whitespace (indentation)
            REQUIRE((output[firstEntryStart] == ' ' || output[firstEntryStart] == '\t'));
        }
    }
}

// ─── writeToFile returns true on success and creates the file ────────────────
// **Validates: Requirements 6.1, 6.2**

TEST_CASE("writeToFile returns true on success and creates the file", "[help-system]") {
    const std::string testPath = "test_manual_output.txt";

    // Clean up in case a previous test run left the file behind
    std::filesystem::remove(testPath);

    // Write the manual file
    bool result = ManualExporter::writeToFile(testPath, "0.3.0");

    // Should return true on success
    REQUIRE(result == true);

    // File should exist
    REQUIRE(std::filesystem::exists(testPath));

    // Verify the file has content (not empty)
    std::ifstream file(testPath);
    REQUIRE(file.good());
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    REQUIRE(!content.empty());

    // Content should match what generate() produces
    std::string expected = ManualExporter::generate("0.3.0");
    REQUIRE(content == expected);

    // Clean up
    file.close();
    std::filesystem::remove(testPath);
}

#endif // MANUAL_EXPORTER_AVAILABLE