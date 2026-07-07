#include "lib/catch_amalgamated.hpp"
#include "main.h"
#include <random>
#include <cstdio>
#include <string>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: rogue-trader-ranged-combat — Property-Based Test (Serialization)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 11: Equippable ranged serialization round-trip ─────────────────
// **Validates: Requirements 15.1, 15.2, 15.4**
//
// For any valid Equippable state containing RangedStats (with any valid
// damageDice, penetration, range, rateOfFire, clipSize, reloadTime) and any
// currentAmmo in [0, clipSize], serializing then deserializing SHALL produce
// an equivalent Equippable state.

TEST_CASE("Property 11: Equippable ranged serialization round-trip", "[pbt][serialization][ranged]") {
    std::mt19937 rng(42); // seeded for reproducibility

    // Distributions for RangedStats fields
    std::uniform_int_distribution<int> diceCountDist(1, 5);
    std::uniform_int_distribution<int> diceSidesDist(1, 20);
    std::uniform_int_distribution<int> penDist(0, 15);
    std::uniform_int_distribution<int> rangeDist(1, 100);
    std::uniform_int_distribution<int> rofDist(1, 6);
    std::uniform_int_distribution<int> clipDist(1, 50);
    std::uniform_int_distribution<int> reloadDist(1, 3);

    // Distributions for Equippable base fields
    std::uniform_int_distribution<int> slotDist(0, 3);
    std::uniform_real_distribution<float> floatDist(-5.0f, 10.0f);
    std::uniform_int_distribution<int> skillDist(-10, 10);
    std::uniform_real_distribution<float> weightDist(0.0f, 20.0f);
    std::uniform_int_distribution<int> valueDist(0, 500);

    // Distributions for optional MeleeStats
    std::uniform_int_distribution<int> boolDist(0, 1);
    std::uniform_int_distribution<int> qualCountDist(0, 3);

    const char* testFile = "test_ranged_serialization_roundtrip.sav";

    for (int i = 0; i < 200; i++) {
        // Build random Equippable with RangedStats
        EquipmentSlot slot = static_cast<EquipmentSlot>(slotDist(rng));
        StatModifiers mods;
        mods.power = floatDist(rng);
        mods.defense = floatDist(rng);
        mods.maxHp = floatDist(rng);
        mods.skill = skillDist(rng);

        float weight = weightDist(rng);
        int value = valueDist(rng);

        Equippable original(slot, mods, weight, value);

        // Always add RangedStats
        RangedStats rs;
        rs.damageDice = DiceSpec{diceCountDist(rng), diceSidesDist(rng)};
        rs.penetration = penDist(rng);
        rs.range = rangeDist(rng);
        rs.rateOfFire = rofDist(rng);
        rs.clipSize = clipDist(rng);
        rs.reloadTime = reloadDist(rng);
        original.rangedStats = rs;

        // Set currentAmmo in [0, clipSize]
        std::uniform_int_distribution<int> ammoDist(0, rs.clipSize);
        original.currentAmmo = ammoDist(rng);

        // Optionally add MeleeStats
        if (boolDist(rng)) {
            MeleeStats ms;
            ms.damageDice = DiceSpec{diceCountDist(rng), diceSidesDist(rng)};
            ms.penetration = penDist(rng);
            int qCount = qualCountDist(rng);
            for (int q = 0; q < qCount; q++) {
                ms.qualities.push_back("Quality" + std::to_string(q));
            }
            original.meleeStats = ms;
        }

        // Optionally add ArmourProfile
        if (boolDist(rng)) {
            ArmourProfile ap;
            std::uniform_int_distribution<int> armDist(0, 10);
            for (int loc = 0; loc < static_cast<int>(HitLocation::COUNT); loc++) {
                ap.values[loc] = armDist(rng);
            }
            original.armourProfile = ap;
        }

        // Serialize
        TCODZip zip;
        original.save(zip);
        zip.saveToFile(testFile);

        // Deserialize
        TCODZip zip2;
        zip2.loadFromFile(testFile);
        Equippable loaded(EquipmentSlot::WEAPON, StatModifiers{}, 0.0f, 0);
        loaded.load(zip2);

        // Verify base fields
        REQUIRE(static_cast<int>(loaded.slot) == static_cast<int>(original.slot));
        REQUIRE(loaded.modifiers.power == Catch::Approx(original.modifiers.power));
        REQUIRE(loaded.modifiers.defense == Catch::Approx(original.modifiers.defense));
        REQUIRE(loaded.modifiers.maxHp == Catch::Approx(original.modifiers.maxHp));
        REQUIRE(loaded.modifiers.skill == original.modifiers.skill);
        REQUIRE(loaded.weight == Catch::Approx(original.weight));
        REQUIRE(loaded.value == original.value);

        // Verify RangedStats present and equivalent
        REQUIRE(loaded.rangedStats.has_value());
        REQUIRE(loaded.rangedStats->damageDice.count == original.rangedStats->damageDice.count);
        REQUIRE(loaded.rangedStats->damageDice.sides == original.rangedStats->damageDice.sides);
        REQUIRE(loaded.rangedStats->penetration == original.rangedStats->penetration);
        REQUIRE(loaded.rangedStats->range == original.rangedStats->range);
        REQUIRE(loaded.rangedStats->rateOfFire == original.rangedStats->rateOfFire);
        REQUIRE(loaded.rangedStats->clipSize == original.rangedStats->clipSize);
        REQUIRE(loaded.rangedStats->reloadTime == original.rangedStats->reloadTime);

        // Verify currentAmmo (clamped to [0, clipSize])
        int expectedAmmo = std::clamp(original.currentAmmo, 0, original.rangedStats->clipSize);
        REQUIRE(loaded.currentAmmo == expectedAmmo);

        // Verify MeleeStats if present
        REQUIRE(loaded.meleeStats.has_value() == original.meleeStats.has_value());
        if (original.meleeStats) {
            REQUIRE(loaded.meleeStats->damageDice.count == original.meleeStats->damageDice.count);
            REQUIRE(loaded.meleeStats->damageDice.sides == original.meleeStats->damageDice.sides);
            REQUIRE(loaded.meleeStats->penetration == original.meleeStats->penetration);
            REQUIRE(loaded.meleeStats->qualities.size() == original.meleeStats->qualities.size());
            for (size_t q = 0; q < original.meleeStats->qualities.size(); q++) {
                REQUIRE(loaded.meleeStats->qualities[q] == original.meleeStats->qualities[q]);
            }
        }

        // Verify ArmourProfile if present
        REQUIRE(loaded.armourProfile.has_value() == original.armourProfile.has_value());
        if (original.armourProfile) {
            for (int loc = 0; loc < static_cast<int>(HitLocation::COUNT); loc++) {
                REQUIRE(loaded.armourProfile->values[loc] == original.armourProfile->values[loc]);
            }
        }
    }

    // Cleanup
    std::remove(testFile);
}
