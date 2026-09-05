// test_equipment_region_loading.cpp
//
// Feature: equipment-region-assignment — Tasks 4.1, 4.2, 4.3, 4.4
// Loader-surface tests for the engine-isolated region-parsing helpers declared in
// Headers/WorldMap.hpp and implemented in Source/WorldMap.cpp:
//
//   bool          isValidRegionName(const std::string&)
//   RegionWeights parseRegionWeights(const sol::table&)
//   void          applyRegionDefault(RegionWeights&)
//
// These validate the loader's parse+default layer (design Surface 2 — "Equipment.lua
// Schema + C++ Loader"), which the equipment load loop in Source/Engine.cpp calls to
// populate EquipmentTemplate::regionWeights. Task 3.2 (the helper implementation) is
// already done, so these tests are EXPECTED TO PASS once they compile.
//
// Engine isolation (per test-isolation steering): sol2 `region` tables are built in a
// local sol::state; the global Engine (engine.gui / engine.map / engine.player) is
// NEVER initialized. The helpers take a plain std::string / sol::table and touch no
// engine state, so they are safe to exercise here.
//
// RapidCheck (Tests/lib/rapidcheck.h stub): rc::gen::inRange(a, b) uses INCLUSIVE
// bounds [a, b]. Property tests run a minimum of 100 iterations.
//
// Requirements traceability:
//   Property 1 (Task 4.1): backward-compat equivalence  — Reqs 5.3, 6.1, 6.2, 6.5
//   Property 2 (Task 4.2): malformed field -> default    — Req  6.3
//   Property 3 (Task 4.3): unknown keys ignored, valid kept — Reqs 5.1, 6.4
//   Unit tests (Task 4.4): concrete loader cases         — Reqs 5.3, 6.1, 6.2, 6.5

#include "catch_amalgamated.hpp"
#include "rapidcheck.h"

#include "WorldMap.hpp"

#include <sol/sol.hpp>

#include <string>
#include <vector>

namespace {

// The valid Region_Name taxonomy plus the Universal_Tag (design "Taxonomy values").
// Kept local so the tests assert against a literal list rather than re-deriving it
// from the production helper under test.
const std::vector<std::string>& validRegionNames() {
    static const std::vector<std::string> names = {
        "Ork", "Eldar", "DarkEldar", "Necron", "Tau", "Tyranid",
        "Kroot", "Chaos", "ImperialHuman", "Servitor", "Universal"
    };
    return names;
}

// Keys deliberately OUTSIDE the taxonomy (case variants, race-adjacent-but-wrong,
// junk). isValidRegionName must reject every one of these.
const std::vector<std::string>& invalidRegionNames() {
    static const std::vector<std::string> names = {
        "ork", "ORK", "orks", "Imperial", "Human", "imperialhuman",
        "universal", "UNIVERSAL", "Elder", "Nekron", "Chaos ", " Ork",
        "Bogus", "Xenos", "", "Tauu", "Necrons", "Space Marine"
    };
    return names;
}

// The documented default the loader applies to an untagged / fully-malformed entry.
const RegionWeights kImperialHumanDefault = { { "ImperialHuman", DEFAULT_REGION_WEIGHT } };

// Convenience: parse a sol table then apply the default, matching the loader-loop
// order in Source/Engine.cpp (parseRegionWeights -> applyRegionDefault).
RegionWeights loadRegion(const sol::table& t) {
    RegionWeights w = parseRegionWeights(t);
    applyRegionDefault(w);
    return w;
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Sanity: the documented default constant is what the design specifies.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Region default weight constant is 100",
          "[equipment-region-assignment][loader]") {
    REQUIRE(DEFAULT_REGION_WEIGHT == 100);
    REQUIRE(kImperialHumanDefault.at("ImperialHuman") == DEFAULT_REGION_WEIGHT);
}

// ═══════════════════════════════════════════════════════════════════════════════
// isValidRegionName — supports the taxonomy checks used by the properties below.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("isValidRegionName accepts every taxonomy name and the Universal tag",
          "[equipment-region-assignment][loader]") {
    for (const auto& name : validRegionNames()) {
        INFO("valid region name: " << name);
        REQUIRE(isValidRegionName(name));
    }
}

TEST_CASE("isValidRegionName rejects names outside the taxonomy",
          "[equipment-region-assignment][loader]") {
    for (const auto& name : invalidRegionNames()) {
        INFO("expected-invalid region name: [" << name << "]");
        REQUIRE_FALSE(isValidRegionName(name));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 4.1 — Property 1: backward-compatibility equivalence for pre-feature entries
// ═══════════════════════════════════════════════════════════════════════════════
//
// Feature: equipment-region-assignment, Property 1: For any Equipment_Entry that omits
// the region field, the parse+default layer yields regionWeights == { ImperialHuman:
// DEFAULT_REGION_WEIGHT }. Modelled at the parse+default level: for any table with no
// valid region pairs, parseRegionWeights returns empty and applyRegionDefault sets
// exactly { ImperialHuman: 100 }.
// Validates: Requirements 5.3, 6.1, 6.2, 6.5
TEST_CASE("Property 1: absent/empty region field defaults to ImperialHuman:100",
          "[equipment-region-assignment][loader][pbt]") {
    sol::state lua;

    rc::check("empty-or-no-valid-pairs table -> { ImperialHuman = 100 }", [&] {
        // Model the "omitted region field" case: an entry with no *valid* region
        // pairs. We randomly build either a truly empty table, or a table containing
        // only pairs that parseRegionWeights must drop (non-string keys / unknown
        // keys), so parsing yields an empty map every iteration.
        sol::table t = lua.create_table();

        const int mode = rc::generate(rc::gen::inRange(0, 2));
        if (mode == 1) {
            // Only non-string (numeric array) keys -> ignored as non-string pairs.
            const int count = rc::generate(rc::gen::inRange(1, 4));
            for (int i = 0; i < count; ++i) {
                t[i + 1] = rc::generate(rc::gen::inRange(0, 200));
            }
        } else if (mode == 2) {
            // Only keys outside the taxonomy -> dropped as unrecognized.
            const auto bad = invalidRegionNames();
            const int count = rc::generate(rc::gen::inRange(1, 4));
            for (int i = 0; i < count; ++i) {
                const std::string key =
                    bad[rc::generate(rc::gen::inRange(0, static_cast<int>(bad.size()) - 1))];
                if (!key.empty()) {
                    t[key] = rc::generate(rc::gen::inRange(0, 200));
                }
            }
        }
        // mode == 0: leave the table empty.

        const RegionWeights parsed = parseRegionWeights(t);
        RC_ASSERT(parsed.empty());

        RegionWeights weights = parsed;
        applyRegionDefault(weights);

        // Exactly { ImperialHuman = 100 } — no other keys, correct weight.
        RC_ASSERT(weights.size() == 1u);
        RC_ASSERT(weights.count("ImperialHuman") == 1u);
        RC_ASSERT(weights.at("ImperialHuman") == DEFAULT_REGION_WEIGHT);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 4.2 — Property 2: malformed region field falls back to the documented default
// ═══════════════════════════════════════════════════════════════════════════════
//
// Feature: equipment-region-assignment, Property 2: For any table containing only
// non-integer / negative / unrecognized entries, parseRegionWeights returns empty and
// applyRegionDefault yields { ImperialHuman: 100 } without throwing. The loader never
// aborts on malformed content — each bad pair is dropped independently.
// Validates: Requirements 6.3
TEST_CASE("Property 2: fully-malformed region field defaults to ImperialHuman:100",
          "[equipment-region-assignment][loader][pbt]") {
    sol::state lua;

    rc::check("malformed-only region table -> { ImperialHuman = 100 }", [&] {
        sol::table t = lua.create_table();

        // Build 1..5 pairs, each individually malformed in one of three ways so that
        // NONE survive parsing:
        //   0: valid key with a NEGATIVE weight        -> dropped (weight < 0)
        //   1: valid key with a NON-INTEGER (string)   -> dropped (not int)
        //   2: UNRECOGNIZED key with a valid weight     -> dropped (unknown key)
        const auto valid = validRegionNames();
        const auto bad   = invalidRegionNames();
        const int count  = rc::generate(rc::gen::inRange(1, 5));

        for (int i = 0; i < count; ++i) {
            const int kind = rc::generate(rc::gen::inRange(0, 2));
            if (kind == 0) {
                const std::string key =
                    valid[rc::generate(rc::gen::inRange(0, static_cast<int>(valid.size()) - 1))];
                t[key] = -rc::generate(rc::gen::inRange(1, 500)); // strictly negative
            } else if (kind == 1) {
                const std::string key =
                    valid[rc::generate(rc::gen::inRange(0, static_cast<int>(valid.size()) - 1))];
                t[key] = std::string("not-an-int"); // non-integer weight
            } else {
                std::string key =
                    bad[rc::generate(rc::gen::inRange(0, static_cast<int>(bad.size()) - 1))];
                if (key.empty()) key = "Bogus"; // avoid empty-string key edge in Lua
                t[key] = rc::generate(rc::gen::inRange(0, 200)); // valid weight, bad key
            }
        }

        // Must not throw and must drop every pair.
        const RegionWeights parsed = parseRegionWeights(t);
        RC_ASSERT(parsed.empty());

        RegionWeights weights = parsed;
        applyRegionDefault(weights);

        RC_ASSERT(weights.size() == 1u);
        RC_ASSERT(weights.count("ImperialHuman") == 1u);
        RC_ASSERT(weights.at("ImperialHuman") == DEFAULT_REGION_WEIGHT);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 4.3 — Property 3: unrecognized region keys are ignored, valid keys retained
// ═══════════════════════════════════════════════════════════════════════════════
//
// Feature: equipment-region-assignment, Property 3: For any table mixing valid
// Region_Name/Universal keys (non-negative int weights) with keys outside the taxonomy,
// parseRegionWeights returns exactly the valid keys with their weights and none of the
// unrecognized keys.
// Validates: Requirements 5.1, 6.4
TEST_CASE("Property 3: unrecognized region keys dropped, valid keys retained",
          "[equipment-region-assignment][loader][pbt]") {
    sol::state lua;

    rc::check("mixed valid + unknown keys -> only valid keys survive", [&] {
        sol::table t = lua.create_table();

        const auto valid = validRegionNames();
        const auto bad   = invalidRegionNames();

        // Choose a random, distinct subset of valid keys with random non-negative
        // weights; track the expected surviving map alongside.
        RegionWeights expected;
        const int validCount = rc::generate(rc::gen::inRange(1, static_cast<int>(valid.size())));
        // Shuffle-free distinct pick: walk the taxonomy and include each with ~50%
        // probability until we've selected validCount (guarantee at least one).
        std::vector<std::string> chosen;
        for (const auto& name : valid) {
            if (static_cast<int>(chosen.size()) >= validCount) break;
            if (rc::generate(rc::gen::arbitrary_bool())) {
                chosen.push_back(name);
            }
        }
        if (chosen.empty()) {
            chosen.push_back(valid[rc::generate(rc::gen::inRange(0, static_cast<int>(valid.size()) - 1))]);
        }
        for (const auto& key : chosen) {
            const int w = rc::generate(rc::gen::inRange(0, 500)); // non-negative
            t[key] = w;
            expected[key] = w; // later duplicates overwrite, matching Lua table semantics
        }

        // Add 1..4 unrecognized keys with valid weights; these must NOT survive.
        const int badCount = rc::generate(rc::gen::inRange(1, 4));
        for (int i = 0; i < badCount; ++i) {
            std::string key =
                bad[rc::generate(rc::gen::inRange(0, static_cast<int>(bad.size()) - 1))];
            if (key.empty()) key = "Bogus";
            // Only add if it does not collide with a chosen valid key or another
            // (invalid keys can't collide with valid ones, but guard the empty case).
            if (!isValidRegionName(key)) {
                t[key] = rc::generate(rc::gen::inRange(0, 200));
            }
        }

        const RegionWeights parsed = parseRegionWeights(t);

        // Exactly the valid keys with their weights, and nothing else.
        RC_ASSERT(parsed.size() == expected.size());
        for (const auto& kv : expected) {
            RC_ASSERT(parsed.count(kv.first) == 1u);
            RC_ASSERT(parsed.at(kv.first) == kv.second);
        }
        // No unrecognized key leaked through.
        for (const auto& kv : parsed) {
            RC_ASSERT(isValidRegionName(kv.first));
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 4.4 — Loader unit / example tests (Catch2 TEST_CASE, concrete cases)
// Validates: Requirements 5.3, 6.1, 6.2, 6.5 (plus 6.3, 6.4 spot-checks)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Loader unit: absent region field -> { ImperialHuman = 100 }",
          "[equipment-region-assignment][loader]") {
    sol::state lua;
    // An Equipment_Entry that omits `region`: the loader parses an empty table.
    sol::table empty = lua.create_table();

    const RegionWeights weights = loadRegion(empty);

    REQUIRE(weights.size() == 1u);
    REQUIRE(weights.count("ImperialHuman") == 1u);
    REQUIRE(weights.at("ImperialHuman") == 100);
}

TEST_CASE("Loader unit: single-key region { Ork = 100 } parses exactly",
          "[equipment-region-assignment][loader]") {
    sol::state lua;
    sol::table region = lua.create_table();
    region["Ork"] = 100;

    const RegionWeights parsed = parseRegionWeights(region);

    REQUIRE(parsed.size() == 1u);
    REQUIRE(parsed.count("Ork") == 1u);
    REQUIRE(parsed.at("Ork") == 100);

    // Default must NOT overwrite a populated map.
    RegionWeights withDefault = parsed;
    applyRegionDefault(withDefault);
    REQUIRE(withDefault == parsed);
    REQUIRE(withDefault.count("ImperialHuman") == 0u);
}

TEST_CASE("Loader unit: multi-key region { ImperialHuman = 80, Chaos = 20 } parses both",
          "[equipment-region-assignment][loader]") {
    sol::state lua;
    sol::table region = lua.create_table();
    region["ImperialHuman"] = 80;
    region["Chaos"] = 20;

    const RegionWeights parsed = parseRegionWeights(region);

    REQUIRE(parsed.size() == 2u);
    REQUIRE(parsed.at("ImperialHuman") == 80);
    REQUIRE(parsed.at("Chaos") == 20);
}

TEST_CASE("Loader unit: Universal key is accepted and retained",
          "[equipment-region-assignment][loader]") {
    sol::state lua;
    sol::table region = lua.create_table();
    region["Universal"] = 100;

    const RegionWeights parsed = parseRegionWeights(region);

    REQUIRE(parsed.size() == 1u);
    REQUIRE(parsed.at("Universal") == 100);

    // Explicit Universal entry stays as-is; the ImperialHuman default is not applied.
    RegionWeights withDefault = parsed;
    applyRegionDefault(withDefault);
    REQUIRE(withDefault.count("Universal") == 1u);
    REQUIRE(withDefault.count("ImperialHuman") == 0u);
}

TEST_CASE("Loader unit: unrecognized key dropped, valid key retained ({ Bogus = 50, Ork = 30 } -> only Ork)",
          "[equipment-region-assignment][loader]") {
    sol::state lua;
    sol::table region = lua.create_table();
    region["Bogus"] = 50; // outside the taxonomy -> ignored
    region["Ork"] = 30;   // valid -> retained

    const RegionWeights parsed = parseRegionWeights(region);

    REQUIRE(parsed.size() == 1u);
    REQUIRE(parsed.count("Bogus") == 0u);
    REQUIRE(parsed.count("Ork") == 1u);
    REQUIRE(parsed.at("Ork") == 30);
}

TEST_CASE("Loader unit: malformed weights dropped -> empty -> ImperialHuman default",
          "[equipment-region-assignment][loader]") {
    sol::state lua;

    SECTION("negative weight { Ork = -5 } is dropped") {
        sol::table region = lua.create_table();
        region["Ork"] = -5;

        const RegionWeights parsed = parseRegionWeights(region);
        REQUIRE(parsed.empty());

        const RegionWeights weights = loadRegion(region);
        REQUIRE(weights.size() == 1u);
        REQUIRE(weights.at("ImperialHuman") == 100);
    }

    SECTION("non-integer weight { Ork = \"x\" } is dropped") {
        sol::table region = lua.create_table();
        region["Ork"] = std::string("x");

        const RegionWeights parsed = parseRegionWeights(region);
        REQUIRE(parsed.empty());

        const RegionWeights weights = loadRegion(region);
        REQUIRE(weights.size() == 1u);
        REQUIRE(weights.at("ImperialHuman") == 100);
    }
}

TEST_CASE("Loader unit: zero weight is a valid non-negative weight and is retained",
          "[equipment-region-assignment][loader]") {
    // Req 5.1: weights are non-negative integers, so 0 is valid at parse time (the
    // selection layer, not the loader, treats a zero weight as not-selectable).
    sol::state lua;
    sol::table region = lua.create_table();
    region["Ork"] = 0;

    const RegionWeights parsed = parseRegionWeights(region);
    REQUIRE(parsed.size() == 1u);
    REQUIRE(parsed.count("Ork") == 1u);
    REQUIRE(parsed.at("Ork") == 0);
}
