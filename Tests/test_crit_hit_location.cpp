#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.hpp"

#include "HitLocation.hpp"
#include "InjuryTracker.hpp"

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: crit-hit-location-display — Bug-Condition Exploration Test (Task 1)
//
// This is the observation-first exploration test for the "Critical Injuries always
// shows Body" bug. It has TWO parts with DIFFERENT expected outcomes on UNFIXED code:
//
//   PART A (pure seam, HitLocationTable::resolve): EXPECTED TO PASS (Outcome (i)).
//     A passing Part A CONFIRMS resolve is correct across the full 1..100 domain and
//     that the "always Body" symptom is a sampling artifact (Body = 40% of the table)
//     plus the unrecorded fatal crit surfaced by Part B. Do NOT change resolve if it
//     passes. Only a counterexample here (Outcome (ii)) would drive the conditional
//     resolve fix in task 4.
//
//   PART B (near-pure InjuryTracker seam): DEMONSTRATES THE GAP on unfixed code.
//     There is no recordFatalCrit API yet, and applyCrit returns false WITHOUT pushing
//     a record when the new cumulative magnitude reaches FATAL_MAGNITUDE. So a fatal
//     crit leaves getRecords() empty — the fatal blow's location is never recorded.
//     This assertion PASSES on unfixed code and documents the real defect the Part B
//     fix (task 3) will close.
//
// Engine isolation (test-isolation.md): all InjuryTracker calls pass owner == nullptr,
// so applyDebuff / applyCrit never touch owner->characteristics or any engine global.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// ─── Test-side reference RT hit-location table (design "expectedLocation(X)") ───
// Reverse the digits of X (roll 100 -> reversed 00); then bucket:
//   reversed == 0 -> LEFT_LEG (the "00" / 81-00 bucket)
//   <= 10 -> HEAD        (01-10)
//   <= 20 -> RIGHT_ARM   (11-20)
//   <= 30 -> LEFT_ARM    (21-30)
//   <= 70 -> BODY        (31-70)
//   <= 80 -> RIGHT_LEG   (71-80)
//   else  -> LEFT_LEG    (81-99)
HitLocation expectedLocation(int X)
{
    int reversed;
    if (X == 100) {
        reversed = 0;
    } else {
        reversed = (X % 10) * 10 + (X / 10);
    }

    if (reversed == 0)       return HitLocation::LEFT_LEG;
    else if (reversed <= 10) return HitLocation::HEAD;
    else if (reversed <= 20) return HitLocation::RIGHT_ARM;
    else if (reversed <= 30) return HitLocation::LEFT_ARM;
    else if (reversed <= 70) return HitLocation::BODY;
    else if (reversed <= 80) return HitLocation::RIGHT_LEG;
    else                     return HitLocation::LEFT_LEG;
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// PART A — Pure seam exploration: HitLocationTable::resolve over the full domain
// Property 1 (Bug Condition): for all X in [1,100], resolve(X) == expectedLocation(X).
// **Validates: Requirements 1.1, 1.2, 4.1**
// EXPECTED: PASS on unfixed code (Outcome (i)).
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("PBT Property 1: resolve matches the RT hit-location table over 1..100",
          "[pbt][property][crit-hit-location]")
{
    rc::prop("resolve(X) == expectedLocation(X) for X in [1,100]", []() {
        // INCLUSIVE bounds: rc::gen::inRange(1, 100) yields [1, 100].
        const int roll = *rc::gen::inRange(1, 100);
        RC_ASSERT(HitLocationTable::resolve(roll) == expectedLocation(roll));
    });
}

TEST_CASE("Part A: exhaustive sweep resolve(X) == expectedLocation(X) for X in 1..100",
          "[crit-hit-location]")
{
    for (int X = 1; X <= 100; ++X) {
        INFO("roll X = " << X
             << " resolve=" << HitLocationTable::name(HitLocationTable::resolve(X))
             << " expected=" << HitLocationTable::name(expectedLocation(X)));
        CHECK(HitLocationTable::resolve(X) == expectedLocation(X));
    }
}

TEST_CASE("Part A: Body share — exactly 40 of the 100 rolls resolve to BODY",
          "[crit-hit-location]")
{
    int bodyCount = 0;
    for (int X = 1; X <= 100; ++X) {
        if (HitLocationTable::resolve(X) == HitLocation::BODY) {
            ++bodyCount;
        }
    }
    CHECK(bodyCount == 40);
}

TEST_CASE("Part A: boundary rolls map to the expected buckets",
          "[crit-hit-location]")
{
    // 01 -> reversed 10 -> HEAD
    CHECK(HitLocationTable::resolve(1) == HitLocation::HEAD);
    // 10 -> reversed 01 -> HEAD
    CHECK(HitLocationTable::resolve(10) == HitLocation::HEAD);
    // 11 -> reversed 11 -> RIGHT_ARM
    CHECK(HitLocationTable::resolve(11) == HitLocation::RIGHT_ARM);
    // 20 -> reversed 02 -> HEAD
    CHECK(HitLocationTable::resolve(20) == HitLocation::HEAD);
    // 21 -> reversed 12 -> RIGHT_ARM
    CHECK(HitLocationTable::resolve(21) == HitLocation::RIGHT_ARM);
    // 30 -> reversed 03 -> HEAD
    CHECK(HitLocationTable::resolve(30) == HitLocation::HEAD);
    // 31 -> reversed 13 -> RIGHT_ARM
    CHECK(HitLocationTable::resolve(31) == HitLocation::RIGHT_ARM);
    // 70 -> reversed 07 -> HEAD
    CHECK(HitLocationTable::resolve(70) == HitLocation::HEAD);
    // 71 -> reversed 17 -> RIGHT_ARM
    CHECK(HitLocationTable::resolve(71) == HitLocation::RIGHT_ARM);
    // 80 -> reversed 08 -> HEAD
    CHECK(HitLocationTable::resolve(80) == HitLocation::HEAD);
    // 81 -> reversed 18 -> RIGHT_ARM
    CHECK(HitLocationTable::resolve(81) == HitLocation::RIGHT_ARM);
    // 100 -> reversed 00 -> LEFT_LEG (special case)
    CHECK(HitLocationTable::resolve(100) == HitLocation::LEFT_LEG);

    // Every boundary roll must also agree with the reference table.
    for (int X : { 1, 10, 11, 20, 21, 30, 31, 70, 71, 80, 81, 100 }) {
        INFO("boundary roll X = " << X);
        CHECK(HitLocationTable::resolve(X) == expectedLocation(X));
    }
}

TEST_CASE("Part A: roll-100 special case resolves to LEFT_LEG",
          "[crit-hit-location]")
{
    CHECK(HitLocationTable::resolve(100) == HitLocation::LEFT_LEG);
    CHECK(HitLocationTable::resolve(100) == expectedLocation(100));
}

// ═══════════════════════════════════════════════════════════════════════════════
// PART B — Recording-gap exploration: fatal crit is never recorded on unfixed code.
// **Validates: Requirements 1.3, 4.2**
//
// The recordFatalCrit API does NOT exist yet, so we demonstrate the gap with the
// EXISTING API only: applyCrit with a magnitude that reaches/exceeds the fatal
// threshold returns false AND does NOT push an InjuryRecord — the fatal blow's
// location is lost. owner == nullptr keeps this engine-free (applyDebuff is guarded).
// EXPECTED: PASS on unfixed code (documents the defect).
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Part B: fatal crit leaves no InjuryRecord on unfixed code (recording gap)",
          "[crit-hit-location]")
{
    InjuryTracker tracker;
    REQUIRE(tracker.getRecords().empty());
    REQUIRE(tracker.getMagnitude() == 0);

    // A single crit whose magnitude reaches the fatal threshold (FATAL_MAGNITUDE = 10).
    // The observed encounter was a fatal LEFT_LEG (leg) crit.
    const HitLocation fatalLoc = HitLocation::LEFT_LEG;
    const int fatalMagnitude = InjuryTracker::FATAL_MAGNITUDE; // 10 -> newTotal >= 10 -> fatal

    // Engine-free: null owner means applyDebuff never touches characteristics.
    bool survived = tracker.applyCrit(nullptr, fatalLoc, fatalMagnitude);

    // On unfixed code: applyCrit returns false (caller triggers death) and pushes NO record.
    CHECK(survived == false);
    CHECK(tracker.getRecords().empty());   // <-- the gap: fatal blow's location is lost
    CHECK(tracker.hasInjuries() == false);
    CHECK(tracker.activeCount() == 0);
    // Magnitude is not advanced when the fatal threshold is reached (applyCrit early-returns).
    CHECK(tracker.getMagnitude() == 0);
}

TEST_CASE("Part B: fatal crit at any non-body location is still unrecorded (gap holds broadly)",
          "[crit-hit-location]")
{
    // Demonstrate the recording gap is not location-specific: for every hit location,
    // a fatal-magnitude crit leaves no record on unfixed code.
    for (int i = 0; i < static_cast<int>(HitLocation::COUNT); ++i) {
        HitLocation loc = static_cast<HitLocation>(i);
        InjuryTracker tracker;
        bool survived = tracker.applyCrit(nullptr, loc, InjuryTracker::FATAL_MAGNITUDE);
        INFO("fatal crit at location " << HitLocationTable::name(loc));
        CHECK(survived == false);
        CHECK(tracker.getRecords().empty());
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// PRESERVATION PROPERTY TESTS (Task 2)
//
// Property 2: Preservation — baseline behaviour the fix must NOT change.
// Observation-first: these are written to capture the CURRENT (unfixed) behaviour
// so it stays locked against regression. They are all EXPECTED TO PASS on unfixed
// code. Every InjuryTracker call passes owner == nullptr, keeping the seam
// engine-free (applyDebuff is a no-op with a null owner) per test-isolation.md.
// ═══════════════════════════════════════════════════════════════════════════════

#include "libtcod.hpp"   // TCODZip for the save/load round-trip preservation test
#include <cstdio>        // std::remove
#include <vector>

// ─── Preservation 1: Genuine-Body rolls still return BODY ──────────────────────
// Property 2 (Preservation): for rolls whose reversed value lies in 31..70 (genuine
// Body), resolve(X) == HitLocation::BODY. We generate rolls across the full domain
// and only assert on those whose reversed value is genuinely a Body roll.
// **Validates: Requirements 3.1**

TEST_CASE("PBT Property 2: genuine-Body rolls still resolve to BODY",
          "[pbt][property][crit-hit-location]")
{
    rc::prop("reversed value in 31..70 => resolve(X) == BODY", []() {
        // INCLUSIVE bounds: rc::gen::inRange(1, 100) yields [1, 100].
        const int roll = *rc::gen::inRange(1, 100);

        // Compute the reversed value the same way the reference table does.
        int reversed;
        if (roll == 100) {
            reversed = 0;
        } else {
            reversed = (roll % 10) * 10 + (roll / 10);
        }

        // Only the genuine-Body band is under test here (the non-buggy inputs).
        RC_PRE(reversed >= 31 && reversed <= 70);

        RC_ASSERT(HitLocationTable::resolve(roll) == HitLocation::BODY);
        // And it must equal the independent reference table too.
        RC_ASSERT(HitLocationTable::resolve(roll) == expectedLocation(roll));
    });
}

// ─── Preservation 2: Non-fatal crit recording is unchanged ─────────────────────
// Property 2 (Preservation): a non-fatal applyCrit (magnitude below FATAL_MAGNITUDE)
// returns true, appends exactly one InjuryRecord whose location == the crit location
// and whose magnitude == the cumulative total, and getRecords().back().location == loc.
// owner == nullptr keeps applyDebuff a no-op; the record storage is what we assert.
// **Validates: Requirements 3.4**

TEST_CASE("PBT Property 2: non-fatal crit records location and cumulative magnitude",
          "[pbt][property][crit-hit-location]")
{
    rc::prop("applyCrit(non-fatal) appends {loc, cumulativeTotal} and returns true", []() {
        // A single non-fatal crit: magnitude in [1, FATAL_MAGNITUDE - 1] = [1, 9].
        const int locIdx = *rc::gen::inRange(0, static_cast<int>(HitLocation::COUNT) - 1);
        const HitLocation loc = static_cast<HitLocation>(locIdx);
        const int mag = *rc::gen::inRange(1, InjuryTracker::FATAL_MAGNITUDE - 1);

        InjuryTracker tracker;
        // Engine-free: null owner => applyDebuff never touches characteristics.
        const bool survived = tracker.applyCrit(nullptr, loc, mag);

        // Non-fatal crit survives and is recorded (Req 3.4).
        RC_ASSERT(survived == true);
        RC_ASSERT(tracker.getRecords().size() == 1u);

        const InjuryRecord& rec = tracker.getRecords().back();
        RC_ASSERT(rec.location == loc);
        // Cumulative magnitude of the first crit equals the crit magnitude.
        RC_ASSERT(rec.magnitude == mag);
        RC_ASSERT(tracker.getMagnitude() == mag);
        RC_ASSERT(tracker.hasInjuries() == true);
        RC_ASSERT(tracker.activeCount() == 1);
    });
}

TEST_CASE("Preservation: sequence of non-fatal crits records each location with cumulative magnitude",
          "[crit-hit-location]")
{
    // A concrete multi-crit sequence keeps cumulative magnitude below FATAL (10):
    //   +2 (BODY)  -> total 2
    //   +3 (HEAD)  -> total 5
    //   +4 (LEFT_LEG) -> total 9
    InjuryTracker tracker;

    REQUIRE(tracker.applyCrit(nullptr, HitLocation::BODY, 2) == true);
    REQUIRE(tracker.applyCrit(nullptr, HitLocation::HEAD, 3) == true);
    REQUIRE(tracker.applyCrit(nullptr, HitLocation::LEFT_LEG, 4) == true);

    const auto& records = tracker.getRecords();
    REQUIRE(records.size() == 3u);

    // Each record retains its own location, in insertion order.
    CHECK(records[0].location == HitLocation::BODY);
    CHECK(records[1].location == HitLocation::HEAD);
    CHECK(records[2].location == HitLocation::LEFT_LEG);

    // Each record stores the CUMULATIVE magnitude at the time of that crit.
    CHECK(records[0].magnitude == 2);
    CHECK(records[1].magnitude == 5);
    CHECK(records[2].magnitude == 9);

    // The most recent record is the last crit's location.
    CHECK(records.back().location == HitLocation::LEFT_LEG);
    CHECK(tracker.getMagnitude() == 9);
}

// ─── Preservation 3: Magnitude display is unchanged ────────────────────────────
// Property 2 (Preservation): after a non-fatal applyCrit, getRecords().back().magnitude
// equals the cumulative magnitude and getMagnitude() matches — the value the sidebar
// renders per record must be stored/returned unchanged.
// **Validates: Requirements 3.2**

TEST_CASE("PBT Property 2: magnitude display value matches cumulative magnitude",
          "[pbt][property][crit-hit-location]")
{
    rc::prop("back().magnitude == getMagnitude() after a non-fatal crit", []() {
        const int locIdx = *rc::gen::inRange(0, static_cast<int>(HitLocation::COUNT) - 1);
        const HitLocation loc = static_cast<HitLocation>(locIdx);
        const int mag = *rc::gen::inRange(1, InjuryTracker::FATAL_MAGNITUDE - 1);

        InjuryTracker tracker;
        const bool survived = tracker.applyCrit(nullptr, loc, mag);
        RC_ASSERT(survived == true);
        RC_ASSERT(!tracker.getRecords().empty());

        // The displayed magnitude value equals the tracker's cumulative magnitude (Req 3.2).
        RC_ASSERT(tracker.getRecords().back().magnitude == tracker.getMagnitude());
        RC_ASSERT(tracker.getRecords().back().magnitude == mag);
    });
}

// ─── Preservation 4: Save/load round-trip preserves records ────────────────────
// Property 2 (Preservation): build a tracker from a random sequence of non-fatal
// crits (cumulative magnitude kept below FATAL_MAGNITUDE), save to a TCODZip flushed
// to a temp file, load into a fresh tracker, and assert per-record location and
// magnitude equality plus getMagnitude() equality. Follows the temp-file pattern used
// by test_high_score.cpp / test_persistent_levels.cpp.
// **Validates: Requirements 3.3**

TEST_CASE("PBT Property 2: save/load round-trip preserves record location and magnitude",
          "[pbt][property][crit-hit-location]")
{
    rc::prop("save then load yields identical records and cumulative magnitude", []() {
        // Build a random sequence of non-fatal crits. Keep the running cumulative
        // magnitude strictly below FATAL_MAGNITUDE so every applyCrit records a
        // (non-fatal) InjuryRecord.
        const int numCrits = *rc::gen::inRange(0, 12);

        InjuryTracker original;
        int runningTotal = 0;
        for (int i = 0; i < numCrits; ++i) {
            // Remaining headroom before hitting the fatal threshold.
            const int remaining = (InjuryTracker::FATAL_MAGNITUDE - 1) - runningTotal;
            if (remaining < 1) break; // no non-fatal room left
            const int mag = *rc::gen::inRange(1, remaining);
            const int locIdx = *rc::gen::inRange(0, static_cast<int>(HitLocation::COUNT) - 1);
            const HitLocation loc = static_cast<HitLocation>(locIdx);

            const bool survived = original.applyCrit(nullptr, loc, mag);
            RC_ASSERT(survived == true); // stayed below fatal by construction
            runningTotal += mag;
        }

        const char* tempFile = "__test_injury_roundtrip.dat";

        // Serialize to a TCODZip archive and flush to disk.
        {
            TCODZip zip;
            original.save(zip);
            zip.saveToFile(tempFile);
        }

        // Deserialize into a fresh tracker.
        InjuryTracker loaded;
        {
            TCODZip zip;
            zip.loadFromFile(tempFile);
            loaded.load(zip);
        }

        std::remove(tempFile);

        // Cumulative magnitude survives the round trip (Req 3.3).
        RC_ASSERT(loaded.getMagnitude() == original.getMagnitude());

        // Same number of records, each with identical location and magnitude (Req 3.3).
        const auto& orig = original.getRecords();
        const auto& got = loaded.getRecords();
        RC_ASSERT(got.size() == orig.size());
        for (size_t i = 0; i < orig.size(); ++i) {
            RC_ASSERT(got[i].location == orig[i].location);
            RC_ASSERT(got[i].magnitude == orig[i].magnitude);
        }
    });
}
