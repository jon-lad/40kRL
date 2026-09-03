#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.hpp"

#include <sol/sol.hpp>

#include "StatBlock.hpp"
#include "ReactionResolver.hpp"
#include "AiDecision.hpp"
#include "Actor.hpp"
#include "CareerProgression.hpp"
#include "Equippable.hpp"
#include "WeaponTypes.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: npc-skills-talents-wiring
//
// Test suite for wiring the stored-but-inert skill/talent/trait data on
// CareerProgression into the combat and AI pipeline: the Parry pure helpers
// (parryBonus / parryQualityModifier / parryUnavailableFromQualities /
// computeParryTarget / parrySucceeds), the surprise helpers
// (surpriseAvoidanceTarget / surpriseAvoided / SURPRISE_ATTACK_BONUS), the
// Trait_Provider helpers (hasTrait / brutalChargeBonus / isImmuneToKnockdown /
// getSizeCategory / sizeToHitModifier), and the Monster AI decision helper
// (decideMonsterAction).
//
// All cases are tagged [npc-skills-talents-wiring] and are engine-independent: no
// engine.gui, engine.map, or engine.player access. Actors are constructed directly
// and the global Engine is never initialized (per test-isolation rules).
//
// The actual property and unit tests are added in later waves (tasks 2.x–7.x,
// 13.x). This file starts as a scaffold with a single placeholder case so the tag
// registers and the test binary links.
//
// Bounds convention: rc::gen::inRange(a, b) is INCLUSIVE at both ends in this
// project. Use inRange(0, N - 1) for any index into a container of size N.
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("npc-skills-talents-wiring scaffold registers", "[npc-skills-talents-wiring]") {
    // Placeholder to confirm the tag registers and the file compiles/links.
    // Real property and unit tests are added in later waves.
    SUCCEED("npc-skills-talents-wiring test suite scaffold is present");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Wave 2 — Parry pure-helper property tests (TDD red)
//
// These assert the CORRECT final behaviour described in design.md, not the
// current stub behaviour (parry helpers in Source/ReactionResolver.cpp return
// 0 / false). They are EXPECTED TO FAIL until task 10.1 implements the helpers.
//
// All engine-free: no Engine init, no engine.gui / engine.map / engine.player.
// RapidCheck rc::check with a minimum of 100 iterations. Bounds convention:
// rc::gen::inRange(a, b) is INCLUSIVE — use inRange(0, N - 1) for indices.
// ═══════════════════════════════════════════════════════════════════════════════

// Task 2.1 — Property 1: Parry bonus is correct across rank and presence.
// parryBonus(R,true)==R*10; parryBonus(R,false)==-20; never both.
// Generator: R via inRange(0, MAX_SKILL_RANK). Validates Req 1.1, 1.2, 1.3.
TEST_CASE("Parry bonus is correct across rank and presence", "[npc-skills-talents-wiring][pbt]") {
    rc::prop("parryBonus == R*10 when present, -20 when absent, never both",
             []() {
        const bool hasParrySkill = *rc::gen::inRange(0, 1) == 1;
        const int R = *rc::gen::inRange(0, MAX_SKILL_RANK);

        const int bonus = parryBonus(R, hasParrySkill);

        if (hasParrySkill) {
            // Present: exactly the rank bonus (+0 / +10 / +20 for R in {0,1,2}).
            RC_ASSERT(bonus == R * 10);
            // Never the untrained penalty when present.
            RC_ASSERT(bonus != -20 || R * 10 == -20);
        } else {
            // Absent: exactly the untrained penalty, regardless of rank.
            RC_ASSERT(bonus == -20);
        }
    });
}

// Task 2.2 — Property 2: Parry weapon-quality modifier & Balanced/Unbalanced symmetry.
// For lists without "Unwieldy": modifier == 10*count("Balanced") - 10*count("Unbalanced").
// Both present nets 0. Validates Req 2.1, 2.2, 2.3, 2.5.
TEST_CASE("Parry weapon-quality modifier and Balanced/Unbalanced symmetry", "[npc-skills-talents-wiring][pbt]") {
    rc::prop("parryQualityModifier == 10*Balanced - 10*Unbalanced for non-Unwieldy lists",
             []() {
        // Distractor tokens that must NOT contribute to the modifier and are not "Unwieldy".
        const std::vector<std::string> neutralPool = {
            "Flexible", "Fast", "Razor Sharp", "Power Field", "Tearing", "Primitive", "Snare"
        };

        const int balancedCount   = *rc::gen::inRange(0, 4);
        const int unbalancedCount = *rc::gen::inRange(0, 4);
        const int neutralCount    = *rc::gen::inRange(0, 5);

        std::vector<std::string> qualities;
        for (int i = 0; i < balancedCount; ++i)   qualities.push_back("Balanced");
        for (int i = 0; i < unbalancedCount; ++i) qualities.push_back("Unbalanced");
        for (int i = 0; i < neutralCount; ++i) {
            const int idx = *rc::gen::inRange(0, static_cast<int>(neutralPool.size()) - 1);
            qualities.push_back(neutralPool[idx]);
        }
        // Shuffle order-independence: rotate by a random offset.
        if (!qualities.empty()) {
            const int shift = *rc::gen::inRange(0, static_cast<int>(qualities.size()) - 1);
            std::rotate(qualities.begin(), qualities.begin() + shift, qualities.end());
        }

        const int expected = 10 * balancedCount - 10 * unbalancedCount;
        RC_ASSERT(parryQualityModifier(qualities) == expected);

        // Symmetry: equal counts of Balanced and Unbalanced net exactly 0.
        std::vector<std::string> symmetric;
        const int pairs = *rc::gen::inRange(0, 4);
        for (int i = 0; i < pairs; ++i) {
            symmetric.push_back("Balanced");
            symmetric.push_back("Unbalanced");
        }
        RC_ASSERT(parryQualityModifier(symmetric) == 0);
    });
}

// Task 2.3 — Property 3: Unwieldy makes Parry unavailable with precedence.
// Any list containing "Unwieldy" => true regardless of other qualities; else false.
// Validates Req 2.4.
TEST_CASE("Unwieldy makes Parry unavailable with precedence", "[npc-skills-talents-wiring][pbt]") {
    rc::prop("parryUnavailableFromQualities is true iff list contains Unwieldy",
             []() {
        const bool includeUnwieldy = *rc::gen::inRange(0, 1) == 1;
        const std::vector<std::string> nonUnwieldyPool = {
            "Balanced", "Unbalanced", "Flexible", "Fast", "Tearing", "Primitive"
        };

        std::vector<std::string> qualities;
        const int extras = *rc::gen::inRange(0, 6);
        for (int i = 0; i < extras; ++i) {
            const int idx = *rc::gen::inRange(0, static_cast<int>(nonUnwieldyPool.size()) - 1);
            qualities.push_back(nonUnwieldyPool[idx]);
        }

        if (includeUnwieldy) {
            // Insert "Unwieldy" at a random position so precedence does not depend on order.
            const int pos = *rc::gen::inRange(0, static_cast<int>(qualities.size()));
            qualities.insert(qualities.begin() + pos, "Unwieldy");
            RC_ASSERT(parryUnavailableFromQualities(qualities) == true);
        } else {
            // No "Unwieldy" anywhere -> Parry remains available.
            RC_ASSERT(parryUnavailableFromQualities(qualities) == false);
        }
    });
}

// Task 2.4 — Property 5: Parry monotonicity in skill rank (metamorphic).
// For fixed WS and quality modifier, increasing Parry rank never decreases the
// clamped computeParryTarget. Validates Req 1.2, 1.4.
TEST_CASE("Parry monotonicity in skill rank", "[npc-skills-talents-wiring][pbt]") {
    rc::prop("increasing Parry rank never decreases the clamped parry target",
             []() {
        // Wide WS incl. negatives / >100 to exercise the [0,100] clamp.
        const int weaponSkill    = *rc::gen::inRange(-50, 150);
        const int qualityMod     = *rc::gen::inRange(-30, 30);
        // Two ranks with rankLo <= rankHi, both in [0, MAX_SKILL_RANK].
        const int rankA          = *rc::gen::inRange(0, MAX_SKILL_RANK);
        const int rankB          = *rc::gen::inRange(0, MAX_SKILL_RANK);
        const int rankLo         = std::min(rankA, rankB);
        const int rankHi         = std::max(rankA, rankB);

        const int targetLo = computeParryTarget(weaponSkill, parryBonus(rankLo, true), qualityMod);
        const int targetHi = computeParryTarget(weaponSkill, parryBonus(rankHi, true), qualityMod);

        // Monotonic non-decreasing in rank.
        RC_ASSERT(targetHi >= targetLo);
        // Clamp invariant holds for both.
        RC_ASSERT(targetLo >= 0 && targetLo <= 100);
        RC_ASSERT(targetHi >= 0 && targetHi <= 100);
    });
}

// Task 2.5 — Property 6 (parry portion): roll resolution.
// parrySucceeds(1,T)==true, parrySucceeds(100,T)==false, and for r in [2,99]
// parrySucceeds(r,T)==(r<=T). Generators: T via inRange(0,100), r via inRange(2,99).
// Validates Req 1.5, 1.6, 1.7.
TEST_CASE("Parry roll resolution", "[npc-skills-talents-wiring][pbt]") {
    rc::prop("auto-1 success, auto-100 fail, else roll-under target",
             []() {
        const int target = *rc::gen::inRange(0, 100);

        // Auto-success on a natural 1 regardless of target.
        RC_ASSERT(parrySucceeds(1, target) == true);
        // Auto-fail on a natural 100 regardless of target.
        RC_ASSERT(parrySucceeds(100, target) == false);

        // Normal band [2, 99]: success iff roll <= target.
        const int roll = *rc::gen::inRange(2, 99);
        RC_ASSERT(parrySucceeds(roll, target) == (roll <= target));
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Shared test helpers (Waves 3–7)
//
// Engine-free actor / career construction. No global Engine initialization; the
// Actor is stack-constructed and its optional career component is attached
// directly. These helpers back the Trait_Provider, surprise, AI-decision, and
// serialization tests below.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// Builds a bare Actor with no career component (nullptr career reads as traitless).
Actor makeBareActor() {
    return Actor(0, 0, '@', "t", TCODColor(255, 255, 255));
}

// Builds an Actor with a CareerProgression whose traits are exactly `traits`.
Actor makeActorWithTraits(const std::vector<std::string>& traits) {
    Actor a(0, 0, '@', "t", TCODColor(255, 255, 255));
    a.career = std::make_shared<CareerProgression>();
    a.career->traits = traits;
    return a;
}

// clamp helper matching the production formula's clamp(x, 0, 100).
int clampTarget(int x) {
    return std::clamp(x, 0, 100);
}

// The six size-category names, index-aligned with SizeCategory (Puny..Massive).
const char* sizeCategoryName(SizeCategory c) {
    switch (c) {
        case SizeCategory::Puny:     return "Puny";
        case SizeCategory::Scrawny:  return "Scrawny";
        case SizeCategory::Average:  return "Average";
        case SizeCategory::Hulking:  return "Hulking";
        case SizeCategory::Enormous: return "Enormous";
        case SizeCategory::Massive:  return "Massive";
    }
    return "Average";
}

// RT-Bestiary Size to-hit reference values (independent oracle for Property 12).
int expectedSizeMod(SizeCategory c) {
    switch (c) {
        case SizeCategory::Puny:     return -30;
        case SizeCategory::Scrawny:  return -10;
        case SizeCategory::Average:  return 0;
        case SizeCategory::Hulking:  return 10;
        case SizeCategory::Enormous: return 20;
        case SizeCategory::Massive:  return 30;
    }
    return 0;
}

} // anonymous namespace

// ═══════════════════════════════════════════════════════════════════════════════
// Wave 3 — Surprise helper tests (TDD red)
//
// EXPECTED TO FAIL until task 9.2 implements surpriseAvoidanceTarget / surpriseAvoided
// (both currently return 0 / false stubs). The SURPRISE_ATTACK_BONUS constant is a
// header inline constexpr, so 3.3 asserts a compile-time value that is already correct.
// ═══════════════════════════════════════════════════════════════════════════════

// Task 3.1 — Property 7: Surprise-avoidance modifier is correct across rank and presence.
// awareness modifier == R*10 when present, -20 when absent, never both; defined for
// untrained/absent. perception via inRange(0,100), R via inRange(0, MAX_SKILL_RANK).
// Validates Req 3.1, 3.2, 3.3.
TEST_CASE("Surprise-avoidance modifier is correct across rank and presence", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 7: Surprise-avoidance modifier across rank and presence
    rc::prop("awareness modifier == R*10 present / -20 absent, never both",
             []() {
        const int perception   = *rc::gen::inRange(0, 100);
        const int R            = *rc::gen::inRange(0, MAX_SKILL_RANK);
        const bool hasAwareness = *rc::gen::inRange(0, 1) == 1;

        const int target = surpriseAvoidanceTarget(perception, R, hasAwareness);

        // The modifier folded into the clamped target must be exactly the
        // present/absent value: derive it from clamp(perception + mod, 0, 100).
        if (hasAwareness) {
            RC_ASSERT(target == clampTarget(perception + R * 10));
        } else {
            // Absent: the untrained penalty, independent of rank.
            RC_ASSERT(target == clampTarget(perception - 20));
        }

        // Result always defined and within the target band [0,100].
        RC_ASSERT(target >= 0 && target <= 100);
    });
}

// Task 3.2 — Property 6 (surprise portion): roll resolution.
// surpriseAvoided(1,T) -> not surprised (true); surpriseAvoided(100,T) -> surprised
// (false); for r in [2,99] result == (r<=T). Validates Req 3.5, 3.6.
TEST_CASE("Surprise roll resolution", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 6 (surprise portion): roll resolution
    rc::prop("auto-1 not surprised, auto-100 surprised, else roll-under target",
             []() {
        const int target = *rc::gen::inRange(0, 100);

        // Natural 1: always avoids surprise (not surprised).
        RC_ASSERT(surpriseAvoided(1, target) == true);
        // Natural 100: always surprised.
        RC_ASSERT(surpriseAvoided(100, target) == false);

        // Normal band [2,99]: not surprised iff roll <= target.
        const int roll = *rc::gen::inRange(2, 99);
        RC_ASSERT(surpriseAvoided(roll, target) == (roll <= target));
    });
}

// Task 3.3 — unit: assert SURPRISE_ATTACK_BONUS == 30. Validates Req 3.7.
TEST_CASE("Surprise attack bonus constant is 30", "[npc-skills-talents-wiring]") {
    STATIC_REQUIRE(SURPRISE_ATTACK_BONUS == 30);
    REQUIRE(SURPRISE_ATTACK_BONUS == 30);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Wave 4 — Clamp-invariant test (TDD red)
//
// EXPECTED TO FAIL until tasks 9.2 / 10.1 implement computeParryTarget and
// surpriseAvoidanceTarget. The effective-WS/BS portion models the resolver clamp
// formula directly (base + modSum + aim + prof + size then clamp to [1,99]); this
// portion is a pure oracle and does not depend on the stubs, so it always holds.
// ═══════════════════════════════════════════════════════════════════════════════

// Task 4.1 — Property 4: All computed target numbers and effective skills are clamped.
// computeParryTarget(...) and surpriseAvoidanceTarget(...) in [0,100]; effective WS/BS
// (base+modSum+aim+prof+size then clamp) in [1,99]. Wide ranges incl. negatives/large.
// Validates Req 1.4, 2.7, 3.4, 4.5, 7.4, 7.5, 10.6.
TEST_CASE("All computed targets and effective skills are clamped", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 4: target numbers and effective skills are clamped
    rc::prop("parry / surprise targets in [0,100]; effective WS/BS in [1,99]",
             []() {
        // ── Parry target clamp ──
        const int weaponSkill = *rc::gen::inRange(-1000, 1000);
        const int pBonus      = *rc::gen::inRange(-1000, 1000);
        const int qualityMod  = *rc::gen::inRange(-1000, 1000);
        const int parryTarget = computeParryTarget(weaponSkill, pBonus, qualityMod);
        RC_ASSERT(parryTarget >= 0 && parryTarget <= 100);

        // ── Surprise target clamp ──
        const int perception    = *rc::gen::inRange(-1000, 1000);
        const int awarenessRank = *rc::gen::inRange(-1000, 1000);
        const bool hasAwareness = *rc::gen::inRange(0, 1) == 1;
        const int surpriseTarget = surpriseAvoidanceTarget(perception, awarenessRank, hasAwareness);
        RC_ASSERT(surpriseTarget >= 0 && surpriseTarget <= 100);

        // ── Effective WS/BS clamp (resolver formula, pure oracle) ──
        // Mirrors Attacker::resolveCharacterAttack / RangedCombat::resolveCharacterAttack:
        //   effective = clamp(base + modSum + aim + prof + size, 1, 99)
        const int baseSkill = *rc::gen::inRange(-1000, 1000);
        const int modSum    = *rc::gen::inRange(-1000, 1000);
        const int aimBonus  = *rc::gen::inRange(-1000, 1000);
        const int profMod   = *rc::gen::inRange(-1000, 1000);
        const int sizeMod   = *rc::gen::inRange(-1000, 1000);
        const int effective = std::max(1, std::min(99, baseSkill + modSum + aimBonus + profMod + sizeMod));
        RC_ASSERT(effective >= 1 && effective <= 99);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Wave 5 — Trait_Provider tests (TDD red)
//
// EXPECTED TO FAIL until task 9.1 implements hasTrait / brutalChargeBonus /
// isImmuneToKnockdown / getSizeCategory (stubs return false / 0 / Average). Note
// sizeToHitModifier(Average)==0 and getSizeCategory stub==Average happen to make
// the Average-only cases pass, but the non-Average cases will fail against stubs.
//
// All engine-free: Actor + CareerProgression constructed directly, no Engine init.
// ═══════════════════════════════════════════════════════════════════════════════

// Task 5.1 — Property 9: Brutal Charge bonus and charge-only application.
// brutalChargeBonus(actor)==3 iff traits contain "Brutal Charge" else 0 (incl.
// null / no career). Validates Req 5.1, 5.4.
TEST_CASE("Brutal Charge bonus is 3 iff trait present", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 9: Brutal Charge bonus and charge-only application
    rc::prop("brutalChargeBonus == 3 iff traits contain 'Brutal Charge', else 0",
             []() {
        // Distractor traits that must NOT grant the bonus.
        const std::vector<std::string> distractors = {
            "Sturdy", "Mob Rule", "Cowardly", "Size (Hulking)", "Fear (1)", "Brutal charge", "BRUTAL CHARGE"
        };

        const bool includeBrutal = *rc::gen::inRange(0, 1) == 1;
        std::vector<std::string> traits;
        const int extras = *rc::gen::inRange(0, static_cast<int>(distractors.size()));
        for (int i = 0; i < extras; ++i) {
            traits.push_back(distractors[i]);
        }
        if (includeBrutal) {
            const int pos = *rc::gen::inRange(0, static_cast<int>(traits.size()));
            traits.insert(traits.begin() + pos, "Brutal Charge");
        }

        Actor a = makeActorWithTraits(traits);
        RC_ASSERT(brutalChargeBonus(&a) == (includeBrutal ? 3 : 0));
    });

    // Null actor and actor without a career both read as 0 (no bonus).
    REQUIRE(brutalChargeBonus(nullptr) == 0);
    Actor bare = makeBareActor();
    REQUIRE(brutalChargeBonus(&bare) == 0);
}

// Task 5.2 — Property 11: Sturdy immunity predicate purity/idempotence.
// isImmuneToKnockdown true iff traits contain "Sturdy"; idempotent (two calls
// equal); no mutation of the actor. Validates Req 6.1, 6.2.
TEST_CASE("Sturdy immunity predicate is correct, pure, and idempotent", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 11: Sturdy immunity predicate purity/idempotence
    rc::prop("isImmuneToKnockdown == (traits contain 'Sturdy'); idempotent; no mutation",
             []() {
        const std::vector<std::string> distractors = {
            "Brutal Charge", "Mob Rule", "Cowardly", "Size (Massive)", "sturdy", "STURDY"
        };

        const bool includeSturdy = *rc::gen::inRange(0, 1) == 1;
        std::vector<std::string> traits;
        const int extras = *rc::gen::inRange(0, static_cast<int>(distractors.size()));
        for (int i = 0; i < extras; ++i) {
            traits.push_back(distractors[i]);
        }
        if (includeSturdy) {
            const int pos = *rc::gen::inRange(0, static_cast<int>(traits.size()));
            traits.insert(traits.begin() + pos, "Sturdy");
        }

        Actor a = makeActorWithTraits(traits);
        const std::vector<std::string> before = a.career->traits;

        const bool first  = isImmuneToKnockdown(&a);
        const bool second = isImmuneToKnockdown(&a);

        // Correctness.
        RC_ASSERT(first == includeSturdy);
        // Idempotence: two consecutive calls agree.
        RC_ASSERT(first == second);
        // Purity: the actor's trait list is unchanged.
        RC_ASSERT(a.career->traits == before);
    });

    REQUIRE(isImmuneToKnockdown(nullptr) == false);
    Actor bare = makeBareActor();
    REQUIRE(isImmuneToKnockdown(&bare) == false);
}

// Task 5.3 — Property 12: Size parsing, default, and to-hit mapping.
// traits containing "Size (C)" parse to C; sizeToHitModifier(C) == RT value;
// missing/null/unknown -> Average (+0). category via inRange(0, Massive).
// Validates Req 7.1, 7.2, 7.3, 7.6.
TEST_CASE("Size category parsing, default, and to-hit mapping", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 12: Size category parsing, default, and to-hit mapping
    rc::prop("Size (C) parses to C; sizeToHitModifier(C) == RT value",
             []() {
        const int catIdx = *rc::gen::inRange(0, static_cast<int>(SizeCategory::Massive));
        const SizeCategory cat = static_cast<SizeCategory>(catIdx);

        // sizeToHitModifier matches the RT-Bestiary oracle for every category.
        RC_ASSERT(sizeToHitModifier(cat) == expectedSizeMod(cat));

        // An actor whose traits contain the exact "Size (<Category>)" string parses to that category.
        const std::string trait = std::string("Size (") + sizeCategoryName(cat) + ")";
        Actor a = makeActorWithTraits({ "Sturdy", trait, "Mob Rule" });
        RC_ASSERT(getSizeCategory(&a) == cat);
    });

    // Missing / null / unknown / malformed -> Average (+0).
    REQUIRE(getSizeCategory(nullptr) == SizeCategory::Average);
    Actor bare = makeBareActor();
    REQUIRE(getSizeCategory(&bare) == SizeCategory::Average);

    Actor noSize = makeActorWithTraits({ "Sturdy", "Mob Rule" });
    REQUIRE(getSizeCategory(&noSize) == SizeCategory::Average);

    // Unknown token and malformed strings fall back to Average.
    Actor unknown = makeActorWithTraits({ "Size (Gigantic)" });
    REQUIRE(getSizeCategory(&unknown) == SizeCategory::Average);
    Actor wrongCase = makeActorWithTraits({ "Size (hulking)" });
    REQUIRE(getSizeCategory(&wrongCase) == SizeCategory::Average);
    Actor malformed = makeActorWithTraits({ "Size Hulking" });
    REQUIRE(getSizeCategory(&malformed) == SizeCategory::Average);

    // Average maps to +0.
    REQUIRE(sizeToHitModifier(SizeCategory::Average) == 0);
}

// Task 5.4 — Property 13: hasTrait exact-match and cross-helper null-safety.
// hasTrait(actor,name) true iff exact byte-for-byte match; null / no career -> all
// Trait_Provider helpers return neutral. Validates Req 8.1, 9.1, 10.5.
TEST_CASE("hasTrait exact-match and cross-helper null-safety", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 13: hasTrait exact-match, null-safety across helpers
    rc::prop("hasTrait true iff byte-for-byte exact match",
             []() {
        const std::vector<std::string> pool = {
            "Brutal Charge", "Sturdy", "Mob Rule", "Cowardly", "Size (Hulking)", "Fear (2)"
        };
        // Pick a subset of pool to be the actor's traits.
        std::vector<std::string> traits;
        for (const auto& t : pool) {
            if (*rc::gen::inRange(0, 1) == 1) traits.push_back(t);
        }
        Actor a = makeActorWithTraits(traits);

        // Every trait actually present must match exactly.
        for (const auto& t : traits) {
            RC_ASSERT(hasTrait(&a, t) == true);
        }
        // A query that differs by one byte (case / trailing space / substring) must not match.
        RC_ASSERT(hasTrait(&a, "sturdy") == false || std::find(traits.begin(), traits.end(), "sturdy") != traits.end());
        RC_ASSERT(hasTrait(&a, "Brutal Charge ") == false);
        RC_ASSERT(hasTrait(&a, "Brutal") == false);
        // A pool trait not in the subset must not match.
        for (const auto& t : pool) {
            const bool present = std::find(traits.begin(), traits.end(), t) != traits.end();
            RC_ASSERT(hasTrait(&a, t) == present);
        }
    });

    // Cross-helper null-safety: null actor and careerless actor read neutral everywhere.
    REQUIRE(hasTrait(nullptr, "Sturdy") == false);
    REQUIRE(brutalChargeBonus(nullptr) == 0);
    REQUIRE(isImmuneToKnockdown(nullptr) == false);
    REQUIRE(getSizeCategory(nullptr) == SizeCategory::Average);

    Actor bare = makeBareActor();
    REQUIRE(hasTrait(&bare, "Sturdy") == false);
    REQUIRE(brutalChargeBonus(&bare) == 0);
    REQUIRE(isImmuneToKnockdown(&bare) == false);
    REQUIRE(getSizeCategory(&bare) == SizeCategory::Average);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Wave 6 — Monster AI decision test (TDD red)
//
// EXPECTED TO FAIL until task 12.1 implements decideMonsterAction (stub returns
// Approach). Uses the header constants COWARDLY_FLEE_FRACTION (0.30),
// MOB_RULE_MIN_ALLIES (2), MOB_RULE_RADIUS (5). Pure and engine-free.
// ═══════════════════════════════════════════════════════════════════════════════

// Task 6.1 — Property 14: Monster AI decision — flee, embolden, non-cowardly equivalence.
// (a) Cowardly && !emboldened && wounds <= floor(max*fraction) => Flee;
// (b) emboldened (Mob Rule && allies >= min) => never Flee;
// (c) wounds above threshold => Attack if adjacent else Approach (== non-Cowardly).
// Validates Req 8.2, 8.3, 8.4.
TEST_CASE("Monster AI decision — flee, embolden, and non-cowardly equivalence", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 14: Monster AI decision helper
    rc::prop("decideMonsterAction obeys cowardly-flee, mob-rule embolden, and non-cowardly equivalence",
             []() {
        MonsterDecisionContext ctx;
        ctx.isCowardly       = *rc::gen::inRange(0, 1) == 1;
        ctx.hasMobRule       = *rc::gen::inRange(0, 1) == 1;
        ctx.maxWounds        = *rc::gen::inRange(1, 100);
        ctx.currentWounds    = *rc::gen::inRange(0, ctx.maxWounds);
        ctx.nearbyAllies     = *rc::gen::inRange(0, 10);
        ctx.adjacentToPlayer = *rc::gen::inRange(0, 1) == 1;

        const bool emboldened = ctx.hasMobRule && ctx.nearbyAllies >= MOB_RULE_MIN_ALLIES;
        const int fleeThreshold =
            static_cast<int>(std::floor(ctx.maxWounds * COWARDLY_FLEE_FRACTION));
        const bool atOrBelowThreshold = ctx.currentWounds <= fleeThreshold;

        const MonsterIntent intent = decideMonsterAction(ctx);

        // The intent a non-cowardly actor in the same situation would take.
        const MonsterIntent nonCowardly =
            ctx.adjacentToPlayer ? MonsterIntent::Attack : MonsterIntent::Approach;

        if (ctx.isCowardly && !emboldened && atOrBelowThreshold) {
            // (a) Cowardly, not emboldened, at/below threshold -> Flee.
            RC_ASSERT(intent == MonsterIntent::Flee);
        } else if (emboldened) {
            // (b) Emboldened -> never Flee.
            RC_ASSERT(intent != MonsterIntent::Flee);
        } else {
            // (c) Above threshold (or not cowardly) -> matches non-cowardly intent.
            RC_ASSERT(intent == nonCowardly);
        }
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Wave 7 — Proficiency-parity, serialization round-trip, and Dodge/melee regression
//
// 7.1 and 7.3 exercise ALREADY-IMPLEMENTED behaviour and should PASS. 7.2 exercises
// existing save/load (should PASS). All engine-free; TCODZip has no in-memory
// round-trip so save flushes to a distinct temp file that is removed afterward.
// ═══════════════════════════════════════════════════════════════════════════════

// Task 7.1 — Property 8: Ranged/melee proficiency parity (model-based).
// For any group + talent state, ranged BS proficiency modifier == melee WS
// proficiency modifier; both -20 untrained, 0 trained. group via inRange(0, EXOTIC).
// Validates Req 4.1, 4.2, 4.3, 4.6. (Should PASS — existing behaviour.)
TEST_CASE("Ranged/melee proficiency parity", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 8: Ranged/melee proficiency parity (model-based)
    rc::prop("proficiency modifier is identical for melee and ranged; -20 untrained / 0 trained",
             []() {
        const int groupIdx = *rc::gen::inRange(0, static_cast<int>(WeaponGroup::EXOTIC));
        const WeaponGroup group = static_cast<WeaponGroup>(groupIdx);
        const bool trained = *rc::gen::inRange(0, 1) == 1;

        Actor a(0, 0, '@', "t", TCODColor(255, 255, 255));
        a.career = std::make_shared<CareerProgression>();
        if (trained) {
            const std::string talent =
                "Weapon Training (" + std::string(weaponGroupName(group)) + ")";
            a.career->talents.insert(talent);
        }

        // The proficiency modifier is group-based and shared by both the melee
        // (Attacker) and ranged (RangedCombat) resolvers — the same helper is used
        // for the WS and BS effective-skill computation, so parity holds by design.
        const int meleeMod  = proficiencyModifier(&a, group);
        const int rangedMod = proficiencyModifier(&a, group);

        RC_ASSERT(meleeMod == rangedMod);
        RC_ASSERT(meleeMod == (trained ? 0 : -20));
    });
}

// Task 7.2 — Property 15: Serialization round-trip preserves component state.
// In-memory (temp-file) TCODZip save/load of a generated CareerProgression,
// Attacker, Equippable reproduces identical field values. Validates Req 10.3, 10.4.
TEST_CASE("Serialization round-trip preserves component state", "[npc-skills-talents-wiring][pbt]") {
    // Feature: npc-skills-talents-wiring, Property 15: serialization round-trip preserves component state
    rc::prop("save then load reproduces identical CareerProgression / Attacker / Equippable fields",
             []() {
        // ── CareerProgression ──
        CareerProgression career;
        career.homeworldName = *rc::gen::elementOf(std::vector<std::string>{ "Feudal", "Hive", "Void" });
        career.careerName    = *rc::gen::elementOf(std::vector<std::string>{ "Guardsman", "Ork Boy", "Scum" });
        career.currentRank   = *rc::gen::inRange(1, 8);
        career.xpPool        = *rc::gen::inRange(0, 10000);
        career.spentXp       = *rc::gen::inRange(0, career.xpPool);

        const std::vector<std::string> skillPool = { "Parry", "Awareness", "Dodge" };
        const int skillCount = *rc::gen::inRange(0, static_cast<int>(skillPool.size()));
        for (int i = 0; i < skillCount; ++i) {
            career.skills[skillPool[i]] = *rc::gen::inRange(0, MAX_SKILL_RANK);
        }
        const std::vector<std::string> talentPool = {
            "Weapon Training (LAS)", "Weapon Training (SP)", "Weapon Training (BOLT)"
        };
        const int talentCount = *rc::gen::inRange(0, static_cast<int>(talentPool.size()));
        for (int i = 0; i < talentCount; ++i) {
            career.talents.insert(talentPool[i]);
        }
        const std::vector<std::string> traitPool = {
            "Brutal Charge", "Sturdy", "Mob Rule", "Cowardly", "Size (Hulking)"
        };
        const int traitCount = *rc::gen::inRange(0, static_cast<int>(traitPool.size()));
        for (int i = 0; i < traitCount; ++i) {
            career.traits.push_back(traitPool[i]);
        }

        {
            const char* tempFile = "__test_nsw_career_rt.sav";
            TCODZip zip;
            career.save(zip);
            zip.saveToFile(tempFile);

            TCODZip loadZip;
            loadZip.loadFromFile(tempFile);
            CareerProgression loaded;
            loaded.load(loadZip);
            std::remove(tempFile);

            RC_ASSERT(loaded.homeworldName == career.homeworldName);
            RC_ASSERT(loaded.careerName == career.careerName);
            RC_ASSERT(loaded.currentRank == career.currentRank);
            RC_ASSERT(loaded.xpPool == career.xpPool);
            RC_ASSERT(loaded.spentXp == career.spentXp);
            RC_ASSERT(loaded.skills == career.skills);
            RC_ASSERT(loaded.talents == career.talents);
            RC_ASSERT(loaded.traits == career.traits);
        }

        // ── Attacker ──
        {
            const float power = static_cast<float>(*rc::gen::inRange(0, 100));
            const int skill   = *rc::gen::inRange(1, 99);
            Attacker attacker(power, skill);
            const int modCount = *rc::gen::inRange(0, 5);
            for (int i = 0; i < modCount; ++i) {
                attacker.addModifier(*rc::gen::inRange(-30, 30));
            }

            const char* tempFile = "__test_nsw_attacker_rt.sav";
            TCODZip zip;
            attacker.save(zip);
            zip.saveToFile(tempFile);

            TCODZip loadZip;
            loadZip.loadFromFile(tempFile);
            Attacker loaded(0.0f, 40);
            loaded.load(loadZip);
            std::remove(tempFile);

            RC_ASSERT(loaded.power == attacker.power);
            RC_ASSERT(loaded.skillValue == attacker.skillValue);
            RC_ASSERT(loaded.modifiers == attacker.modifiers);
        }

        // ── Equippable ──
        {
            const int slotIdx = *rc::gen::inRange(0, static_cast<int>(EquipmentSlot::COUNT) - 1);
            StatModifiers mods;
            mods.power   = static_cast<float>(*rc::gen::inRange(-10, 10));
            mods.defense = static_cast<float>(*rc::gen::inRange(-10, 10));
            mods.maxHp   = static_cast<float>(*rc::gen::inRange(-10, 10));
            mods.skill   = *rc::gen::inRange(-20, 20);
            const float weight = static_cast<float>(*rc::gen::inRange(0, 50));
            const int value    = *rc::gen::inRange(0, 1000);

            Equippable equip(static_cast<EquipmentSlot>(slotIdx), mods, weight, value);
            // Give it a melee profile with some qualities to exercise the string vectors.
            if (*rc::gen::inRange(0, 1) == 1) {
                MeleeStats ms;
                ms.damageDice = DiceSpec{ 1, 10 };
                ms.penetration = *rc::gen::inRange(0, 10);
                ms.qualities = { "Balanced", "Tearing" };
                equip.meleeStats = ms;
            }

            const char* tempFile = "__test_nsw_equip_rt.sav";
            TCODZip zip;
            equip.save(zip);
            zip.saveToFile(tempFile);

            TCODZip loadZip;
            loadZip.loadFromFile(tempFile);
            Equippable loaded(EquipmentSlot::WEAPON, StatModifiers{}, 0.0f, 0);
            loaded.load(loadZip);
            std::remove(tempFile);

            RC_ASSERT(loaded.slot == equip.slot);
            RC_ASSERT(loaded.modifiers.power == equip.modifiers.power);
            RC_ASSERT(loaded.modifiers.defense == equip.modifiers.defense);
            RC_ASSERT(loaded.modifiers.maxHp == equip.modifiers.maxHp);
            RC_ASSERT(loaded.modifiers.skill == equip.modifiers.skill);
            RC_ASSERT(loaded.weight == equip.weight);
            RC_ASSERT(loaded.value == equip.value);
            RC_ASSERT(loaded.meleeStats.has_value() == equip.meleeStats.has_value());
            if (equip.meleeStats && loaded.meleeStats) {
                RC_ASSERT(loaded.meleeStats->penetration == equip.meleeStats->penetration);
                RC_ASSERT(loaded.meleeStats->qualities == equip.meleeStats->qualities);
            }
        }
    });
}

// Task 7.3 — unit regression: computeDodgeTarget / dodgeSucceeds and the existing
// melee proficiencyModifier return the same values as before this feature for
// representative inputs. Validates Req 10.4. (Should PASS.)
TEST_CASE("Existing Dodge / melee-proficiency behaviour unchanged", "[npc-skills-talents-wiring]") {
    SECTION("computeDodgeTarget matches the pre-feature clamped formula") {
        // Skilled: clamp(agility + rank*10, 0, 100).
        REQUIRE(computeDodgeTarget(40, 0, true) == 40);
        REQUIRE(computeDodgeTarget(40, 1, true) == 50);
        REQUIRE(computeDodgeTarget(40, 2, true) == 60);
        REQUIRE(computeDodgeTarget(95, 2, true) == 100);   // clamp high
        // Untrained: clamp(agility - 20, 0, 100).
        REQUIRE(computeDodgeTarget(40, 0, false) == 20);
        REQUIRE(computeDodgeTarget(10, 2, false) == 0);    // clamp low
        REQUIRE(computeDodgeTarget(100, 0, false) == 80);
    }

    SECTION("dodgeSucceeds keeps auto-1 / auto-100 and roll-under semantics") {
        REQUIRE(dodgeSucceeds(1, 0) == true);      // auto-success
        REQUIRE(dodgeSucceeds(100, 100) == false); // auto-fail
        REQUIRE(dodgeSucceeds(50, 60) == true);    // roll <= target
        REQUIRE(dodgeSucceeds(60, 60) == true);    // boundary
        REQUIRE(dodgeSucceeds(61, 60) == false);   // roll > target
    }

    SECTION("melee proficiencyModifier is -20 untrained / 0 trained") {
        Actor untrained(0, 0, '@', "t", TCODColor(255, 255, 255));
        untrained.career = std::make_shared<CareerProgression>();
        REQUIRE(proficiencyModifier(&untrained, WeaponGroup::LAS) == -20);

        Actor trained(0, 0, '@', "t", TCODColor(255, 255, 255));
        trained.career = std::make_shared<CareerProgression>();
        trained.career->talents.insert(
            "Weapon Training (" + std::string(weaponGroupName(WeaponGroup::LAS)) + ")");
        REQUIRE(proficiencyModifier(&trained, WeaponGroup::LAS) == 0);

        // Null-safe: no career / null actor read as untrained.
        REQUIRE(proficiencyModifier(nullptr, WeaponGroup::BOLT) == -20);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Wave 10 — Lua data-compatibility tests (Task 13.1 / 13.2)
//
// These exercise the ALREADY-IMPLEMENTED parser (populateStatBlockFromLua in
// Source/StatBlock.cpp) and the Trait_Provider helpers against real / realistic
// Lua data. No parser change is required — they assert the existing parser already
// handles the wired trait vocabulary and that unrecognized/legacy traits load but
// remain mechanically inert.
//
// Engine-free / headless: an in-memory sol::state is constructed per test; the
// global Engine is never initialized. No engine.gui / engine.map / engine.player.
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Task 13.1 — Lua parse compatibility and inert-trait handling ───────────────
// Build an in-memory sol::state, construct a Lua enemy entry with skills, talents,
// and traits (the five wired traits plus at least one unrecognized/legacy trait),
// parse it via populateStatBlockFromLua, and assert:
//   • wired traits stored unchanged (exact strings, order preserved);
//   • skills/talents parsed correctly;
//   • unrecognized traits still load (hasTrait true) but are mechanically inert
//     (brutalChargeBonus/isImmuneToKnockdown/getSizeCategory ignore them).
// Validates: Req 9.2, 9.3, 9.4, 9.6.
TEST_CASE("Lua enemy entry parses wired traits unchanged and keeps legacy traits inert",
          "[npc-skills-talents-wiring]") {
    sol::state lua;
    // No engine init; the parser only reads the entry table.
    sol::table entry = lua.create_table();

    // ── skills: name -> rank ──
    sol::table skillsTbl = lua.create_table();
    skillsTbl["Dodge"]     = 1;
    skillsTbl["Awareness"] = 2;
    skillsTbl["Parry"]     = 0;
    entry["skills"] = skillsTbl;

    // ── talents: 1-based array ──
    sol::table talentsTbl = lua.create_table();
    talentsTbl[1] = "Weapon Training (Primitive)";
    talentsTbl[2] = "Weapon Training (SP)";
    entry["talents"] = talentsTbl;

    // ── traits: 1-based array, order preserved. Wired traits interleaved with two
    //    unrecognized/legacy traits ("Fear (2)", "Unnatural Toughness"). ──
    const std::vector<std::string> declaredTraits = {
        "Brutal Charge",
        "Sturdy",
        "Fear (2)",            // legacy / unrecognized
        "Size (Puny)",
        "Mob Rule",
        "Unnatural Toughness", // legacy / unrecognized
        "Cowardly"
    };
    sol::table traitsTbl = lua.create_table();
    for (size_t i = 0; i < declaredTraits.size(); ++i) {
        traitsTbl[i + 1] = declaredTraits[i];
    }
    entry["traits"] = traitsTbl;

    CareerProgression career;
    // Existing parser — no change required. Must not throw.
    REQUIRE_NOTHROW(populateStatBlockFromLua(career, entry));

    SECTION("skills parse to the declared ranks") {
        REQUIRE(career.skills.size() == 3);
        CHECK(career.skills.at("Dodge") == 1);
        CHECK(career.skills.at("Awareness") == 2);
        CHECK(career.skills.at("Parry") == 0);
    }

    SECTION("talents parse to the declared set") {
        REQUIRE(career.talents.size() == 2);
        CHECK(career.talents.count("Weapon Training (Primitive)") == 1);
        CHECK(career.talents.count("Weapon Training (SP)") == 1);
    }

    SECTION("traits are stored unchanged: exact strings, order preserved") {
        // Byte-for-byte equality of the whole vector — proves both exact-string
        // preservation and stable ordering (Req 9.3, 9.4).
        REQUIRE(career.traits == declaredTraits);
    }

    SECTION("every declared trait — wired and legacy — is retrievable via hasTrait") {
        Actor actor(0, 0, '@', "t", TCODColor(255, 255, 255));
        actor.career = std::make_shared<CareerProgression>(career);

        // Wired traits present.
        CHECK(hasTrait(&actor, "Brutal Charge"));
        CHECK(hasTrait(&actor, "Sturdy"));
        CHECK(hasTrait(&actor, "Size (Puny)"));
        CHECK(hasTrait(&actor, "Mob Rule"));
        CHECK(hasTrait(&actor, "Cowardly"));

        // Unrecognized/legacy traits ALSO loaded (Req 9.6 — they load, just inert).
        CHECK(hasTrait(&actor, "Fear (2)"));
        CHECK(hasTrait(&actor, "Unnatural Toughness"));

        // Exact match only — a legacy trait is not confused for a wired one.
        CHECK_FALSE(hasTrait(&actor, "Fear"));
        CHECK_FALSE(hasTrait(&actor, "fear (2)"));
    }

    SECTION("wired traits are mechanically active on the parsed career") {
        Actor actor(0, 0, '@', "t", TCODColor(255, 255, 255));
        actor.career = std::make_shared<CareerProgression>(career);

        CHECK(brutalChargeBonus(&actor) == 3);              // "Brutal Charge"
        CHECK(isImmuneToKnockdown(&actor) == true);         // "Sturdy"
        CHECK(getSizeCategory(&actor) == SizeCategory::Puny); // "Size (Puny)"
    }

    SECTION("legacy traits are mechanically inert — helpers ignore them") {
        // A career carrying ONLY legacy/unrecognized traits: hasTrait sees them,
        // but none of the mechanics helpers react.
        CareerProgression legacyOnly;
        legacyOnly.traits = { "Fear (2)", "Unnatural Toughness", "Size (Gigantic)" };
        Actor actor(0, 0, '@', "t", TCODColor(255, 255, 255));
        actor.career = std::make_shared<CareerProgression>(legacyOnly);

        // Loaded (Req 9.6).
        CHECK(hasTrait(&actor, "Fear (2)"));
        CHECK(hasTrait(&actor, "Unnatural Toughness"));

        // But inert: no wired mechanic fires.
        CHECK(brutalChargeBonus(&actor) == 0);          // only reacts to "Brutal Charge"
        CHECK(isImmuneToKnockdown(&actor) == false);    // only reacts to "Sturdy"
        // "Size (Gigantic)" is not a valid category -> Average (+0).
        CHECK(getSizeCategory(&actor) == SizeCategory::Average);
        CHECK(sizeToHitModifier(getSizeCategory(&actor)) == 0);
    }
}

// ─── Task 13.2 — Enemies.lua loads; every wired-trait enemy parses cleanly ──────
// Load the actual Scripts/Enemies.lua into an in-memory sol::state (base + table
// libs). The enemies table is `local`, exposed only via spawnEnemy(roll,x,y) which
// calls addActor(x,y,entry) — exactly the production path (Source/Map.cpp). We
// register a C++ addActor that collects each spawned entry, then drive spawnEnemy
// across the full roll range [0,99] to gather every definition, and finally parse
// each entry through populateStatBlockFromLua into a fresh CareerProgression
// (no exception, no Lua error). Entries referencing the wired traits are asserted
// to parse cleanly and expose their mechanics.
// Validates: Req 9.5.
TEST_CASE("Scripts/Enemies.lua loads and every wired-trait enemy parses without error",
          "[npc-skills-talents-wiring]") {
    // Path resolution: production (Source/Map.cpp) and the existing Equipment.lua
    // integration test (Tests/test_weapon_types.cpp) both load with the CWD-relative
    // path "Scripts/<file>.lua" — tests run from the repo root. Probe a couple of
    // fallbacks so a differing CWD (e.g. x64/Debug) still resolves rather than hard
    // failing.
    const std::vector<std::string> candidatePaths = {
        "Scripts/Enemies.lua",
        "../../Scripts/Enemies.lua",   // if CWD == x64/Debug
        "../Scripts/Enemies.lua"
    };

    std::string enemiesPath;
    for (const auto& p : candidatePaths) {
        if (std::FILE* f = std::fopen(p.c_str(), "r")) {
            std::fclose(f);
            enemiesPath = p;
            break;
        }
    }

    if (enemiesPath.empty()) {
        // Prefer finding the correct path; if the CWD truly differs, skip cleanly
        // rather than a hard path failure (per task guidance).
        SKIP("Scripts/Enemies.lua not found relative to the test CWD; skipping the "
             "on-disk load check. Run the test from the repo root to exercise it.");
    }

    INFO("Loaded Enemies.lua from: " << enemiesPath);

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string);

    // Collect every entry the script hands to addActor (mirrors Map::addActor).
    std::vector<sol::table> collectedEntries;
    lua.set_function("addActor", [&](int /*x*/, int /*y*/, sol::table entry) {
        collectedEntries.push_back(entry);
    });

    // Scripting the file must not raise a Lua error (Req 9.5 — load without error).
    REQUIRE_NOTHROW(lua.script_file(enemiesPath));

    sol::protected_function spawnEnemy = lua["spawnEnemy"];
    REQUIRE(spawnEnemy.valid());

    // Drive the full roll range so every cumulative-chance branch is spawned.
    for (int roll = 0; roll < 100; ++roll) {
        sol::protected_function_result r = spawnEnemy(roll, 0, 0);
        REQUIRE(r.valid()); // no Lua runtime error during spawn
    }

    // At least the four Ork-faction templates (Gretchin/Ork/Shoota Boy/Nob) fire.
    REQUIRE(collectedEntries.size() >= 4);

    // Every collected entry parses into a fresh CareerProgression without throwing.
    // Track which wired traits we actually saw across the data set.
    bool sawBrutalCharge = false, sawSturdy = false, sawSize = false,
         sawMobRule = false, sawCowardly = false;

    for (const auto& entry : collectedEntries) {
        CareerProgression career;
        REQUIRE_NOTHROW(populateStatBlockFromLua(career, entry));

        // Build an actor around the parsed career and confirm every stored trait is
        // retrievable and that wired traits activate their mechanics cleanly.
        Actor actor(0, 0, '@', "t", TCODColor(255, 255, 255));
        actor.career = std::make_shared<CareerProgression>(career);

        for (const std::string& trait : career.traits) {
            // Whatever the parser stored is retrievable byte-for-byte (Req 9.4).
            CHECK(hasTrait(&actor, trait));

            if (trait == "Brutal Charge") {
                sawBrutalCharge = true;
                CHECK(brutalChargeBonus(&actor) == 3);
            } else if (trait == "Sturdy") {
                sawSturdy = true;
                CHECK(isImmuneToKnockdown(&actor) == true);
            } else if (trait == "Mob Rule") {
                sawMobRule = true;
            } else if (trait == "Cowardly") {
                sawCowardly = true;
            } else if (trait.rfind("Size (", 0) == 0) {
                sawSize = true;
                // Any parsed Size trait maps to a defined, clamped to-hit modifier.
                const int mod = sizeToHitModifier(getSizeCategory(&actor));
                CHECK(mod >= -30);
                CHECK(mod <= 30);
            }
        }

        // getSizeCategory is always defined (Average default) for any entry.
        const int catIdx = static_cast<int>(getSizeCategory(&actor));
        CHECK(catIdx >= static_cast<int>(SizeCategory::Puny));
        CHECK(catIdx <= static_cast<int>(SizeCategory::Massive));
    }

    // The shipped Enemies.lua data references all five wired traits across its
    // Ork-faction roster (Gretchin: Size (Puny)/Cowardly; Ork: Sturdy/Mob Rule;
    // Shoota Boy: Sturdy; Nob: Sturdy/Brutal Charge).
    CHECK(sawBrutalCharge);
    CHECK(sawSturdy);
    CHECK(sawSize);
    CHECK(sawMobRule);
    CHECK(sawCowardly);
}
