// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — sol2-based data test harness (Task 1.1)
// See BestiaryTestHarness.hpp for the contract and rationale.
// ═══════════════════════════════════════════════════════════════════════════════

#include "BestiaryTestHarness.hpp"

#include <cstdio>

namespace bestiary {

namespace {

// Resolve a Scripts/<file> path relative to the test CWD. Production and the other
// Lua-load tests load "Scripts/<file>.lua" from the repo root; probe a couple of
// fallbacks so a differing CWD (e.g. x64/Debug) still resolves. Mirrors the probe
// used by test_enemies_schema.cpp / test_region_spawn_selection.cpp.
std::string resolveScriptPath(const char* relative) {
    const std::string rel(relative);
    const std::string candidates[] = {
        "Scripts/" + rel,
        "../../Scripts/" + rel, // if CWD == x64/Debug
        "../Scripts/" + rel,
    };
    for (const auto& p : candidates) {
        if (std::FILE* f = std::fopen(p.c_str(), "r")) {
            std::fclose(f);
            return p;
        }
    }
    return {};
}

} // namespace

BestiaryTestHarness::BestiaryTestHarness() {
    enemiesPath_ = resolveScriptPath("Enemies.lua");
    equipmentPath_ = resolveScriptPath("Equipment.lua");
    if (enemiesPath_.empty() || equipmentPath_.empty()) {
        return; // loaded_ stays false; callers SKIP when a script is missing.
    }

    lua_.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string,
                        sol::lib::math);

    // ── Equipment.lua exposes a GLOBAL `equipment = { ... }` table (mirrors the
    //    C++ loader in Engine.cpp). Load it and copy each entry in order. ──────────
    {
        sol::protected_function_result r = lua_.safe_script_file(
            equipmentPath_, sol::script_pass_on_error);
        if (!r.valid()) {
            return;
        }
        sol::object equipObj = lua_["equipment"];
        if (equipObj.get_type() == sol::type::table) {
            sol::table equipTbl = equipObj.as<sol::table>();
            for (std::size_t i = 1; i <= equipTbl.size(); ++i) {
                sol::optional<sol::table> e = equipTbl[i];
                if (!e) continue;
                EquipmentEntry entry;
                entry.table = *e;
                sol::optional<std::string> n = (*e)["name"];
                entry.name = n.value_or(std::string());
                equipment_.push_back(std::move(entry));
            }
        }
    }

    // ── Enemies.lua keeps its `enemies` list LOCAL, exposed only through
    //    spawnEnemy(roll, x, y, region) -> addActor(x, y, entry). Register a spy
    //    addActor that collects each unique entry, then drive spawnEnemy across the
    //    full roll range for every faction column to surface every declared entry
    //    in first-seen (declaration) order. ────────────────────────────────────────
    {
        std::vector<EnemyEntry>& collected = enemies_;
        // Track seen names to dedupe across the many spawn probes.
        auto alreadySeen = [&collected](const std::string& name) {
            for (const auto& e : collected) {
                if (e.name == name) return true;
            }
            return false;
        };

        lua_.set_function("addActor", [&](int /*x*/, int /*y*/, sol::table entry) {
            sol::optional<std::string> n = entry["name"];
            const std::string name = n.value_or(std::string());
            if (!alreadySeen(name)) {
                EnemyEntry e;
                e.name = name;
                e.table = entry;
                collected.push_back(std::move(e));
            }
        });

        sol::protected_function_result r = lua_.safe_script_file(
            enemiesPath_, sol::script_pass_on_error);
        if (!r.valid()) {
            return;
        }

        sol::protected_function spawnEnemy = lua_["spawnEnemy"];
        if (!spawnEnemy.valid()) {
            return;
        }

        // Probe every faction column across the full roll domain [0, 100]. Each
        // column surfaces its own entries in declaration order; the dedupe keeps
        // first-seen order stable across columns.
        for (const auto& region : factionRegions()) {
            for (int roll = 0; roll <= 100; ++roll) {
                (void)spawnEnemy(roll, 0, 0, region);
            }
        }
    }

    loaded_ = true;
}

std::vector<EnemyEntry> BestiaryTestHarness::columnFor(const std::string& region) const {
    std::vector<EnemyEntry> column;
    for (const auto& e : enemies_) {
        if (e.chanceFor(region).has_value()) {
            column.push_back(e);
        }
    }
    return column;
}

std::optional<EnemyEntry> BestiaryTestHarness::enemyByName(const std::string& name) const {
    for (const auto& e : enemies_) {
        if (e.name == name) return e;
    }
    return std::nullopt;
}

std::optional<EquipmentEntry> BestiaryTestHarness::equipmentByName(const std::string& name) const {
    for (const auto& e : equipment_) {
        if (e.name == name) return e;
    }
    return std::nullopt;
}

BestiaryTestHarness::SpawnResult BestiaryTestHarness::spawn(int roll, int x, int y,
                                                            const std::string& region) {
    SpawnResult result;
    if (!loaded_) {
        return result;
    }

    // Swap in a stub addActor for this call that records the selected entry (name +
    // coordinates) instead of building an Actor. This keeps the harness engine-
    // isolated: no MonsterDestructible / Attacker / MonsterAi construction.
    lua_.set_function("addActor", [&](int ax, int ay, sol::table entry) {
        result.called = true;
        sol::optional<std::string> n = entry["name"];
        result.name = n.value_or(std::string());
        result.x = ax;
        result.y = ay;
    });

    sol::protected_function spawnEnemy = lua_["spawnEnemy"];
    if (!spawnEnemy.valid()) {
        return result;
    }
    sol::protected_function_result r = spawnEnemy(roll, x, y, region);
    result.ok = r.valid();
    return result;
}

} // namespace bestiary
