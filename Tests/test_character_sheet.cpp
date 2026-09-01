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

// ═══════════════════════════════════════════════════════════════════════════════
// Serialization round-trip tests (tasks 2.1 – 2.4)
//
// TCODZip has no in-memory round-trip; the buffer must be flushed to disk and
// reloaded (saveToFile / loadFromFile). Each test uses a distinct temp filename
// and removes it afterward so the cases are independent and leave no artifacts.
//
// These tests are EXPECTED TO FAIL against the current empty CareerProgression::
// save/load stubs (TDD red state): all career data is dropped across the cycle,
// so careerEqual / sheetEqual will report a mismatch. They will pass once
// serialization is implemented (task 6).
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: charactersheet-tests, Property 1: CareerProgression serialization round-trip
// For any CareerProgression with arbitrary scalars and collections, saving to a
// TCODZip and loading into a fresh instance produces an equal instance.
// Validates: Requirements 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.1, 2.3
TEST_CASE("CareerProgression serialization round-trip", "[character-sheet][pbt]")
{
	rc::check("saving and loading a CareerProgression preserves every field", [] {
		const CareerProgression original = genCareerProgression();

		TCODZip zip;
		CareerProgression saveTarget = original;
		saveTarget.save(zip);
		zip.saveToFile("__test_career_rt.sav");

		TCODZip loadZip;
		loadZip.loadFromFile("__test_career_rt.sav");
		CareerProgression loaded;
		loaded.load(loadZip);

		std::remove("__test_career_rt.sav");

		RC_ASSERT(careerEqual(original, loaded));
	});
}

// Feature: charactersheet-tests, Property 2: Load clears stale state
// For any source and dirty CareerProgression, saving source and loading the
// produced data into the pre-populated dirty instance yields an instance equal
// to source — the dirty instance's prior skills/talents/traits do not survive.
// Validates: Requirements 1.10
TEST_CASE("CareerProgression load clears stale state", "[character-sheet][pbt]")
{
	rc::check("loading over a pre-populated instance leaves no stale entries", [] {
		const CareerProgression source = genCareerProgression();

		// A dirty target that already carries its own skills/talents/traits.
		CareerProgression dirty = genCareerProgression();
		dirty.skills["__stale_skill__"] = 2;
		dirty.talents.insert("__stale_talent__");
		dirty.traits.push_back("__stale_trait__");

		TCODZip zip;
		CareerProgression saveTarget = source;
		saveTarget.save(zip);
		zip.saveToFile("__test_career_stale.sav");

		TCODZip loadZip;
		loadZip.loadFromFile("__test_career_stale.sav");
		dirty.load(loadZip);

		std::remove("__test_career_stale.sav");

		RC_ASSERT(careerEqual(source, dirty));
	});
}

// Feature: charactersheet-tests, Property 3: CharacterSheet serialization round-trip
// For any CharacterSheet with nine base characteristics in [1, 99] and arbitrary
// CareerProgression state, saving to a TCODZip and loading into a fresh instance
// produces an equal instance (nine base characteristics + all career fields).
// Validates: Requirements 3.1, 3.2, 3.3, 3.4
TEST_CASE("CharacterSheet serialization round-trip", "[character-sheet][pbt]")
{
	rc::check("saving and loading a CharacterSheet preserves characteristics and career", [] {
		const CharacterSheet original = genCharacterSheet();

		TCODZip zip;
		CharacterSheet saveTarget = original;
		saveTarget.save(zip);
		zip.saveToFile("__test_sheet_rt.sav");

		TCODZip loadZip;
		loadZip.loadFromFile("__test_sheet_rt.sav");
		CharacterSheet loaded;
		loaded.load(loadZip);

		std::remove("__test_sheet_rt.sav");

		RC_ASSERT(sheetEqual(original, loaded));
	});
}

// Feature: charactersheet-tests, Empty-collections edge case (example-based)
// A CareerProgression with empty skills/talents/traits but known scalar fields
// round-trips with all three collections restored empty and scalars preserved.
// Validates: Requirements 2.3
TEST_CASE("CareerProgression with empty collections round-trips", "[character-sheet]")
{
	CareerProgression original;
	original.homeworldName = "Death World";
	original.careerName    = "Arch-Militant";
	original.currentRank   = 3;
	original.xpPool        = 5000;
	original.spentXp       = 2200;
	// skills / talents / traits left empty on purpose.

	TCODZip zip;
	original.save(zip);
	zip.saveToFile("__test_career_empty.sav");

	TCODZip loadZip;
	loadZip.loadFromFile("__test_career_empty.sav");
	CareerProgression loaded;
	loaded.load(loadZip);

	std::remove("__test_career_empty.sav");

	CHECK(loaded.homeworldName == "Death World");
	CHECK(loaded.careerName == "Arch-Militant");
	CHECK(loaded.currentRank == 3);
	CHECK(loaded.xpPool == 5000);
	CHECK(loaded.spentXp == 2200);
	CHECK(loaded.skills.empty());
	CHECK(loaded.talents.empty());
	CHECK(loaded.traits.empty());
}
