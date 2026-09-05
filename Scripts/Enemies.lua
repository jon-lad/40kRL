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
    -- ─────────────────────────────────────────────────────────────────────────
    -- Chaos Faction_Region column (Bestiary IV.1 Renegades/Heretics/Mutants +
    -- IV.3 Daemons of Chaos). 14 entries, cumulative chance strictly ascending in
    -- declaration order, terminal value 100. Characteristics copied verbatim as the
    -- base integer (em-dash -> 0; Unnatural parentheticals excluded from the stored
    -- integer and recorded as `Unnatural X (xN)` traits). hp = profile Wounds.
    -- Equipment left empty for now; weapon/armour wiring is handled by tasks 12.3/12.4.
    -- ─────────────────────────────────────────────────────────────────────────

    -- IV.1 — Chaos Cultist (Troop): WS 30 BS 30 S 30 T 30 Ag 30 Int 25 Per 30 WP 25 Fel 30, Wounds 9
    {
        chance  = { Chaos = 15 },
        glyph   = string.byte("c"),
        name    = "Chaos Cultist",
        color   = "darkRed",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Chaos Cultist",
        xp      = 18,
        power   = 2.0,
        skill   = 30,                    -- WS (melee/pistol)
        ws = 30, bs = 30, s = 30, t = 30, ag = 30, int = 25, per = 30, wp = 25, fel = 30,
        equipment = {},
        skills  = { Awareness = 0, Deceive = 1, Dodge = 0,
                    ["Forbidden Lore (Heresy)"] = 0, Interrogation = 0,
                    Invocation = 0, Stealth = 1 },
        talents = { "Consumed By Spite" },
        traits  = {},
    },

    -- IV.1 — Mutant Fighter (Troop): WS 40 BS 35 S 40 T 40 Ag 30 Int 25 Per 35 WP 30 Fel 20, Wounds 12
    {
        chance  = { Chaos = 25 },
        glyph   = string.byte("m"),
        name    = "Mutant Fighter",
        color   = "darkRed",
        hp      = 12.0,
        defense = 0.0,
        corpse  = "dead Mutant Fighter",
        xp      = 26,
        power   = 3.0,
        skill   = 40,                    -- WS (melee-primary)
        ws = 40, bs = 35, s = 40, t = 40, ag = 30, int = 25, per = 35, wp = 30, fel = 20,
        equipment = {},
        skills  = { Acrobatics = 1, Athletics = 1, Awareness = 0, Command = 1,
                    Dodge = 1, Fortitude = 0, Parry = 1, Stealth = 0, Survival = 0 },
        talents = { "Berserk Charge", "Combat Rage", "Double Team", "Frenzy", "Jaded" },
        traits  = { "Fear (1)" },
    },

    -- IV.1 — Dark Disciple (Troop): WS 20 BS 20 S 25 T 25 Ag 25 Int 40 Per 25 WP 40 Fel 25, Wounds 6
    {
        chance  = { Chaos = 32 },
        glyph   = string.byte("d"),
        name    = "Dark Disciple",
        color   = "violet",
        hp      = 6.0,
        defense = 0.0,
        corpse  = "dead Dark Disciple",
        xp      = 22,
        power   = 2.0,
        skill   = 20,                    -- WS (Ritual Dagger melee)
        ws = 20, bs = 20, s = 25, t = 25, ag = 25, int = 40, per = 25, wp = 40, fel = 25,
        equipment = {},
        skills  = { ["Deny The Witch"] = 0,
                    ["Forbidden Lore (Daemonology, Heresy, The Warp)"] = 1,
                    Invocation = 1, Psyniscience = 0,
                    ["Scholastic Lore (Numerology, Occult)"] = 1, Scrutiny = 1 },
        talents = { "Cold Hearted", "Consumed By Spite", "Jaded" },
        traits  = {},
    },

    -- IV.1 — Mutant Wretch (Troop): WS 30 BS 20 S 20 T 25 Ag 35 Int 20 Per 40 WP 25 Fel 15, Wounds 6
    {
        chance  = { Chaos = 42 },
        glyph   = string.byte("w"),
        name    = "Mutant Wretch",
        color   = "brown",
        hp      = 6.0,
        defense = 0.0,
        corpse  = "dead Mutant Wretch",
        xp      = 16,
        power   = 2.0,
        skill   = 30,                    -- WS (Mutated Limbs melee)
        ws = 30, bs = 20, s = 20, t = 25, ag = 35, int = 20, per = 40, wp = 25, fel = 15,
        equipment = {},
        skills  = { Acrobatics = 1, Athletics = 2, Awareness = 1, Dodge = 0,
                    ["Navigate (Interior -or- Surface)"] = 1, Stealth = 1, Survival = 1 },
        talents = { "Ambush", "Combat Sense", "Deference For Darkness",
                    "Keen Intuition", "Overlooked", "Wrestler" },
        traits  = { "Deadly Natural Weapons (Mutated Limbs)", "Fear (1)" },
    },

    -- IV.1 — Renegade Soldier (Troop): WS 35 BS 35 S 35 T 35 Ag 30 Int 25 Per 35 WP 30 Fel 30, Wounds 9
    {
        chance  = { Chaos = 55 },
        glyph   = string.byte("r"),
        name    = "Renegade Soldier",
        color   = "darkRed",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Renegade Soldier",
        xp      = 24,
        power   = 3.0,
        skill   = 35,                    -- BS (Lasgun-primary)
        ws = 35, bs = 35, s = 35, t = 35, ag = 30, int = 25, per = 35, wp = 30, fel = 30,
        equipment = {},
        skills  = { Athletics = 1, Awareness = 0, ["Common Lore (War)"] = 0,
                    Discipline = 0, Dodge = 1, Fortitude = 0, Intimidate = 0,
                    ["Operate (Surface)"] = 1, Survival = 0 },
        talents = { "Jaded", "Ripper Charge" },
        traits  = {},
    },

    -- IV.1 — Renegade Veteran (Troop): WS 45 BS 45 S 45 T 45 Ag 35 Int 25 Per 40 WP 35 Fel 30, Wounds 12
    {
        chance  = { Chaos = 62 },
        glyph   = string.byte("R"),
        name    = "Renegade Veteran",
        color   = "red",
        hp      = 12.0,
        defense = 1.0,
        corpse  = "dead Renegade Veteran",
        xp      = 48,
        power   = 3.0,
        skill   = 45,                    -- BS (Hellgun/Hot-Shot-primary)
        ws = 45, bs = 45, s = 45, t = 45, ag = 35, int = 25, per = 40, wp = 35, fel = 30,
        equipment = {},
        skills  = { Athletics = 1, Awareness = 1, Command = 1, ["Common Lore (War)"] = 0,
                    Discipline = 1, Dodge = 2, Fortitude = 1, Intimidate = 0,
                    ["Operate (Surface)"] = 1, Parry = 1, Survival = 0 },
        talents = { "Jaded", "Lasgun Drill", "Last Man Standing", "Litany Against Fear",
                    "Marksman", "Paranoia", "Pity The Weak", "Quick Draw",
                    "Rapid Reload", "Ripper Charge", "Vigilant" },
        traits  = {},
    },

    -- IV.3 — Nurgling (Troop): WS 40 BS 20 S 20 T 20(4) Ag 40 Int 30 Per 20 WP 30(4) Fel 20, Wounds 8
    {
        chance  = { Chaos = 70 },
        glyph   = string.byte("n"),
        name    = "Nurgling",
        color   = "desaturatedGreen",
        hp      = 8.0,
        defense = 0.0,
        corpse  = "burst Nurgling",
        xp      = 20,
        power   = 2.0,
        skill   = 40,                    -- WS (Claws & Teeth melee)
        ws = 40, bs = 20, s = 20, t = 20, ag = 40, int = 30, per = 20, wp = 30, fel = 20,
        equipment = {},
        skills  = { Awareness = 1, Dodge = 0, ["Forbidden Lore (Daemonology)"] = 0,
                    Fortitude = 0, Stealth = 1 },
        talents = { "Underfoot Assault" },
        traits  = { "Daemonic (1)", "Dark-Sight", "Natural Weapons (Claws & Teeth)",
                    "Fear (1)", "From Beyond", "Nauseating", "Size (2)", "Toxic (1)",
                    "Unnatural Toughness (x1)", "Unnatural Willpower (x1)" },
    },

    -- IV.3 — Brimstone Horror (Troop): WS 10 BS 15 S 10 T 15(2) Ag 35 Int 10 Per 25 WP 30(4) Fel 01, Wounds 4
    {
        chance  = { Chaos = 76 },
        glyph   = string.byte("h"),
        name    = "Brimstone Horror",
        color   = "orange",
        hp      = 4.0,
        defense = 0.0,
        corpse  = "guttered Brimstone Horror",
        xp      = 12,
        power   = 2.0,
        skill   = 10,                    -- WS (Flaming Claws & Teeth melee)
        ws = 10, bs = 15, s = 10, t = 15, ag = 35, int = 10, per = 25, wp = 30, fel = 1,
        equipment = {},
        skills  = { Awareness = 0, Dodge = 0, ["Forbidden Lore (Daemonology)"] = 0,
                    Invocation = 0, Psyniscience = 1 },
        talents = { "Ambidextrous", "Psy Rating (2)", "Two Weapon Wielder (Melee)",
                    "Warp Sense" },
        traits  = { "Blessing of Tzeentch", "Daemonic (1)", "Dark-Sight",
                    "Deadly Natural Weapons (Flaming Claws & Teeth)", "Fear (2)",
                    "From Beyond", "Multiple Arms (1d5+1)", "Size (2)",
                    "Unnatural Willpower (x1)", "Warp Instability" },
    },

    -- IV.3 — Blue Horror (Troop): WS 20 BS 25 S 20 T 30(5) Ag 40 Int 25 Per 30 WP 30(5) Fel 05, Wounds 10
    {
        chance  = { Chaos = 82 },
        glyph   = string.byte("h"),
        name    = "Blue Horror",
        color   = "lightBlue",
        hp      = 10.0,
        defense = 0.0,
        corpse  = "split Blue Horror",
        xp      = 18,
        power   = 2.0,
        skill   = 20,                    -- WS (Claws & Jaws melee)
        ws = 20, bs = 25, s = 20, t = 30, ag = 40, int = 25, per = 30, wp = 30, fel = 5,
        equipment = {},
        skills  = { Awareness = 0, Discipline = 0, Dodge = 0,
                    ["Forbidden Lore (Daemonology)"] = 0, Invocation = 0,
                    Psyniscience = 1 },
        talents = { "Ambidextrous", "Psy Rating (2)", "Two Weapon Wielder (Melee)",
                    "Warp Sense" },
        traits  = { "Baneful Presence (5)", "Blessing of Tzeentch", "Daemonic (2)",
                    "Dark-Sight", "Deadly Natural Weapons (Claws & Jaws)", "Fear (2)",
                    "From Beyond", "Multiple Arms (1d5+1)", "Size (3)",
                    "Psyker (Daemonic)", "Unnatural Willpower (x2)", "Warp Instability" },
    },

    -- IV.3 — Screamer of Tzeentch (Troop): WS 45 BS 30 S 50 T 40(7) Ag 50 Int 15 Per 35 WP 40(6) Fel 10, Wounds 31
    -- NOTE: the tasks.md line flags Screamer as a Wounds-0 profile to floor to hp >= 1,
    -- but Reference/RT-Bestiary.md §IV.3 prints Wounds 31 for this profile, so hp = 31.
    -- (The Wounds-0 "values appear omitted" note in the reference belongs to the
    -- Keeper of Secrets Master profile, which is out of scope.)
    {
        chance  = { Chaos = 87 },
        glyph   = string.byte("S"),
        name    = "Screamer of Tzeentch",
        color   = "lightBlue",
        hp      = 31.0,
        defense = 0.0,
        corpse  = "dissipated Screamer",
        xp      = 40,
        power   = 4.0,
        skill   = 45,                    -- WS (Warp Jaws melee)
        ws = 45, bs = 30, s = 50, t = 40, ag = 50, int = 15, per = 35, wp = 40, fel = 10,
        equipment = {},
        skills  = { Acrobatics = 2, Awareness = 2, Dodge = 0, Fortitude = 1,
                    Psyniscience = 0 },
        talents = { "Assassin Strike", "Berserk Charge", "Hammer Blow", "Hard Target",
                    "Inescapable Attack", "War Cry", "Warp Sense" },
        traits  = { "Baneful Presence (10)", "Bestial (Frenzy)", "Blessing of Tzeentch",
                    "Daemonic (3)", "Dark-Sight", "Deadly Natural Weapons (Warp Jaws)",
                    "Fear (2)", "Flyer (8)", "From Beyond", "Mount", "Size (5)",
                    "Unnatural Willpower (x2)", "Warp Instability" },
    },

    -- IV.3 — Ebon Gheist (Troop): WS 35 BS — S 35 T 40(6) Ag 45(5) Int 15 Per 45 WP 40 Fel 01, Wounds 18
    {
        chance  = { Chaos = 91 },
        glyph   = string.byte("G"),
        name    = "Ebon Gheist",
        color   = "darkGrey",
        hp      = 18.0,
        defense = 0.0,
        corpse  = "banished Ebon Gheist",
        xp      = 34,
        power   = 3.0,
        skill   = 35,                    -- WS (Chill Talons melee; BS em-dash -> 0)
        ws = 35, bs = 0, s = 35, t = 40, ag = 45, int = 15, per = 45, wp = 40, fel = 1,
        equipment = {},
        skills  = { Discipline = 0, Dodge = 0, Psyniscience = 0, Stealth = 2 },
        talents = { "Ambush", "Hard Target", "Heightened Senses (All)" },
        traits  = { "Baneful Presence (20)", "Daemonic (2)", "Dark-Sight",
                    "Deadly Natural Weapons (Chill Talons)", "Fear (2)", "Phase",
                    "Size (4)", "Unnatural Agility (x1)", "Warp Instability" },
    },

    -- IV.3 — Chaos Fury (Troop): WS 35 BS 15 S 30 T 35(4) Ag 40 Int 20 Per 45 WP 25(3) Fel 10, Wounds 12
    {
        chance  = { Chaos = 95 },
        glyph   = string.byte("f"),
        name    = "Chaos Fury",
        color   = "darkRed",
        hp      = 12.0,
        defense = 0.0,
        corpse  = "dead Chaos Fury",
        xp      = 26,
        power   = 3.0,
        skill   = 35,                    -- WS (Jaws & Claws melee)
        ws = 35, bs = 15, s = 30, t = 35, ag = 40, int = 20, per = 45, wp = 25, fel = 10,
        equipment = {},
        skills  = { Acrobatics = 2, Dodge = 1, Psyniscience = 0 },
        talents = { "Assassin Strike", "Berserk Charge", "Heightened Sense (Smell)",
                    "Raptor" },
        traits  = { "Baneful Presence (5)", "Bestial (Flee)", "Daemonic (1)",
                    "Dark-Sight", "Deadly Natural Weapons (Jaws & Claws)", "Flyer (8)",
                    "From Beyond", "Size (4)", "Unnatural Willpower (x1)" },
    },

    -- IV.3 — Nether Spawn (Troop): WS 35 BS 10 S 15 T 20 Ag 50 Int 10 Per 40 WP 20 Fel 01, Wounds 4
    {
        chance  = { Chaos = 98 },
        glyph   = string.byte("f"),
        name    = "Nether Spawn",
        color   = "violet",
        hp      = 4.0,
        defense = 0.0,
        corpse  = "dead Nether Spawn",
        xp      = 14,
        power   = 2.0,
        skill   = 35,                    -- WS (Needle Teeth melee)
        ws = 35, bs = 10, s = 15, t = 20, ag = 50, int = 10, per = 40, wp = 20, fel = 1,
        equipment = {},
        skills  = { Awareness = 0, Dodge = 1, Psyniscience = 1, Stealth = 2 },
        talents = { "Heightened Senses (Smell)" },
        traits  = { "Bestial (Flee)", "Daemonic (1)",
                    "Deadly Natural Weapons (Needle Teeth)", "Flyer (3)",
                    "From Beyond", "Size (2)" },
    },

    -- IV.3 — Gibbering Malingerer (Troop): WS 25 BS 01 S 20 T 25(3) Ag 40 Int 05 Per 25 WP 25 Fel 01, Wounds 6
    {
        chance  = { Chaos = 100 },
        glyph   = string.byte("g"),
        name    = "Gibbering Malingerer",
        color   = "violet",
        hp      = 6.0,
        defense = 0.0,
        corpse  = "dead Gibbering Malingerer",
        xp      = 16,
        power   = 2.0,
        skill   = 25,                    -- WS (Entropic Touch melee)
        ws = 25, bs = 1, s = 20, t = 25, ag = 40, int = 5, per = 25, wp = 25, fel = 1,
        equipment = {},
        skills  = { Athletics = 2, Awareness = 2, Psyniscience = 1 },
        talents = { "Double Team", "Preternatural Speed", "Wrestler" },
        traits  = { "Burrower (3)", "Daemonic (1)",
                    "Deadly Natural Weapons (Entropic Touch)", "Fear (2)",
                    "From Beyond", "Size (3)", "Critical Mass (Horde)",
                    "The Stuff of Nightmares", "Warp Instability" },
    },

    -- ─────────────────────────────────────────────────────────────────────────
    -- Ork Faction_Region column — LEGACY flat distribution, rebuilt from the
    -- bestiary IV.7 Troop set by task 6.2. Left unchanged here.
    -- ─────────────────────────────────────────────────────────────────────────
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
        glyph   = string.byte("s"),
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
