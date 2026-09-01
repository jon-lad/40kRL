#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "HighScore.hpp"

#include "libtcod.hpp"

// main.hpp brings in the Menu / Menu::MenuItemCode declarations (Headers/Gui.hpp).
// Only Menu::clear()/addItem() are exercised here — both are pure list operations
// with no engine.gui/map/player/SDL access — so no Engine initialization is needed
// (per test-isolation.md).
#include "main.hpp"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: high-score-system — Property-Based & Unit Tests for the pure core
//
// Covers tasks 2.1, 2.2 (ranking), 3.1, 3.2 (leaderboard insertion), and
// 4.1 (serialization round-trip). The pure core never touches
// engine.gui/map/player, so these tests need no Engine initialization.
//
// NOTE (TDD): These tests are written before the real implementations
// (tasks 2.3 / 3.3 / 4.4). They are expected to COMPILE and LINK against the
// current stubs, and some assertions will FAIL until the pure core is
// implemented. That failure is correct TDD behavior.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// ─── Generators ──────────────────────────────────────────────────────────────
//
// RapidCheck rc::gen::inRange uses INCLUSIVE bounds [a, b] in this project's
// custom header (see test-isolation.md). All bounds below are inclusive.

// Generate a random ScoreEntry with a non-negative totalXp and a positive
// deepestLevel, per Requirements 1.2 (>= 0) and 1.3 (>= 1).
//
// Called from within an rc::check lambda; it uses the *rc::gen::... dereference
// syntax to draw each field from the current generator RNG.
ScoreEntry genScoreEntry()
{
    ScoreEntry e;
    e.characterName = *rc::gen::string(0, 12); // may be empty
    e.careerName    = *rc::gen::string(0, 12);
    e.homeworldName = *rc::gen::string(0, 12);
    e.rankTitle     = *rc::gen::string(0, 12);
    e.totalXp       = *rc::gen::inRange(0, 100000); // >= 0 (Req 1.2)
    e.deepestLevel  = *rc::gen::inRange(1, 50);     // >= 1 (Req 1.3)
    e.outcome       = *rc::gen::string(0, 12);
    e.date          = *rc::gen::string(0, 12);
    return e;
}

// Structural equality across every serialized field (Req 8.2).
bool entriesFieldEqual(const ScoreEntry& a, const ScoreEntry& b)
{
    return a.characterName == b.characterName
        && a.careerName    == b.careerName
        && a.homeworldName == b.homeworldName
        && a.rankTitle     == b.rankTitle
        && a.totalXp       == b.totalXp
        && a.deepestLevel  == b.deepestLevel
        && a.outcome       == b.outcome
        && a.date          == b.date;
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Task 8.3 — Unit tests for outcome formatting
// **Validates: Requirements 1.6, 1.7**
//
// formatOutcome() is the pure-core helper extracted from Engine::recordRunOutcome
// so the "Slain by <cause>" / "Slain" rule is testable without the Engine
// (per test-isolation.md). recordRunOutcome now delegates to it.
//   - non-empty cause -> "Slain by " + cause  (Req 1.6)
//   - empty cause     -> "Slain"              (Req 1.7)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Outcome: known cause formats as 'Slain by <cause>'", "[high-score-system]")
{
    // A typical enemy name yields the "Slain by" phrasing (Req 1.6).
    REQUIRE(formatOutcome("Ork Nob") == "Slain by Ork Nob");
}

TEST_CASE("Outcome: empty cause formats as 'Slain'", "[high-score-system]")
{
    // An unavailable cause records the bare "Slain" outcome (Req 1.7).
    REQUIRE(formatOutcome("") == "Slain");
}

TEST_CASE("Outcome: cause with spaces and unicode is preserved verbatim", "[high-score-system]")
{
    // Multi-word names keep their internal spacing (Req 1.6).
    REQUIRE(formatOutcome("Chaos Space Marine") == "Slain by Chaos Space Marine");

    // Multibyte UTF-8 content is appended intact — no truncation or re-encoding.
    REQUIRE(formatOutcome(u8"Ørk Nöb") == std::string(u8"Slain by Ørk Nöb"));

    // A single-space cause is non-empty, so it takes the "Slain by" branch
    // rather than being treated as an unknown cause (Req 1.6 vs 1.7 boundary).
    REQUIRE(formatOutcome(" ") == "Slain by  ");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 2.1 — Property test for ranking total order
// Feature: high-score-system, Property 1: Ranking is a deterministic total order
//                                          with XP primary and depth tiebreak
// **Validates: Requirements 2.1, 2.2, 2.3, 2.4, 5.2**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 1 — ranking is a deterministic total order", "[pbt][property][high-score-system]")
{
    // rc::check runs >= 100 iterations by default.
    rc::check("scoreCompare: XP primary, depth tiebreak, antisymmetric, total order", []() {
        const ScoreEntry a = genScoreEntry();
        const ScoreEntry b = genScoreEntry();
        const ScoreEntry c = genScoreEntry();

        const int ab = scoreCompare(a, b);
        const int ba = scoreCompare(b, a);

        // (1) Reflexivity: an entry compares equal to itself.
        RC_ASSERT(scoreCompare(a, a) == 0);

        // (2) Antisymmetry of sign: compare(a,b) and compare(b,a) have opposite signs
        //     (and are simultaneously zero only when they rank equal).
        RC_ASSERT((ab > 0) == (ba < 0));
        RC_ASSERT((ab < 0) == (ba > 0));
        RC_ASSERT((ab == 0) == (ba == 0));

        // (3) Totality: exactly one of a>b, b>a, a==b holds.
        const int ranksAbove = (ab > 0) ? 1 : 0;
        const int ranksBelow = (ab < 0) ? 1 : 0;
        const int ranksEqual = (ab == 0) ? 1 : 0;
        RC_ASSERT(ranksAbove + ranksBelow + ranksEqual == 1);

        // (4) XP is the primary key: greater totalXp ranks strictly higher.
        if (a.totalXp != b.totalXp) {
            if (a.totalXp > b.totalXp) {
                RC_ASSERT(ab > 0);
            } else {
                RC_ASSERT(ab < 0);
            }
        } else if (a.deepestLevel != b.deepestLevel) {
            // (5) Depth tiebreak: equal XP -> greater deepestLevel ranks higher.
            if (a.deepestLevel > b.deepestLevel) {
                RC_ASSERT(ab > 0);
            } else {
                RC_ASSERT(ab < 0);
            }
        }

        // (6) scoreRanksHigher is defined as scoreCompare > 0.
        RC_ASSERT(scoreRanksHigher(a, b) == (ab > 0));

        // (7) Transitivity of the total order: if a>=b and b>=c then a>=c.
        const int bc = scoreCompare(b, c);
        const int ac = scoreCompare(a, c);
        if (ab >= 0 && bc >= 0) {
            RC_ASSERT(ac >= 0);
        }
        if (ab <= 0 && bc <= 0) {
            RC_ASSERT(ac <= 0);
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 2.2 — Unit tests for ranking tiebreak examples
// **Validates: Requirements 2.2, 2.3, 2.4**
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Ranking: greater totalXp ranks higher", "[high-score-system]")
{
    ScoreEntry low;  low.totalXp = 100;  low.deepestLevel = 9;
    ScoreEntry high; high.totalXp = 500; high.deepestLevel = 1;

    // Higher XP wins even with a shallower depth (Req 2.2).
    REQUIRE(scoreCompare(high, low) > 0);
    REQUIRE(scoreCompare(low, high) < 0);
    REQUIRE(scoreRanksHigher(high, low));
    REQUIRE_FALSE(scoreRanksHigher(low, high));
}

TEST_CASE("Ranking: equal XP -> greater deepestLevel ranks higher", "[high-score-system]")
{
    ScoreEntry shallow; shallow.totalXp = 300; shallow.deepestLevel = 2;
    ScoreEntry deep;    deep.totalXp    = 300; deep.deepestLevel    = 7;

    // Equal XP, so the deeper run wins the tiebreak (Req 2.3).
    REQUIRE(scoreCompare(deep, shallow) > 0);
    REQUIRE(scoreCompare(shallow, deep) < 0);
}

TEST_CASE("Ranking: equal XP and level -> deterministic lexicographic fallback", "[high-score-system]")
{
    // Same ranking keys; differ only on a fallback field (characterName).
    ScoreEntry a; a.totalXp = 200; a.deepestLevel = 3; a.characterName = "Aaron";
    ScoreEntry b; b.totalXp = 200; b.deepestLevel = 3; b.characterName = "Zelda";

    const int ab = scoreCompare(a, b);

    // The order must be deterministic and non-zero (a strict total order,
    // Req 2.4) so sorting is stable and repeatable.
    REQUIRE(ab != 0);
    // And it must be antisymmetric.
    REQUIRE(scoreCompare(b, a) == -ab);
    // Repeated comparison yields the same result (determinism).
    REQUIRE(scoreCompare(a, b) == ab);
}

TEST_CASE("Ranking: identical entries compare equal", "[high-score-system]")
{
    ScoreEntry a;
    a.characterName = "Same";
    a.totalXp = 42;
    a.deepestLevel = 5;
    a.outcome = "Slain";
    a.date = "2024-01-01 00:00";

    ScoreEntry b = a;
    REQUIRE(scoreCompare(a, b) == 0);
    REQUIRE(scoreCompare(b, a) == 0);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 3.1 — Property test for leaderboard insertion invariant
// Feature: high-score-system, Property 2: Leaderboard insertion preserves the
//   sorted, capacity-bounded invariant and reports placement correctly
// **Validates: Requirements 3.1, 3.2, 3.3, 3.5**
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// True iff the board's entries are sorted descending by scoreCompare.
bool isSortedDescending(const std::vector<ScoreEntry>& entries)
{
    for (size_t i = 1; i < entries.size(); ++i) {
        if (scoreCompare(entries[i - 1], entries[i]) < 0) {
            return false;
        }
    }
    return true;
}

// True iff some board entry is field-equal to `entry`.
bool boardContains(const Leaderboard& board, const ScoreEntry& entry)
{
    for (const auto& e : board.entries()) {
        if (entriesFieldEqual(e, entry)) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST_CASE("PBT: Property 2 — leaderboard insertion invariant", "[pbt][property][high-score-system]")
{
    rc::check("insert keeps board sorted, bounded, and reports placement correctly", []() {
        // Small capacity so overflow paths are exercised often.
        const int capacity = *rc::gen::inRange(1, 10);

        // Build a pre-filled, sorted board by inserting random entries. Because
        // insert() is the API under test, we construct the "pre-sorted board"
        // through it; the board's invariant is asserted after the focus insert.
        const int preCount = *rc::gen::inRange(0, 15);
        Leaderboard board(capacity);
        for (int i = 0; i < preCount; ++i) {
            board.insert(genScoreEntry());
        }

        // Precondition sanity: the pre-state must itself be sorted & bounded.
        RC_ASSERT(isSortedDescending(board.entries()));
        RC_ASSERT(board.size() <= board.capacity());

        const int sizeBefore = board.size();
        const bool wasBelowCapacity = sizeBefore < capacity;

        // The lowest-ranked retained entry before insertion (if any) — used to
        // check overflow eviction of exactly the lowest element.
        const bool hadEntries = sizeBefore > 0;
        ScoreEntry lowestBefore;
        if (hadEntries) {
            lowestBefore = board.entries().back();
        }

        const ScoreEntry entry = genScoreEntry();
        const bool earned = board.insert(entry);

        // (a) Entries remain sorted descending (Req 3.1).
        RC_ASSERT(isSortedDescending(board.entries()));

        // (b) Size never exceeds capacity (Req 3.3).
        RC_ASSERT(board.size() <= capacity);

        // (e) Return value == "entry present after truncation" (Req 3.5).
        RC_ASSERT(earned == boardContains(board, entry));

        if (wasBelowCapacity) {
            // (c) Below capacity: the entry is retained and size grows by one,
            //     and no prior entry is lost (Req 3.2).
            RC_ASSERT(earned);
            RC_ASSERT(board.size() == sizeBefore + 1);
        } else {
            // Board was full before insertion. Size stays at capacity (Req 3.3).
            RC_ASSERT(board.size() == capacity);

            // (d) If it earned a place, exactly the previously-lowest entry was
            //     evicted; if it did not, the board is unchanged and the entry
            //     ranked at or below the previous lowest (Req 3.3, 3.4/3.5).
            if (!earned) {
                // A full board that rejects the entry: the entry does not
                // outrank the lowest retained entry.
                RC_ASSERT(scoreCompare(entry, lowestBefore) <= 0);
            }
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 3.2 — Unit tests for insertion boundary and capacity examples
// **Validates: Requirements 3.2, 3.3, 3.4, 3.5, 3.6**
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Build a ScoreEntry whose ranking key is driven purely by totalXp.
ScoreEntry entryWithXp(int xp, const std::string& name = "")
{
    ScoreEntry e;
    e.totalXp = xp;
    e.deepestLevel = 1;
    e.characterName = name;
    return e;
}

} // namespace

TEST_CASE("Insertion: below-capacity insert retains all entries", "[high-score-system]")
{
    Leaderboard board(5);
    REQUIRE(board.insert(entryWithXp(100)));
    REQUIRE(board.insert(entryWithXp(300)));
    REQUIRE(board.insert(entryWithXp(200)));

    // All three retained (Req 3.2) and sorted descending (Req 3.1).
    REQUIRE(board.size() == 3);
    REQUIRE(board.entries()[0].totalXp == 300);
    REQUIRE(board.entries()[1].totalXp == 200);
    REQUIRE(board.entries()[2].totalXp == 100);
}

TEST_CASE("Insertion: full board evicts the lowest-ranked entry", "[high-score-system]")
{
    Leaderboard board(3);
    board.insert(entryWithXp(100));
    board.insert(entryWithXp(200));
    board.insert(entryWithXp(300));
    REQUIRE(board.size() == 3);

    // Insert an entry that outranks the current lowest (100).
    const bool earned = board.insert(entryWithXp(250));

    // Earned a place; size stays at capacity; lowest (100) evicted (Req 3.3, 3.5).
    REQUIRE(earned);
    REQUIRE(board.size() == 3);
    REQUIRE(board.entries()[0].totalXp == 300);
    REQUIRE(board.entries()[1].totalXp == 250);
    REQUIRE(board.entries()[2].totalXp == 200);

    // The evicted entry (100) is no longer present.
    for (const auto& e : board.entries()) {
        REQUIRE(e.totalXp != 100);
    }
}

TEST_CASE("Insertion: below-all insert into full board is a no-op returning false", "[high-score-system]")
{
    Leaderboard board(3);
    board.insert(entryWithXp(100));
    board.insert(entryWithXp(200));
    board.insert(entryWithXp(300));

    // Entry ranks below every retained entry on a full board (Req 3.4).
    const bool earned = board.insert(entryWithXp(50));

    REQUIRE_FALSE(earned);
    REQUIRE(board.size() == 3);
    REQUIRE(board.entries()[0].totalXp == 300);
    REQUIRE(board.entries()[1].totalXp == 200);
    REQUIRE(board.entries()[2].totalXp == 100);

    // The rejected entry (50) is not present.
    for (const auto& e : board.entries()) {
        REQUIRE(e.totalXp != 50);
    }
}

TEST_CASE("Insertion: default capacity is 100", "[high-score-system]")
{
    Leaderboard board; // default capacity
    REQUIRE(board.capacity() == 100);
    REQUIRE(Leaderboard::DEFAULT_CAPACITY == 100);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 4.1 — Property test for serialization round-trip
// Feature: high-score-system, Property 3: Serialization round-trip preserves the
//   leaderboard exactly
// **Validates: Requirements 6.7, 8.1, 8.2, 8.3**
//
// TCODZip has no in-memory round-trip API, so (as in the project's other
// serialization tests) the archive is flushed to a temp file and reloaded.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT: Property 3 — serialization round-trip preserves the leaderboard", "[pbt][property][high-score-system]")
{
    rc::check("serialize then deserialize yields identical count, fields, and order", []() {
        // Build a random leaderboard by inserting random entries.
        const int capacity = *rc::gen::inRange(1, 20);
        const int count = *rc::gen::inRange(0, 30);
        Leaderboard original(capacity);
        for (int i = 0; i < count; ++i) {
            original.insert(genScoreEntry());
        }

        const char* tempFile = "__test_highscore_roundtrip.dat";

        // Serialize to a TCODZip archive and flush to disk.
        {
            TCODZip zip;
            serializeLeaderboard(original, zip);
            zip.saveToFile(tempFile);
        }

        // Deserialize from a fresh archive.
        Leaderboard loaded(capacity);
        {
            TCODZip zip;
            zip.loadFromFile(tempFile);
            loaded = deserializeLeaderboard(zip);
        }

        std::remove(tempFile);

        // Identical entry count (Req 8.1).
        RC_ASSERT(loaded.size() == original.size());

        // Identical field values in identical order (Req 8.1, 8.2).
        const auto& orig = original.entries();
        const auto& got = loaded.entries();
        RC_ASSERT(orig.size() == got.size());
        for (size_t i = 0; i < orig.size(); ++i) {
            RC_ASSERT(entriesFieldEqual(orig[i], got[i]));
        }

        // Descending order preserved across the round trip (Req 8.3).
        RC_ASSERT(isSortedDescending(got));
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 4.2 — Property test for deserialization robustness
// Feature: high-score-system, Property 4: Deserialization of invalid data yields
//   an empty leaderboard without failing
// **Validates: Requirements 7.3**
//
// For any byte buffer that is NOT a valid serialized leaderboard (arbitrary,
// corrupt, foreign, or empty archive content), deserializeLeaderboard must
// return an empty Leaderboard and never throw or abort, so the game can
// continue. The valid format begins with the sentinel 0x48534452; anything
// that does not match that sentinel is invalid input.
//
// TCODZip has no in-memory-only API, so (as with the round-trip test) each
// crafted archive is flushed to a temp file and reloaded before it is fed to
// deserializeLeaderboard.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// The valid format sentinel (design: "HSDR"). Any archive whose first int does
// not equal this value is, by definition, invalid leaderboard content.
constexpr int HIGHSCORE_SENTINEL = 0x48534452;

} // namespace

TEST_CASE("PBT: Property 4 — deserialization of invalid data yields an empty leaderboard", "[pbt][property][high-score-system]")
{
    rc::check("arbitrary/corrupt/foreign/empty archive content deserializes to an empty board without throwing", []() {
        const char* tempFile = "__test_highscore_invalid.dat";

        // Choose one of several "invalid archive" shapes each iteration so we
        // exercise empty, foreign, and wrong-sentinel content broadly.
        //   0 = empty archive (nothing written)
        //   1 = wrong/foreign sentinel followed by arbitrary ints & strings
        //   2 = arbitrary leading int (that happens NOT to be the sentinel)
        //       followed by random junk
        //   3 = a valid-looking sentinel but a corrupt/garbage body
        //       (bogus entry count, missing fields)
        const int shape = *rc::gen::inRange(0, 3);

        {
            TCODZip zip;

            switch (shape) {
                case 0:
                    // Empty archive — write nothing at all.
                    break;

                case 1: {
                    // Foreign sentinel: pick a value guaranteed to differ from
                    // the real HIGHSCORE_SENTINEL, then append random content.
                    int foreign = *rc::gen::inRange(0, 1000000);
                    if (foreign == HIGHSCORE_SENTINEL) foreign ^= 0x1; // ensure mismatch
                    zip.putInt(foreign);
                    const int junkInts = *rc::gen::inRange(0, 8);
                    for (int i = 0; i < junkInts; ++i) {
                        zip.putInt(*rc::gen::inRange(-100000, 100000));
                    }
                    const int junkStrings = *rc::gen::inRange(0, 5);
                    for (int i = 0; i < junkStrings; ++i) {
                        zip.putString((*rc::gen::string(0, 16)).c_str());
                    }
                    break;
                }

                case 2: {
                    // A random leading int that is not the sentinel, followed by
                    // arbitrary junk of mixed types.
                    int leading = *rc::gen::inRange(-1000000, 1000000);
                    if (leading == HIGHSCORE_SENTINEL) leading ^= 0x1;
                    zip.putInt(leading);
                    const int junk = *rc::gen::inRange(1, 10);
                    for (int i = 0; i < junk; ++i) {
                        if (*rc::gen::arbitrary_bool()) {
                            zip.putInt(*rc::gen::inRange(-100000, 100000));
                        } else {
                            zip.putString((*rc::gen::string(0, 12)).c_str());
                        }
                    }
                    break;
                }

                case 3: {
                    // Correct sentinel but a corrupt body: a bogus entry count
                    // and truncated / mismatched fields. deserialize must not
                    // trust the count blindly in a way that throws/aborts; on any
                    // parse failure it yields an empty board (Req 7.3).
                    zip.putInt(HIGHSCORE_SENTINEL);
                    zip.putInt(*rc::gen::inRange(1, 5)); // format version (arbitrary)
                    // Claim more entries than we actually write.
                    zip.putInt(*rc::gen::inRange(1, 1000));
                    // Write only a partial, mismatched smattering of fields.
                    const int partial = *rc::gen::inRange(0, 4);
                    for (int i = 0; i < partial; ++i) {
                        zip.putString((*rc::gen::string(0, 8)).c_str());
                    }
                    break;
                }
            }

            zip.saveToFile(tempFile);
        }

        // Feed the crafted archive to deserializeLeaderboard. This must not
        // throw or abort, and must return an empty leaderboard.
        Leaderboard result(1);
        bool threw = false;
        try {
            TCODZip zip;
            zip.loadFromFile(tempFile);
            result = deserializeLeaderboard(zip);
        } catch (...) {
            threw = true;
        }

        std::remove(tempFile);

        // No throw/abort escaped (Req 7.3).
        RC_ASSERT(!threw);

        // An empty leaderboard is returned for invalid content (Req 7.3).
        RC_ASSERT(result.empty());
        RC_ASSERT(result.size() == 0);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 4.3 — Unit tests for serialization edge cases
// **Validates: Requirements 8.1, 8.2, 7.3**
//
// - Empty board round-trips to empty.
// - Unicode / empty-string fields survive a round trip.
// - A sentinel-mismatch archive deserializes to an empty board.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Serialization: empty board round-trips to empty", "[high-score-system]")
{
    const char* tempFile = "__test_highscore_empty_roundtrip.dat";

    Leaderboard original(10); // no entries inserted
    REQUIRE(original.empty());

    {
        TCODZip zip;
        serializeLeaderboard(original, zip);
        zip.saveToFile(tempFile);
    }

    Leaderboard loaded(10);
    {
        TCODZip zip;
        zip.loadFromFile(tempFile);
        loaded = deserializeLeaderboard(zip);
    }

    std::remove(tempFile);

    // An empty board must round-trip to an empty board (Req 8.1).
    REQUIRE(loaded.empty());
    REQUIRE(loaded.size() == 0);
}

TEST_CASE("Serialization: unicode and empty-string fields survive a round trip", "[high-score-system]")
{
    const char* tempFile = "__test_highscore_unicode_roundtrip.dat";

    // One entry with UTF-8 multibyte content, one with all-empty string fields
    // (Req 8.2 — every field must survive, including edge-case strings).
    ScoreEntry unicodeEntry;
    unicodeEntry.characterName = u8"Ürsün-Kâhïn";       // multibyte UTF-8
    unicodeEntry.careerName    = u8"Rögue Träder";
    unicodeEntry.homeworldName = u8"武神の世界";          // CJK characters
    unicodeEntry.rankTitle     = u8"Séneschal ★";
    unicodeEntry.totalXp       = 4200;
    unicodeEntry.deepestLevel  = 12;
    unicodeEntry.outcome       = u8"Slain by Ørk Nöb";
    unicodeEntry.date          = u8"2024-07-01 13:37";

    ScoreEntry emptyFields;                              // all strings empty
    emptyFields.characterName = "";
    emptyFields.careerName    = "";
    emptyFields.homeworldName = "";
    emptyFields.rankTitle     = "";
    emptyFields.totalXp       = 0;
    emptyFields.deepestLevel  = 1;
    emptyFields.outcome       = "";
    emptyFields.date          = "";

    Leaderboard original(10);
    original.insert(unicodeEntry);
    original.insert(emptyFields);

    {
        TCODZip zip;
        serializeLeaderboard(original, zip);
        zip.saveToFile(tempFile);
    }

    Leaderboard loaded(10);
    {
        TCODZip zip;
        zip.loadFromFile(tempFile);
        loaded = deserializeLeaderboard(zip);
    }

    std::remove(tempFile);

    // Same number of entries survived (Req 8.1).
    REQUIRE(loaded.size() == original.size());
    REQUIRE(loaded.entries().size() == original.entries().size());

    // Every field of every entry survived intact, in order (Req 8.1, 8.2).
    for (size_t i = 0; i < original.entries().size(); ++i) {
        REQUIRE(entriesFieldEqual(loaded.entries()[i], original.entries()[i]));
    }
}

TEST_CASE("Serialization: sentinel-mismatch archive deserializes to empty board", "[high-score-system]")
{
    const char* tempFile = "__test_highscore_bad_sentinel.dat";

    // Write an archive whose leading int is NOT the high-score sentinel,
    // followed by some plausible-looking but foreign data (Req 7.3).
    {
        TCODZip zip;
        zip.putInt(0xDEADBEEF);          // wrong sentinel
        zip.putInt(3);                   // looks like an entry count
        zip.putString("not a real entry");
        zip.putInt(999);
        zip.saveToFile(tempFile);
    }

    Leaderboard loaded(10);
    bool threw = false;
    try {
        TCODZip zip;
        zip.loadFromFile(tempFile);
        loaded = deserializeLeaderboard(zip);
    } catch (...) {
        threw = true;
    }

    std::remove(tempFile);

    // Must not throw, and must yield an empty board on sentinel mismatch (Req 7.3).
    REQUIRE_FALSE(threw);
    REQUIRE(loaded.empty());
    REQUIRE(loaded.size() == 0);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 7.3 — Unit test: the main menu contains a "High Scores" item
// **Validates: Requirements 9.1**
//
// Isolation decision (per test-isolation.md): the Menu can be built and
// populated entirely in isolation. Menu::clear() and Menu::addItem() are pure
// list operations that never touch engine.gui/map/player or SDL/libtcod — only
// Menu::pick() enters a blocking render loop, which we do NOT call here. Since
// Menu::items is protected and Menu exposes no public accessor, we use a tiny
// test-only subclass to inspect the populated items.
//
// This test verifies the mechanism Requirement 9.1 depends on: that the Menu
// supports a HIGH_SCORES item labelled "High Scores". The production wiring that
// inserts this item into the live main menu lives in Engine::load()
// (Source/Persistent.cpp) and is added by task 10.2. This test replicates the
// menu-building step so it exercises the Menu type directly without the Engine.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Test-only subclass that exposes the protected item list for inspection.
class InspectableMenu : public Menu {
public:
    // Returns true iff an item with the given code and label is present.
    bool containsItem(MenuItemCode code, std::string_view label) const {
        for (const auto& item : items) {
            if (item->code == code && item->label == label) {
                return true;
            }
        }
        return false;
    }

    // Returns true iff any item carries the given code (label-agnostic).
    bool containsCode(MenuItemCode code) const {
        for (const auto& item : items) {
            if (item->code == code) {
                return true;
            }
        }
        return false;
    }
};

} // namespace

TEST_CASE("Menu contains a High Scores item in MAIN display mode", "[high-score-system]")
{
    InspectableMenu menu;

    // Populate the menu exactly as the main-menu build path will (task 10.2),
    // with the "High Scores" entry sitting between "Continue" and "Help"
    // (design: Engine integration point 4).
    menu.clear();
    menu.addItem(Menu::MenuItemCode::NEW_GAME, "New Game");
    menu.addItem(Menu::MenuItemCode::CONTINUE, "Continue");
    menu.addItem(Menu::MenuItemCode::HIGH_SCORES, "High Scores");
    menu.addItem(Menu::MenuItemCode::HELP, "Help");
    menu.addItem(Menu::MenuItemCode::EXIT, "Exit");

    // The menu must present a "High Scores" entry (Req 9.1).
    REQUIRE(menu.containsCode(Menu::MenuItemCode::HIGH_SCORES));
    REQUIRE(menu.containsItem(Menu::MenuItemCode::HIGH_SCORES, "High Scores"));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 10.3 — Example tests for the empty-board view and paginator wiring
// **Validates: Requirements 9.6, 9.8**
//
// Feature: high-score-system
//
// The high-scores view (Engine::beginHighScores/renderHighScores) needs the live
// Engine + GUI to render, so it cannot run in the engine-free test binary
// (per test-isolation.md). These example tests therefore exercise the underlying
// LOGIC the view is built on:
//
//   - Empty board  -> renderHighScores() shows the "no runs recorded" message.
//     The condition that view uses is the pure Leaderboard state
//     (empty()/size()) plus a Paginator sized with totalItems == 0. We assert on
//     those pure pieces (Req 9.8).
//   - Page indicator -> renderLeaderboard() shows Paginator::indicator() only when
//     totalPages() > 1. We construct a Paginator exactly as beginHighScores()
//     sizes it (pageSize from content height, totalItems == board size) and assert
//     the indicator string and totalPages() threshold (Req 9.6).
// ═══════════════════════════════════════════════════════════════════════════════

#include "Paginator.hpp"

namespace {

// Mirror Engine::beginHighScores()'s paginator sizing (Source/Engine.cpp):
//   contentHeight = screenHeight - 5
//   pageSize      = max(1, contentHeight / HS_ROWS_PER_ENTRY)
// Kept local to the test so it validates the same arithmetic the view relies on
// without needing the Engine. HS_ROWS_PER_ENTRY is 3 (two lines + spacer).
constexpr int HS_ROWS_PER_ENTRY = 3;

Paginator makeHighScoresPaginator(int totalItems, int screenHeight)
{
    Paginator pag;
    pag.currentPage = 0;
    pag.totalItems = totalItems;
    const int contentHeight = screenHeight - 5;
    pag.pageSize = std::max(1, contentHeight / HS_ROWS_PER_ENTRY);
    return pag;
}

} // namespace

TEST_CASE("HighScores view: empty board is detectable via pure Leaderboard state (no runs recorded)", "[high-score-system]")
{
    // renderHighScores() shows the "no runs recorded yet." message when the
    // board is empty. That decision is driven entirely by the pure Leaderboard
    // state, which is what we assert here (Req 9.8).
    Leaderboard board; // default capacity, no entries inserted
    REQUIRE(board.empty());
    REQUIRE(board.size() == 0);

    // A paginator sized exactly as beginHighScores() would for an empty board:
    // zero items means a single (empty) page with nothing to display.
    const Paginator pag = makeHighScoresPaginator(board.size(), 50);
    REQUIRE(pag.totalItems == 0);
    REQUIRE(pag.totalPages() == 1);   // clamped minimum of 1 page
    REQUIRE(pag.displayCount() == 0); // nothing to draw -> empty-state message
}

TEST_CASE("HighScores view: single page hides the indicator (totalPages() == 1)", "[high-score-system]")
{
    // When every entry fits on one page, renderLeaderboard() must NOT show the
    // "Page x/y" indicator. That is gated on totalPages() > 1 (Req 9.6).
    // Build a board whose size is <= pageSize.
    const int screenHeight = 50; // pageSize = max(1, (50-5)/3) = 15
    const Paginator sizingProbe = makeHighScoresPaginator(0, screenHeight);
    const int pageSize = sizingProbe.pageSize;
    REQUIRE(pageSize == 15);

    Leaderboard board(200);
    for (int i = 0; i < pageSize; ++i) {           // exactly one full page
        ScoreEntry e = entryWithXp(1000 - i, "runner");
        board.insert(e);
    }
    REQUIRE(board.size() == pageSize);

    const Paginator pag = makeHighScoresPaginator(board.size(), screenHeight);
    REQUIRE(pag.totalItems <= pag.pageSize);
    REQUIRE(pag.totalPages() == 1); // single page -> indicator hidden (Req 9.6)
}

TEST_CASE("HighScores view: multiple pages show the indicator with the expected 'Page x/y' string", "[high-score-system]")
{
    // When the board overflows one page, renderLeaderboard() shows the paginator
    // indicator. Assert totalPages() > 1 and the exact indicator strings the
    // shared Paginator produces (Req 9.6).
    const int screenHeight = 50; // pageSize = 15
    const Paginator sizingProbe = makeHighScoresPaginator(0, screenHeight);
    const int pageSize = sizingProbe.pageSize;
    REQUIRE(pageSize == 15);

    // 2.5 pages worth of entries -> 3 pages total.
    const int totalItems = pageSize * 2 + pageSize / 2; // 37
    Leaderboard board(200);
    for (int i = 0; i < totalItems; ++i) {
        board.insert(entryWithXp(100000 - i, "runner"));
    }
    REQUIRE(board.size() == totalItems);

    Paginator pag = makeHighScoresPaginator(board.size(), screenHeight);

    // More than one page -> the indicator is shown (Req 9.6).
    REQUIRE(pag.totalItems > pag.pageSize);
    REQUIRE(pag.totalPages() == 3);

    // The indicator reflects the current page in "Page x/y" form, 1-based.
    REQUIRE(pag.indicator() == "Page 1/3");
    pag.nextPage();
    REQUIRE(pag.indicator() == "Page 2/3");
    pag.nextPage();
    REQUIRE(pag.indicator() == "Page 3/3");
}

TEST_CASE("HighScores view: indicator threshold flips exactly at pageSize + 1 entries", "[high-score-system]")
{
    // Boundary check for Req 9.6: totalPages() is 1 while items <= pageSize and
    // becomes > 1 the moment items exceed pageSize.
    const int screenHeight = 50; // pageSize = 15
    const int pageSize = makeHighScoresPaginator(0, screenHeight).pageSize;

    const Paginator exactlyFull = makeHighScoresPaginator(pageSize, screenHeight);
    REQUIRE(exactlyFull.totalPages() == 1); // <= pageSize -> indicator hidden

    const Paginator oneOver = makeHighScoresPaginator(pageSize + 1, screenHeight);
    REQUIRE(oneOver.totalPages() == 2);     // > pageSize -> indicator shown
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 11.2 — Example test: the earned-entry page is selected on the death screen
// **Validates: Requirements 10.5**
//
// Feature: high-score-system
//
// Engine::renderDefeat() (Source/Engine.cpp) scrolls the newly recorded entry
// into view by selecting its page with the EXACT formula:
//
//     highScoresPaginator_.currentPage = *lastEntryIndex_ / highScoresPaginator_.pageSize;
//
// The paginator is engine-independent, so we replicate that formula here and
// assert the earned entry falls on the selected page:
//     startIndex() <= lastEntryIndex < endIndex()
// across several indices spanning multiple pages (first entry, middle pages,
// last entry). This validates the real page-selection behavior renderDefeat
// relies on without initializing the Engine (per test-isolation.md).
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Apply the EXACT page-selection formula from Engine::renderDefeat():
//   currentPage = lastEntryIndex / pageSize
// then confirm the entry lies within the selected page's [startIndex, endIndex).
void assertEntryOnSelectedPage(int totalItems, int pageSize, int lastEntryIndex)
{
    REQUIRE(pageSize > 0);
    REQUIRE(lastEntryIndex >= 0);
    REQUIRE(lastEntryIndex < totalItems);

    Paginator pag;
    pag.totalItems = totalItems;
    pag.pageSize = pageSize;
    // The formula renderDefeat() uses verbatim (Req 10.5).
    pag.currentPage = lastEntryIndex / pag.pageSize;

    // The earned entry must fall on the current page.
    REQUIRE(pag.startIndex() <= lastEntryIndex);
    REQUIRE(lastEntryIndex < pag.endIndex());
}

} // namespace

TEST_CASE("Death screen: earned entry on the first page is selected (index 0)", "[high-score-system]")
{
    // First entry always lands on page 0 (Req 10.5).
    assertEntryOnSelectedPage(/*totalItems*/ 100, /*pageSize*/ 15, /*index*/ 0);
}

TEST_CASE("Death screen: earned entry on a middle page is selected", "[high-score-system]")
{
    // Index 20 with pageSize 15 -> page 1 (entries 15..29). 20 is on that page.
    assertEntryOnSelectedPage(/*totalItems*/ 100, /*pageSize*/ 15, /*index*/ 20);

    // Index 44 with pageSize 15 -> page 2 (entries 30..44).
    assertEntryOnSelectedPage(/*totalItems*/ 100, /*pageSize*/ 15, /*index*/ 44);
}

TEST_CASE("Death screen: earned entry as the last board entry is selected", "[high-score-system]")
{
    // The last entry of a full 100-entry board (index 99) must be scrolled into
    // view on its page (Req 10.5).
    assertEntryOnSelectedPage(/*totalItems*/ 100, /*pageSize*/ 15, /*index*/ 99);

    // A board whose size is an exact multiple of pageSize: last index on the
    // final full page.
    assertEntryOnSelectedPage(/*totalItems*/ 45, /*pageSize*/ 15, /*index*/ 44);
}

TEST_CASE("Death screen: page selection places every index on its own page across the board", "[high-score-system]")
{
    // Sweep every index of a multi-page board and confirm the renderDefeat
    // formula always selects a page that contains that index (Req 10.5). This
    // covers first page, all middle pages, and the last entry in one pass.
    const int totalItems = 100;
    const int pageSize = 15; // -> 7 pages (0..6), last page holds 90..99
    for (int index = 0; index < totalItems; ++index) {
        assertEntryOnSelectedPage(totalItems, pageSize, index);
    }
}

TEST_CASE("Death screen: page selection works with a single-entry-per-page paginator", "[high-score-system]")
{
    // Degenerate pageSize == 1: each entry is its own page, so currentPage
    // equals the index and startIndex()==index, endIndex()==index+1 (Req 10.5).
    const int totalItems = 8;
    for (int index = 0; index < totalItems; ++index) {
        assertEntryOnSelectedPage(totalItems, /*pageSize*/ 1, index);

        Paginator pag;
        pag.totalItems = totalItems;
        pag.pageSize = 1;
        pag.currentPage = index / pag.pageSize;
        REQUIRE(pag.currentPage == index);
        REQUIRE(pag.startIndex() == index);
        REQUIRE(pag.endIndex() == index + 1);
    }
}
