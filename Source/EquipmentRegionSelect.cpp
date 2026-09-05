// EquipmentRegionSelect.cpp
//
// Feature: equipment-region-assignment — engine-isolated region-weighted selection.
// See Headers/EquipmentRegionSelect.hpp for the contract and design references.
//
// This translation unit touches NO engine state (no engine.gui / map / player), so it
// is safe to link into the test binary and exercise without an initialized Engine.

#include "EquipmentRegionSelect.hpp"

int regionEligibleWeight(const EquipmentTemplate& tmpl, const std::string& regionName)
{
	// Exact Region_Name key wins; else fall back to the Universal_Tag weight.
	auto it = tmpl.regionWeights.find(regionName);
	int w;
	if (it != tmpl.regionWeights.end()) {
		w = it->second;
	} else {
		auto uni = tmpl.regionWeights.find("Universal");
		if (uni == tmpl.regionWeights.end()) {
			return 0;  // not region-eligible: no matching key, no Universal fallthrough
		}
		w = uni->second;
	}
	// Only strictly-positive weights contribute to cumulative selection. A zero (or
	// negative) weight is treated as not-selectable in this region.
	return w > 0 ? w : 0;
}

int regionEligibleTotal(
    const std::vector<const EquipmentTemplate*>& candidates,
    const std::string& regionName)
{
	int total = 0;
	for (const EquipmentTemplate* t : candidates) {
		if (t == nullptr) continue;
		total += regionEligibleWeight(*t, regionName);
	}
	return total;
}

const EquipmentTemplate* selectRegionEligible(
    const std::vector<const EquipmentTemplate*>& candidates,
    const std::string& regionName,
    int roll)
{
	// Collect region-eligible candidates and their positive weights.
	std::vector<const EquipmentTemplate*> eligible;
	std::vector<int> weights;
	int total = 0;
	for (const EquipmentTemplate* t : candidates) {
		if (t == nullptr) continue;
		int w = regionEligibleWeight(*t, regionName);
		if (w > 0) {
			eligible.push_back(t);
			weights.push_back(w);
			total += w;
		}
	}

	if (eligible.empty() || total <= 0) {
		return nullptr;  // graceful fallback (Req 7.4): nothing region-eligible
	}

	// Defensive clamp so the function is total for any roll (never throws / never
	// returns a dangling pointer). The intended range is [0, total).
	if (roll < 0) roll = 0;
	if (roll >= total) roll = total - 1;

	int acc = 0;
	for (size_t i = 0; i < eligible.size(); ++i) {
		acc += weights[i];
		if (roll < acc) {
			return eligible[i];
		}
	}
	return eligible.back();  // unreachable when total > 0; defensive.
}
