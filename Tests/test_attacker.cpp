#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

// ─── Attacker unit tests ─────────────────────────────────────────────────────
// These tests verify the damage formula: effective = power - defence
// They do not call Attacker::attack (which calls engine.gui) — they test
// the formula via Destructible::takeDamage directly.

TEST_CASE("Damage formula: power - defence", "[attacker]")
{
    SECTION("standard hit")
    {
        MonsterDestructible d(20.0f, 3.0f, "c", 0);
        Actor dummy(0, 0, 'x', "d", TCOD_white);
        dummy.destructible = std::shared_ptr<Destructible>(&d, [](Destructible*){});

        d.takeDamage(&dummy, 8.0f); // effective = 8 - 3 = 5
        REQUIRE(d.hp == Catch::Approx(15.0f));
    }

    SECTION("blocked hit (power <= defence)")
    {
        MonsterDestructible d(20.0f, 5.0f, "c", 0);
        Actor dummy(0, 0, 'x', "d", TCOD_white);
        dummy.destructible = std::shared_ptr<Destructible>(&d, [](Destructible*){});

        d.takeDamage(&dummy, 3.0f); // effective = 3 - 5 = -2 → 0
        REQUIRE(d.hp == Catch::Approx(20.0f));
    }
}

// ─── Property-based tests ─────────────────────────────────────────────────────

TEST_CASE("PBT: effective damage is always non-negative", "[attacker][pbt]")
{
    rc::prop("power - defence >= 0 when power > defence, else 0 damage", []() {
        const float power   = *rc::gen::inRange(0, 50);
        const float defence = *rc::gen::inRange(0, 50);
        const float maxHp   = 1000.0f;

        MonsterDestructible d(maxHp, defence, "c", 0);
        Actor dummy(0, 0, 'x', "d", TCOD_white);
        dummy.destructible = std::shared_ptr<Destructible>(&d, [](Destructible*){});

        const float hpBefore = d.hp;
        d.takeDamage(&dummy, power);
        const float hpAfter = d.hp;

        RC_ASSERT(hpAfter <= hpBefore);          // HP can only decrease
        RC_ASSERT(hpAfter >= hpBefore - (power - std::min(power, defence)));
    });
}
