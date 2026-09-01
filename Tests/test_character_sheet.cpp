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

// ═══════════════════════════════════════════════════════════════════════════════
// Purchase / canPurchase tests (tasks 3.1 – 3.5)
//
// These lock in the EXISTING (already-implemented) canPurchase / purchase logic,
// so they are expected to PASS. purchase() takes a Characteristics&, so each test
// constructs a local `Characteristics chars(40)` — no engine involvement.
//
// AdvanceEntry::Type enum order (from CharacterData.hpp):
//   CHARACTERISTIC = 0, SKILL = 1, TALENT = 2
// Property tests that pick a type use inRange(0, 2) and cast to the enum.
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: charactersheet-tests, Property 4: Insufficient XP always rejects purchase
// For any CareerProgression state and any AdvanceEntry whose cost strictly exceeds
// availableXp(), canPurchase returns false regardless of the advance type
// (CHARACTERISTIC / SKILL / TALENT).
// Validates: Requirement 4.1
TEST_CASE("canPurchase rejects when cost exceeds available XP", "[character-sheet][pbt]")
{
	rc::check("cost > availableXp always yields canPurchase == false for every type", [] {
		CareerProgression cp = genCareerProgression();

		// availableXp() = xpPool - spentXp. Pick a cost strictly greater than it.
		// Guard against int overflow when availableXp is already near INT_MAX by
		// keeping the surplus modest.
		const int available = cp.availableXp();
		const int surplus = *rc::gen::inRange(1, 1000);
		// Clamp so available + surplus does not overflow.
		long long costLL = static_cast<long long>(available) + surplus;
		const int cost = static_cast<int>(costLL);
		RC_PRE(cost > available); // discard degenerate overflow cases

		const std::string name = *rc::gen::string(1, 8);
		const int amount = *rc::gen::inRange(-20, 20);

		for (int t = 0; t <= 2; ++t) {
			AdvanceEntry advance;
			advance.type = static_cast<AdvanceEntry::Type>(t);
			advance.name = name;
			advance.cost = cost;
			advance.amount = amount;
			RC_ASSERT(cp.canPurchase(advance) == false);
		}
	});
}

// Feature: charactersheet-tests, Property 5: Rejected purchase is a no-op
// For any CareerProgression + AdvanceEntry where canPurchase is false (forced here
// via cost > availableXp), purchase returns false and leaves spentXp, skills,
// talents, and the target Characteristics all unchanged.
// Validates: Requirement 5.1
TEST_CASE("rejected purchase leaves all state unchanged", "[character-sheet][pbt]")
{
	rc::check("purchase returns false and mutates nothing when canPurchase is false", [] {
		CareerProgression cp = genCareerProgression();

		// Snapshot the mutable state before the attempted purchase.
		const int spentBefore = cp.spentXp;
		const auto skillsBefore = cp.skills;
		const auto talentsBefore = cp.talents;

		Characteristics chars(40);
		std::array<int, static_cast<int>(CharId::COUNT)> charsBefore{};
		for (int i = 0; i < static_cast<int>(CharId::COUNT); ++i) {
			charsBefore[i] = chars.get(static_cast<CharId>(i));
		}

		// Force rejection: cost strictly greater than availableXp().
		const int available = cp.availableXp();
		const int surplus = *rc::gen::inRange(1, 1000);
		const int cost = static_cast<int>(static_cast<long long>(available) + surplus);
		RC_PRE(cost > available);

		AdvanceEntry advance;
		advance.type = static_cast<AdvanceEntry::Type>(*rc::gen::inRange(0, 2));
		advance.name = *rc::gen::string(1, 8);
		advance.cost = cost;
		advance.amount = *rc::gen::inRange(-20, 20);

		RC_PRE(cp.canPurchase(advance) == false);

		const bool result = cp.purchase(advance, chars);

		RC_ASSERT(result == false);
		RC_ASSERT(cp.spentXp == spentBefore);
		RC_ASSERT(cp.skills == skillsBefore);
		RC_ASSERT(cp.talents == talentsBefore);
		for (int i = 0; i < static_cast<int>(CharId::COUNT); ++i) {
			RC_ASSERT(chars.get(static_cast<CharId>(i)) == charsBefore[i]);
		}
	});
}

// Feature: charactersheet-tests, Property 6: Accepted purchase deducts exactly the cost
// For any CareerProgression + AdvanceEntry where canPurchase is true, purchase
// returns true and increases spentXp by exactly cost. The setup guarantees
// eligibility: availableXp() >= cost, and for SKILL the named skill is not maxed,
// for TALENT the named talent is not already owned.
// Validates: Requirement 5.2
TEST_CASE("accepted purchase deducts exactly the cost", "[character-sheet][pbt]")
{
	rc::check("purchase increases spentXp by exactly cost when canPurchase is true", [] {
		CareerProgression cp = genCareerProgression();

		// Choose a non-negative cost, then set xpPool/spentXp so availableXp >= cost.
		const int cost = *rc::gen::inRange(0, 1000);
		// Set spentXp to a modest baseline and give xpPool enough headroom.
		cp.spentXp = *rc::gen::inRange(0, 1000);
		const int slack = *rc::gen::inRange(0, 1000);
		cp.xpPool = cp.spentXp + cost + slack; // availableXp() = cost + slack >= cost

		// A name that is guaranteed unused for SKILL/TALENT preconditions.
		const std::string freshName = std::string("adv_") + *rc::gen::string(1, 8);

		AdvanceEntry advance;
		advance.type = static_cast<AdvanceEntry::Type>(*rc::gen::inRange(0, 2));
		advance.name = freshName;
		advance.cost = cost;
		advance.amount = *rc::gen::inRange(-20, 20);

		if (advance.type == AdvanceEntry::Type::SKILL) {
			cp.skills.erase(freshName); // ensure absent (< rank 2)
		} else if (advance.type == AdvanceEntry::Type::TALENT) {
			cp.talents.erase(freshName); // ensure not owned
		}

		RC_PRE(cp.canPurchase(advance) == true);

		const int spentBefore = cp.spentXp;
		Characteristics chars(40);
		const bool result = cp.purchase(advance, chars);

		RC_ASSERT(result == true);
		RC_ASSERT(cp.spentXp == spentBefore + cost);
	});
}

// ─── canPurchase branch unit tests (task 3.4) ──────────────────────────────────

// Feature: charactersheet-tests, canPurchase branch coverage (example-based)
// Covers every non-XP branch of canPurchase with concrete inputs.
// Validates: Requirements 4.2, 4.3, 4.4, 4.5, 4.6
TEST_CASE("canPurchase branch coverage", "[character-sheet]")
{
	CareerProgression cp;
	cp.xpPool = 1000;
	cp.spentXp = 0; // availableXp() == 1000, ample for any cost below

	SECTION("SKILL already at rank 2 is rejected (Req 4.2)") {
		cp.skills["Dodge"] = 2;
		AdvanceEntry advance{ AdvanceEntry::Type::SKILL, "Dodge", 100, 5 };
		CHECK(cp.canPurchase(advance) == false);
	}

	SECTION("TALENT already owned is rejected (Req 4.3)") {
		cp.talents.insert("Ambidextrous");
		AdvanceEntry advance{ AdvanceEntry::Type::TALENT, "Ambidextrous", 100, 5 };
		CHECK(cp.canPurchase(advance) == false);
	}

	SECTION("CHARACTERISTIC with sufficient XP is allowed (Req 4.4)") {
		AdvanceEntry advance{ AdvanceEntry::Type::CHARACTERISTIC, "WS", 100, 5 };
		CHECK(cp.canPurchase(advance) == true);
	}

	SECTION("SKILL absent with sufficient XP is allowed (Req 4.5)") {
		AdvanceEntry advance{ AdvanceEntry::Type::SKILL, "Awareness", 100, 5 };
		CHECK(cp.canPurchase(advance) == true);
	}

	SECTION("SKILL at rank 1 with sufficient XP is allowed (Req 4.5)") {
		cp.skills["Awareness"] = 1;
		AdvanceEntry advance{ AdvanceEntry::Type::SKILL, "Awareness", 100, 5 };
		CHECK(cp.canPurchase(advance) == true);
	}

	SECTION("TALENT absent with sufficient XP is allowed (Req 4.6)") {
		AdvanceEntry advance{ AdvanceEntry::Type::TALENT, "Quick Draw", 100, 5 };
		CHECK(cp.canPurchase(advance) == true);
	}
}

// ─── purchase-effect + unrecognized-char unit tests (task 3.5) ─────────────────

// Feature: charactersheet-tests, purchase effect coverage (example-based)
// Covers each purchase mutation branch: CHARACTERISTIC (add amount, clamp-high,
// clamp-low), SKILL insert/increment, and TALENT insertion.
// Validates: Requirements 5.3, 5.4, 5.5, 5.6
TEST_CASE("purchase applies the correct effect per type", "[character-sheet]")
{
	SECTION("CHARACTERISTIC recognized name adds amount (Req 5.3)") {
		CareerProgression cp;
		cp.xpPool = 1000;
		cp.spentXp = 0;
		Characteristics chars(40);
		AdvanceEntry advance{ AdvanceEntry::Type::CHARACTERISTIC, "WS", 100, 5 };
		CHECK(cp.purchase(advance, chars) == true);
		CHECK(chars.get(CharId::WS) == 45); // 40 + 5
		CHECK(cp.spentXp == 100);
	}

	SECTION("CHARACTERISTIC clamps high at 99 (Req 5.3)") {
		CareerProgression cp;
		cp.xpPool = 1000;
		cp.spentXp = 0;
		Characteristics chars(97); // WS starts at 97
		AdvanceEntry advance{ AdvanceEntry::Type::CHARACTERISTIC, "WS", 100, 5 };
		CHECK(cp.purchase(advance, chars) == true);
		CHECK(chars.get(CharId::WS) == 99); // clamp(97 + 5, 1, 99) == 99
	}

	SECTION("CHARACTERISTIC with a positive amount stays >= 1 (Req 5.3, clamp-low)") {
		CareerProgression cp;
		cp.xpPool = 1000;
		cp.spentXp = 0;
		Characteristics chars(1); // already at the floor
		AdvanceEntry advance{ AdvanceEntry::Type::CHARACTERISTIC, "WS", 100, 5 };
		CHECK(cp.purchase(advance, chars) == true);
		CHECK(chars.get(CharId::WS) == 6); // clamp(1 + 5, 1, 99) == 6, never below 1
	}

	SECTION("SKILL absent is inserted at rank 1 (Req 5.4)") {
		CareerProgression cp;
		cp.xpPool = 1000;
		cp.spentXp = 0;
		Characteristics chars(40);
		AdvanceEntry advance{ AdvanceEntry::Type::SKILL, "Awareness", 100, 5 };
		CHECK(cp.purchase(advance, chars) == true);
		REQUIRE(cp.skills.count("Awareness") == 1);
		CHECK(cp.skills.at("Awareness") == 1);
	}

	SECTION("SKILL present has its rank incremented (Req 5.5)") {
		CareerProgression cp;
		cp.xpPool = 1000;
		cp.spentXp = 0;
		cp.skills["Awareness"] = 1;
		Characteristics chars(40);
		AdvanceEntry advance{ AdvanceEntry::Type::SKILL, "Awareness", 100, 5 };
		CHECK(cp.purchase(advance, chars) == true);
		CHECK(cp.skills.at("Awareness") == 2);
	}

	SECTION("TALENT is inserted into the talents set (Req 5.6)") {
		CareerProgression cp;
		cp.xpPool = 1000;
		cp.spentXp = 0;
		Characteristics chars(40);
		AdvanceEntry advance{ AdvanceEntry::Type::TALENT, "Quick Draw", 100, 5 };
		CHECK(cp.purchase(advance, chars) == true);
		CHECK(cp.talents.count("Quick Draw") == 1);
	}
}

// Feature: charactersheet-tests, unrecognized characteristic purchase (example-based)
// A purchasable CHARACTERISTIC advance naming an unrecognized abbreviation deducts
// the cost from spentXp, leaves all nine characteristics unchanged, and returns true.
// Validates: Requirement 5.7
TEST_CASE("purchase of unrecognized characteristic deducts cost but changes no stat", "[character-sheet]")
{
	CareerProgression cp;
	cp.xpPool = 1000;
	cp.spentXp = 0;
	Characteristics chars(40);

	std::array<int, static_cast<int>(CharId::COUNT)> before{};
	for (int i = 0; i < static_cast<int>(CharId::COUNT); ++i) {
		before[i] = chars.get(static_cast<CharId>(i));
	}

	AdvanceEntry advance{ AdvanceEntry::Type::CHARACTERISTIC, "ZZ", 100, 5 };
	const bool result = cp.purchase(advance, chars);

	CHECK(result == true);
	CHECK(cp.spentXp == 100);
	for (int i = 0; i < static_cast<int>(CharId::COUNT); ++i) {
		CHECK(chars.get(static_cast<CharId>(i)) == before[i]);
	}
}
