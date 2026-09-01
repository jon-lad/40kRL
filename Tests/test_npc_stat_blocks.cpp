#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.hpp"

#include <sol/sol.hpp>

#include "StatBlock.hpp"
#include "ReactionResolver.hpp"
#include "WeaponTypes.hpp"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: npc-stat-blocks
//
// Test suite for the shared NPC/player stat-block: the Dodge pure helpers
// (computeDodgeTarget / dodgeSucceeds), the Skill_Provider accessors
// (getSkillRank / hasSkill / hasTalent), the proficiency check
// (proficiencyModifier), the Lua parse helper (populateStatBlockFromLua), and the
// CareerProgression serialization round-trip.
//
// All cases are tagged [npc-stat-blocks] and are engine-independent: no engine.gui,
// engine.map, or engine.player access. Actors are constructed directly and the
// global Engine is never initialized (per test-isolation rules).
//
// TDD-RED: most of these are expected to FAIL against the current stubs
// (computeDodgeTarget returns 0, dodgeSucceeds returns false, getSkillRank returns
// SKILL_UNTRAINED, hasSkill/hasTalent return false, populateStatBlockFromLua is a
// no-op). The serialization round-trip tests (6.x) exercise CareerProgression
// save/load, which is already implemented, so those should PASS now.
//
// Bounds convention: rc::gen::inRange(a, b) is INCLUSIVE at both ends in this
// project. Use inRange(0, N - 1) for any index into a container of size N.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// clamp helper matching the production formula's clamp(x, 0, 100).
int clamp0to100(int x) {
	return std::clamp(x, 0, 100);
}

// Builds a bare Actor suitable for Skill_Provider / dodge tests. No engine needed.
Actor makeActor() {
	return Actor(0, 0, '@', "t", TCODColor(255, 255, 255));
}

} // anonymous namespace

// ═══════════════════════════════════════════════════════════════════════════════
// 2.1 — Dodge pure-helper tests
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: npc-stat-blocks, Property 1: Dodge target formula
// For any Agility A in [1,99] and any Dodge rank R in {0,1,2}:
//   computeDodgeTarget(A,R,true)  == clamp(A + R*10, 0, 100)
//   computeDodgeTarget(A,R,false) == clamp(A - 20,   0, 100)
// and every result lies within [0,100].
// Validates: Requirements 1.2, 1.3, 1.8
TEST_CASE("Dodge target formula matches clamp(A + R*10) skilled / clamp(A - 20) untrained", "[npc-stat-blocks][pbt]")
{
	rc::check("computeDodgeTarget equals the clamped skilled/untrained formula", [] {
		const int agility = *rc::gen::inRange(1, 99);
		const int rank    = *rc::gen::inRange(0, 2);

		const int skilled   = computeDodgeTarget(agility, rank, true);
		const int untrained = computeDodgeTarget(agility, rank, false);

		RC_ASSERT(skilled == clamp0to100(agility + rank * 10));
		RC_ASSERT(untrained == clamp0to100(agility - 20));

		RC_ASSERT(skilled >= 0 && skilled <= 100);
		RC_ASSERT(untrained >= 0 && untrained <= 100);
	});
}

// Feature: npc-stat-blocks, Property 2: Dodge monotonicity
// For fixed Agility A and two skilled ranks R1 <= R2, the effective target for R2 is
// never lower than for R1 (so increasing rank never turns a success into a failure).
// Validates: Requirements 1.2
TEST_CASE("Dodge target is monotonic non-decreasing in skill rank", "[npc-stat-blocks][pbt]")
{
	rc::check("higher Dodge rank never lowers the effective target", [] {
		const int agility = *rc::gen::inRange(1, 99);
		const int r1      = *rc::gen::inRange(0, 2);
		const int r2      = *rc::gen::inRange(0, 2);
		const int lo      = std::min(r1, r2);
		const int hi      = std::max(r1, r2);

		const int targetLo = computeDodgeTarget(agility, lo, true);
		const int targetHi = computeDodgeTarget(agility, hi, true);

		RC_ASSERT(targetHi >= targetLo);

		// A roll that succeeds at the lower rank still succeeds at the higher rank.
		const int roll = *rc::gen::inRange(2, 99);
		if (dodgeSucceeds(roll, targetLo)) {
			RC_ASSERT(dodgeSucceeds(roll, targetHi));
		}
	});
}

// Feature: npc-stat-blocks, Property 3: Roll-under with auto-success and auto-fail
// For any target T in [0,100]: dodgeSucceeds(1,T)==true (auto-success),
// dodgeSucceeds(100,T)==false (auto-fail); for any roll r in [2,99],
// dodgeSucceeds(r,T)==(r<=T).
// Validates: Requirements 1.1, 1.6, 1.7
TEST_CASE("Dodge roll-under honours auto-success on 1 and auto-fail on 100", "[npc-stat-blocks][pbt]")
{
	rc::check("dodgeSucceeds implements roll-under with 1=auto-success, 100=auto-fail", [] {
		const int target = *rc::gen::inRange(0, 100);

		RC_ASSERT(dodgeSucceeds(1, target) == true);
		RC_ASSERT(dodgeSucceeds(100, target) == false);

		const int roll = *rc::gen::inRange(2, 99);
		RC_ASSERT(dodgeSucceeds(roll, target) == (roll <= target));
	});
}

// ═══════════════════════════════════════════════════════════════════════════════
// 3.1 — Skill_Provider accessor semantics
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: npc-stat-blocks, Property 5: Skill_Provider accessor semantics
// For any skill mapping / talent set stored on an Actor's career:
//   - getSkillRank returns the stored rank for present skills, SKILL_UNTRAINED for absent
//   - hasSkill is true iff the skill name is present
//   - hasTalent is true iff the talent string is a member of the talent set
//   - an Actor with no career reads as untrained / talentless
// Results are identical whether the Actor is "player-shaped" or "NPC-shaped" (same data).
// Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5
TEST_CASE("Skill_Provider returns stored ranks, untrained sentinel, and talent membership", "[npc-stat-blocks][pbt]")
{
	rc::check("getSkillRank / hasSkill / hasTalent read career data null-safely", [] {
		// Generate present skill names (unique via set), each with a rank in [0,2].
		const std::vector<std::string> skillNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
		const std::vector<std::string> talentNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));

		Actor a = makeActor();
		a.career = std::make_shared<CareerProgression>();

		std::unordered_map<std::string, int> expectedSkills;
		for (const auto& name : skillNames) {
			const int rank = *rc::gen::inRange(0, 2);
			a.career->skills[name] = rank;
			expectedSkills[name] = rank; // last write wins, mirrors the map
		}
		std::unordered_set<std::string> expectedTalents;
		for (const auto& name : talentNames) {
			a.career->talents.insert(name);
			expectedTalents.insert(name);
		}

		// Present skills read back their stored rank; hasSkill is true.
		for (const auto& entry : expectedSkills) {
			RC_ASSERT(hasSkill(&a, entry.first) == true);
			RC_ASSERT(getSkillRank(&a, entry.first) == entry.second);
		}

		// A name guaranteed absent reads as untrained.
		const std::string absent = "__absent_skill__";
		if (expectedSkills.find(absent) == expectedSkills.end()) {
			RC_ASSERT(hasSkill(&a, absent) == false);
			RC_ASSERT(getSkillRank(&a, absent) == SKILL_UNTRAINED);
		}

		// Talent membership.
		for (const auto& name : expectedTalents) {
			RC_ASSERT(hasTalent(&a, name) == true);
		}
		const std::string absentTalent = "__absent_talent__";
		if (expectedTalents.find(absentTalent) == expectedTalents.end()) {
			RC_ASSERT(hasTalent(&a, absentTalent) == false);
		}

		// An Actor with no career reads untrained / talentless (null-safe).
		Actor nullCareer = makeActor();
		RC_ASSERT(nullCareer.career == nullptr);
		RC_ASSERT(getSkillRank(&nullCareer, "Dodge") == SKILL_UNTRAINED);
		RC_ASSERT(hasSkill(&nullCareer, "Dodge") == false);
		RC_ASSERT(hasTalent(&nullCareer, "Weapon Training (Primitive)") == false);
	});
}

// ═══════════════════════════════════════════════════════════════════════════════
// 3.2 — Player/NPC dodge equivalence
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: npc-stat-blocks, Property 4: Player/NPC dodge equivalence
// Two Actors configured with identical Agility and identical Dodge skill state
// (read through the Skill_Provider) produce identical computeDodgeTarget and
// dodgeSucceeds outcomes. There is no per-actor-type branch in the formula.
// Validates: Requirements 1.4, 1.5, 6.1
TEST_CASE("Dodge resolves identically for two actors with identical Ag + Dodge state", "[npc-stat-blocks][pbt]")
{
	rc::check("identical Agility and Dodge rank yield identical dodge outcomes", [] {
		const int agility  = *rc::gen::inRange(1, 99);
		const bool trained = *rc::gen::arbitrary_bool();
		const int rank     = *rc::gen::inRange(0, 2);

		// "Player-shaped" actor and "NPC-shaped" actor, same Dodge state.
		Actor playerish = makeActor();
		Actor npcish     = makeActor();
		if (trained) {
			playerish.career = std::make_shared<CareerProgression>();
			playerish.career->skills["Dodge"] = rank;
			npcish.career = std::make_shared<CareerProgression>();
			npcish.career->skills["Dodge"] = rank;
		}
		// (when untrained, both keep a null career)

		// Compute each actor's dodge target through the shared Skill_Provider path.
		const bool hasDodgeP = hasSkill(&playerish, "Dodge");
		const int rankP       = hasDodgeP ? getSkillRank(&playerish, "Dodge") : 0;
		const int targetP     = computeDodgeTarget(agility, rankP, hasDodgeP);

		const bool hasDodgeN = hasSkill(&npcish, "Dodge");
		const int rankN       = hasDodgeN ? getSkillRank(&npcish, "Dodge") : 0;
		const int targetN     = computeDodgeTarget(agility, rankN, hasDodgeN);

		RC_ASSERT(targetP == targetN);

		const int roll = *rc::gen::inRange(1, 100);
		RC_ASSERT(dodgeSucceeds(roll, targetP) == dodgeSucceeds(roll, targetN));
	});
}

// ═══════════════════════════════════════════════════════════════════════════════
// 4.1 — Proficiency equivalence
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: npc-stat-blocks, Property 6: Proficiency equivalence
// For any WeaponGroup, proficiencyModifier(actor, group) == 0 iff the actor's
// talent set contains the exact string "Weapon Training (" + weaponGroupName(group)
// + ")", else -20. Holds identically for player-shaped and NPC-shaped actors.
// Validates: Requirements 3.8, 4.1, 4.2, 4.3, 4.4
// (Uses hasTalent via proficiencyModifier — currently stubbed hasTalent returns
// false, so the "trained" branch FAILS until 11.2 is implemented.)
TEST_CASE("proficiencyModifier is 0 iff the matching Weapon Training talent is present", "[npc-stat-blocks][pbt]")
{
	rc::check("proficiency returns 0 for the trained group and -20 otherwise", [] {
		const int groupIdx = *rc::gen::inRange(0, static_cast<int>(WeaponGroup::EXOTIC));
		const WeaponGroup group = static_cast<WeaponGroup>(groupIdx);
		const std::string talentStr =
			"Weapon Training (" + std::string(weaponGroupName(group)) + ")";

		// Actor WITH the matching talent -> modifier 0.
		Actor trained = makeActor();
		trained.career = std::make_shared<CareerProgression>();
		trained.career->talents.insert(talentStr);
		RC_ASSERT(proficiencyModifier(&trained, group) == 0);

		// Actor WITHOUT the talent -> -20.
		Actor untrained = makeActor();
		untrained.career = std::make_shared<CareerProgression>();
		RC_ASSERT(proficiencyModifier(&untrained, group) == -20);

		// Actor with no career at all -> -20 (null-safe).
		Actor noCareer = makeActor();
		RC_ASSERT(proficiencyModifier(&noCareer, group) == -20);
	});
}

// ═══════════════════════════════════════════════════════════════════════════════
// 5.1 — Lua parse round-trip with clamp
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: npc-stat-blocks, Property 7: Lua parse round-trip with clamp
// For a generated Enemies.lua-style entry with skills (including negative and >2
// ranks), a talents list, and a traits list, populateStatBlockFromLua fills the
// CareerProgression so that: skills equal the declared skills with ranks clamped to
// [0,2]; talents equal the declared set; traits equal the declared list in order.
// Validates: Requirements 3.1, 3.2, 3.5, 3.6
// (populateStatBlockFromLua is a no-op stub, so this FAILS until 10.2.)
TEST_CASE("populateStatBlockFromLua parses skills (clamped), talents, and ordered traits", "[npc-stat-blocks][pbt]")
{
	rc::check("Lua entry parse yields clamped skills, exact talents, ordered traits", [] {
		sol::state lua;
		sol::table entry = lua.create_table();

		// ── skills: name -> raw rank (may be negative or > 2) ─────────────────
		const std::vector<std::string> skillNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
		sol::table skillsTbl = lua.create_table();
		std::unordered_map<std::string, int> expectedSkills; // clamped to [0,2]
		for (const auto& name : skillNames) {
			const int rawRank = *rc::gen::inRange(-5, 5);
			skillsTbl[name] = rawRank;
			expectedSkills[name] = std::clamp(rawRank, 0, MAX_SKILL_RANK);
		}
		entry["skills"] = skillsTbl;

		// ── talents: 1-based array of strings ─────────────────────────────────
		const std::vector<std::string> talentNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
		sol::table talentsTbl = lua.create_table();
		std::unordered_set<std::string> expectedTalents;
		for (size_t i = 0; i < talentNames.size(); ++i) {
			talentsTbl[i + 1] = talentNames[i];
			expectedTalents.insert(talentNames[i]);
		}
		entry["talents"] = talentsTbl;

		// ── traits: 1-based array of strings, order preserved ─────────────────
		const std::vector<std::string> traitNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
		sol::table traitsTbl = lua.create_table();
		for (size_t i = 0; i < traitNames.size(); ++i) {
			traitsTbl[i + 1] = traitNames[i];
		}
		entry["traits"] = traitsTbl;

		CareerProgression career;
		populateStatBlockFromLua(career, entry);

		// Skills: clamped mapping equality.
		RC_ASSERT(career.skills.size() == expectedSkills.size());
		for (const auto& kv : expectedSkills) {
			auto it = career.skills.find(kv.first);
			RC_ASSERT(it != career.skills.end());
			RC_ASSERT(it->second == kv.second);
			RC_ASSERT(it->second >= 0 && it->second <= MAX_SKILL_RANK);
		}

		// Talents: set equality (membership + size).
		RC_ASSERT(career.talents.size() == expectedTalents.size());
		for (const auto& name : expectedTalents) {
			RC_ASSERT(career.talents.find(name) != career.talents.end());
		}

		// Traits: order-preserving vector equality.
		RC_ASSERT(career.traits == traitNames);
	});
}

// ═══════════════════════════════════════════════════════════════════════════════
// 5.2 — Backward-compatible parse (unit)
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: npc-stat-blocks, Property 8: Backward-compatible spawn (example-based)
// An entry that omits skills / talents / traits leaves the CareerProgression empty,
// and a fresh CareerProgression reads as empty through the Skill_Provider without
// error (no career on the actor -> untrained / talentless).
// Validates: Requirements 2.7, 3.3, 3.4, 3.7, 3.9
TEST_CASE("populateStatBlockFromLua on an empty entry leaves the career empty", "[npc-stat-blocks]")
{
	sol::state lua;
	sol::table entry = lua.create_table();
	// No skills / talents / traits sections declared.

	CareerProgression career;
	populateStatBlockFromLua(career, entry);

	CHECK(career.skills.empty());
	CHECK(career.talents.empty());
	CHECK(career.traits.empty());

	// A fresh (empty) CareerProgression reads as empty via the Skill_Provider.
	Actor a = makeActor();
	a.career = std::make_shared<CareerProgression>();
	CHECK(getSkillRank(&a, "Dodge") == SKILL_UNTRAINED);
	CHECK(hasSkill(&a, "Dodge") == false);
	CHECK(hasTalent(&a, "Weapon Training (Primitive)") == false);

	// An actor with no career at all is also safe/empty.
	Actor none = makeActor();
	CHECK(getSkillRank(&none, "Dodge") == SKILL_UNTRAINED);
	CHECK(hasSkill(&none, "Dodge") == false);
	CHECK(hasTalent(&none, "Weapon Training (Primitive)") == false);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 6.x — Serialization round-trip
//
// TCODZip has no in-memory round-trip; the buffer is flushed to a temp file and
// reloaded. Each test uses a distinct filename and removes it afterward.
// These exercise the already-implemented CareerProgression::save/load, so they are
// expected to PASS even against the current stubs.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Builds a CareerProgression with generated skills / talents / traits (scalar
// fields are set to representative NPC defaults: fixed, never advanced).
CareerProgression genNpcCareer() {
	CareerProgression cp;
	cp.homeworldName = "";
	cp.careerName    = "";
	cp.currentRank   = 1;
	cp.xpPool        = 0;
	cp.spentXp       = 0;

	const std::vector<std::string> skillNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
	for (const auto& name : skillNames) {
		cp.skills[name] = *rc::gen::inRange(0, 2);
	}
	const std::vector<std::string> talentNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
	for (const auto& name : talentNames) {
		cp.talents.insert(name);
	}
	cp.traits = *rc::gen::container(0, 6, rc::gen::string(1, 8));
	return cp;
}

// Field-by-field equality on the collections that matter for NPC stat-blocks.
bool careerCollectionsEqual(const CareerProgression& a, const CareerProgression& b) {
	if (a.skills.size() != b.skills.size()) return false;
	for (const auto& entry : a.skills) {
		auto it = b.skills.find(entry.first);
		if (it == b.skills.end() || it->second != entry.second) return false;
	}
	if (a.talents.size() != b.talents.size()) return false;
	for (const auto& name : a.talents) {
		if (b.talents.find(name) == b.talents.end()) return false;
	}
	return a.traits == b.traits;
}

} // anonymous namespace

// Feature: npc-stat-blocks, Property 9: Skills/talents/traits serialization round-trip
// For any generated CareerProgression, saving to a TCODZip and loading into a fresh
// instance yields an equivalent skill mapping, talent set, and trait list.
// Validates: Requirements 7.1, 7.2, 7.3
TEST_CASE("CareerProgression skills/talents/traits survive a save/load round-trip", "[npc-stat-blocks][pbt]")
{
	rc::check("save then load preserves an NPC's skills, talents, and traits", [] {
		const CareerProgression original = genNpcCareer();

		TCODZip zip;
		CareerProgression saveTarget = original;
		saveTarget.save(zip);
		zip.saveToFile("__test_npc_career_rt.sav");

		TCODZip loadZip;
		loadZip.loadFromFile("__test_npc_career_rt.sav");
		CareerProgression loaded;
		loaded.load(loadZip);

		std::remove("__test_npc_career_rt.sav");

		RC_ASSERT(careerCollectionsEqual(original, loaded));
	});
}

// Feature: npc-stat-blocks, Property 10: Traits round-trip and inertness
// For any generated trait list, save then load preserves the list in order. Traits
// carry no mechanical effect, so no combat state depends on them; here we assert the
// round-trip and that the traits are stored inertly (they do not appear as skills or
// talents).
// Validates: Requirements 2.6, 8.1, 8.2
TEST_CASE("Trait list round-trips in order and remains inert", "[npc-stat-blocks][pbt]")
{
	rc::check("traits survive save/load in order without leaking into skills/talents", [] {
		const std::vector<std::string> traitList = *rc::gen::container(0, 8, rc::gen::string(1, 10));

		CareerProgression original;
		original.traits = traitList;

		TCODZip zip;
		CareerProgression saveTarget = original;
		saveTarget.save(zip);
		zip.saveToFile("__test_npc_traits_rt.sav");

		TCODZip loadZip;
		loadZip.loadFromFile("__test_npc_traits_rt.sav");
		CareerProgression loaded;
		loaded.load(loadZip);

		std::remove("__test_npc_traits_rt.sav");

		RC_ASSERT(loaded.traits == traitList);
		// Inertness: traits do not populate skills or talents.
		RC_ASSERT(loaded.skills.empty());
		RC_ASSERT(loaded.talents.empty());
	});
}

// Feature: npc-stat-blocks, pre-feature save compatibility (example-based)
// A save that predates this feature stores an NPC with no career (presence flag 0).
// Model that here: an empty CareerProgression round-trips as empty, so a loaded
// pre-feature NPC reads with an empty skill map, empty talent set, empty trait list.
// Validates: Requirement 7.4
TEST_CASE("Pre-feature NPC save loads with an empty stat block", "[npc-stat-blocks]")
{
	// Emulate the Actor::save presence-flag pattern: an int 0 flag means "no career".
	TCODZip zip;
	zip.putInt(0); // career presence flag == 0 (pre-feature NPC)
	zip.saveToFile("__test_npc_prefeature.sav");

	TCODZip loadZip;
	loadZip.loadFromFile("__test_npc_prefeature.sav");
	const int hasCareer = loadZip.getInt();

	std::remove("__test_npc_prefeature.sav");

	CHECK(hasCareer == 0); // flag round-trips as 0 -> no career constructed

	// The resulting NPC (no career) reads as an entirely empty stat block.
	Actor npc = makeActor();
	CHECK(npc.career == nullptr);
	CHECK(getSkillRank(&npc, "Dodge") == SKILL_UNTRAINED);
	CHECK(hasSkill(&npc, "Dodge") == false);
	CHECK(hasTalent(&npc, "Weapon Training (Primitive)") == false);

	// And an empty CareerProgression itself round-trips as empty.
	CareerProgression empty;
	TCODZip zip2;
	empty.save(zip2);
	zip2.saveToFile("__test_npc_empty_career.sav");
	TCODZip loadZip2;
	loadZip2.loadFromFile("__test_npc_empty_career.sav");
	CareerProgression loaded;
	loaded.load(loadZip2);
	std::remove("__test_npc_empty_career.sav");
	CHECK(loaded.skills.empty());
	CHECK(loaded.talents.empty());
	CHECK(loaded.traits.empty());
}

// Feature: npc-stat-blocks, Task 14.1: Actor-level NPC career round-trip (example-based)
// Verifies the full Actor::save / Actor::load path (not just CareerProgression) for an
// NPC-shaped actor that carries a standalone `career` (skills/talents/traits) and NO
// CharacterSheet. Asserts:
//   1. Actor::save writes the career for ANY actor with one (not player-gated).
//   2. Actor::load reconstructs the career and assigns it to actor->career.
//   3. The CharacterSheet re-alias path (player-only) does NOT run for an NPC, so the
//      NPC's standalone career survives load intact.
// This actor deliberately omits Openable / InjuryTracker / StatusEffectTracker so the
// save/load path never touches the global Engine (engine.map / reapply*), keeping the
// test engine-independent per the test-isolation rules.
// Validates: Requirements 7.1, 7.2, 7.3 (via the Actor boundary), design §6 guard.
TEST_CASE("Actor::save/load round-trips a standalone NPC career without re-aliasing", "[npc-stat-blocks]")
{
	// Build an NPC-shaped actor: a career with skills/talents/traits, no CharacterSheet.
	Actor npc = makeActor();
	npc.career = std::make_shared<CareerProgression>();
	npc.career->skills["Dodge"]     = 2;
	npc.career->skills["Awareness"] = 0;
	npc.career->talents.insert("Weapon Training (Primitive)");
	npc.career->talents.insert("Sturdy Grip");
	npc.career->traits.push_back("Sturdy");
	npc.career->traits.push_back("Brutal Charge");

	REQUIRE(npc.characterSheet == nullptr); // NPCs never carry a CharacterSheet

	// Keep a copy of the original career pointer to confirm load builds a fresh one.
	CareerProgression* originalCareerPtr = npc.career.get();

	TCODZip zip;
	npc.save(zip);
	zip.saveToFile("__test_actor_npc_rt.sav");

	// Load into a completely fresh Actor (mirrors Engine::deserializeLevel).
	TCODZip loadZip;
	loadZip.loadFromFile("__test_actor_npc_rt.sav");
	Actor loaded = makeActor();
	loaded.load(loadZip);

	std::remove("__test_actor_npc_rt.sav");

	// The career survived the round-trip and was assigned to a fresh instance.
	REQUIRE(loaded.career != nullptr);
	CHECK(loaded.career.get() != originalCareerPtr);

	// The NPC never gains a CharacterSheet on load (re-alias path is player-only).
	CHECK(loaded.characterSheet == nullptr);

	// Skills round-trip (through the Skill_Provider, which reads loaded.career).
	CHECK(hasSkill(&loaded, "Dodge") == true);
	CHECK(getSkillRank(&loaded, "Dodge") == 2);
	CHECK(hasSkill(&loaded, "Awareness") == true);
	CHECK(getSkillRank(&loaded, "Awareness") == 0);
	CHECK(getSkillRank(&loaded, "__absent__") == SKILL_UNTRAINED);

	// Talents round-trip.
	CHECK(hasTalent(&loaded, "Weapon Training (Primitive)") == true);
	CHECK(hasTalent(&loaded, "Sturdy Grip") == true);
	CHECK(hasTalent(&loaded, "__absent_talent__") == false);

	// Traits round-trip in order.
	REQUIRE(loaded.career->traits.size() == 2);
	CHECK(loaded.career->traits[0] == "Sturdy");
	CHECK(loaded.career->traits[1] == "Brutal Charge");
}

// ═══════════════════════════════════════════════════════════════════════════════
// 7.1 — Fixed-profile invariant
// ═══════════════════════════════════════════════════════════════════════════════

// Feature: npc-stat-blocks, Property 11: Fixed-profile invariant
// An NPC's profile is fixed at spawn: read-only Skill_Provider accessors
// (getSkillRank / hasSkill / hasTalent) never mutate the NPC's skill map, talent
// set, trait list, or its xpPool / spentXp (which stay 0 for NPCs). This is a lighter,
// engine-independent invariant: driving full combat requires the engine, so we assert
// the accessors are read-only and the XP fields remain untouched.
// Validates: Requirements 5.1, 5.2, 5.3, 6.2
TEST_CASE("Read-only Skill_Provider accessors never mutate an NPC's fixed profile", "[npc-stat-blocks][pbt]")
{
	rc::check("getSkillRank / hasTalent leave skills, talents, traits, and XP unchanged", [] {
		Actor npc = makeActor();
		npc.career = std::make_shared<CareerProgression>();
		CareerProgression& cp = *npc.career;

		const std::vector<std::string> skillNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
		for (const auto& name : skillNames) {
			cp.skills[name] = *rc::gen::inRange(0, 2);
		}
		const std::vector<std::string> talentNames = *rc::gen::container(0, 6, rc::gen::string(1, 8));
		for (const auto& name : talentNames) {
			cp.talents.insert(name);
		}
		cp.traits = *rc::gen::container(0, 6, rc::gen::string(1, 8));

		// NPCs never earn/spend XP.
		cp.xpPool = 0;
		cp.spentXp = 0;

		// Snapshot the profile before any accessor calls.
		const auto skillsBefore  = cp.skills;
		const auto talentsBefore = cp.talents;
		const auto traitsBefore  = cp.traits;
		const int xpBefore       = cp.xpPool;
		const int spentBefore    = cp.spentXp;

		// Exercise the read-only accessors, including present, absent, and edge names.
		(void)getSkillRank(&npc, "Dodge");
		(void)hasSkill(&npc, "Dodge");
		(void)hasTalent(&npc, "Weapon Training (Primitive)");
		for (const auto& name : skillNames) {
			(void)getSkillRank(&npc, name);
			(void)hasSkill(&npc, name);
		}
		for (const auto& name : talentNames) {
			(void)hasTalent(&npc, name);
		}

		// Nothing changed.
		RC_ASSERT(cp.skills == skillsBefore);
		RC_ASSERT(cp.talents == talentsBefore);
		RC_ASSERT(cp.traits == traitsBefore);
		RC_ASSERT(cp.xpPool == xpBefore);
		RC_ASSERT(cp.spentXp == spentBefore);
		RC_ASSERT(cp.xpPool == 0);
		RC_ASSERT(cp.spentXp == 0);
	});
}
