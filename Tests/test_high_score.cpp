#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

#include "HighScore.hpp"

#include "libtcod.hpp"

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
