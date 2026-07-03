#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"

// Pull in only what we need — the game headers require libtcod, so we
// include the full main.h here.  The test binary links against libtcod
// and the game object files (excluding main.obj to avoid duplicate main).
#include "main.h"

// ─── Destructible unit tests ──────────────────────────────────────────────────

TEST_CASE("Destructible initialises with full HP", "[destructible]")
{
    MonsterDestructible d(10.0f, 2.0f, "corpse", 5);
    REQUIRE(d.hp    == 10.0f);
    REQUIRE(d.maxHp == 10.0f);
}

TEST_CASE("takeDamage reduces HP by damage minus defence", "[destructible]")
{
    MonsterDestructible d(10.0f, 2.0f, "corpse", 0);

    SECTION("damage greater than defence reduces HP")
    {
        // power=5, defence=2 → effective=3
        // We need an Actor to call takeDamage, use a dummy
        Actor dummy(0, 0, 'x', "dummy", Colors::white);
        dummy.destructible = std::shared_ptr<Destructible>(&d, [](Destructible*){});
        d.takeDamage(&dummy, 5.0f);
        REQUIRE(d.hp == Catch::Approx(7.0f));
    }

    SECTION("damage equal to defence deals zero HP loss")
    {
        Actor dummy(0, 0, 'x', "dummy", Colors::white);
        dummy.destructible = std::shared_ptr<Destructible>(&d, [](Destructible*){});
        d.takeDamage(&dummy, 2.0f);
        REQUIRE(d.hp == Catch::Approx(10.0f));
    }

    SECTION("damage less than defence deals zero HP loss")
    {
        Actor dummy(0, 0, 'x', "dummy", Colors::white);
        dummy.destructible = std::shared_ptr<Destructible>(&d, [](Destructible*){});
        d.takeDamage(&dummy, 1.0f);
        REQUIRE(d.hp == Catch::Approx(10.0f));
    }
}

TEST_CASE("heal caps at maxHp and returns actual amount healed", "[destructible]")
{
    MonsterDestructible d(10.0f, 0.0f, "corpse", 0);
    d.hp = 6.0f;

    SECTION("heal within capacity")
    {
        const float healed = d.heal(3);
        REQUIRE(d.hp    == Catch::Approx(9.0f));
        REQUIRE(healed  == Catch::Approx(3.0f));
    }

    SECTION("heal exceeding maxHp is capped")
    {
        const float healed = d.heal(20);
        REQUIRE(d.hp    == Catch::Approx(10.0f));
        REQUIRE(healed  == Catch::Approx(4.0f)); // only 4 HP were missing
    }

    SECTION("heal when already at maxHp heals nothing")
    {
        d.hp = 10.0f;
        const float healed = d.heal(5);
        REQUIRE(d.hp   == Catch::Approx(10.0f));
        REQUIRE(healed == Catch::Approx(0.0f));
    }
}

TEST_CASE("isDead returns true iff hp <= 0", "[destructible]")
{
    MonsterDestructible d(10.0f, 0.0f, "corpse", 0);
    REQUIRE_FALSE(d.isDead());
    d.hp = 0.0f;
    REQUIRE(d.isDead());
    d.hp = -1.0f;
    REQUIRE(d.isDead());
}

// ─── Property-based tests ─────────────────────────────────────────────────────

TEST_CASE("PBT: heal(amount) never exceeds maxHp", "[destructible][pbt]")
{
    rc::prop("hp after heal is always <= maxHp", []() {
        const float maxHp  = static_cast<float>(*rc::gen::inRange(1, 1000));
        const float startHp = static_cast<float>(*rc::gen::inRange(0, static_cast<int>(maxHp)));
        const float amount  = static_cast<float>(*rc::gen::inRange(0, 2000));

        MonsterDestructible d(maxHp, 0.0f, "c", 0);
        d.hp = startHp;
        d.heal(amount);
        RC_ASSERT(d.hp <= d.maxHp);
    });
}

TEST_CASE("PBT: takeDamage with power <= defence never reduces HP", "[destructible][pbt]")
{
    rc::prop("damage <= defence leaves HP unchanged", []() {
        const float defence = static_cast<float>(*rc::gen::inRange(0, 20));
        const float power   = static_cast<float>(*rc::gen::inRange(0, static_cast<int>(defence)));

        MonsterDestructible d(100.0f, defence, "c", 0);
        Actor dummy(0, 0, 'x', "d", Colors::white);
        dummy.destructible = std::shared_ptr<Destructible>(&d, [](Destructible*){});

        const float hpBefore = d.hp;
        d.takeDamage(&dummy, power);
        RC_ASSERT(d.hp >= hpBefore - 0.001f); // allow tiny float rounding
    });
}
