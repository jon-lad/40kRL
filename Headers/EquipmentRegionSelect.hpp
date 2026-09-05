#pragma once

// EquipmentRegionSelect.hpp
//
// Feature: equipment-region-assignment — engine-isolated region-weighted selection.
//
// The runtime selector Engine::selectEquipmentByTier (Source/Engine.cpp) needs Engine
// state and calls gui->message on failure, so it is NOT test-safe. Per the design's
// test-isolation strategy (design "Testing Strategy" / "Error Handling"), the
// region-eligibility rule and the cumulative-weight pick are factored into these free
// helpers, which touch NO engine state (no engine.gui / map / player). They operate on
// a caller-supplied candidate pool of EquipmentTemplate pointers, so they can be
// exercised over locally-populated std::vector<EquipmentTemplate> pools without an
// Engine (per test-isolation steering).
//
// Task 7.6 wires these into Engine::selectEquipmentByTier: after the existing tier +
// slot filtering produces the candidate pool, the selector calls selectRegionEligible
// with a roll in [0, total) drawn from TCODRandom, applying the region filter on top of
// the tier filter.
//
// EquipmentTemplate and the RegionWeights alias are defined in Engine.hpp, which cannot
// live in WorldMap.hpp (WorldMap.hpp is included by Engine.hpp). This header includes
// Engine.hpp so it can see EquipmentTemplate, and is implemented in
// Source/EquipmentRegionSelect.cpp (added to BOTH 40kRL.vcxproj and
// Tests/40kRL_Tests.vcxproj per the test-project steering rule).

#include <string>
#include <vector>

// EquipmentTemplate / RegionWeights live in Engine.hpp, which depends on libtcod and a
// chain of other project headers being included first (TCODColor, Actor, Map, Gui, ...).
// main.hpp establishes that include order, so we pull it in here to make this header
// self-contained for any translation unit.
#include "main.hpp"  // -> Engine.hpp (EquipmentTemplate, RegionWeights, EquipmentSlot)

// Returns the effective region-selection weight of a single template for the requested
// Region_Name, implementing the design's region-eligibility rule EXACTLY:
//
//   - If regionWeights contains the exact regionName key, use that weight.
//   - Otherwise, if regionWeights contains the "Universal" key, use the Universal weight.
//   - Otherwise the template is not region-eligible.
//
// Returns the chosen weight when it is strictly positive (> 0), meaning the template is
// region-eligible and contributes to cumulative selection. Returns 0 when the template
// is not eligible OR its matching weight is <= 0 (a zero/negative weight contributes
// nothing to the cumulative total, mirroring the NPC-spawn rule that a missing/zero
// column is never chosen).
int regionEligibleWeight(const EquipmentTemplate& tmpl, const std::string& regionName);

// Cumulative-weight selection over a caller-supplied candidate pool, scoped to the
// requested Region_Name. Implements the design's algorithm EXACTLY:
//
//   1. Compute each candidate's region-eligible weight via regionEligibleWeight().
//   2. Keep only candidates with a positive eligible weight; sum those weights (total).
//   3. If no eligible candidate exists (total <= 0), return nullptr (graceful fallback,
//      no throw).
//   4. Otherwise walk the eligible candidates accumulating weights; return the first
//      candidate whose running total is strictly greater than `roll`.
//
// `roll` is expected to be in [0, total); callers (Engine::selectEquipmentByTier at task
// 7.6) draw it from TCODRandom via getInt(0, total - 1). The helper is deterministic:
// the same pool + regionName + roll always yields the same result. Out-of-range rolls
// are clamped defensively so the function is total (never throws, never returns a
// dangling pointer): a negative roll selects the first eligible candidate, and a roll
// >= total selects the last eligible candidate.
const EquipmentTemplate* selectRegionEligible(
    const std::vector<const EquipmentTemplate*>& candidates,
    const std::string& regionName,
    int roll);

// Sum of region-eligible weights over the pool for the requested Region_Name. Exposed
// so callers/tests can compute the valid roll range [0, total) without duplicating the
// eligibility rule. Returns 0 when nothing is eligible.
int regionEligibleTotal(
    const std::vector<const EquipmentTemplate*>& candidates,
    const std::string& regionName);
