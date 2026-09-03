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
