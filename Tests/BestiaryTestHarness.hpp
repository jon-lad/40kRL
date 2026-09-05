#pragma once

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: bestiary-npcs — sol2-based data test harness (Task 1.1)
//
// Loads Scripts/Enemies.lua and Scripts/Equipment.lua through a private sol::state
// WITHOUT initialising the global Engine (no SDL window, no libtcod context, no Gui,
// no Map), per the test-isolation steering. The harness reads the `enemies` list and
// the `equipment` table into C++ structures and drives the real spawnEnemy through a
// stub addActor that records the selected entry instead of constructing an Actor.
//
// Requirements covered by this harness: 5.1 (schema fields), 10.1 (spawnEnemy
// signature unchanged), 10.2 (spawnEnemy calls addActor with the selected entry).
// ═══════════════════════════════════════════════════════════════════════════════

#include <sol/sol.hpp>

#include <optional>
#include <string>
#include <vector>

namespace bestiary {

// The canonical ten Faction_Region column keys, per Requirement 6.1 / design's
// Faction-Region Model. Declaration order is the design's table order.
inline const std::vector<std::string>& factionRegions() {
    static const std::vector<std::string> regions = {
        "Chaos", "Eldar", "DarkEldar", "Necron", "Ork",
        "Tau", "Tyranid", "ImperialHuman", "Servitor", "Warp",
    };
    return regions;
}

// A lightweight view of a single Enemy_Entry: the raw sol::table plus a cached
// `name`. Field access goes through the sol::table so every existing schema field
// (glyph, color, hp, chance, skills, talents, traits, equipment, ...) is reachable
// without duplicating the schema in C++.
struct EnemyEntry {
    std::string name;
    sol::table table;

    // Read a typed field with a fallback. Uses sol::optional coercion to avoid the
    // ambiguous single-template get_or overload in this sol version.
    template <typename T>
    T fieldOr(const char* key, T fallback) const {
        sol::optional<T> v = table[key];
        return v ? *v : fallback;
    }

    // The cumulative chance threshold for a region column, or nullopt when this
    // entry has no key for that region (mirrors spawnEnemy's `e.chance[region]`).
    std::optional<int> chanceFor(const std::string& region) const {
        sol::object chanceObj = table["chance"];
        if (chanceObj.get_type() != sol::type::table) {
            return std::nullopt;
        }
        sol::table chanceTbl = chanceObj.as<sol::table>();
        sol::optional<int> v = chanceTbl[region];
        if (v) return *v;
        return std::nullopt;
    }
};

// A lightweight view of a single Equipment_Entry.
struct EquipmentEntry {
    std::string name;
    sol::table table;

    template <typename T>
    T fieldOr(const char* key, T fallback) const {
        sol::optional<T> v = table[key];
        return v ? *v : fallback;
    }
};

// Loads the two scripts once and exposes the parsed data plus a spawnEnemy driver.
// Construct one per test (or reuse); construction is cheap relative to a test run.
class BestiaryTestHarness {
public:
    BestiaryTestHarness();

    // True when both scripts resolved on disk and loaded without error.
    bool loaded() const { return loaded_; }

    // Resolved script paths (empty when a script could not be found on disk).
    const std::string& enemiesPath() const { return enemiesPath_; }
    const std::string& equipmentPath() const { return equipmentPath_; }

    // All Enemy_Entry records in declaration order (surfaced via the spawnEnemy
    // probe across every faction column and the full roll range).
    const std::vector<EnemyEntry>& enemies() const { return enemies_; }

    // All Equipment_Entry records in declaration order.
    const std::vector<EquipmentEntry>& equipment() const { return equipment_; }

    // The ten Faction_Region keys.
    const std::vector<std::string>& regions() const { return factionRegions(); }

    // The subset of enemies whose `chance` table contains `region`, in declaration
    // order — i.e. the faction column for that region.
    std::vector<EnemyEntry> columnFor(const std::string& region) const;

    // Look up a single enemy / equipment entry by exact name.
    std::optional<EnemyEntry> enemyByName(const std::string& name) const;
    std::optional<EquipmentEntry> equipmentByName(const std::string& name) const;

    // Result of driving spawnEnemy(roll, x, y, region) through the stub addActor.
    struct SpawnResult {
        bool called = false;         // was addActor invoked?
        std::string name;            // selected entry's name (empty when not called)
        int x = 0;                   // x passed through to addActor
        int y = 0;                   // y passed through to addActor
        bool ok = false;             // did the Lua call itself succeed (no error)?
    };

    // Invoke the real spawnEnemy with a stub addActor that records the selected
    // entry instead of building an Actor. Requirement 10.1 (signature) / 10.2
    // (addActor receives the selected entry).
    SpawnResult spawn(int roll, int x, int y, const std::string& region);

private:
    sol::state lua_;
    bool loaded_ = false;
    std::string enemiesPath_;
    std::string equipmentPath_;
    std::vector<EnemyEntry> enemies_;
    std::vector<EquipmentEntry> equipment_;
};

} // namespace bestiary
