#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: critical-injuries — Unit Tests for Characteristics Modifier Overlay
// ═══════════════════════════════════════════════════════════════════════════════
// **Validates: Requirements 1.2, 4.2**

// ─── addModifier / removeModifier adjusts effective value from get() ─────────

TEST_CASE("Modifier: addModifier adjusts effective value from get()", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(40);

    chars.addModifier(CharId::WS, -10);

    REQUIRE(chars.get(CharId::WS) == 30);
}

TEST_CASE("Modifier: removeModifier reverses addModifier effect", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(40);

    chars.addModifier(CharId::BS, -15);
    REQUIRE(chars.get(CharId::BS) == 25);

    chars.removeModifier(CharId::BS, -15);
    REQUIRE(chars.get(CharId::BS) == 40);
}

// ─── getBase() returns raw value unaffected by modifiers ─────────────────────

TEST_CASE("Modifier: getBase() returns raw value unaffected by modifiers", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(50);

    chars.addModifier(CharId::Ag, -20);

    // get() should reflect the modifier
    REQUIRE(chars.get(CharId::Ag) == 30);
    // getBase() should return the original base value
    REQUIRE(chars.getBase(CharId::Ag) == 50);
}

TEST_CASE("Modifier: getBase() unchanged after multiple modifiers", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(45);

    chars.addModifier(CharId::T, -10);
    chars.addModifier(CharId::T, -5);

    REQUIRE(chars.getBase(CharId::T) == 45);
    REQUIRE(chars.get(CharId::T) == 30);
}

// ─── get() clamps result to [1, 99] even with large negative modifiers ──────

TEST_CASE("Modifier: get() clamps to minimum 1 with large negative modifier", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(20);

    // Apply a modifier that would bring the value below 1
    chars.addModifier(CharId::Per, -50);

    REQUIRE(chars.get(CharId::Per) == Characteristics::MIN_VALUE);
    // Base should still be 20
    REQUIRE(chars.getBase(CharId::Per) == 20);
}

TEST_CASE("Modifier: get() clamps to maximum 99 with large positive modifier", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(90);

    // Apply a positive modifier that would exceed 99
    chars.addModifier(CharId::WP, 20);

    REQUIRE(chars.get(CharId::WP) == Characteristics::MAX_VALUE);
    REQUIRE(chars.getBase(CharId::WP) == 90);
}

// ─── Multiple modifiers on same stat are additive ────────────────────────────

TEST_CASE("Modifier: multiple modifiers on same stat are additive", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(50);

    chars.addModifier(CharId::S, -5);
    chars.addModifier(CharId::S, -10);
    chars.addModifier(CharId::S, -3);

    // Total modifier: -18, effective = 50 - 18 = 32
    REQUIRE(chars.get(CharId::S) == 32);
    REQUIRE(chars.getBase(CharId::S) == 50);
}

TEST_CASE("Modifier: additive modifiers across different stats are independent", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(40);

    chars.addModifier(CharId::WS, -10);
    chars.addModifier(CharId::BS, -5);

    REQUIRE(chars.get(CharId::WS) == 30);
    REQUIRE(chars.get(CharId::BS) == 35);
    // Other stats unaffected
    REQUIRE(chars.get(CharId::S) == 40);
}

// ─── Removing a modifier restores previous effective value ───────────────────

TEST_CASE("Modifier: removing a modifier restores previous effective value", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(60);

    chars.addModifier(CharId::Fel, -20);
    REQUIRE(chars.get(CharId::Fel) == 40);

    chars.removeModifier(CharId::Fel, -20);
    REQUIRE(chars.get(CharId::Fel) == 60);
}

TEST_CASE("Modifier: removing one of multiple modifiers partially restores value", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(50);

    chars.addModifier(CharId::Int, -10);
    chars.addModifier(CharId::Int, -5);
    REQUIRE(chars.get(CharId::Int) == 35);

    // Remove just the -10 modifier
    chars.removeModifier(CharId::Int, -10);
    // Should still have -5 applied
    REQUIRE(chars.get(CharId::Int) == 45);
}

TEST_CASE("Modifier: removing all modifiers fully restores base value", "[critical-injuries][characteristics][modifier]")
{
    Characteristics chars(55);

    chars.addModifier(CharId::Ag, -15);
    chars.addModifier(CharId::Ag, -10);
    REQUIRE(chars.get(CharId::Ag) == 30);

    chars.removeModifier(CharId::Ag, -15);
    chars.removeModifier(CharId::Ag, -10);
    REQUIRE(chars.get(CharId::Ag) == 55);
    REQUIRE(chars.getBase(CharId::Ag) == 55);
}
