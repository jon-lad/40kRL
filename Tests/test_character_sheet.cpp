#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: charactersheet-tests
//
// Test suite for CharacterSheet / CareerProgression serialization and logic.
// All cases are tagged [character-sheet] and are engine-independent (no engine.gui,
// engine.map, or engine.player access). Because purchase() takes a Characteristics&,
// tests construct a local Characteristics object directly.
//
// This file provides the shared scaffold used by tasks 2.x / 3.x / 4.x:
//   - genCareerProgression() / genCharacterSheet() RapidCheck generators
//   - careerEqual() / sheetEqual() field-by-field equality helpers
//     (see design Equality Strategy)
//
// Bounds convention: rc::gen::inRange(a, b) is INCLUSIVE at both ends in this
// project. Use inRange(1, 99) for characteristic values and inRange(0, N - 1)
// for any index into a container of size N.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Wide integer bound used for "arbitrary" scalar fields. The round-trip must
// hold for any int; TCODZip stores full 32-bit ints, so a broad range exercises
// negatives, zero, and large magnitudes.
constexpr int kIntLo = -1000000;
constexpr int kIntHi = 1000000;

// ─── Generators ──────────────────────────────────────────────────────────────

// Builds an arbitrary CareerProgression. Scalars are fully arbitrary — the
// round-trip must hold for any value; these fields carry no clamping invariant.
// This project's RapidCheck stub has no arbitrary<T>()/map/set generators, so
// collections are assembled from the primitive string / container / inRange gens.
CareerProgression genCareerProgression() {
	CareerProgression cp;
	cp.homeworldName = *rc::gen::string(0, 12);
	cp.careerName    = *rc::gen::string(0, 12);
	cp.currentRank   = *rc::gen::inRange(kIntLo, kIntHi);
	cp.xpPool        = *rc::gen::inRange(kIntLo, kIntHi);
	cp.spentXp       = *rc::gen::inRange(kIntLo, kIntHi);

	// Skills: build from a generated vector of names, each mapped to a rank.
	const std::vector<std::string> skillNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
	for (const auto& name : skillNames) {
		cp.skills[name] = *rc::gen::inRange(0, 2);
	}

	// Talents: unique names collected from a generated vector.
	const std::vector<std::string> talentNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
	for (const auto& name : talentNames) {
		cp.talents.insert(name);
	}

	// Traits: order-preserving vector of names.
	cp.traits = *rc::gen::container(0, 6, rc::gen::string(1, 8));

	return cp;
}

// Builds an arbitrary CharacterSheet: an arbitrary CareerProgression plus nine
// base characteristics each in the inclusive range [1, 99].
CharacterSheet genCharacterSheet() {
	CharacterSheet sheet;
	sheet.career = genCareerProgression();
	for (int i = 0; i < static_cast<int>(CharId::COUNT); ++i) {
		const int value = *rc::gen::inRange(1, 99);
		sheet.characteristics.set(static_cast<CharId>(i), value);
	}
	return sheet;
}

// ─── Equality helpers (design: Equality Strategy) ──────────────────────────────

// Compares two CareerProgression instances field-by-field:
//   - scalars and traits (vector) by ==
//   - skills (unordered_map) by size + per-key membership/value
//   - talents (unordered_set) by size + per-element membership
bool careerEqual(const CareerProgression& a, const CareerProgression& b) {
	if (a.homeworldName != b.homeworldName) return false;
	if (a.careerName != b.careerName) return false;
	if (a.currentRank != b.currentRank) return false;
	if (a.xpPool != b.xpPool) return false;
	if (a.spentXp != b.spentXp) return false;
	if (a.traits != b.traits) return false;

	if (a.skills.size() != b.skills.size()) return false;
	for (const auto& entry : a.skills) {
		auto it = b.skills.find(entry.first);
		if (it == b.skills.end()) return false;
		if (it->second != entry.second) return false;
	}

	if (a.talents.size() != b.talents.size()) return false;
	for (const auto& name : a.talents) {
		if (b.talents.find(name) == b.talents.end()) return false;
	}

	return true;
}

// Compares two CharacterSheet instances: the career via careerEqual plus the
// nine base characteristic values (getBase, not modifier-adjusted).
bool sheetEqual(const CharacterSheet& a, const CharacterSheet& b) {
	if (!careerEqual(a.career, b.career)) return false;
	for (int i = 0; i < static_cast<int>(CharId::COUNT); ++i) {
		const CharId id = static_cast<CharId>(i);
		if (a.characteristics.getBase(id) != b.characteristics.getBase(id)) return false;
	}
	return true;
}

} // anonymous namespace

// ─── Placeholder scaffold test ─────────────────────────────────────────────────
// A trivial case that confirms the file compiles and links. Real property/unit
// tests are added by tasks 2.x / 3.x / 4.x.
TEST_CASE("CharacterSheet scaffold: default currentRank is 1", "[character-sheet]")
{
	CharacterSheet sheet;
	CHECK(sheet.career.currentRank == 1);
	CHECK(sheet.career.availableXp() == 0);
}
