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
