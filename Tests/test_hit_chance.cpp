#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <algorithm>
#include <numeric>

// ─── Property-based tests for hit chance system ──────────────────────────────

// Feature: hit-chance-system, Property 1: Skill value clamping
// **Validates: Requirements 1.1**
TEST_CASE("PBT: skill value is clamped to [1, 99]", "[hit-chance][pbt]")
{
    rc::prop("for any int v in [-1000, 1000], Attacker skill == clamp(v, 1, 99)", []() {
        const int v = *rc::gen::inRange(-1000, 1000);
        Attacker a(5.0f, v);

        const int expected = std::max(1, std::min(99, v));
        RC_ASSERT(a.skillValue == expected);
    });
}

// Feature: hit-chance-system, Property 2: Hit roll range
// **Validates: Requirements 2.1**
TEST_CASE("PBT: defaultRoll via rollD100 always returns [1, 100]", "[hit-chance][pbt]")
{
    rc::prop("rollD100() returns values in [1, 100] across 100+ calls", []() {
        Attacker a(5.0f); // default-constructed uses defaultRoll internally
        for (int i = 0; i < 120; ++i) {
            const int roll = a.rollD100();
            RC_ASSERT(roll >= 1);
            RC_ASSERT(roll <= 100);
        }
    });
}

// Feature: hit-chance-system, Property 4: Threshold computation
// **Validates: Requirements 2.4, 5.1, 5.2**
TEST_CASE("PBT: computeThreshold equals clamp(skill + sum(modifiers), 1, 99)", "[hit-chance][pbt]")
{
    rc::prop("threshold = clamp(skill + sum(mods), 1, 99)", []() {
        const int skill = *rc::gen::inRange(1, 99);
        const auto mods = *rc::gen::container<int>(0, 10, rc::gen::inRange(-99, 99));

        Attacker a(5.0f, skill);
        for (int m : mods) {
            a.addModifier(m);
        }

        const int sum = std::accumulate(mods.begin(), mods.end(), 0);
        const int expected = std::max(1, std::min(99, skill + sum));
        RC_ASSERT(a.computeThreshold() == expected);
    });
}
