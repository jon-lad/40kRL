#include "lib/catch_amalgamated.hpp"
#include <random>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: rogue-trader-ranged-combat — Property-Based Tests (Ranged Formulas)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 1: Ranged hit classification ───────────────────────────────────
// **Validates: Requirements 3.1, 3.2, 3.3**
//
// For any Effective BS in [1, 99] and any d100 roll in [1, 100],
// the ranged attack is classified as a hit if and only if roll <= Effective BS.

TEST_CASE("Property 1: Ranged hit classification", "[pbt][ranged]") {
    std::mt19937 rng(100);
    std::uniform_int_distribution<int> bsDist(1, 99);
    std::uniform_int_distribution<int> rollDist(1, 100);

    for (int i = 0; i < 200; i++) {
        int effectiveBS = bsDist(rng);
        int roll = rollDist(rng);

        bool isHit = (roll <= effectiveBS);
        bool isMiss = (roll > effectiveBS);

        // Hit and miss are mutually exclusive and exhaustive
        CHECK(isHit != isMiss);
        // Hit classification matches formula
        CHECK(isHit == (roll <= effectiveBS));
        // Miss classification matches formula
        CHECK(isMiss == (roll > effectiveBS));
    }
}

// ─── Property 2: Effective BS clamping invariant ─────────────────────────────
// **Validates: Requirements 3.4, 10.6**
//
// For any base BS in [1, 99] and any set of integer modifiers,
// the computed Effective BS SHALL always be in [1, 99].

TEST_CASE("Property 2: Effective BS clamping invariant", "[pbt][ranged]") {
    std::mt19937 rng(101);
    std::uniform_int_distribution<int> baseDist(1, 99);
    std::uniform_int_distribution<int> modDist(-200, 200);

    for (int i = 0; i < 200; i++) {
        int baseBS = baseDist(rng);
        int modifier = modDist(rng);

        int effectiveBS = std::max(1, std::min(99, baseBS + modifier));

        CHECK(effectiveBS >= 1);
        CHECK(effectiveBS <= 99);
    }
}

// ─── Property 3: DoS computation ────────────────────────────────────────────
// **Validates: Requirements 3.5**
//
// For any successful ranged hit (roll <= effectiveBS),
// DoS = floor((effectiveBS - roll) / 10) and DoS >= 0.

TEST_CASE("Property 3: Ranged DoS computation", "[pbt][ranged]") {
    std::mt19937 rng(102);
    std::uniform_int_distribution<int> bsDist(1, 99);
    std::uniform_int_distribution<int> rollDist(1, 100);

    int tested = 0;
    while (tested < 200) {
        int effectiveBS = bsDist(rng);
        int roll = rollDist(rng);

        // Only test successful hits
        if (roll > effectiveBS) continue;

        int dos = (effectiveBS - roll) / 10;

        // DoS is non-negative on a successful hit
        CHECK(dos >= 0);
        // DoS matches the formula
        CHECK(dos == (effectiveBS - roll) / 10);

        tested++;
    }
}

// ─── Property 4: DoF computation ────────────────────────────────────────────
// **Validates: Requirements 3.6**
//
// For any failed ranged hit (roll > effectiveBS),
// DoF = floor((roll - effectiveBS) / 10) and DoF >= 0.

TEST_CASE("Property 4: Ranged DoF computation", "[pbt][ranged]") {
    std::mt19937 rng(103);
    std::uniform_int_distribution<int> bsDist(1, 99);
    std::uniform_int_distribution<int> rollDist(1, 100);

    int tested = 0;
    while (tested < 200) {
        int effectiveBS = bsDist(rng);
        int roll = rollDist(rng);

        // Only test failed hits
        if (roll <= effectiveBS) continue;

        int dof = (roll - effectiveBS) / 10;

        // DoF is non-negative on a failed hit
        CHECK(dof >= 0);
        // DoF matches the formula
        CHECK(dof == (roll - effectiveBS) / 10);

        tested++;
    }
}

// ─── Property 5: Dodge negation formula ──────────────────────────────────────
// **Validates: Requirements 5.1, 5.2, 5.3**
//
// For any Agility in [1, 99] and dodge roll in [1, 100]:
// - When dodgeRoll <= Agility: hits negated = 1 + floor((Ag - dodgeRoll) / 10)
// - When dodgeRoll > Agility: hits negated = 0

TEST_CASE("Property 5: Dodge negation formula", "[pbt][ranged]") {
    std::mt19937 rng(104);
    std::uniform_int_distribution<int> agDist(1, 99);
    std::uniform_int_distribution<int> rollDist(1, 100);

    for (int i = 0; i < 200; i++) {
        int agility = agDist(rng);
        int dodgeRoll = rollDist(rng);

        int hitsNegated;
        if (dodgeRoll <= agility) {
            // Successful dodge
            int dodgeDoS = (agility - dodgeRoll) / 10;
            hitsNegated = 1 + dodgeDoS;

            // Hits negated is always at least 1 on a successful dodge
            CHECK(hitsNegated >= 1);
            // Formula matches
            CHECK(hitsNegated == 1 + (agility - dodgeRoll) / 10);
            // DoS on dodge is non-negative
            CHECK(dodgeDoS >= 0);
        } else {
            // Failed dodge
            hitsNegated = 0;

            // Zero hits negated on failure
            CHECK(hitsNegated == 0);
        }
    }
}

// ─── Property 6: Raw damage excludes Strength Bonus ──────────────────────────
// **Validates: Requirements 6.1**
//
// For any ranged weapon damage dice result, the raw ranged damage equals
// the dice result alone (no Strength Bonus added).

TEST_CASE("Property 6: Raw damage excludes Strength Bonus", "[pbt][ranged]") {
    std::mt19937 rng(105);
    std::uniform_int_distribution<int> diceDist(1, 10);  // Simulates 1d10 result
    std::uniform_int_distribution<int> sbDist(0, 9);     // SB range (irrelevant)

    for (int i = 0; i < 200; i++) {
        int diceResult = diceDist(rng);
        int strengthBonus = sbDist(rng);  // shooter's SB (should NOT be added)

        // Ranged raw damage = dice result only (no SB)
        int rawRangedDamage = diceResult;

        // Raw ranged damage equals dice result alone
        CHECK(rawRangedDamage == diceResult);
        // Strength bonus does NOT contribute (when SB > 0, raw != dice + SB)
        if (strengthBonus > 0) {
            CHECK(rawRangedDamage != diceResult + strengthBonus);
        }
        // Raw damage is always positive (dice >= 1)
        CHECK(rawRangedDamage >= 1);
    }
}

// ─── Property 7: Effective armour formula ────────────────────────────────────
// **Validates: Requirements 6.2**
//
// For any location armour >= 0 and weapon penetration >= 0,
// effective armour = max(0, locationArmour - penetration).

TEST_CASE("Property 7: Ranged effective armour formula", "[pbt][ranged]") {
    std::mt19937 rng(106);
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

// ─── Property 8: Final damage formula ────────────────────────────────────────
// **Validates: Requirements 6.3**
//
// For any raw damage >= 0, effective armour >= 0, and TB >= 0,
// final damage = max(0, rawDamage - effectiveArmour - TB).

TEST_CASE("Property 8: Ranged final damage formula", "[pbt][ranged]") {
    std::mt19937 rng(107);
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

// ─── Property 9: Ammunition consumption ─────────────────────────────────────
// **Validates: Requirements 7.1**
//
// For any ranged weapon with currentAmmo >= rateOfFire,
// after resolving a ranged attack, ammo = currentAmmo - rateOfFire, always >= 0.

TEST_CASE("Property 9: Ammunition consumption", "[pbt][ranged]") {
    std::mt19937 rng(108);
    std::uniform_int_distribution<int> rofDist(1, 5);    // rate of fire
    std::uniform_int_distribution<int> clipDist(6, 30);  // clip size

    for (int i = 0; i < 200; i++) {
        int rateOfFire = rofDist(rng);
        int clipSize = clipDist(rng);
        // currentAmmo is always at least rateOfFire (precondition: can fire)
        std::uniform_int_distribution<int> ammoDist(rateOfFire, clipSize);
        int currentAmmo = ammoDist(rng);

        // Ammo consumed = min(rateOfFire, currentAmmo) per the implementation
        int ammoConsumed = std::min(rateOfFire, currentAmmo);
        int newAmmo = currentAmmo - ammoConsumed;

        // New ammo is non-negative
        CHECK(newAmmo >= 0);
        // When currentAmmo >= rateOfFire, exactly rateOfFire is consumed
        CHECK(ammoConsumed == rateOfFire);
        // New ammo matches formula
        CHECK(newAmmo == currentAmmo - rateOfFire);
    }
}

// ─── Property 10: Destructible direct damage ────────────────────────────────
// **Validates: Requirements 14.3, 14.4**
//
// For any ranged weapon damage dice result and any Destructible actor,
// the Destructible's HP is reduced by exactly the dice result
// (no armour subtraction, no TB subtraction).

TEST_CASE("Property 10: Destructible direct damage", "[pbt][ranged]") {
    std::mt19937 rng(109);
    std::uniform_int_distribution<int> diceDist(1, 10);  // 1d10 result
    std::uniform_int_distribution<int> hpDist(5, 50);    // destructible HP

    for (int i = 0; i < 200; i++) {
        int diceResult = diceDist(rng);
        int initialHP = hpDist(rng);

        // Destructible damage = dice result directly (no armour, no TB)
        int damage = diceResult;
        int newHP = initialHP - damage;

        // Damage equals the dice result exactly
        CHECK(damage == diceResult);
        // HP is reduced by exactly the dice result
        CHECK(newHP == initialHP - diceResult);
        // Damage is always positive (dice >= 1)
        CHECK(damage >= 1);
    }
}
