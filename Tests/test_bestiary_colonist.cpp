#include "lib/catch_amalgamated.hpp"

#include "BestiaryTestHarness.hpp"

#include <algorithm>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — Example test for the Colonist base and its six variants
// (Task 9.1, TDD: written before the Colonist data in Task 9.2, so these cases are
// EXPECTED TO FAIL until the Colonist base + variants are added to Enemies.lua).
//
// Reference: RT-Bestiary.md §5.1 (Colonist base template + Adept, Bloodskinner,
// Entertainer, Hired Gun, Scum, Voidfarer variants).
//
// Validates:
//   Requirement 4.1 — Colonist base characteristics (WS 25, BS 20, S 30, T 30,
//                     Ag 30, Int 25, Per 25, WP 25, Fel 30).
//   Requirement 4.2 — each variant equals the base with only its cited overrides
//                     applied; every non-overridden characteristic retains the base.
//   Requirement 4.3 — Hired Gun hp == 12; the base and the other five variants
//                     hp == 9.
//   Requirement 4.4 — each variant's skills/talents/traits include the base entries
//                     plus its cited additions.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// The nine Rogue Trader characteristic field keys, in schema order.
const char* const kCharKeys[9] = {
    "ws", "bs", "s", "t", "ag", "int", "per", "wp", "fel",
};

// The Colonist base characteristics, per RT-Bestiary.md §5.1.
struct Chars {
    int ws, bs, s, t, ag, i, per, wp, fel;
};
constexpr Chars kColonistBase{ 25, 20, 30, 30, 30, 25, 25, 25, 30 };

int charOf(const bestiary::EnemyEntry& e, const char* key) {
    return e.fieldOr<int>(key, -1);
}

// Read a Lua string-list field (talents / traits) into a C++ vector.
std::vector<std::string> stringList(const bestiary::EnemyEntry& e, const char* key) {
    std::vector<std::string> out;
    sol::object obj = e.table[key];
    if (obj.get_type() != sol::type::table) return out;
    sol::table t = obj.as<sol::table>();
    for (std::size_t i = 1; i <= t.size(); ++i) {
        sol::optional<std::string> s = t[i];
        if (s) out.push_back(*s);
    }
    return out;
}

// Read the keys of a Lua name->rank table (skills) into a C++ vector.
std::vector<std::string> tableKeys(const bestiary::EnemyEntry& e, const char* key) {
    std::vector<std::string> out;
    sol::object obj = e.table[key];
    if (obj.get_type() != sol::type::table) return out;
    sol::table t = obj.as<sol::table>();
    for (auto& kv : t) {
        if (kv.first.get_type() == sol::type::string) {
            out.push_back(kv.first.as<std::string>());
        }
    }
    return out;
}

bool contains(const std::vector<std::string>& hay, const std::string& needle) {
    return std::find(hay.begin(), hay.end(), needle) != hay.end();
}

// Assert `superset` contains every element of `base` (the variant retains all of the
// Colonist base's entries in the given collection).
void requireContainsAll(const std::vector<std::string>& superset,
                        const std::vector<std::string>& base,
                        const std::string& variant, const std::string& field) {
    for (const auto& item : base) {
        INFO(variant << " " << field << " should include base entry \"" << item << "\"");
        REQUIRE(contains(superset, item));
    }
}

} // namespace

// ─── Colonist base characteristics and wounds (Req 4.1, 4.3) ────────────────────
TEST_CASE("Colonist base has the cited characteristics and Wounds 9",
          "[bestiary-npcs][colonist]") {
    bestiary::BestiaryTestHarness h;
    if (!h.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found relative to the test CWD; "
             "run from the repo root.");
    }

    auto base = h.enemyByName("Colonist");
    REQUIRE(base.has_value());

    CHECK(charOf(*base, "ws") == kColonistBase.ws);
    CHECK(charOf(*base, "bs") == kColonistBase.bs);
    CHECK(charOf(*base, "s") == kColonistBase.s);
    CHECK(charOf(*base, "t") == kColonistBase.t);
    CHECK(charOf(*base, "ag") == kColonistBase.ag);
    CHECK(charOf(*base, "int") == kColonistBase.i);
    CHECK(charOf(*base, "per") == kColonistBase.per);
    CHECK(charOf(*base, "wp") == kColonistBase.wp);
    CHECK(charOf(*base, "fel") == kColonistBase.fel);

    // Wounds 9 (Req 4.3).
    CHECK(base->fieldOr<double>("hp", -1.0) == Catch::Approx(9.0));
}

// ─── Each variant = base + only its cited overrides (Req 4.2) ───────────────────
TEST_CASE("Colonist variants equal the base with only their cited overrides applied",
          "[bestiary-npcs][colonist]") {
    bestiary::BestiaryTestHarness h;
    if (!h.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found; run from the repo root.");
    }

    // Expected characteristic set per variant = base with the §5.1 overrides applied.
    struct VariantChars {
        const char* name;
        Chars chars;
    };
    const VariantChars variants[] = {
        // Adept:        Int 30
        { "Adept",       { 25, 20, 30, 30, 30, 30, 25, 25, 30 } },
        // Bloodskinner: WS 35, BS 30, Per 35
        { "Bloodskinner", { 35, 30, 30, 30, 30, 25, 35, 25, 30 } },
        // Entertainer:  Fel 35
        { "Entertainer", { 25, 20, 30, 30, 30, 25, 25, 25, 35 } },
        // Hired Gun:    BS 35
        { "Hired Gun",   { 25, 35, 30, 30, 30, 25, 25, 25, 30 } },
        // Scum:         WS 30, Per 30
        { "Scum",        { 30, 20, 30, 30, 30, 25, 30, 25, 30 } },
        // Voidfarer:    S 38, T 38
        { "Voidfarer",   { 25, 20, 38, 38, 30, 25, 25, 25, 30 } },
    };

    for (const auto& v : variants) {
        INFO("variant: " << v.name);
        auto entry = h.enemyByName(v.name);
        REQUIRE(entry.has_value());

        CHECK(charOf(*entry, "ws") == v.chars.ws);
        CHECK(charOf(*entry, "bs") == v.chars.bs);
        CHECK(charOf(*entry, "s") == v.chars.s);
        CHECK(charOf(*entry, "t") == v.chars.t);
        CHECK(charOf(*entry, "ag") == v.chars.ag);
        CHECK(charOf(*entry, "int") == v.chars.i);
        CHECK(charOf(*entry, "per") == v.chars.per);
        CHECK(charOf(*entry, "wp") == v.chars.wp);
        CHECK(charOf(*entry, "fel") == v.chars.fel);
    }
}

// ─── hp overrides: Hired Gun 12, others (and base) 9 (Req 4.3) ──────────────────
TEST_CASE("Hired Gun hp == 12; Colonist base and other variants hp == 9",
          "[bestiary-npcs][colonist]") {
    bestiary::BestiaryTestHarness h;
    if (!h.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found; run from the repo root.");
    }

    const char* const hp9[] = {
        "Colonist", "Adept", "Bloodskinner", "Entertainer", "Scum", "Voidfarer",
    };
    for (const char* name : hp9) {
        INFO("entry: " << name);
        auto entry = h.enemyByName(name);
        REQUIRE(entry.has_value());
        CHECK(entry->fieldOr<double>("hp", -1.0) == Catch::Approx(9.0));
    }

    auto hiredGun = h.enemyByName("Hired Gun");
    REQUIRE(hiredGun.has_value());
    CHECK(hiredGun->fieldOr<double>("hp", -1.0) == Catch::Approx(12.0));
}

// ─── Variant skills/talents/traits include base entries plus additions (Req 4.4) ─
TEST_CASE("Colonist variants include the base skills/talents/traits plus their additions",
          "[bestiary-npcs][colonist]") {
    bestiary::BestiaryTestHarness h;
    if (!h.loaded()) {
        SKIP("Scripts/Enemies.lua / Equipment.lua not found; run from the repo root.");
    }

    auto base = h.enemyByName("Colonist");
    REQUIRE(base.has_value());

    const std::vector<std::string> baseSkills = tableKeys(*base, "skills");
    const std::vector<std::string> baseTalents = stringList(*base, "talents");
    const std::vector<std::string> baseTraits = stringList(*base, "traits");

    const char* const variantNames[] = {
        "Adept", "Bloodskinner", "Entertainer", "Hired Gun", "Scum", "Voidfarer",
    };

    for (const char* name : variantNames) {
        INFO("variant: " << name);
        auto entry = h.enemyByName(name);
        REQUIRE(entry.has_value());

        // Each variant's collections must be a superset of the Colonist base's.
        requireContainsAll(tableKeys(*entry, "skills"), baseSkills, name, "skills");
        requireContainsAll(stringList(*entry, "talents"), baseTalents, name, "talents");
        requireContainsAll(stringList(*entry, "traits"), baseTraits, name, "traits");
    }

    // Spot-check the cited per-variant additions (RT-Bestiary.md §5.1) are present.
    SECTION("cited additions are present per variant") {
        auto adept = h.enemyByName("Adept");
        REQUIRE(adept.has_value());
        CHECK(contains(tableKeys(*adept, "skills"),
                       "Common Knowledge (Imperium)"));

        auto bloodskinner = h.enemyByName("Bloodskinner");
        REQUIRE(bloodskinner.has_value());
        CHECK(contains(tableKeys(*bloodskinner, "skills"), "Survival"));

        auto entertainer = h.enemyByName("Entertainer");
        REQUIRE(entertainer.has_value());
        CHECK(contains(tableKeys(*entertainer, "skills"), "Charm"));

        auto hiredGun = h.enemyByName("Hired Gun");
        REQUIRE(hiredGun.has_value());
        CHECK(contains(tableKeys(*hiredGun, "skills"), "Intimidate"));

        auto scum = h.enemyByName("Scum");
        REQUIRE(scum.has_value());
        CHECK(contains(tableKeys(*scum, "skills"), "Chem-Use"));

        auto voidfarer = h.enemyByName("Voidfarer");
        REQUIRE(voidfarer.has_value());
        CHECK(contains(tableKeys(*voidfarer, "skills"), "Tech-Use"));
    }
}
