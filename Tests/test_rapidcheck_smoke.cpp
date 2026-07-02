// Smoke test to verify the minimal RapidCheck stub compiles and works with Catch2.
#include "catch_amalgamated.hpp"
#include "rapidcheck.h"
#include "rapidcheck_catch.h"

TEST_CASE("RapidCheck stub smoke test", "[rapidcheck][smoke]") {
    // Test rc::check() basic usage
    SECTION("rc::check runs property without failure") {
        rc::check("addition is commutative", []() {
            auto a = rc::generate(rc::gen::inRange(-1000, 1000));
            auto b = rc::generate(rc::gen::inRange(-1000, 1000));
            RC_ASSERT(a + b == b + a);
        });
    }

    // Test rc::prop() bridge with Catch2
    SECTION("rc::prop runs property via Catch2 SECTION") {
        rc::prop("multiplication by zero is zero", []() {
            auto a = rc::generate(rc::gen::inRange(-1000, 1000));
            RC_ASSERT(a * 0 == 0);
        });
    }

    // Test string generator
    SECTION("string generator produces strings in range") {
        rc::check("string length is within bounds", []() {
            auto s = rc::generate(rc::gen::string(1, 10));
            RC_ASSERT(s.size() >= 1 && s.size() <= 10);
        });
    }

    // Test bool generator
    SECTION("bool generator produces booleans") {
        rc::check("bool generator works", []() {
            auto b = rc::generate(rc::gen::arbitrary_bool());
            // Just verify it doesn't crash - bool is always true or false
            RC_ASSERT(b == true || b == false);
        });
    }
}
