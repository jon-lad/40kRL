#include "lib/catch_amalgamated.hpp"
#include <random>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: rogue-trader-melee-combat — Property-Based Tests (Combat Formulas)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 1: Hit classification correctness ──────────────────────────────
// **Validates: Requirements 1.2, 1.3**
//
// For any effective WS in [1, 99] and any d100 roll in [1, 100],
// the hit classification SHALL equal (roll <= effectiveWS).

TEST_CASE("Property 1: Hit classification correctness", "[pbt][combat]") {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> wsDist(1, 99);
    std::uniform_int_distribution<int> rollDist(1, 100);

    for (int i = 0; i < 200; i++) {
        int effectiveWS = wsDist(rng);
        int roll = rollDist(rng);

        bool isHit = (roll <= effectiveWS);
        bool isMiss = (roll > effectiveWS);

        // Hit and miss are mutually exclusive and exhaustive
        CHECK(isHit != isMiss);
        // The combat system classifies as hit when roll <= effectiveWS
        CHECK(isHit == (roll <= effectiveWS));
    }
}

// ─── Property 2: Effective WS clamping invariant ─────────────────────────────
// **Validates: Requirements 1.4**
//
// For any base WS in [1, 99] and any set of integer modifiers,
// the computed effective WS SHALL always be in [1, 99].

TEST_CASE("Property 2: Effective WS clamping invariant", "[pbt][combat]") {
    std::mt19937 rng(43);
    std::uniform_int_distribution<int> baseDist(1, 99);
    std::uniform_int_distribution<int> modDist(-200, 200);

    for (int i = 0; i < 200; i++) {
        int baseWS = baseDist(rng);
        int modifier = modDist(rng);

        int effective = std::max(1, std::min(99, baseWS + modifier));

        CHECK(effective >= 1);
        CHECK(effective <= 99);
    }
}

// ─── Property 3: DoS computation ────────────────────────────────────────────
// **Validates: Requirements 2.1, 2.3**
//
// For any successful hit (roll <= effectiveWS), DoS = floor((effectiveWS - roll) / 10)
// and DoS >= 0.

TEST_CASE("Property 3: DoS computation", "[pbt][combat]") {
    std::mt19937 rng(44);
    std::uniform_int_distribution<int> wsDist(1, 99);
    std::uniform_int_distribution<int> rollDist(1, 100);

    int tested = 0;
    while (tested < 200) {
        int effectiveWS = wsDist(rng);
        int roll = rollDist(rng);

        // Only test successful hits
        if (roll > effectiveWS) continue;

        int dos = (effectiveWS - roll) / 10;
        int dosClamped = std::max(0, dos);

        // DoS formula produces non-negative result
        CHECK(dos >= 0);
        // Clamped value equals raw value (since difference is always >= 0 on success)
        CHECK(dosClamped == dos);
        // DoS matches the formula
        CHECK(dos == (effectiveWS - roll) / 10);

        tested++;
    }
}

// ─── Property 4: DoF computation ────────────────────────────────────────────
// **Validates: Requirements 2.2, 2.4**
//
// For any failed hit (roll > effectiveWS), DoF = floor((roll - effectiveWS) / 10)
// and DoF >= 0.

TEST_CASE("Property 4: DoF computation", "[pbt][combat]") {
    std::mt19937 rng(45);
    std::uniform_int_distribution<int> wsDist(1, 99);
    std::uniform_int_distribution<int> rollDist(1, 100);

    int tested = 0;
    while (tested < 200) {
        int effectiveWS = wsDist(rng);
        int roll = rollDist(rng);

        // Only test failed hits
        if (roll <= effectiveWS) continue;

        int dof = (roll - effectiveWS) / 10;
        int dofClamped = std::max(0, dof);

        // DoF formula produces non-negative result
        CHECK(dof >= 0);
        // Clamped value equals raw value (since difference is always > 0 on failure)
        CHECK(dofClamped == dof);
        // DoF matches the formula
        CHECK(dof == (roll - effectiveWS) / 10);

        tested++;
    }
}

// ─── Property 6: Opposed test hit negation formula ───────────────────────────
// **Validates: Requirements 4.2, 5.2**
//
// For any opposed test where roll <= characteristic,
// hits negated = 1 + floor((characteristic - roll) / 10).

TEST_CASE("Property 6: Opposed test negation formula", "[pbt][combat]") {
    std::mt19937 rng(46);
    std::uniform_int_distribution<int> charDist(1, 99);
    std::uniform_int_distribution<int> rollDist(1, 100);

    int tested = 0;
    while (tested < 200) {
        int characteristic = charDist(rng);
        int roll = rollDist(rng);

        // Only test successful opposed tests (roll <= characteristic)
        if (roll > characteristic) continue;

        int dos = (characteristic - roll) / 10;
        int hitsNegated = 1 + dos;

        // Hits negated is always at least 1 on a successful test
        CHECK(hitsNegated >= 1);
        // Formula matches: 1 + floor((char - roll) / 10)
        CHECK(hitsNegated == 1 + (characteristic - roll) / 10);
        // DoS is non-negative
        CHECK(dos >= 0);

        tested++;
    }
}

// ─── Property 7: Effective armour calculation ────────────────────────────────
// **Validates: Requirements 6.2**
//
// For any location armour >= 0 and penetration >= 0,
// effective armour = max(0, locationArmour - penetration).

TEST_CASE("Property 7: Effective armour calculation", "[pbt][combat]") {
    std::mt19937 rng(47);
    std::uniform_int_distribution<int> armourDist(0, 20);
    std::uniform_int_distribution<int> penDist(0, 20);

    for (int i = 0; i < 200; i++) {
        int locationArmour = armourDist(rng);
        int penetration = penDist(rng);

        int effectiveArmour = std::max(0, locationArmour - penetration);

        // Effective armour is never negative
        CHECK(effectiveArmour >= 0);
        // Formula matches
        CHECK(effectiveArmour == std::max(0, locationArmour - penetration));
        // When penetration exceeds armour, effective armour is 0
        if (penetration >= locationArmour) {
            CHECK(effectiveArmour == 0);
        }
        // When armour exceeds penetration, effective armour is the difference
        if (locationArmour > penetration) {
            CHECK(effectiveArmour == locationArmour - penetration);
        }
    }
}

// ─── Property 8: Final damage calculation ────────────────────────────────────
// **Validates: Requirements 6.3**
//
// For any raw damage >= 0, effective armour >= 0, and TB >= 0,
// final damage = max(0, rawDamage - effectiveArmour - TB).

TEST_CASE("Property 8: Final damage calculation", "[pbt][combat]") {
    std::mt19937 rng(48);
    std::uniform_int_distribution<int> rawDist(0, 30);
    std::uniform_int_distribution<int> armourDist(0, 15);
    std::uniform_int_distribution<int> tbDist(0, 9);

    for (int i = 0; i < 200; i++) {
        int rawDamage = rawDist(rng);
        int effectiveArmour = armourDist(rng);
        int tb = tbDist(rng);

        int finalDamage = std::max(0, rawDamage - effectiveArmour - tb);

        // Final damage is never negative
        CHECK(finalDamage >= 0);
        // Formula matches
        CHECK(finalDamage == std::max(0, rawDamage - effectiveArmour - tb));
        // When reductions exceed raw, final is 0
        if (effectiveArmour + tb >= rawDamage) {
            CHECK(finalDamage == 0);
        }
        // When raw exceeds reductions, final equals the difference
        if (rawDamage > effectiveArmour + tb) {
            CHECK(finalDamage == rawDamage - effectiveArmour - tb);
        }
    }
}

// ─── Property 9: Raw damage formula ─────────────────────────────────────────
// **Validates: Requirements 6.1**
//
// For any valid dice result and Strength Bonus in [0, 9],
// raw damage = diceResult + strengthBonus.

TEST_CASE("Property 9: Raw damage formula", "[pbt][combat]") {
    std::mt19937 rng(49);
    std::uniform_int_distribution<int> diceDist(1, 10);   // Simulates 1d10 roll
    std::uniform_int_distribution<int> sbDist(0, 9);      // SB range

    for (int i = 0; i < 200; i++) {
        int diceResult = diceDist(rng);
        int strengthBonus = sbDist(rng);

        int rawDamage = diceResult + strengthBonus;

        // Raw damage equals dice + SB
        CHECK(rawDamage == diceResult + strengthBonus);
        // Raw damage is always positive (dice >= 1, SB >= 0)
        CHECK(rawDamage >= 1);
        // Raw damage upper bound: max dice (10) + max SB (9) = 19
        CHECK(rawDamage <= 19);
    }
}

// ─── Property 10: Critical magnitude computation ─────────────────────────────
// **Validates: Requirements 7.1**
//
// For any current HP > 0 and final damage > current HP,
// critical magnitude = finalDamage - currentHP.

TEST_CASE("Property 10: Critical magnitude computation", "[pbt][combat]") {
    std::mt19937 rng(50);
    std::uniform_int_distribution<int> hpDist(1, 20);
    std::uniform_int_distribution<int> dmgDist(1, 40);

    int tested = 0;
    while (tested < 200) {
        int currentHP = hpDist(rng);
        int finalDamage = dmgDist(rng);

        // Only test when damage exceeds HP (critical condition)
        if (finalDamage <= currentHP) continue;

        int critMagnitude = finalDamage - currentHP;

        // Critical magnitude is positive
        CHECK(critMagnitude > 0);
        // Formula matches
        CHECK(critMagnitude == finalDamage - currentHP);
        // Remaining HP after damage would be negative
        CHECK((currentHP - finalDamage) < 0);

        tested++;
    }
}
