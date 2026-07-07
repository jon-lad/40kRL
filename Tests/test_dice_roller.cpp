#include "lib/catch_amalgamated.hpp"
#include "DiceRoller.h"

#include <random>
#include <string>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: rogue-trader-melee-combat — DiceRoller Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 13: DiceRoller NdX round-trip ──────────────────────────────────
// **Validates: Requirements 12.4**
//
// For any valid DiceSpec (count in [1, 9], sides in [1, 100]), formatting to
// string then parsing back SHALL produce an equivalent DiceSpec.

TEST_CASE("Property 13: DiceRoller NdX round-trip", "[pbt][dice]") {
    std::mt19937 rng(42); // seeded for reproducibility
    std::uniform_int_distribution<int> countDist(1, 9);
    std::uniform_int_distribution<int> sidesDist(1, 100);

    for (int i = 0; i < 100; i++) {
        DiceSpec original{ countDist(rng), sidesDist(rng) };
        std::string formatted = DiceRoller::format(original);
        auto parsed = DiceRoller::parse(formatted);
        REQUIRE(parsed.has_value());
        CHECK(parsed->count == original.count);
        CHECK(parsed->sides == original.sides);
    }
}

// ─── Property 14: DiceRoller roll bounds ─────────────────────────────────────
// **Validates: Requirements 12.2**
//
// For any valid DiceSpec with count N and sides X, rolling with any valid die
// function (each die returning a value in [1, X]) SHALL produce a sum in the
// range [N, N * X].

TEST_CASE("Property 14: DiceRoller roll bounds", "[pbt][dice]") {
    std::mt19937 rng(123); // seeded for reproducibility
    std::uniform_int_distribution<int> countDist(1, 9);
    std::uniform_int_distribution<int> sidesDist(1, 100);

    for (int i = 0; i < 100; i++) {
        int count = countDist(rng);
        int sides = sidesDist(rng);
        DiceSpec spec{ count, sides };

        // Roll using default RNG (which produces values in [1, sides])
        int result = DiceRoller::roll(spec);

        int minExpected = count;        // all dice roll 1
        int maxExpected = count * sides; // all dice roll max

        CHECK(result >= minExpected);
        CHECK(result <= maxExpected);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: rogue-trader-melee-combat — DiceRoller Unit Tests (Edge Cases)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_CASE("DiceRoller::parse rejects invalid strings", "[dice]") {
    SECTION("Non-dice strings") {
        CHECK_FALSE(DiceRoller::parse("abc").has_value());
        CHECK_FALSE(DiceRoller::parse("").has_value());
        CHECK_FALSE(DiceRoller::parse("d6").has_value());
        CHECK_FALSE(DiceRoller::parse("roll").has_value());
    }

    SECTION("Zero count") {
        CHECK_FALSE(DiceRoller::parse("0d6").has_value());
    }

    SECTION("Zero sides") {
        CHECK_FALSE(DiceRoller::parse("1d0").has_value());
    }

    SECTION("Count exceeds 9") {
        CHECK_FALSE(DiceRoller::parse("10d10").has_value());
    }

    SECTION("Sides exceeds 100") {
        CHECK_FALSE(DiceRoller::parse("1d101").has_value());
        CHECK_FALSE(DiceRoller::parse("2d999").has_value());
    }

    SECTION("Negative values") {
        CHECK_FALSE(DiceRoller::parse("-1d6").has_value());
        CHECK_FALSE(DiceRoller::parse("1d-6").has_value());
    }

    SECTION("Whitespace and special characters") {
        CHECK_FALSE(DiceRoller::parse(" 1d6").has_value());
        CHECK_FALSE(DiceRoller::parse("1d6 ").has_value());
        CHECK_FALSE(DiceRoller::parse("1 d 6").has_value());
        CHECK_FALSE(DiceRoller::parse("1D6").has_value());  // uppercase D
    }
}

TEST_CASE("DiceRoller::parse accepts valid boundary specs", "[dice]") {
    SECTION("Minimum: 1d1") {
        auto result = DiceRoller::parse("1d1");
        REQUIRE(result.has_value());
        CHECK(result->count == 1);
        CHECK(result->sides == 1);
    }

    SECTION("Maximum count: 9d6") {
        auto result = DiceRoller::parse("9d6");
        REQUIRE(result.has_value());
        CHECK(result->count == 9);
        CHECK(result->sides == 6);
    }

    SECTION("Maximum sides: 1d100") {
        auto result = DiceRoller::parse("1d100");
        REQUIRE(result.has_value());
        CHECK(result->count == 1);
        CHECK(result->sides == 100);
    }

    SECTION("Max both: 9d100") {
        auto result = DiceRoller::parse("9d100");
        REQUIRE(result.has_value());
        CHECK(result->count == 9);
        CHECK(result->sides == 100);
    }

    SECTION("Common RPG dice: 2d10") {
        auto result = DiceRoller::parse("2d10");
        REQUIRE(result.has_value());
        CHECK(result->count == 2);
        CHECK(result->sides == 10);
    }
}

TEST_CASE("DiceRoller::roll with injectable die function", "[dice]") {
    SECTION("All dice roll minimum (1)") {
        DiceSpec spec{ 3, 6 };
        int result = DiceRoller::roll(spec, [](int /*sides*/) { return 1; });
        CHECK(result == 3); // 3 dice * 1 = 3
    }

    SECTION("All dice roll maximum") {
        DiceSpec spec{ 3, 6 };
        int result = DiceRoller::roll(spec, [](int sides) { return sides; });
        CHECK(result == 18); // 3 * 6 = 18
    }

    SECTION("Single die") {
        DiceSpec spec{ 1, 20 };
        int result = DiceRoller::roll(spec, [](int /*sides*/) { return 15; });
        CHECK(result == 15);
    }
}

TEST_CASE("DiceRoller::rollFromString returns 0 on invalid spec", "[dice]") {
    CHECK(DiceRoller::rollFromString("abc") == 0);
    CHECK(DiceRoller::rollFromString("0d6") == 0);
    CHECK(DiceRoller::rollFromString("1d0") == 0);
    CHECK(DiceRoller::rollFromString("10d10") == 0);
    CHECK(DiceRoller::rollFromString("") == 0);
}

TEST_CASE("DiceRoller::rollFromString with valid spec and injectable RNG", "[dice]") {
    int result = DiceRoller::rollFromString("2d6", [](int /*sides*/) { return 4; });
    CHECK(result == 8); // 2 dice * 4 = 8
}

TEST_CASE("DiceRoller::format produces expected strings", "[dice]") {
    CHECK(DiceRoller::format(DiceSpec{ 1, 6 }) == "1d6");
    CHECK(DiceRoller::format(DiceSpec{ 2, 10 }) == "2d10");
    CHECK(DiceRoller::format(DiceSpec{ 9, 100 }) == "9d100");
    CHECK(DiceRoller::format(DiceSpec{ 1, 1 }) == "1d1");
}
