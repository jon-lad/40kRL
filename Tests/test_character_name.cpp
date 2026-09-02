#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

// The pure seam under test. This header does NOT exist yet on the unfixed code
// (created by task 3.1). Including it here is deliberate: on unfixed code the
// test project fails to build (missing header / unresolved symbol), which is
// the documented exploration signal that confirms the bug — character
// generation never sanitizes or assigns a name.
#include "CharacterName.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: character-name-entry (bugfix) — Property-Based & Unit Tests
//
// Tests the pure, engine-free seam:
//     std::string sanitizeCharacterName(const std::string& input)
// declared in Headers/CharacterName.hpp, with:
//     constexpr int MAX_NAME_LENGTH = 24
//     default fallback name "Rogue Trader"
//
// Sanitization rules (from design.md):
//   1. strip non-printable characters
//   2. truncate to MAX_NAME_LENGTH
//   3. trim leading/trailing whitespace
//   4. if the result is empty, return "Rogue Trader"
//
// Per test-isolation.md: sanitizeCharacterName touches no engine/SDL/GUI/libtcod
// state, so no Engine initialization is needed here.
//
// RapidCheck rc::gen::inRange uses INCLUSIVE bounds [a, b] in this project's
// custom header (see test-isolation.md). All bounds below are inclusive.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

const std::string DEFAULT_NAME = "Rogue Trader";

// A character is "printable" iff std::isprint treats it as such. Cast through
// unsigned char to avoid UB on negative char values (per cppreference guidance).
bool isPrintableChar(char c)
{
    return std::isprint(static_cast<unsigned char>(c)) != 0;
}

bool allPrintable(const std::string& s)
{
    return std::all_of(s.begin(), s.end(), isPrintableChar);
}

// True iff the string is all whitespace (or empty).
bool isEmptyOrWhitespace(const std::string& s)
{
    return std::all_of(s.begin(), s.end(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c)) != 0;
    });
}

// Compute what the input "reduces to" after stripping non-printables, truncating
// to MAX_NAME_LENGTH, and trimming surrounding whitespace. Used to decide whether
// the sanitized result should be the default fallback (reduces to empty).
bool reducesToEmpty(const std::string& input)
{
    // Strip non-printable characters.
    std::string stripped;
    for (char c : input) {
        if (isPrintableChar(c)) {
            stripped.push_back(c);
        }
    }
    // Truncate to MAX_NAME_LENGTH.
    if (static_cast<int>(stripped.size()) > MAX_NAME_LENGTH) {
        stripped.resize(MAX_NAME_LENGTH);
    }
    // Trim leading/trailing whitespace.
    return isEmptyOrWhitespace(stripped);
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Task 1 — Bug condition exploration property test
// Feature: character-name-entry, Property 1: Bug Condition — Name Sanitization
//   Produces a Valid Name
// **Validates: Requirements 1.1, 1.2, 2.2, 2.3, 2.4, 2.5**
//
// CRITICAL: This test MUST FAIL on unfixed code (the CharacterName.hpp seam does
// not exist yet). Its failure — a build/link failure — confirms the bug: character
// generation has no NAME step and never sanitizes/assigns a name. Once the helper
// exists (task 3.1) this same test validates the fix.
//
// For any raw input satisfying the Bug Condition (empty/whitespace-only,
// over-length > 24, or containing non-printable characters), the sanitized
// result must be non-empty, length <= MAX_NAME_LENGTH, printable only, and equal
// to "Rogue Trader" when the input reduces to empty.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 1 — sanitizeCharacterName corrects buggy inputs", "[pbt][property][character-name-entry]")
{
    // rc::check runs >= 100 iterations by default.
    rc::check("buggy inputs -> non-empty, bounded, printable name (empty-reducing -> default)", []() {
        // Choose one of the three Bug Condition shapes each iteration so we
        // exercise empty/whitespace, over-length, and non-printable broadly.
        //   0 = empty or whitespace-only
        //   1 = over-length (> MAX_NAME_LENGTH) printable content
        //   2 = contains non-printable / control characters
        const int shape = *rc::gen::inRange(0, 2);

        std::string input;

        switch (shape) {
            case 0: {
                // Empty or whitespace-only: a run of space/tab characters.
                const int len = *rc::gen::inRange(0, 30);
                const char ws = *rc::gen::elementOf(std::vector<char>{ ' ', '\t' });
                input.assign(static_cast<size_t>(len), ws);
                break;
            }
            case 1: {
                // Over-length: length strictly greater than MAX_NAME_LENGTH,
                // built from printable, non-space characters so it does not
                // reduce to empty.
                const int len = *rc::gen::inRange(MAX_NAME_LENGTH + 1, MAX_NAME_LENGTH + 40);
                for (int i = 0; i < len; ++i) {
                    input.push_back(static_cast<char>(*rc::gen::inRange('A', 'Z')));
                }
                break;
            }
            case 2: {
                // Contains non-printable characters: interleave printable chars
                // with control characters (0x00-0x1F).
                const int len = *rc::gen::inRange(1, 30);
                for (int i = 0; i < len; ++i) {
                    if (*rc::gen::arbitrary_bool()) {
                        input.push_back(static_cast<char>(*rc::gen::inRange('a', 'z')));
                    } else {
                        input.push_back(static_cast<char>(*rc::gen::inRange(0, 31)));
                    }
                }
                break;
            }
        }

        const std::string result = sanitizeCharacterName(input);

        // (1) Result is non-empty (Req 2.3 — always a usable name).
        RC_ASSERT(!result.empty());

        // (2) Result is no longer than MAX_NAME_LENGTH (Req 2.4).
        RC_ASSERT(static_cast<int>(result.size()) <= MAX_NAME_LENGTH);

        // (3) Result is composed of printable characters only (Req 2.5).
        RC_ASSERT(allPrintable(result));

        // (4) When the input reduces to empty, the result is the default (Req 2.3).
        if (reducesToEmpty(input)) {
            RC_ASSERT(result == DEFAULT_NAME);
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 1 — Concrete unit assertions for the Bug Condition
// **Validates: Requirements 2.2, 2.3, 2.4, 2.5**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Sanitize: empty input falls back to the default name", "[character-name-entry]")
{
    // Empty input reduces to empty -> default fallback (Req 2.3).
    REQUIRE(sanitizeCharacterName("") == "Rogue Trader");
}

TEST_CASE("Sanitize: whitespace-only input falls back to the default name", "[character-name-entry]")
{
    // Whitespace-only trims to empty -> default fallback (Req 2.3).
    REQUIRE(sanitizeCharacterName("   ") == "Rogue Trader");
}

TEST_CASE("Sanitize: over-length input is truncated to MAX_NAME_LENGTH", "[character-name-entry]")
{
    // A 40-character printable input must truncate to exactly 24 chars (Req 2.4).
    const std::string input(40, 'A');
    const std::string result = sanitizeCharacterName(input);

    REQUIRE(result.size() == static_cast<size_t>(MAX_NAME_LENGTH));
    REQUIRE(result == std::string(24, 'A'));
}

TEST_CASE("Sanitize: control characters are stripped", "[character-name-entry]")
{
    // A control character (0x01) embedded in the name is removed (Req 2.5),
    // leaving only the printable characters.
    REQUIRE(sanitizeCharacterName("Ro\x01gue") == "Rogue");

    // A leading control character alone is stripped, and (once trimmed) the
    // remainder is a valid printable name.
    REQUIRE(sanitizeCharacterName("\x01") == "Rogue Trader");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 2 — Preservation property test
// Feature: character-name-entry, Property 2: Preservation — Valid Names Are
//   Unchanged
// **Validates: Requirements 3.1, 3.2, 3.3, 3.4**
//
// For any already-valid name (length in [1, MAX_NAME_LENGTH], printable
// characters only, no leading/trailing whitespace), sanitizeCharacterName is the
// identity: it returns the input unchanged.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 2 — sanitizeCharacterName is identity for valid names", "[pbt][property][character-name-entry]")
{
    rc::check("already-valid names are returned unchanged", []() {
        // Length in [1, MAX_NAME_LENGTH] (inclusive bounds).
        const int len = *rc::gen::inRange(1, MAX_NAME_LENGTH);

        // Build a printable name. To guarantee no leading/trailing whitespace,
        // draw every character from a printable, non-space range ('A'..'Z' plus
        // lowercase and digits). Internal characters are all non-space, so the
        // whole string has no surrounding whitespace by construction.
        std::string name;
        name.reserve(static_cast<size_t>(len));
        for (int i = 0; i < len; ++i) {
            // Printable, non-whitespace ASCII range: '!' (0x21) .. '~' (0x7E).
            const char c = static_cast<char>(*rc::gen::inRange('!', '~'));
            name.push_back(c);
        }

        // Sanity: the generated name is a valid name per the preservation domain.
        RC_ASSERT(!name.empty());
        RC_ASSERT(static_cast<int>(name.size()) <= MAX_NAME_LENGTH);
        RC_ASSERT(allPrintable(name));
        RC_ASSERT(!std::isspace(static_cast<unsigned char>(name.front())));
        RC_ASSERT(!std::isspace(static_cast<unsigned char>(name.back())));

        // Identity: a valid name is preserved exactly (Req 3.3).
        RC_ASSERT(sanitizeCharacterName(name) == name);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 2 — Concrete preservation unit assertions
// **Validates: Requirements 3.3**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Sanitize: a 24-char printable name is returned unchanged", "[character-name-entry]")
{
    // Boundary: a name exactly at MAX_NAME_LENGTH (24), all printable, no
    // surrounding whitespace, is preserved verbatim (Req 3.3).
    const std::string name(24, 'B');
    REQUIRE(name.size() == static_cast<size_t>(MAX_NAME_LENGTH));
    REQUIRE(sanitizeCharacterName(name) == name);
}

TEST_CASE("Sanitize: a typical valid name is returned unchanged", "[character-name-entry]")
{
    // A short, ordinary name is the identity under sanitization (Req 3.3).
    REQUIRE(sanitizeCharacterName("Cordelia") == "Cordelia");
}
