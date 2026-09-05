// test_equipment_region_selection.cpp
//
// Feature: equipment-region-assignment — Tasks 7.1, 7.2, 7.3, 7.4, 7.5
// Selection-surface tests for the design's "Runtime Region-Weighted Selection"
// (design Surface 3) and Correctness Properties 4–7.
//
// The real runtime selector, Engine::selectEquipmentByTier (Source/Engine.cpp), needs
// Engine state and calls gui->message on failure, so it is NOT test-safe under the
// test-isolation steering rule. Per the design's test-isolation strategy, the
// region-eligibility rule and the cumulative-weight pick are factored into the free,
// engine-isolated helpers declared in Headers/EquipmentRegionSelect.hpp:
//
//   int  regionEligibleWeight(const EquipmentTemplate&, regionName)
//   int  regionEligibleTotal (candidates, regionName)
//   const EquipmentTemplate* selectRegionEligible(candidates, regionName, roll)
//
// These implement the design's rule EXACTLY: an entry is region-eligible iff its
// regionWeights contains regionName (use that weight) OR "Universal" (use the Universal
// weight when the exact key is absent), and only strictly-positive weights count;
// cumulative-weight selection over eligible candidates with a roll in [0, total);
// nullptr when total <= 0. Task 7.6 wires this into Engine::selectEquipmentByTier.
//
// Engine isolation: every pool below is a locally-populated std::vector<EquipmentTemplate>
// built on the stack; the global Engine (engine.gui / map / player) is NEVER initialized
// and never touched. selectRegionEligible takes an explicit `roll`, so selection is
// deterministic and needs no RNG / TCODRandom.
//
// RapidCheck (Tests/lib/rapidcheck.h): rc::gen::inRange(a, b) uses INCLUSIVE bounds
// [a, b]. When indexing a container of size N use inRange(0, N - 1). Property tests run
// a minimum of 100 iterations (Property 6 uses a large fixed count with a statistical
// tolerance band).
//
// Requirements traceability:
//   Property 4 (Task 7.1): region-scoped eligibility        — Reqs 7.2, 7.5
//   Property 5 (Task 7.2): Universal selectable everywhere   — Reqs 5.5, 7.2
//   Property 6 (Task 7.3): cumulative-weight proportionality — Req  7.3
//   Property 7 (Task 7.4): graceful fallback -> nullptr      — Req  7.4
//   Unit tests (Task 7.5): concrete selection cases          — Reqs 7.2, 7.4, 5.5

#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

// EquipmentRegionSelect.hpp includes main.hpp, which establishes the libtcod + project
// header include order and brings in EquipmentTemplate / EquipmentSlot / RegionWeights.
#include "EquipmentRegionSelect.hpp"  // regionEligibleWeight / regionEligibleTotal / selectRegionEligible

#include <string>
#include <vector>

namespace {

// The valid Region_Name taxonomy (excluding the Universal_Tag). Region requests in the
// runtime path are always one of these (resolved via regionForBiome / resolveDefaultRegion).
const std::vector<std::string>& taxonomyRegions() {
    static const std::vector<std::string> names = {
        "Ork", "Eldar", "DarkEldar", "Necron", "Tau", "Tyranid",
        "Kroot", "Chaos", "ImperialHuman", "Servitor"
    };
    return names;
}

// Builds a minimal EquipmentTemplate for selection tests. Only the fields the selector
// inspects (slot, regionWeights) carry meaning; the rest are set to harmless defaults so
// the pointer identity is what matters. `name` gives each template a stable identity for
// assertions and frequency counting.
EquipmentTemplate makeTemplate(const std::string& name,
                               EquipmentSlot slot,
                               RegionWeights regionWeights) {
    EquipmentTemplate t;
    t.name = name;
    t.glyph = '/';
    t.color = TCODColor::white;
    t.slot = slot;
    t.weight = 1.0f;
    t.value = 10;
    t.tier = ItemTier::COMMON;
    t.regionWeights = std::move(regionWeights);
    return t;
}

// Produces a vector of pointers into a stable backing store. Callers keep `store` alive
// for the lifetime of the returned pointers.
std::vector<const EquipmentTemplate*> asPool(const std::vector<EquipmentTemplate>& store) {
    std::vector<const EquipmentTemplate*> pool;
    pool.reserve(store.size());
    for (const auto& t : store) pool.push_back(&t);
    return pool;
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Task 7.5 — Selection unit / example TEST_CASEs (Reqs 7.2, 7.4, 5.5)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("Selection: Ork request over {Ork, Universal} pool can return either entry",
          "[equipment-region-assignment][selection]") {
    // Pool: one Ork-keyed entry and one Universal-keyed entry, same slot. Asked for
    // "Ork", BOTH are region-eligible (the Ork entry by exact key, the Universal entry
    // by Universal fallthrough), so cumulative selection can return either (Req 7.2, 5.5).
    std::vector<EquipmentTemplate> store = {
        makeTemplate("OrkChoppa", EquipmentSlot::WEAPON, { { "Ork", 100 } }),
        makeTemplate("CombatKnife", EquipmentSlot::WEAPON, { { "Universal", 100 } }),
    };
    auto pool = asPool(store);

    const int total = regionEligibleTotal(pool, "Ork");
    REQUIRE(total == 200);  // both eligible: 100 + 100

    // A roll in the first weight window selects the Ork entry; a roll in the second
    // window selects the Universal entry. Both outcomes are reachable.
    const EquipmentTemplate* low = selectRegionEligible(pool, "Ork", 0);
    const EquipmentTemplate* high = selectRegionEligible(pool, "Ork", 199);
    REQUIRE(low != nullptr);
    REQUIRE(high != nullptr);
    REQUIRE(low->name == "OrkChoppa");
    REQUIRE(high->name == "CombatKnife");
}

TEST_CASE("Selection: Eldar request over {Ork, Universal} pool returns only the Universal entry",
          "[equipment-region-assignment][selection]") {
    // Asked for "Eldar": the Ork entry has neither an Eldar key nor a Universal key, so
    // it is NOT eligible; only the Universal entry is selectable (Req 7.2, 5.5).
    std::vector<EquipmentTemplate> store = {
        makeTemplate("OrkChoppa", EquipmentSlot::WEAPON, { { "Ork", 100 } }),
        makeTemplate("CombatKnife", EquipmentSlot::WEAPON, { { "Universal", 100 } }),
    };
    auto pool = asPool(store);

    REQUIRE(regionEligibleTotal(pool, "Eldar") == 100);  // only the Universal entry

    // Every valid roll in [0, total) resolves to the Universal entry.
    for (int roll = 0; roll < 100; ++roll) {
        const EquipmentTemplate* sel = selectRegionEligible(pool, "Eldar", roll);
        REQUIRE(sel != nullptr);
        REQUIRE(sel->name == "CombatKnife");
    }
}

TEST_CASE("Selection: empty-eligible pool returns nullptr without throwing",
          "[equipment-region-assignment][selection]") {
    // A pool whose only entry is Ork-keyed, asked for "Eldar" (no Eldar key, no
    // Universal), is empty-eligible: selection returns nullptr (Req 7.4).
    std::vector<EquipmentTemplate> store = {
        makeTemplate("OrkChoppa", EquipmentSlot::WEAPON, { { "Ork", 100 } }),
    };
    auto pool = asPool(store);

    REQUIRE(regionEligibleTotal(pool, "Eldar") == 0);
    REQUIRE(selectRegionEligible(pool, "Eldar", 0) == nullptr);

    // A genuinely empty pool is also handled gracefully.
    std::vector<const EquipmentTemplate*> emptyPool;
    REQUIRE(selectRegionEligible(emptyPool, "Ork", 0) == nullptr);
}

TEST_CASE("Selection: exact Region_Name key wins over Universal fallthrough for weighting",
          "[equipment-region-assignment][selection]") {
    // An entry carrying BOTH an exact key and Universal uses the EXACT key's weight when
    // that region is requested (design: exact key wins), and the Universal weight for
    // any other region.
    EquipmentTemplate dual = makeTemplate("DualKeyed", EquipmentSlot::BODY,
                                          { { "Ork", 30 }, { "Universal", 70 } });
    REQUIRE(regionEligibleWeight(dual, "Ork") == 30);       // exact key wins
    REQUIRE(regionEligibleWeight(dual, "Eldar") == 70);     // Universal fallthrough
    REQUIRE(regionEligibleWeight(dual, "ImperialHuman") == 70);
}

TEST_CASE("Selection: zero-weight matching key is treated as not selectable",
          "[equipment-region-assignment][selection]") {
    // A zero weight contributes nothing to the cumulative total, mirroring the NPC rule
    // that a missing/zero column is never chosen.
    EquipmentTemplate zeroOrk = makeTemplate("ZeroOrk", EquipmentSlot::WEAPON, { { "Ork", 0 } });
    REQUIRE(regionEligibleWeight(zeroOrk, "Ork") == 0);

    std::vector<EquipmentTemplate> store = { zeroOrk };
    auto pool = asPool(store);
    REQUIRE(regionEligibleTotal(pool, "Ork") == 0);
    REQUIRE(selectRegionEligible(pool, "Ork", 0) == nullptr);

    // A zero Universal weight is likewise not selectable in any region.
    EquipmentTemplate zeroUni = makeTemplate("ZeroUni", EquipmentSlot::WEAPON, { { "Universal", 0 } });
    REQUIRE(regionEligibleWeight(zeroUni, "Tau") == 0);
}

TEST_CASE("Selection: slot is not filtered by the region helper (caller supplies same-slot pool)",
          "[equipment-region-assignment][selection]") {
    // The region helper operates on a caller-supplied candidate pool; per the design the
    // Engine builds that pool from same-slot + same-tier templates before calling. This
    // test documents that contract: a mixed-slot pool is the caller's responsibility, so
    // here we build a single-slot pool and confirm the returned template matches (Property
    // 4 models the slot invariant by only putting same-slot candidates in the pool).
    std::vector<EquipmentTemplate> store = {
        makeTemplate("BodyArmour", EquipmentSlot::BODY, { { "ImperialHuman", 100 } }),
    };
    auto pool = asPool(store);
    const EquipmentTemplate* sel = selectRegionEligible(pool, "ImperialHuman", 0);
    REQUIRE(sel != nullptr);
    REQUIRE(sel->slot == EquipmentSlot::BODY);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 7.1 — Property 4: region-scoped selection only returns eligible templates
// ═══════════════════════════════════════════════════════════════════════════════
//
// Feature: equipment-region-assignment, Property 4: For any set of loaded
// EquipmentTemplates (modelled as a same-slot candidate pool), any requested
// Region_Name, and any roll, if selectRegionEligible returns a non-null template, that
// template's regionWeights contains either the requested Region_Name key OR the
// "Universal" key with a positive weight, and the template's slot equals the requested
// slot. Slot is modelled by only placing same-slot candidates in the pool (per design's
// test note), so a non-null return necessarily matches the requested slot.
//
// Validates: Requirements 7.2, 7.5
TEST_CASE("Property 4: non-null selection is region-eligible and matches the requested slot",
          "[equipment-region-assignment][selection][pbt]") {
    rc::check("selectRegionEligible returns only eligible, same-slot templates", []() {
        const auto& regions = taxonomyRegions();

        // Requested region and the (single) slot every candidate in the pool shares.
        const std::string regionName = regions[*rc::gen::inRange(0, static_cast<int>(regions.size()) - 1)];
        const EquipmentSlot slot =
            static_cast<EquipmentSlot>(*rc::gen::inRange(0, static_cast<int>(EquipmentSlot::COUNT) - 1));

        // Build a random same-slot pool. Each entry gets a random region-weight map that
        // may or may not make it eligible for regionName.
        const int poolSize = *rc::gen::inRange(1, 8);
        std::vector<EquipmentTemplate> store;
        store.reserve(poolSize);
        for (int i = 0; i < poolSize; ++i) {
            RegionWeights rw;
            // Randomly attach an exact-region key (possibly zero weight).
            if (*rc::gen::arbitrary_bool()) {
                rw[regionName] = *rc::gen::inRange(0, 200);
            }
            // Randomly attach a Universal key (possibly zero weight).
            if (*rc::gen::arbitrary_bool()) {
                rw["Universal"] = *rc::gen::inRange(0, 200);
            }
            // Randomly attach an unrelated region key that should never make it eligible
            // for regionName (unless it happens to equal regionName, which we avoid).
            if (*rc::gen::arbitrary_bool()) {
                const std::string other = regions[*rc::gen::inRange(0, static_cast<int>(regions.size()) - 1)];
                if (other != regionName) {
                    rw[other] = *rc::gen::inRange(1, 200);
                }
            }
            store.push_back(makeTemplate("t" + std::to_string(i), slot, std::move(rw)));
        }

        auto pool = asPool(store);
        const int total = regionEligibleTotal(pool, regionName);

        // Roll spans a range wider than [0, total) to also exercise the defensive clamp.
        const int roll = *rc::gen::inRange(-5, total + 5);
        const EquipmentTemplate* sel = selectRegionEligible(pool, regionName, roll);

        if (sel == nullptr) {
            // Null is only returned when nothing is eligible (total == 0).
            RC_ASSERT(total == 0);
        } else {
            // Non-null: the returned template must be region-eligible with positive weight
            // and must be the requested slot.
            RC_ASSERT(sel->slot == slot);
            const int w = regionEligibleWeight(*sel, regionName);
            RC_ASSERT(w > 0);
            const bool hasRegionKey = sel->regionWeights.count(regionName) > 0
                                      && sel->regionWeights.at(regionName) > 0;
            const bool hasUniversal = sel->regionWeights.count("Universal") > 0
                                      && sel->regionWeights.at("Universal") > 0;
            // Eligible via exact key OR via Universal fallthrough (with positive weight).
            RC_ASSERT(hasRegionKey || hasUniversal);
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 7.2 — Property 5: Universal entries are selectable in every region
// ═══════════════════════════════════════════════════════════════════════════════
//
// Feature: equipment-region-assignment, Property 5: For any requested Region_Name in
// the valid taxonomy, given a same-slot pool in which at least one entry carries a
// positive "Universal" weight, region-scoped selection can return a non-null template —
// it never returns null solely because the exact Region_Name key is absent from every
// entry.
//
// Validates: Requirements 5.5, 7.2
TEST_CASE("Property 5: a positive-Universal entry is selectable in every region",
          "[equipment-region-assignment][selection][pbt]") {
    rc::check("pool with a positive Universal entry never yields null for any region", []() {
        const auto& regions = taxonomyRegions();
        const std::string regionName = regions[*rc::gen::inRange(0, static_cast<int>(regions.size()) - 1)];
        const EquipmentSlot slot =
            static_cast<EquipmentSlot>(*rc::gen::inRange(0, static_cast<int>(EquipmentSlot::COUNT) - 1));

        std::vector<EquipmentTemplate> store;

        // Add some noise entries keyed ONLY to regions other than regionName (so they are
        // NOT eligible via the exact key, and have no Universal fallthrough).
        const int noise = *rc::gen::inRange(0, 5);
        for (int i = 0; i < noise; ++i) {
            std::string other = regions[*rc::gen::inRange(0, static_cast<int>(regions.size()) - 1)];
            if (other == regionName) other = "Necron"; // force a non-matching key
            if (other == regionName) other = "Tau";     // (defensive if regionName == "Necron")
            RegionWeights rw;
            rw[other] = *rc::gen::inRange(1, 100);
            store.push_back(makeTemplate("noise" + std::to_string(i), slot, std::move(rw)));
        }

        // The guaranteed Universal entry with a strictly-positive weight.
        const int uniWeight = *rc::gen::inRange(1, 200);
        store.push_back(makeTemplate("universalItem", slot, { { "Universal", uniWeight } }));

        auto pool = asPool(store);
        const int total = regionEligibleTotal(pool, regionName);
        RC_ASSERT(total >= uniWeight);  // Universal entry always contributes

        // Selection must be able to return non-null; pick a roll in the valid window.
        const int roll = *rc::gen::inRange(0, total - 1);
        const EquipmentTemplate* sel = selectRegionEligible(pool, regionName, roll);
        RC_ASSERT(sel != nullptr);
        // And a roll landing in the Universal entry's cumulative window returns it (the
        // Universal item is appended last, so the top of the range selects it).
        const EquipmentTemplate* top = selectRegionEligible(pool, regionName, total - 1);
        RC_ASSERT(top != nullptr);
        RC_ASSERT(top->name == "universalItem");
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 7.3 — Property 6: cumulative-weight selection is proportional to weights
// ═══════════════════════════════════════════════════════════════════════════════
//
// Feature: equipment-region-assignment, Property 6: For any pool of region-eligible
// templates (same slot, positive integer weights for the requested Region_Name), walking
// selectRegionEligible over every roll in [0, total) selects each candidate a number of
// times exactly equal to its weight — the cumulative-weight partition is exact. Sweeping
// all rolls (rather than sampling an RNG) makes the proportionality exact and removes RNG
// flakiness while still validating the Cumulative_Chance_Selection model. A large random
// pool exercises the property across many iterations.
//
// Validates: Requirements 7.3
TEST_CASE("Property 6: selection frequency over all rolls is proportional to weights",
          "[equipment-region-assignment][selection][pbt]") {
    rc::check("each candidate is selected exactly `weight` times across [0, total)", []() {
        const std::string regionName = "Ork";
        const EquipmentSlot slot = EquipmentSlot::WEAPON;

        // Build a pool where every entry is region-eligible for "Ork" with a positive
        // weight (mix of exact-Ork-keyed and Universal-keyed entries).
        const int poolSize = *rc::gen::inRange(1, 6);
        std::vector<EquipmentTemplate> store;
        std::vector<int> expectedWeights;
        store.reserve(poolSize);
        expectedWeights.reserve(poolSize);
        for (int i = 0; i < poolSize; ++i) {
            const int w = *rc::gen::inRange(1, 40);  // strictly positive
            RegionWeights rw;
            if (*rc::gen::arbitrary_bool()) {
                rw[regionName] = w;                  // eligible via exact key
            } else {
                rw["Universal"] = w;                 // eligible via Universal fallthrough
            }
            store.push_back(makeTemplate("c" + std::to_string(i), slot, std::move(rw)));
            expectedWeights.push_back(w);
        }

        auto pool = asPool(store);
        const int total = regionEligibleTotal(pool, regionName);
        RC_ASSERT(total > 0);

        // Sweep every roll in [0, total) and count how many times each pool entry wins.
        std::vector<int> counts(store.size(), 0);
        for (int roll = 0; roll < total; ++roll) {
            const EquipmentTemplate* sel = selectRegionEligible(pool, regionName, roll);
            RC_ASSERT(sel != nullptr);
            // Map the returned pointer back to its index in the backing store.
            int idx = -1;
            for (size_t k = 0; k < store.size(); ++k) {
                if (&store[k] == sel) { idx = static_cast<int>(k); break; }
            }
            RC_ASSERT(idx >= 0);
            counts[idx]++;
        }

        // Cumulative selection partitions [0, total) into windows of size == weight, so
        // each candidate wins exactly `weight` rolls — proportionality is exact.
        int summed = 0;
        for (size_t k = 0; k < store.size(); ++k) {
            RC_ASSERT(counts[k] == expectedWeights[k]);
            summed += counts[k];
        }
        RC_ASSERT(summed == total);  // totality: every roll selects exactly one candidate
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Task 7.4 — Property 7: graceful fallback when nothing is region-eligible
// ═══════════════════════════════════════════════════════════════════════════════
//
// Feature: equipment-region-assignment, Property 7: For any requested Region_Name and a
// same-slot pool in which NO entry is region-eligible (no matching Region_Name key with
// positive weight and no positive "Universal" key), selectRegionEligible returns nullptr
// and does not throw.
//
// Validates: Requirements 7.4
TEST_CASE("Property 7: no-eligible pool yields nullptr without throwing",
          "[equipment-region-assignment][selection][pbt]") {
    rc::check("selection over a pool with nothing eligible returns null", []() {
        const auto& regions = taxonomyRegions();
        const std::string regionName = regions[*rc::gen::inRange(0, static_cast<int>(regions.size()) - 1)];
        const EquipmentSlot slot =
            static_cast<EquipmentSlot>(*rc::gen::inRange(0, static_cast<int>(EquipmentSlot::COUNT) - 1));

        // Build a pool guaranteed to have NO eligible entry for regionName:
        //   - keys are always some OTHER region (never regionName, never "Universal"), OR
        //   - a zero-weight regionName / Universal key (present but not selectable).
        const int poolSize = *rc::gen::inRange(0, 8);
        std::vector<EquipmentTemplate> store;
        store.reserve(poolSize);
        for (int i = 0; i < poolSize; ++i) {
            RegionWeights rw;
            const int shape = *rc::gen::inRange(0, 2);
            if (shape == 0) {
                // A non-matching region key with a positive weight.
                std::string other = regions[*rc::gen::inRange(0, static_cast<int>(regions.size()) - 1)];
                if (other == regionName) other = (regionName == "Ork") ? "Eldar" : "Ork";
                rw[other] = *rc::gen::inRange(1, 100);
            } else if (shape == 1) {
                // A zero-weight exact key (present but not selectable).
                rw[regionName] = 0;
            } else {
                // A zero-weight Universal key (present but not selectable).
                rw["Universal"] = 0;
            }
            store.push_back(makeTemplate("x" + std::to_string(i), slot, std::move(rw)));
        }

        auto pool = asPool(store);
        RC_ASSERT(regionEligibleTotal(pool, regionName) == 0);

        // Any roll (including out-of-range) must return nullptr without throwing.
        const int roll = *rc::gen::inRange(-3, 10);
        RC_ASSERT(selectRegionEligible(pool, regionName, roll) == nullptr);
    });
}
