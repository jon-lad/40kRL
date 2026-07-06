#include "lib/catch_amalgamated.hpp"
#include "HitLocation.h"

// ─── Property-based test (exhaustive) ────────────────────────────────────────
// **Validates: Requirements 3.1, 3.2, 3.3**

TEST_CASE("Property 5: Hit location digit reversal and table mapping", "[pbt][hitlocation]")
{
    for (int roll = 1; roll <= 100; roll++) {
        HitLocation loc = HitLocationTable::resolve(roll);

        // Must produce a valid enum value
        REQUIRE(static_cast<int>(loc) >= 0);
        REQUIRE(static_cast<int>(loc) < static_cast<int>(HitLocation::COUNT));

        // Verify the reversal formula
        int reversed;
        if (roll == 100) {
            reversed = 0;
        } else {
            reversed = (roll % 10) * 10 + (roll / 10);
        }

        // Verify mapping matches expected ranges per Hit_Location_Table
        if (reversed == 0) {
            CHECK(loc == HitLocation::LEFT_LEG);
        } else if (reversed <= 10) {
            CHECK(loc == HitLocation::HEAD);
        } else if (reversed <= 20) {
            CHECK(loc == HitLocation::RIGHT_ARM);
        } else if (reversed <= 30) {
            CHECK(loc == HitLocation::LEFT_ARM);
        } else if (reversed <= 70) {
            CHECK(loc == HitLocation::BODY);
        } else if (reversed <= 80) {
            CHECK(loc == HitLocation::RIGHT_LEG);
        } else {
            CHECK(loc == HitLocation::LEFT_LEG);
        }
    }
}

// ─── Unit test: name() returns valid string for all locations ─────────────────

TEST_CASE("HitLocationTable::name returns non-empty string for each valid HitLocation", "[hitlocation]")
{
    for (int i = 0; i < static_cast<int>(HitLocation::COUNT); i++) {
        HitLocation loc = static_cast<HitLocation>(i);
        const char* locName = HitLocationTable::name(loc);
        REQUIRE(locName != nullptr);
        REQUIRE(std::strlen(locName) > 0);
    }
}
