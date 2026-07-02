#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

// ─── Container unit tests ────────────────────────────────────────────────────

TEST_CASE("Container::add accepts items up to capacity", "[container]")
{
    Container c(2);

    auto a1 = std::make_unique<Actor>(0, 0, '!', "potion", Colors::white);
    auto a2 = std::make_unique<Actor>(0, 0, '!', "scroll", Colors::white);
    auto a3 = std::make_unique<Actor>(0, 0, '!', "bomb",   Colors::white);

    REQUIRE(c.add(std::move(a1)) == true);
    REQUIRE(c.add(std::move(a2)) == true);
    REQUIRE(c.add(std::move(a3)) == false); // full
    REQUIRE(c.inventory.size()   == 2);
}

TEST_CASE("Container with size 0 is unlimited", "[container]")
{
    Container c(0);
    for (int i = 0; i < 50; ++i) {
        auto actor = std::make_unique<Actor>(0, 0, 'x', "item", Colors::white);
        REQUIRE(c.add(std::move(actor)) == true);
    }
    REQUIRE(c.inventory.size() == 50);
}

TEST_CASE("Container::remove erases the correct item by pointer identity", "[container]")
{
    Container c(10);
    auto a1 = std::make_unique<Actor>(0, 0, '!', "keep",   Colors::white);
    auto a2 = std::make_unique<Actor>(0, 0, '!', "remove", Colors::white);

    Actor* rawA2 = a2.get();
    c.add(std::move(a1));
    c.add(std::move(a2));

    REQUIRE(c.inventory.size() == 2);
    c.remove(rawA2);
    REQUIRE(c.inventory.size() == 1);
    REQUIRE(c.inventory.front()->name == "keep");
}

// ─── Property-based tests ─────────────────────────────────────────────────────

TEST_CASE("PBT: inventory size never exceeds capacity", "[container][pbt]")
{
    rc::prop("add never exceeds declared capacity", []() {
        const int capacity = *rc::gen::inRange(1, 30);
        const int attempts = *rc::gen::inRange(1, 50);

        Container c(capacity);
        for (int i = 0; i < attempts; ++i) {
            c.add(std::make_unique<Actor>(0, 0, 'x', "i", Colors::white));
        }
        RC_ASSERT(static_cast<int>(c.inventory.size()) <= capacity);
    });
}
