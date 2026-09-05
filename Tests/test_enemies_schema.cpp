#include "lib/catch_amalgamated.hpp"

#include <sol/sol.hpp>

#include <cstdio>
#include <map>
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: region-based-npc-spawns — Enemy schema and retained Ork set (Task 4.2)
//
// These EXAMPLE-based unit tests load the real Scripts/Enemies.lua and assert:
//   1. Schema (Req 4.1)  — every Enemy_Entry's `chance` field is a keyed TABLE
//      whose values are integers in [0, 100].
//   2. Ork set retained (Req 7.1, 7.2) — Gretchin / Ork / Shoota Boy / Nob still
//      exist with their expected template fields, and each entry's `chance.Ork`
//      cumulative value equals 50 / 75 / 90 / 100 respectively.
//
// TDD-RED: Enemies.lua currently uses a FLAT integer `chance` (chance = 50, ...),
// so both the schema (chance is not a table) and the Ork-set (chance.Ork is nil)
// assertions are EXPECTED TO FAIL until Task 4.3 converts `chance` to a per-region
// keyed table { Ork = <n> }.
//
// Engine isolation: this suite opens its own sol::state and never touches the
// global Engine (no engine.gui / engine.map / engine.player), per test-isolation.
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

// The `enemies` table in Enemies.lua is local and exposed only via
// spawnEnemy(roll, x, y[, region]) -> addActor(x, y, entry) — mirroring the
// production path in Source/Map.cpp. We register a spy addActor that collects each
// entry, then drive spawnEnemy across the full roll range (and the Ork region) to
// surface every declared entry. Entries are keyed/deduped by their `name` field.
struct LoadedEnemies {
    bool loaded = false;
    std::string path;
    std::map<std::string, sol::table> byName; // name -> entry
    std::vector<sol::table> inOrder;           // first-seen order
};

// Unambiguous typed field read: index the table then coerce via sol::optional.
// (The single-template sol::get_or overload is ambiguous in this sol version.)
template <typename T>
T fieldOr(const sol::table& t, const char* key, T fallback) {
    sol::optional<T> v = t[key];
    return v ? *v : fallback;
}

// Resolve Enemies.lua relative to the test CWD (repo root normally; x64/Debug when
// run from the build output). Mirrors the probe used by the other Lua-load tests.
std::string resolveEnemiesPath() {
    const std::vector<std::string> candidates = {
        "Scripts/Enemies.lua",
        "../../Scripts/Enemies.lua",
        "../Scripts/Enemies.lua",
    };
    for (const auto& p : candidates) {
        if (std::FILE* f = std::fopen(p.c_str(), "r")) {
            std::fclose(f);
            return p;
        }
    }
    return {};
}

// Collect all Enemy_Entry tables by driving spawnEnemy through a spy addActor.
LoadedEnemies collectEnemies(sol::state& lua) {
    LoadedEnemies result;
    result.path = resolveEnemiesPath();
    if (result.path.empty()) {
        return result;
    }

    lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string);

    lua.set_function("addActor", [&](int /*x*/, int /*y*/, sol::table entry) {
        const std::string name = fieldOr<std::string>(entry, "name", "");
        if (result.byName.find(name) == result.byName.end()) {
            result.byName.emplace(name, entry);
            result.inOrder.push_back(entry);
        }
    });

    lua.script_file(result.path);

    sol::protected_function spawnEnemy = lua["spawnEnemy"];
    if (!spawnEnemy.valid()) {
        return result;
    }

    // Drive the full roll range. Pass the "Ork" region as a 4th argument: the
    // post-4.3 signature is spawnEnemy(roll, x, y, region); the pre-4.3 signature
    // spawnEnemy(roll, x, y) simply ignores the extra argument, so this probe works
    // against both and always surfaces the Ork-column entries.
    for (int roll = 0; roll < 100; ++roll) {
        (void)spawnEnemy(roll, 0, 0, std::string("Ork"));
    }

    result.loaded = true;
    return result;
}

} // namespace

// ─── Schema: chance is a keyed table of integers in [0, 100] (Req 4.1) ──────────
TEST_CASE("Enemies.lua schema: every entry's chance is a keyed table of ints in [0,100]",
          "[region-based-npc-spawns][enemies-schema]") {
    sol::state lua;
    LoadedEnemies enemies = collectEnemies(lua);

    if (enemies.path.empty()) {
        SKIP("Scripts/Enemies.lua not found relative to the test CWD; run from the repo root.");
    }
    REQUIRE(enemies.loaded);
    REQUIRE_FALSE(enemies.inOrder.empty());

    for (const auto& entry : enemies.inOrder) {
        const std::string name = fieldOr<std::string>(entry, "name", "<unnamed>");
        INFO("enemy = " << name);

        sol::object chanceObj = entry["chance"];
        // Req 4.1: chance is a per-region keyed table (NOT a flat integer).
        REQUIRE(chanceObj.get_type() == sol::type::table);

        sol::table chanceTbl = chanceObj.as<sol::table>();
        bool sawAtLeastOneKey = false;
        for (auto& kv : chanceTbl) {
            sawAtLeastOneKey = true;
            // Keys are Region_Name strings.
            CHECK(kv.first.get_type() == sol::type::string);
            // Values are integers within [0, 100].
            REQUIRE(kv.second.get_type() == sol::type::number);
            const double raw = kv.second.as<double>();
            const int asInt = kv.second.as<int>();
            CHECK(static_cast<double>(asInt) == raw); // integral, no fractional part
            CHECK(asInt >= 0);
            CHECK(asInt <= 100);
        }
        CHECK(sawAtLeastOneKey);
    }
}

// ─── Ork column reconciled to the bestiary IV.7 Troop set (bestiary-npcs Req 8) ──
// This test was superseded by the bestiary-npcs spec (Requirement 8, Ork
// reconciliation), which rebuilt the Ork Faction_Region column from the Rogue
// Trader bestiary IV.7 Troop set (+ Ork Freebooter §5.3). The legacy names
// "Ork", "Shoota Boy" and "Nob" were retired ("Ork" -> "Ork Boy", "Shoota Boy"
// folded into "Ork Boy", and the bestiary "Nob" is Elite / out of scope);
// "Gretchin" was kept but realigned to the IV.7 profile. The old flat cumulative
// distribution (Gretchin 50 / Ork 75 / Shoota Boy 90 / Nob 100) no longer applies,
// so this case now asserts the reconciled roster and the structural invariant
// (strictly ascending cumulative chances terminating at exactly 100) rather than
// the retired hard-coded values.
TEST_CASE("Enemies.lua Ork column reconciled to the IV.7 Troop set (legacy names retired)",
          "[region-based-npc-spawns][enemies-schema]") {
    sol::state lua;
    LoadedEnemies enemies = collectEnemies(lua);

    if (enemies.path.empty()) {
        SKIP("Scripts/Enemies.lua not found relative to the test CWD; run from the repo root.");
    }
    REQUIRE(enemies.loaded);

    // collectEnemies drives spawnEnemy with the "Ork" region, so inOrder holds the
    // Ork Faction_Region column (entries whose chance table carries an "Ork" key).
    // The column still exists and is non-empty.
    REQUIRE_FALSE(enemies.inOrder.empty());

    // Reconciled rank-and-file exist (Gretchin realigned; the bare "Ork" renamed to
    // "Ork Boy") and belong to the Ork column (their chance table carries "Ork").
    auto isOrkColumnEntry = [](const sol::table& e) {
        sol::object chanceObj = e["chance"];
        if (chanceObj.get_type() != sol::type::table) return false;
        sol::object ork = chanceObj.as<sol::table>()["Ork"];
        return ork.get_type() == sol::type::number;
    };

    for (const char* name : { "Gretchin", "Ork Boy" }) {
        INFO("expected reconciled Ork entry = " << name);
        REQUIRE(enemies.byName.count(name) == 1);
        CHECK(isOrkColumnEntry(enemies.byName.at(name)));
    }

    // Legacy names retired by the bestiary-npcs Ork reconciliation.
    CHECK(enemies.byName.count("Ork") == 0);
    CHECK(enemies.byName.count("Shoota Boy") == 0);
    CHECK(enemies.byName.count("Nob") == 0);

    // Structural invariant: within the Ork column the cumulative chance.Ork values
    // are strictly ascending in declaration order and the final one is exactly 100.
    int prev = 0;
    int last = -1;
    bool sawAny = false;
    for (const auto& e : enemies.inOrder) {
        if (!isOrkColumnEntry(e)) continue;
        const std::string name = fieldOr<std::string>(e, "name", "<unnamed>");
        INFO("Ork column entry = " << name);
        const int cum = e["chance"].get<sol::table>()["Ork"].get<int>();
        CHECK(cum > prev); // strictly ascending
        CHECK(cum >= 0);
        CHECK(cum <= 100);
        prev = cum;
        last = cum;
        sawAny = true;
    }
    REQUIRE(sawAny);
    CHECK(last == 100); // terminal cumulative value is exactly 100
}
