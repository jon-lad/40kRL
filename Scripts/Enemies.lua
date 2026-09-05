-- Enemies.lua
-- Defines enemy templates used by Map::addMonster.
-- C++ calls: spawnEnemy(roll, x, y, region)
--   roll   : int    (0-100 random roll)
--   x, y   : int    (world position)
--   region : string (Region_Name naming the active level's region column)
-- The function calls back into C++ via the injected addActor(x, y, entry) function,
-- passing the entire enemy table entry so C++ can read all fields including equipment config.

-- ─────────────────────────────────────────────────────────────────────────────
-- Faction_Region convention
-- ─────────────────────────────────────────────────────────────────────────────
-- Each faction owns exactly one Faction_Region column. There are ten Region_Name
-- values, and every Enemy_Entry declares a SINGLE-KEY `chance` table naming its own
-- faction, e.g. `chance = { Tyranid = 40 }`. Because spawnEnemy reads
-- `e.chance[region]` and skips entries whose table lacks the requested key, an entry
-- is only ever selectable in its own faction's region (faction purity).
--
-- The ten Faction_Region keys (declaration/design order):
--   Chaos          -- Renegades/Heretics/Mutants + Daemons (Bestiary IV.1, IV.3)
--   Eldar          -- Craftworld Eldar + Eldar Corsair     (IV.4, §5.3)
--   DarkEldar      -- Harlequins & Dark Eldar              (IV.5)
--   Necron         -- Necrons                              (IV.6)
--   Ork            -- Orks + Ork Freebooter                (IV.7, §5.3)
--   Tau            -- Tau/Kroot/Vespid + Kroot Mercenary   (IV.8, §5.3)
--   Tyranid        -- Tyranids                             (IV.9)
--   ImperialHuman  -- Imperial Humans (Colonist + variants + officers) (§5.1)
--   Servitor       -- Servitors                            (§5.2)
--   Warp           -- Denizens of the Warp                 (§5.4)
--
-- Within a Faction_Region column the cumulative `chance` values are strictly
-- ascending in declaration order, and the final entry in each column is exactly 100.
--
-- ─────────────────────────────────────────────────────────────────────────────
-- Deferred scope (future work — intentionally NOT in this data set)
-- ─────────────────────────────────────────────────────────────────────────────
--   * Elite-tier and Master-tier NPCs. Only Troop-tagged profiles and the six
--     Colonist variants are in scope; Elite/Master profiles are deferred. The schema
--     and the spawnEnemy signature are unchanged, so Elite/Master entries can be
--     added later without structural change.
--   * Weighted multi-faction regions. A whole level is designated a single
--     Faction_Region; weighted mixes across factions are deferred.
--   * Per-tile spawn granularity. Region designation is per level, not per tile.
--   * Vehicles and war machines live in `Reference/vehicles.md`, not the bestiary
--     creature set, and are out of scope for this feature.
-- ─────────────────────────────────────────────────────────────────────────────

-- Enemy definitions table.
-- Fields: chance (single-key per-faction cumulative % table keyed by Region_Name),
--         glyph, name, color, hp, defense, corpse, xp, power, skill,
--         and the nine characteristics (ws, bs, s, t, ag, int, per, wp, fel).
-- Optional equipment fields: equipment (list of strings), dropChance (float), equipTier (table)
-- Optional stat-block fields: skills (name->rank table), talents (string list), traits (string list)
--
-- NOTE: The faction data entries are populated by subsequent tasks (Chaos, Eldar,
-- DarkEldar, Necron, Ork reconciliation, Tau, Tyranid, ImperialHuman, Servitor,
-- Warp). The Ork column below is the legacy flat distribution retained until the
-- Ork reconciliation task rebuilds it from the bestiary IV.7 Troop set:
--   Gretchin 50, Ork 75, Shoota Boy 90, Nob 100.
local enemies = {
    {
        chance  = { Ork = 50 },
        glyph   = string.byte("g"),
        name    = "Gretchin",
        color   = "desaturatedGreen",
        hp      = 15.0,
        defense = 0.0,
        corpse  = "dead Gretchin",
        xp      = 15,
        power   = 2.0,
        skill   = 25,
        ws = 25, bs = 15, s = 20, t = 20, ag = 30, int = 15, per = 25, wp = 15, fel = 10,
        equipment = { "Choppa" },
        dropChance = 0.3,
        -- Choppa is weaponGroup "Primitive" (Equipment.lua) -> Weapon Training (Primitive).
        skills  = { Dodge = 0, Awareness = 0 },
        talents = { "Weapon Training (Primitive)" },
        traits  = { "Size (Puny)", "Cowardly" },
    },
    {
        chance  = { Ork = 75 },
        glyph   = string.byte("o"),
        name    = "Ork",
        color   = "desaturatedGreen",
        hp      = 20.0,
        defense = 0.0,
        corpse  = "dead Ork",
        xp      = 35,
        power   = 3.0,
        skill   = 35,
        ws = 35, bs = 20, s = 40, t = 40, ag = 25, int = 15, per = 25, wp = 25, fel = 15,
        equipTier = { common = 80, uncommon = 18, rare = 2 },
        dropChance = 0.4,
        -- Orks favour primitive choppas (common/uncommon melee are weaponGroup "Primitive").
        skills  = { Dodge = 0 },
        talents = { "Weapon Training (Primitive)" },
        traits  = { "Sturdy", "Mob Rule" },
    },
    {
        chance  = { Ork = 90 },
        glyph   = string.byte("o"),
        name    = "Shoota Boy",
        color   = "desaturatedGreen",
        hp      = 20.0,
        defense = 0.0,
        corpse  = "dead Shoota Boy",
        xp      = 40,
        power   = 3.0,
        skill   = 35,
        ws = 30, bs = 25, s = 35, t = 40, ag = 25, int = 15, per = 25, wp = 25, fel = 15,
        equipment = { "Shoota" },
        dropChance = 0.4,
        -- Shoota is weaponGroup "SP" (Equipment.lua) -> Weapon Training (SP).
        skills  = { Dodge = 0, Awareness = 0 },
        talents = { "Weapon Training (SP)" },
        traits  = { "Sturdy" },
    },
    {
        chance  = { Ork = 100 },
        glyph   = string.byte("N"),
        name    = "Nob",
        color   = "darkerGreen",
        hp      = 26.0,
        defense = 1.0,
        corpse  = "Nob carcass",
        xp      = 100,
        power   = 4.0,
        skill   = 45,
        ws = 45, bs = 25, s = 50, t = 50, ag = 30, int = 20, per = 30, wp = 35, fel = 25,
        equipment = { "Big Choppa", "Ork Armor" },
        dropChance = 0.5,
        -- Big Choppa is weaponGroup "Primitive" (Equipment.lua) -> Weapon Training (Primitive).
        skills  = { Dodge = 1, Awareness = 1 },
        talents = { "Weapon Training (Primitive)" },
        traits  = { "Sturdy", "Brutal Charge" },
    },
}

function spawnEnemy(roll, x, y, region)
    for _, e in ipairs(enemies) do
        local threshold = e.chance[region]           -- nil if this entry has no column for region
        if threshold ~= nil and roll < threshold then
            addActor(x, y, e)
            return
        end
    end
    -- No matching entry for this region column -> spawn nothing, leaving the tile unoccupied.
end
