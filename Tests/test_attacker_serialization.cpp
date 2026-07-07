#include "lib/catch_amalgamated.hpp"
#include "main.h"
#include <random>
#include <cstdio>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: rogue-trader-melee-combat — Attacker Serialization Property Test
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 15: Attacker serialization round-trip ──────────────────────────
// **Validates: Requirements 13.4**
//
// For any valid Attacker state (power as float, skillValue in [1, 99],
// arbitrary modifier list), serializing then deserializing SHALL produce an
// Attacker with equivalent power, skillValue, and modifiers.

TEST_CASE("Property 15: Attacker serialization round-trip", "[pbt][serialization]") {
    std::mt19937 rng(99); // seeded for reproducibility
    std::uniform_real_distribution<float> powerDist(0.0f, 10.0f);
    std::uniform_int_distribution<int> skillDist(1, 99);
    std::uniform_int_distribution<int> modCountDist(0, 5);
    std::uniform_int_distribution<int> modDist(-20, 20);

    const char* testFile = "test_attacker_roundtrip.sav";

    for (int i = 0; i < 100; i++) {
        float power = powerDist(rng);
        int skillValue = skillDist(rng);

        Attacker original(power, skillValue);
        int modCount = modCountDist(rng);
        for (int m = 0; m < modCount; m++) {
            original.addModifier(modDist(rng));
        }

        // Serialize
        TCODZip zip;
        original.save(zip);
        zip.saveToFile(testFile);

        // Deserialize
        TCODZip zip2;
        zip2.loadFromFile(testFile);
        Attacker loaded(0.0f, 40);
        loaded.load(zip2);

        // Verify
        REQUIRE(loaded.power == Catch::Approx(original.power));
        REQUIRE(loaded.skillValue == original.skillValue);
        REQUIRE(loaded.modifiers.size() == original.modifiers.size());
        for (size_t m = 0; m < original.modifiers.size(); m++) {
            REQUIRE(loaded.modifiers[m] == original.modifiers[m]);
        }
    }

    // Cleanup
    std::remove(testFile);
}
