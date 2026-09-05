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
-- Warp). The Ork column has been rebuilt from the bestiary IV.7 Troop set plus the
-- Ork Freebooter (§5.3); the legacy flat distribution (Gretchin/Ork/Shoota Boy/Nob)
-- has been retired per the reconciliation table documented above the Ork column.
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
    -- Eldar Faction_Region column (Bestiary IV.4 Craftworld Eldar + Eldar Corsair
    -- §5.3). 3 entries, cumulative chance strictly ascending in declaration order,
    -- terminal value 100. Characteristics copied verbatim as the base integer
    -- (Unnatural parentheticals excluded from the stored integer and recorded as
    -- `Unnatural X (xN)` traits). hp = profile Wounds. Each (glyph, color) pair is
    -- distinct from every other entry across the Bestiary, using colors defined in
    -- Headers/Colors.hpp. Equipment left empty for now; weapon/armour wiring is
    -- handled by tasks 12.3/12.4.
    -- ─────────────────────────────────────────────────────────────────────────

    -- IV.4 — Eldar Guardian (Troop): WS 40 BS 40 S 35 T 35 Ag 40(6) Int 35 Per 40(5) WP 35 Fel 30, Wounds 9
    {
        chance  = { Eldar = 45 },
        glyph   = string.byte("e"),
        name    = "Eldar Guardian",
        color   = "lightBlue",
        hp      = 9.0,
        defense = 1.0,
        corpse  = "dead Eldar Guardian",
        xp      = 30,
        power   = 3.0,
        skill   = 40,                    -- BS (Shuriken Catapult ranged-primary)
        ws = 40, bs = 40, s = 35, t = 35, ag = 40, int = 35, per = 40, wp = 35, fel = 30,
        equipment = {},
        skills  = { Acrobatics = 1, Awareness = 0, Deceive = 1, Discipline = 0,
                    Dodge = 1, ["Linguistics (Aeldari)"] = 0, Medicae = 0,
                    ["Navigate (Surface, Webway)"] = 0,
                    ["Operate (Aeronautica, Surface)"] = 0, Parry = 0, Stealth = 1 },
        talents = { "Catfall", "Hip Shooting", "Leap Up", "Quick Draw" },
        traits  = { "Psyker", "Unnatural Agility (x2)", "Unnatural Perception (x1)" },
    },

    -- IV.4 — Eldar Ranger (Troop): WS 40 BS 50 S 35 T 35 Ag 50(8) Int 35 Per 40(6) WP 35 Fel 30, Wounds 9
    {
        chance  = { Eldar = 78 },
        glyph   = string.byte("e"),
        name    = "Eldar Ranger",
        color   = "lightGreen",
        hp      = 9.0,
        defense = 1.0,
        corpse  = "dead Eldar Ranger",
        xp      = 44,
        power   = 3.0,
        skill   = 50,                    -- BS (Eldar Long Rifle sniper ranged-primary)
        ws = 40, bs = 50, s = 35, t = 35, ag = 50, int = 35, per = 40, wp = 35, fel = 30,
        equipment = {},
        skills  = { Acrobatics = 1, Awareness = 2, Deceive = 1, Discipline = 0,
                    Dodge = 2, ["Linguistics (Aeldari)"] = 0, Medicae = 0,
                    ["Navigate (Surface, Webway)"] = 2,
                    ["Operate (Aeronautica, Surface)"] = 0, Stealth = 2, Survival = 1 },
        talents = { "Catfall", "Hip Shooting", "Keen Intuition", "Leap Up", "Marksman",
                    "Quick Draw", "Stealth Sniper", "Surefooted Wayfinder",
                    "Talented (Stealth)", "Target Selection" },
        traits  = { "Psyker", "Unnatural Agility (x3)", "Unnatural Perception (x2)" },
    },

    -- §5.3 — Eldar Corsair (Troop): WS 48 BS 48 S 33 T 35 Ag 52(10) Int 39 Per 40 WP 43 Fel 25, Wounds 12
    {
        chance  = { Eldar = 100 },
        glyph   = string.byte("E"),
        name    = "Eldar Corsair",
        color   = "cyan",
        hp      = 12.0,
        defense = 1.0,
        corpse  = "dead Eldar Corsair",
        xp      = 60,
        power   = 3.0,
        skill   = 48,                    -- BS (Shuriken Catapult ranged-primary)
        ws = 48, bs = 48, s = 33, t = 35, ag = 52, int = 39, per = 40, wp = 43, fel = 25,
        equipment = {},
        skills  = { Acrobatics = 0, Awareness = 1, Barter = 0, Deceive = 1, Dodge = 0,
                    Evaluate = 1,
                    ["Forbidden Lore (The Black Library, Xenos, The Warp)"] = 0,
                    Gamble = 0, ["Navigation (Stellar)"] = 0,
                    ["Pilot (Interface Craft, Jump Pack)"] = 0, Medicae = 0,
                    ["Silent Move"] = 1,
                    ["Speak Language (Eldar, Low Gothic, Void Cant)"] = 1 },
        talents = { "Basic Weapon Training (Las)", "Catfall",
                    "Exotic Weapon Training (Shuriken Catapult, Shuriken Pistol)",
                    "Leap Up", "Melee Weapon Training (Power, Primitive)",
                    "Pistol Weapon Training (Las)", "Quick Draw",
                    "Resistance (Fear, Psychic Techniques)", "Sprint" },
        -- Dark-Sight from gear; inbuilt void impellors grant Flyer (12) in null gravity.
        traits  = { "Unnatural Agility (x2)", "Dark-Sight", "Flyer (12)" },
    },

    -- ─────────────────────────────────────────────────────────────────────────
    -- DarkEldar Faction_Region column (Bestiary IV.5 Harlequins & Dark Eldar).
    -- 5 Troop entries, cumulative chance strictly ascending in declaration order,
    -- terminal value 100. Characteristics copied verbatim as the base integer
    -- (em-dash / "00" -> 0; Unnatural parentheticals excluded from the stored
    -- integer and recorded as `Unnatural X (xN)` traits). hp = profile Wounds.
    -- Each (glyph, color) pair is distinct from every other entry across the
    -- Bestiary, using colors defined in Headers/Colors.hpp. Equipment left empty
    -- for now; weapon/armour wiring is handled by tasks 12.3/12.4.
    -- ─────────────────────────────────────────────────────────────────────────

    -- IV.5 — Kabalite (Troop): WS 40 BS 40 S 30 T 35 Ag 55(8) Int 35 Per 40 WP 35 Fel 45, Wounds 9
    {
        chance  = { DarkEldar = 30 },
        glyph   = string.byte("k"),
        name    = "Kabalite",
        color   = "violet",
        hp      = 9.0,
        defense = 1.0,
        corpse  = "dead Kabalite",
        xp      = 34,
        power   = 3.0,
        skill   = 40,                    -- BS (Splinter Rifle ranged-primary)
        ws = 40, bs = 40, s = 30, t = 35, ag = 55, int = 35, per = 40, wp = 35, fel = 45,
        equipment = {},
        skills  = { Acrobatics = 1, Awareness = 1, Deceive = 1, Dodge = 1,
                    Interrogation = 0, Intimidate = 1, ["Linguistics (Aeldari)"] = 0,
                    ["Navigate (Interior, Webway)"] = 0,
                    ["Operate (Aeronautica, Surface)"] = 1, Parry = 0, Scrutiny = 0,
                    Stealth = 1 },
        talents = { "Catfall", "Hard Target", "Jaded", "Leap Up",
                    "Lightning Reflexes", "Quick Draw" },
        traits  = { "Dark-Sight", "Unnatural Agility (x3)" },
    },

    -- IV.5 — Wych (Troop): WS 45 BS 40 S 30 T 35 Ag 55(8) Int 35 Per 45 WP 30 Fel 45, Wounds 9
    -- NOTE: the tasks.md line flags Wych as a Wounds-0 profile to floor to hp >= 1,
    -- but Reference/RT-Bestiary.md §IV.5 prints Wounds 9 for this profile, so hp = 9.
    -- (The Wounds-0 "values appear omitted" note belongs to the Khymera/Sslyth Elite
    -- profiles, which are out of scope. hp is floored to >= 1 regardless.)
    {
        chance  = { DarkEldar = 55 },
        glyph   = string.byte("y"),
        name    = "Wych",
        color   = "darkRed",
        hp      = 9.0,                   -- Wounds 9 (>= 1 per floor rule)
        defense = 1.0,
        corpse  = "dead Wych",
        xp      = 36,
        power   = 3.0,
        skill   = 45,                    -- WS (melee/dueling-primary)
        ws = 45, bs = 40, s = 30, t = 35, ag = 55, int = 35, per = 45, wp = 30, fel = 45,
        equipment = {},
        skills  = { Acrobatics = 1, Athletics = 1, Awareness = 1, Charm = 1,
                    Deceive = 0, Dodge = 2, Intimidate = 0,
                    ["Linguistics (Aeldari)"] = 0, Parry = 1, Stealth = 1 },
        talents = { "Ambidextrous", "Assassin Strike", "Catfall", "Leap Up",
                    "Sprint", "Swift Attack", "Two-Weapon Wielder (Melee and Ranged)" },
        traits  = { "Dark-Sight", "Unnatural Agility (x3)" },
    },

    -- IV.5 — Hellion (Troop): WS 45 BS 45 S 30 T 35 Ag 60(9) Int 35 Per 40 WP 30 Fel 30, Wounds 9
    {
        chance  = { DarkEldar = 75 },
        glyph   = string.byte("H"),
        name    = "Hellion",
        color   = "lighterOrange",
        hp      = 9.0,
        defense = 1.0,
        corpse  = "dead Hellion",
        xp      = 40,
        power   = 3.0,
        skill   = 45,                    -- WS (Hellglaive melee-primary)
        ws = 45, bs = 45, s = 30, t = 35, ag = 60, int = 35, per = 40, wp = 30, fel = 30,
        equipment = {},
        skills  = { Acrobatics = 2, Awareness = 1, Deceive = 1, Dodge = 1,
                    Interrogation = 0, ["Linguistics (Aeldari)"] = 0,
                    ["Navigate (Interior, Webway)"] = 0,
                    ["Operate (Aeronautica, Surface)"] = 1, Parry = 0, Stealth = 1 },
        talents = { "Assassin Strike", "Catfall", "Furious Assault", "Hard Target",
                    "Hip Shooting", "Jaded", "Leap Up", "Lightning Reflexes",
                    "Quick Draw" },
        -- Skyboard grants Hoverer (AgB x2) -> represented as Flyer while mounted.
        traits  = { "Dark-Sight", "Power Through Pain (Perseverance)",
                    "Unnatural Agility (x3)", "Flyer (18)" },
    },

    -- IV.5 — Razorwing Flock (Troop): WS 47 BS — S 30 T 30 Ag 50 Int 30 Per 40 WP 20 Fel —, Wounds 6/29 (individual/swarm; individual used)
    {
        chance  = { DarkEldar = 90 },
        glyph   = string.byte("v"),
        name    = "Razorwing Flock",
        color   = "lightGrey",
        hp      = 6.0,                   -- Wounds 6 (individual profile)
        defense = 0.0,
        corpse  = "razorwing remains",
        xp      = 24,
        power   = 3.0,
        skill   = 47,                    -- WS (Beaks & Wings melee; BS em-dash -> 0)
        ws = 47, bs = 0, s = 30, t = 30, ag = 50, int = 30, per = 40, wp = 20, fel = 0,
        equipment = {},
        skills  = { Awareness = 0, Dodge = 1 },
        talents = { "Frenzy", "Furious Assault", "Lightning Attack",
                    "Lightning Reflexes", "Raptor", "Step Aside", "Swift Attack" },
        traits  = { "Bestial (Frenzy)", "Deadly Natural Weapons (Beaks, Wings)",
                    "Flyer (10)", "Natural Armour (2)", "Size (Swarm)", "Swarm" },
    },

    -- IV.5 — Ariadne Helspider (Troop): WS 40 BS 30 S 50 T 40 Ag 50(6) Int 10 Per 45 WP 30 Fel —, Wounds 8
    {
        chance  = { DarkEldar = 100 },
        glyph   = string.byte("a"),
        name    = "Ariadne Helspider",
        color   = "darkGrey",
        hp      = 8.0,
        defense = 1.0,
        corpse  = "dead Ariadne Helspider",
        xp      = 44,
        power   = 4.0,
        skill   = 40,                    -- WS (Time-Rending Mandible melee-primary)
        ws = 40, bs = 30, s = 50, t = 40, ag = 50, int = 10, per = 45, wp = 30, fel = 0,
        equipment = {},
        skills  = { Acrobatics = 2, Athletics = 3, Awareness = 0, Dodge = 1,
                    Psyniscience = 2, Stealth = 2 },
        talents = { "Ambush", "Assassin Strike", "Heightened Senses (Sight)",
                    "Leaping Strike", "Lightning Reflexes", "Step Aside", "Sure Strike",
                    "Talented (Athletics)", "Warp Sense" },
        traits  = { "Bestial (Flee)", "Dark-Sight",
                    "Deadly Natural Weapons (Time-Rending Mandibles, Webbing)",
                    "From Beyond", "Multiple Arms", "Natural Armour (4)", "Quadruped",
                    "Regeneration (2)", "Size (3)", "Strange Physiology",
                    "Unnatural Agility (x6)", "Unnatural Senses (20)", "Warp Weapons" },
    },

    -- ─────────────────────────────────────────────────────────────────────────
    -- Necron Faction_Region column (Bestiary IV.6 Necrons). 3 Troop entries,
    -- cumulative chance strictly ascending in declaration order, terminal value 100.
    -- Characteristics copied verbatim as the base integer (Fel "00"/"01" -> 0/1;
    -- Unnatural parentheticals excluded from the stored integer and recorded as
    -- `Unnatural X (xN)` traits). hp = profile Wounds. Machine and Size traits are
    -- recorded as trait strings (Req 3.3). Each (glyph, color) pair is distinct from
    -- every other entry across the Bestiary, using colors defined in
    -- Headers/Colors.hpp. Equipment left empty for now; weapon/armour wiring is
    -- handled by tasks 12.3/12.4.
    -- ─────────────────────────────────────────────────────────────────────────

    -- IV.6 — Necron Warrior (Troop): WS 30 BS 45 S 45(6) T 45(6) Ag 25 Int 25 Per 30 WP 45 Fel 01, Wounds 28
    {
        chance  = { Necron = 55 },
        glyph   = string.byte("n"),
        name    = "Necron Warrior",
        color   = "lightGrey",
        hp      = 28.0,
        defense = 1.0,
        corpse  = "collapsed Necron Warrior",
        xp      = 56,
        power   = 4.0,
        skill   = 45,                    -- BS (Gauss Flayer ranged-primary)
        ws = 30, bs = 45, s = 45, t = 45, ag = 25, int = 25, per = 30, wp = 45, fel = 1,
        equipment = {},
        skills  = { Athletics = 1, Fortitude = 0, Intimidate = 1 },
        talents = { "Crack Shot", "Hip Shooting" },
        traits  = { "Machine (6)", "Regeneration (3, Endless)", "Size (5)",
                    "Unnatural Strength (x2)", "Unnatural Toughness (x2)" },
    },

    -- IV.6 — Canoptek Scarab (Troop): WS 35 BS 01 S 10 T 20 Ag 35 Int 05 Per 20 WP 20 Fel 00, Wounds 4
    {
        chance  = { Necron = 78 },
        glyph   = string.byte("c"),
        name    = "Canoptek Scarab",
        color   = "lightGrey",
        hp      = 4.0,
        defense = 0.0,
        corpse  = "shattered Scarab",
        xp      = 12,
        power   = 2.0,
        skill   = 35,                    -- WS (Disruptor Mandibles melee; BS 01)
        ws = 35, bs = 1, s = 10, t = 20, ag = 35, int = 5, per = 20, wp = 20, fel = 0,
        equipment = {},
        skills  = { Awareness = 1, Dodge = 0, Stealth = 2 },
        talents = { "Underfoot Assault" },
        traits  = { "Deadly Natural Weapons (Disruptor Mandibles)", "Hoverer (4)",
                    "Machine (4)", "Processor Link", "Size (2)" },
    },

    -- IV.6 — Canoptek Scarab Swarm (Troop): WS 35 BS 01 S 10 T 20 Ag 35 Int 05 Per 20 WP 20 Fel 00, Wounds 16
    {
        chance  = { Necron = 100 },
        glyph   = string.byte("C"),
        name    = "Canoptek Scarab Swarm",
        color   = "lightGrey",
        hp      = 16.0,
        defense = 0.0,
        corpse  = "scattered Scarab Swarm",
        xp      = 32,
        power   = 3.0,
        skill   = 35,                    -- WS (Disruptor Mandibles melee; BS 01)
        ws = 35, bs = 1, s = 10, t = 20, ag = 35, int = 5, per = 20, wp = 20, fel = 0,
        equipment = {},
        skills  = { Awareness = 1, Dodge = 0, Stealth = 2 },
        talents = { "Combat Master", "Swift Attack" },
        traits  = { "Deadly Natural Weapons (Disruptor Mandibles)", "Hoverer (8)",
                    "Machine (4)", "Processor Link", "Regeneration (1d5)", "Size (5)" },
    },

    -- ─────────────────────────────────────────────────────────────────────────
    -- Ork Faction_Region column (Bestiary IV.7 Orks + Ork Freebooter §5.3).
    -- REBUILT from the bestiary IV.7 Troop set (task 6.2): the legacy four-entry
    -- flat distribution (Gretchin 50, Ork 75, Shoota Boy 90, Nob 100) is replaced.
    -- Reconciliation of the legacy names:
    --   * Gretchin  -> kept as a name; realigned to the IV.7 Gretchin profile.
    --   * Ork       -> retired; folded into "Ork Boy" (the IV.7 rank-and-file).
    --   * Shoota Boy -> retired; a shoota is an Ork Boy weapon option, folds into Ork Boy.
    --   * Nob       -> retired; the bestiary Nob is Elite (out of scope). Troop-tier
    --                  leadership is covered by Runtherd and Ork Freebooter.
    -- 14 entries (13 IV.7 Troops + Ork Freebooter), cumulative chance strictly
    -- ascending in declaration order, terminal value 100. Characteristics copied
    -- verbatim as the base integer (BS "00" -> 0; Unnatural parentheticals excluded
    -- from the stored integer and recorded as `Unnatural X (xN)` traits). hp =
    -- profile Wounds. Each (glyph, color) pair is distinct from every other entry
    -- across the Bestiary, using colors defined in Headers/Colors.hpp. Equipment
    -- reuses existing Ork gear references (weapon/armour wiring reconciled by 12.4).
    -- ─────────────────────────────────────────────────────────────────────────

    -- IV.7 — Snotling (Troop): WS 15 BS 00 S 10 T 10(2) Ag 35 Int 05 Per 20 WP 25 Fel 05, Wounds 4
    {
        chance  = { Ork = 8 },
        glyph   = string.byte("s"),
        name    = "Snotling",
        color   = "desaturatedGreen",
        hp      = 4.0,
        defense = 0.0,
        corpse  = "squashed Snotling",
        xp      = 8,
        power   = 2.0,
        skill   = 15,                    -- WS (Tiny Teeth melee; BS 00 -> 0)
        ws = 15, bs = 0, s = 10, t = 10, ag = 35, int = 5, per = 20, wp = 25, fel = 5,
        equipment = {},
        skills  = { Athletics = 0, Stealth = 0 },
        talents = { "Overlooked" },
        traits  = { "Bestial (Flee)", "Natural Weapons (Tiny Teeth)", "Size (2)",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.7 — Snotling Mob (Troop): WS 15 BS 00 S 10 T 10(2) Ag 35 Int 05 Per 20 WP 15 Fel 05, Wounds 14
    {
        chance  = { Ork = 14 },
        glyph   = string.byte("S"),
        name    = "Snotling Mob",
        color   = "desaturatedGreen",
        hp      = 14.0,
        defense = 0.0,
        corpse  = "scattered Snotling Mob",
        xp      = 16,
        power   = 2.0,
        skill   = 15,                    -- WS (Tiny Teeth melee; BS 00 -> 0)
        ws = 15, bs = 0, s = 10, t = 10, ag = 35, int = 5, per = 20, wp = 15, fel = 5,
        equipment = {},
        skills  = { Athletics = 0, Stealth = 0 },
        talents = { "Furious Assault" },
        traits  = { "Bestial (Flee)", "Natural Weapons (Tiny Teeth)", "Size (5)",
                    "Swarm", "Unnatural Toughness (x2)" },
    },

    -- IV.7 — Gretchin (Troop): WS 15 BS 35 S 20 T 20(3) Ag 45 Int 35 Per 35 WP 20 Fel 25, Wounds 6
    {
        chance  = { Ork = 26 },
        glyph   = string.byte("g"),
        name    = "Gretchin",
        color   = "desaturatedGreen",
        hp      = 6.0,
        defense = 0.0,
        corpse  = "dead Gretchin",
        xp      = 12,
        power   = 2.0,
        skill   = 35,                    -- BS (Grot Blasta pistol-primary)
        ws = 15, bs = 35, s = 20, t = 20, ag = 45, int = 35, per = 35, wp = 20, fel = 25,
        equipment = {},                  -- Grot Blasta not yet in Equipment.lua; wiring reconciled by 12.4
        dropChance = 0.3,
        skills  = { Awareness = 0, Athletics = 0, Dodge = 0, Stealth = 0 },
        talents = { "Overlooked" },
        traits  = { "Mob Rule", "Size (3)", "Unnatural Toughness (x3)" },
    },

    -- IV.7 — Ork Boy (Troop): WS 35 BS 25 S 45 T 45(6) Ag 30 Int 20 Per 30 WP 25 Fel 20, Wounds 18
    {
        chance  = { Ork = 45 },
        glyph   = string.byte("o"),
        name    = "Ork Boy",
        color   = "desaturatedGreen",
        hp      = 18.0,
        defense = 0.0,
        corpse  = "dead Ork Boy",
        xp      = 40,
        power   = 3.0,
        skill   = 35,                    -- WS (Choppa melee-primary)
        ws = 35, bs = 25, s = 45, t = 45, ag = 30, int = 20, per = 30, wp = 25, fel = 20,
        equipment = { "Choppa", "Slugga" },
        dropChance = 0.4,
        equipTier = { common = 80, uncommon = 18, rare = 2 },
        skills  = { Athletics = 1, Fortitude = 0, Intimidate = 0 },
        talents = { "Bulging Biceps", "Furious Assault", "Hardy", "Iron Jaw",
                    "Street Fighting", "Unarmed Warrior" },
        traits  = { "Brutal Charge (1)", "Mob Rule", "Size (4)", "Sturdy",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.7 — Burna Boy (Troop): WS 35 BS 25 S 45 T 45(6) Ag 30 Int 20 Per 30 WP 25 Fel 20, Wounds 18
    {
        chance  = { Ork = 52 },
        glyph   = string.byte("b"),
        name    = "Burna Boy",
        color   = "darkOrange",
        hp      = 18.0,
        defense = 0.0,
        corpse  = "charred Burna Boy",
        xp      = 44,
        power   = 3.0,
        skill   = 35,                    -- WS (Burna melee-primary)
        ws = 35, bs = 25, s = 45, t = 45, ag = 30, int = 20, per = 30, wp = 25, fel = 20,
        equipment = {},                  -- Burna not yet in Equipment.lua; wiring reconciled by 12.4
        dropChance = 0.4,
        skills  = { Athletics = 1, Fortitude = 0, Intimidate = 0 },
        talents = { "Bulging Biceps", "Furious Assault", "Hardy", "Hip Shooting",
                    "Iron Jaw", "Pyromaniac", "Street Fighting", "Sure Strike",
                    "Unarmed Warrior" },
        traits  = { "Brutal Charge (1)", "Mob Rule", "Size (4)", "Sturdy",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.7 — Tankbusta (Troop): WS 35 BS 25 S 45 T 45(6) Ag 30 Int 20 Per 30 WP 25 Fel 20, Wounds 18
    {
        chance  = { Ork = 59 },
        glyph   = string.byte("k"),
        name    = "Tankbusta",
        color   = "darkOrange",
        hp      = 18.0,
        defense = 0.0,
        corpse  = "dead Tankbusta",
        xp      = 46,
        power   = 3.0,
        skill   = 35,                    -- WS (Choppa melee); Rokkit Launcha ranged option
        ws = 35, bs = 25, s = 45, t = 45, ag = 30, int = 20, per = 30, wp = 25, fel = 20,
        equipment = { "Choppa" },
        dropChance = 0.4,
        skills  = { Athletics = 1, Fortitude = 0, Intimidate = 0 },
        talents = { "Bulging Biceps", "Furious Assault", "Hardy", "Iron Jaw",
                    "Street Fighting", "Tank Hunter", "Unarmed Warrior" },
        traits  = { "Brutal Charge (1)", "Mob Rule", "Size (4)", "Sturdy",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.7 — Loota (Troop): WS 35 BS 25 S 45 T 45(6) Ag 30 Int 20 Per 30 WP 25 Fel 20, Wounds 18
    {
        chance  = { Ork = 66 },
        glyph   = string.byte("l"),
        name    = "Loota",
        color   = "brown",
        hp      = 18.0,
        defense = 0.0,
        corpse  = "dead Loota",
        xp      = 46,
        power   = 3.0,
        skill   = 25,                    -- BS (Deffgun heavy ranged-primary)
        ws = 35, bs = 25, s = 45, t = 45, ag = 30, int = 20, per = 30, wp = 25, fel = 20,
        equipment = {},                  -- Deffgun not yet in Equipment.lua; wiring reconciled by 12.4
        dropChance = 0.4,
        skills  = { Athletics = 1, Commerce = 0, Fortitude = 0, Intimidate = 0, Stealth = 1 },
        talents = { "Bulging Biceps", "Furious Assault", "Hammering Storm", "Hardy",
                    "Iron Jaw", "Street Fighting", "Unarmed Warrior" },
        traits  = { "Brutal Charge (1)", "Mob Rule", "Size (4)", "Sturdy",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.7 — Storm Boy (Troop): WS 35 BS 25 S 45 T 45(6) Ag 35 Int 20 Per 30 WP 25 Fel 20, Wounds 18
    {
        chance  = { Ork = 72 },
        glyph   = string.byte("z"),
        name    = "Storm Boy",
        color   = "desaturatedGreen",
        hp      = 18.0,
        defense = 0.0,
        corpse  = "dead Storm Boy",
        xp      = 44,
        power   = 3.0,
        skill   = 35,                    -- WS (Choppa melee-primary)
        ws = 35, bs = 25, s = 45, t = 45, ag = 35, int = 20, per = 30, wp = 25, fel = 20,
        equipment = { "Choppa", "Slugga" },
        dropChance = 0.4,
        skills  = { Athletics = 1, Fortitude = 0, Intimidate = 0, ["Operate (Aeronautica)"] = 0 },
        talents = { "Berserk Charge", "Bulging Biceps", "Furious Assault", "Hardy",
                    "Iron Jaw", "Raptor", "Street Fighting", "Unarmed Warrior" },
        traits  = { "Brutal Charge (2)", "Mob Rule", "Size (4)", "Sturdy",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.7 — Kommando (Troop): WS 35 BS 25 S 45 T 45(6) Ag 35 Int 20 Per 30 WP 25 Fel 20, Wounds 18
    {
        chance  = { Ork = 78 },
        glyph   = string.byte("K"),
        name    = "Kommando",
        color   = "darkGreen",
        hp      = 18.0,
        defense = 0.0,
        corpse  = "dead Kommando",
        xp      = 48,
        power   = 3.0,
        skill   = 35,                    -- WS (Choppa melee-primary)
        ws = 35, bs = 25, s = 45, t = 45, ag = 35, int = 20, per = 30, wp = 25, fel = 20,
        equipment = { "Choppa", "Slugga" },
        dropChance = 0.4,
        skills  = { Athletics = 1, Intimidate = 0, Fortitude = 0, Stealth = 1, Survival = 1 },
        talents = { "Ambush", "Bulging Biceps", "Deference for Darkness",
                    "Furious Assault", "Hardy", "Iron Jaw", "Street Fighting",
                    "Unarmed Warrior" },
        traits  = { "Brutal Charge (1)", "Mob Rule", "Size (4)", "Sturdy",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.7 — 'Ard Boy (Troop): WS 35 BS 25 S 45 T 45(6) Ag 30 Int 20 Per 30 WP 25 Fel 20, Wounds 18
    {
        chance  = { Ork = 84 },
        glyph   = string.byte("a"),
        name    = "'Ard Boy",
        color   = "darkGreen",
        hp      = 18.0,
        defense = 1.0,
        corpse  = "dead 'Ard Boy",
        xp      = 48,
        power   = 3.0,
        skill   = 35,                    -- WS (Choppa melee-primary)
        ws = 35, bs = 25, s = 45, t = 45, ag = 30, int = 20, per = 30, wp = 25, fel = 20,
        equipment = { "Choppa", "Slugga", "Ork Armor" },
        dropChance = 0.4,
        skills  = { Athletics = 1, Fortitude = 0, Intimidate = 0, Parry = 0 },
        talents = { "Ambidextrous", "Bulging Biceps", "Furious Assault", "Hardy",
                    "Iron Jaw", "Shield Wall", "Street Fighting", "Unarmed Warrior" },
        traits  = { "Brutal Charge (1)", "Mob Rule", "Size (4)", "Sturdy",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.7 — Skar Boy (Troop): WS 45 BS 25 S 50 T 50(7) Ag 30 Int 20 Per 30 WP 25 Fel 20, Wounds 21
    {
        chance  = { Ork = 89 },
        glyph   = string.byte("O"),
        name    = "Skar Boy",
        color   = "darkGreen",
        hp      = 21.0,
        defense = 0.0,
        corpse  = "dead Skar Boy",
        xp      = 55,
        power   = 4.0,
        skill   = 45,                    -- WS (Choppa melee-primary)
        ws = 45, bs = 25, s = 50, t = 50, ag = 30, int = 20, per = 30, wp = 25, fel = 20,
        equipment = { "Choppa", "Slugga" },
        dropChance = 0.4,
        skills  = { Athletics = 1, Fortitude = 1, Intimidate = 1 },
        talents = { "Bulging Biceps", "Furious Assault", "Hardy", "Iron Jaw",
                    "Litany Against Fear", "Street Fighting", "Unarmed Warrior" },
        traits  = { "Brutal Charge (1)", "Mob Rule", "Size (4)", "Sturdy",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.7 — Runtherd (Troop): WS 35 BS 20 S 45(7) T 45(7) Ag 30 Int 25 Per 45 WP 25 Fel 20, Wounds 31
    {
        chance  = { Ork = 93 },
        glyph   = string.byte("R"),
        name    = "Runtherd",
        color   = "brown",
        hp      = 31.0,
        defense = 1.0,
        corpse  = "dead Runtherd",
        xp      = 70,
        power   = 4.0,
        skill   = 35,                    -- WS (Grabba Stikk / Grot-Prod melee-primary)
        ws = 35, bs = 20, s = 45, t = 45, ag = 30, int = 25, per = 45, wp = 25, fel = 20,
        equipment = { "Slugga" },
        dropChance = 0.5,
        skills  = { Awareness = 1, Discipline = 0, Fortitude = 1, Intimidate = 2,
                    Survival = 2, ["Trade (Squigherd, Storyteller)"] = 2 },
        talents = { "Bulging Biceps", "Furious Assault", "Galvanizing Presence",
                    "Hardy", "Iron Jaw", "Pity The Weak", "Street Fighting",
                    "Surefoot Wayfinder", "Unarmed Warrior" },
        traits  = { "Brutal Charge (2)", "Mob Rule", "Size (5)", "Sturdy",
                    "Unnatural Strength (x3)", "Unnatural Toughness (x3)" },
    },

    -- IV.7 — Attack Squig (Troop): WS 40 BS 00 S 35 T 35(5) Ag 30 Int 10 Per 30 WP 30 Fel 00, Wounds 15
    {
        chance  = { Ork = 97 },
        glyph   = string.byte("q"),
        name    = "Attack Squig",
        color   = "darkOrange",
        hp      = 15.0,
        defense = 0.0,
        corpse  = "dead Attack Squig",
        xp      = 32,
        power   = 3.0,
        skill   = 40,                    -- WS (Huge Jaws melee; BS 00 -> 0)
        ws = 40, bs = 0, s = 35, t = 35, ag = 30, int = 10, per = 30, wp = 30, fel = 0,
        equipment = {},                  -- Huge Jaws is a natural weapon (represented as a trait)
        skills  = { Awareness = 1, Fortitude = 0, Survival = 1 },
        talents = { "Berserk Charge", "Bloodlust", "Frenzy", "Swift Attack" },
        traits  = { "Bestial (Frenzy)", "Deadly Natural Weapons (Huge Jaws)",
                    "Size (4)", "Unnatural Toughness (x2)" },
    },

    -- §5.3 — Ork Freebooter (Troop): WS 45 BS 20 S 50(8) T 45 Ag 30 Int 26 Per 30 WP 28 Fel 22, Wounds 16
    {
        chance  = { Ork = 100 },
        glyph   = string.byte("F"),
        name    = "Ork Freebooter",
        color   = "gold",
        hp      = 16.0,
        defense = 1.0,
        corpse  = "dead Ork Freebooter",
        xp      = 65,
        power   = 4.0,
        skill   = 45,                    -- WS (Chain axe melee-primary)
        ws = 45, bs = 20, s = 50, t = 45, ag = 30, int = 26, per = 30, wp = 28, fel = 22,
        equipment = { "Shoota", "Ork Armor" },
        dropChance = 0.5,
        skills  = { Awareness = 0, Barter = 0, Intimidate = 0 },
        talents = { "Basic Weapon Training (Primitive, SP)", "Bulging Biceps",
                    "Common Lore (Ork)", "Crushing Blow", "Furious Assault", "Hardy",
                    "Heavy Weapon Training (SP)", "Iron Jaw",
                    "Melee Weapon Training (Chain, Primitive, Power)",
                    "Pistol Weapon Training (Primitive, SP)",
                    "Speak Language (Ork, Low Gothic)", "True Grit" },
        traits  = { "Brutal Charge", "Mob Rule", "Resistance (Cold, Heat, Radiation)",
                    "Sturdy", "Unnatural Toughness (x2)" },
    },

    -- ─────────────────────────────────────────────────────────────────────────
    -- Tau Faction_Region column (Bestiary IV.8 Tau/Kroot/Vespid + Kroot Mercenary
    -- §5.3). 8 Troop entries, cumulative chance strictly ascending in declaration
    -- order, terminal value 100. Characteristics copied verbatim as the base integer
    -- (BS/Fel "00" -> 0; Unnatural parentheticals excluded from the stored integer
    -- and recorded as `Unnatural X (xN)` traits). hp = profile Wounds. Size and
    -- Flyer traits are recorded as trait strings (Req 3.3). Each (glyph, color) pair
    -- is distinct from every other entry across the Bestiary, using colors defined
    -- in Headers/Colors.hpp. Equipment left empty for now; weapon/armour wiring is
    -- handled by tasks 12.3/12.4.
    -- ─────────────────────────────────────────────────────────────────────────

    -- IV.8 — Fire Warrior (Troop): WS 25 BS 40 S 30 T 30 Ag 35 Int 30 Per 20 WP 30 Fel 30, Wounds 9
    {
        chance  = { Tau = 22 },
        glyph   = string.byte("t"),
        name    = "Fire Warrior",
        color   = "lightCyan",
        hp      = 9.0,
        defense = 1.0,
        corpse  = "dead Fire Warrior",
        xp      = 30,
        power   = 3.0,
        skill   = 40,                    -- BS (Pulse Rifle ranged-primary)
        ws = 25, bs = 40, s = 30, t = 30, ag = 35, int = 30, per = 20, wp = 30, fel = 30,
        equipment = {},
        skills  = { Athletics = 1, Awareness = 1, Discipline = 0, Dodge = 1,
                    Fortitude = 0, ["Linguistics (Tau)"] = 0, Medicae = 0,
                    ["Navigate (Surface)"] = 0, ["Operate (Surface)"] = 0,
                    ["Tactics (Assault Doctrine, Defensive Doctrine)"] = 0,
                    ["Tech-Use"] = 0 },
        talents = { "Deadeye Shot", "Hammering Storm", "Improve Cover",
                    "Litany Against Fear", "Nowhere to Hide", "Rapid Reload",
                    "Weapon Drill (Pulse)" },
        traits  = { "Size (4)" },
    },

    -- IV.8 — Pathfinder (Troop): WS 25 BS 45 S 30 T 30 Ag 40 Int 35 Per 30 WP 30 Fel 30, Wounds 9
    {
        chance  = { Tau = 38 },
        glyph   = string.byte("p"),
        name    = "Pathfinder",
        color   = "steelBlue",
        hp      = 9.0,
        defense = 1.0,
        corpse  = "dead Pathfinder",
        xp      = 36,
        power   = 3.0,
        skill   = 45,                    -- BS (Pulse Carbine / Rail Rifle ranged-primary)
        ws = 25, bs = 45, s = 30, t = 30, ag = 40, int = 35, per = 30, wp = 30, fel = 30,
        equipment = {},
        skills  = { Athletics = 1, Awareness = 2, Discipline = 1, Dodge = 1,
                    Fortitude = 0, ["Linguistics (Tau)"] = 0, Medicae = 0,
                    ["Navigate (Surface)"] = 1, ["Operate (Surface)"] = 0,
                    Stealth = 1, Survival = 1, ["Tactics (Reconnaissance)"] = 0,
                    ["Tech-Use"] = 0 },
        talents = { "Combat Formation", "Deference For Darkness", "Litany Against Fear",
                    "Rapid Reload", "Surefoot Wayfinder" },
        traits  = { "Size (4)" },
    },

    -- IV.8 — Kroot Carnivore (Troop): WS 40 BS 35 S 45(6) T 40 Ag 45 Int 25 Per 45(6) WP 30 Fel 20, Wounds 12
    {
        chance  = { Tau = 55 },
        glyph   = string.byte("K"),
        name    = "Kroot Carnivore",
        color   = "tan",
        hp      = 12.0,
        defense = 1.0,
        corpse  = "dead Kroot Carnivore",
        xp      = 44,
        power   = 4.0,
        skill   = 40,                    -- WS (Kroot Rifle melee attachment / Beak melee-primary)
        ws = 40, bs = 35, s = 45, t = 40, ag = 45, int = 25, per = 45, wp = 30, fel = 20,
        equipment = {},
        skills  = { Acrobatics = 0, Athletics = 1, Awareness = 1, Commerce = 0,
                    Dodge = 1, Fortitude = 0,
                    ["Linguistics (Low Gothic, Kroot, Tau)"] = 0, Stealth = 2,
                    Survival = 2 },
        talents = { "Furious Assault", "Leap Up", "Leaping Dodge", "Leaping Strike",
                    "Lightning Reflexes", "Resistance (Fear)", "Swift Attack" },
        traits  = { "Eaters of the Dead", "Hyperactive Nyumen Organ",
                    "Natural Weapons (Beak)", "Unnatural Strength (x2)",
                    "Unnatural Perception (x2)" },
    },

    -- IV.8 — Kroot Hound (Troop): WS 40 BS 00 S 40(6) T 30 Ag 40 Int 20 Per 45 WP 30 Fel 00, Wounds 9
    {
        chance  = { Tau = 68 },
        glyph   = string.byte("h"),
        name    = "Kroot Hound",
        color   = "darkGreen",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Kroot Hound",
        xp      = 30,
        power   = 3.0,
        skill   = 40,                    -- WS (Beak melee; BS 00 -> 0)
        ws = 40, bs = 0, s = 40, t = 30, ag = 40, int = 20, per = 45, wp = 30, fel = 0,
        equipment = {},
        skills  = { Athletics = 1, Awareness = 2, Dodge = 1, Fortitude = 0,
                    Stealth = 1, Survival = 2 },
        talents = { "Berserk Charge", "Double Team", "Heightened Senses (Sight, Sound)",
                    "Sprint" },
        traits  = { "Bestial (Flee)", "Deadly Natural Weapons (Beak)",
                    "Eaters of the Dead", "Hyperactive Nyumen Organ", "Quadruped",
                    "Size (4)", "Unnatural Strength (x2)" },
    },

    -- IV.8 — Vespid Stingwing (Troop): WS 35 BS 45 S 30 T 45 Ag 50 Int 25 Per 30 WP 30 Fel 15, Wounds 12
    {
        chance  = { Tau = 80 },
        glyph   = string.byte("v"),
        name    = "Vespid Stingwing",
        color   = "cyan",
        hp      = 12.0,
        defense = 1.0,
        corpse  = "dead Vespid Stingwing",
        xp      = 42,
        power   = 3.0,
        skill   = 45,                    -- BS (Neutron Blaster ranged-primary)
        ws = 35, bs = 45, s = 30, t = 45, ag = 50, int = 25, per = 30, wp = 30, fel = 15,
        equipment = {},
        skills  = { Acrobatics = 1, Awareness = 0, Dodge = 1,
                    ["Linguistics (Vespid)"] = 0, Survival = 0 },
        talents = { "Catfall", "Chem-Geld", "Hard Target", "Leaping Dodge",
                    "Lightning Reflexes" },
        traits  = { "Dark-Sight", "Deadly Natural Weapons (Claws)", "Flyer (10)",
                    "Natural Armour (3)", "Unnatural Senses (30)" },
    },

    -- §5.3 — Kroot Mercenary (Troop): WS 42 BS 33 S 35(8) T 40 Ag 44 Int 25 Per 44(6) WP 30 Fel 18, Wounds 12
    {
        chance  = { Tau = 90 },
        glyph   = string.byte("k"),
        name    = "Kroot Mercenary",
        color   = "tan",
        hp      = 12.0,
        defense = 1.0,
        corpse  = "dead Kroot Mercenary",
        xp      = 48,
        power   = 4.0,
        skill   = 42,                    -- WS (Kroot Rifle melee / Beak melee-primary)
        ws = 42, bs = 33, s = 35, t = 40, ag = 44, int = 25, per = 44, wp = 30, fel = 18,
        equipment = {},
        skills  = { Acrobatics = 0, Awareness = 0, Barter = 0, Climb = 1,
                    Concealment = 2, Dodge = 1, ["Silent Move"] = 2,
                    ["Speak Language (Low Gothic, Kroot)"] = 0, Tracking = 1,
                    Survival = 2 },
        talents = { "Basic Weapon Training (SP, Primitive)", "Furious Attack",
                    "Leap Up", "Lightning Reflexes", "Melee Weapon Training (Primitive)",
                    "Resistance (Fear)", "Sprint", "Swift Attack" },
        traits  = { "Natural Weapons (Beak)", "Unnatural Perception (x2)",
                    "Unnatural Strength (x2)" },
    },

    -- IV.8 — Earth Caste Engineer (Troop): WS 20 BS 25 S 30 T 30 Ag 25 Int 40 Per 30 WP 30 Fel 25, Wounds 9
    {
        chance  = { Tau = 96 },
        glyph   = string.byte("e"),
        name    = "Earth Caste Engineer",
        color   = "boneWhite",
        hp      = 9.0,
        defense = 1.0,
        corpse  = "dead Earth Caste Engineer",
        xp      = 26,
        power   = 2.0,
        skill   = 25,                    -- BS (Pulse Pistol) / Int-focused support
        ws = 20, bs = 25, s = 30, t = 30, ag = 25, int = 40, per = 30, wp = 30, fel = 25,
        equipment = {},
        skills  = { Athletics = 2, Fortitude = 1, Logic = 1, ["Navigate (Surface)"] = 0,
                    ["Operate (Surface)"] = 1, Scrutiny = 0,
                    ["Trade (Armourer, Technomat)"] = 2, ["Tech-Use"] = 2 },
        talents = { "Labourer", "Talented (Tech-Use)", "Technical Knock", "Tireless" },
        traits  = { "Size (4)" },
    },

    -- IV.8 — Water Caste Diplomat (Troop): WS 05 BS 10 S 20 T 20 Ag 25 Int 35 Per 40 WP 45 Fel 50, Wounds 6
    {
        chance  = { Tau = 100 },
        glyph   = string.byte("w"),
        name    = "Water Caste Diplomat",
        color   = "teal",
        hp      = 6.0,
        defense = 0.0,
        corpse  = "dead Water Caste Diplomat",
        xp      = 20,
        power   = 1.0,
        skill   = 5,                     -- WS (unarmed; no weapons carried)
        ws = 5, bs = 10, s = 20, t = 20, ag = 25, int = 35, per = 40, wp = 45, fel = 50,
        equipment = {},
        skills  = { Awareness = 1, Charm = 2, Command = 1, Commerce = 3, Deceive = 2,
                    Discipline = 1, Inquiry = 1, ["Linguistics (Tau, Low Gothic)"] = 1,
                    Scrutiny = 1 },
        talents = { "Air of Authority (1)", "Ear to the Ground", "Hard Bargain",
                    "Keen Intuition", "Lexographer", "Operative Conditioning",
                    "Orthoproxy", "Persuasive Charm", "Polyglot",
                    "Talented (Commerce, Inquiry)", "Whispers" },
        traits  = { "Size (4)" },
    },

    -- ─────────────────────────────────────────────────────────────────────────
    -- Tyranid Faction_Region column (Bestiary IV.9 Tyranids §13.8). 12 Troop
    -- entries, cumulative chance strictly ascending in declaration order, terminal
    -- value 100. Characteristics copied verbatim as the base integer (BS/Fel "00"
    -- em-dash -> 0 per Req 2.2; Unnatural parentheticals excluded from the stored
    -- integer and recorded as `Unnatural X (xN)` traits per Req 2.3/2.4). hp =
    -- profile Wounds (Req 3.1). Size and Flyer traits are recorded as trait strings
    -- (Req 3.3). Each (glyph, color) pair is distinct from every other entry across
    -- the Bestiary, using colors defined in Headers/Colors.hpp (Req 1.12). Tyranids
    -- fight with natural bio-weapons, so equipment = {} on every entry (weapon/armour
    -- wiring reconciled by tasks 12.3/12.4).
    -- ─────────────────────────────────────────────────────────────────────────

    -- IV.9 §13.8.1 — Late Generation Hybrid (Troop): WS 40 BS 30 S 30 T 30 Ag 35(5) Int 25 Per 40 WP 25 Fel 30, Wounds 9
    {
        chance  = { Tyranid = 10 },
        glyph   = string.byte("y"),
        name    = "Late Generation Hybrid",
        color   = "crimson",
        hp      = 9.0,
        defense = 1.0,
        corpse  = "dead Late Generation Hybrid",
        xp      = 30,
        power   = 3.0,
        skill   = 40,                    -- WS (Chainsword/Monoknife melee-primary)
        ws = 40, bs = 30, s = 30, t = 30, ag = 35, int = 25, per = 40, wp = 25, fel = 30,
        equipment = {},
        skills  = { Acrobatics = 1, Athletics = 0, Awareness = 0, Deceive = 1,
                    Discipline = 0, Dodge = 0, Stealth = 1, Survival = 0 },
        talents = { "Ambidextrous", "Double Team", "Hard Target", "Leap Up",
                    "Lightning Reflexes", "Swift Attack", "Unarmed Warrior" },
        traits  = { "Dark-Sight", "Mindlinked", "Size (4)", "Tyranid",
                    "Unnatural Agility (x2)" },
    },

    -- IV.9 §13.8.1 — Early Generation Hybrid (Troop): WS 50 BS 30 S 35(5) T 35(5) Ag 40(6) Int 25 Per 50 WP 35 Fel 30, Wounds 9
    {
        chance  = { Tyranid = 16 },
        glyph   = string.byte("Y"),
        name    = "Early Generation Hybrid",
        color   = "crimson",
        hp      = 9.0,
        defense = 1.0,
        corpse  = "dead Early Generation Hybrid",
        xp      = 40,
        power   = 3.0,
        skill   = 50,                    -- WS (Rending Claw / Bio-Weapons melee-primary)
        ws = 50, bs = 30, s = 35, t = 35, ag = 40, int = 25, per = 50, wp = 35, fel = 30,
        equipment = {},
        skills  = { Acrobatics = 1, Athletics = 1, Awareness = 1, Discipline = 0,
                    Dodge = 1, Fortitude = 0, Stealth = 1, Survival = 1 },
        talents = { "Ambidextrous", "Assassin Strike", "Double Team", "Hard Target",
                    "Leap Up", "Lightning Reflexes", "Litany Against Fear", "Sprint",
                    "Swift Attack" },
        traits  = { "Dark-Sight", "Deadly Natural Weapons (Bio-Weapons)", "Fear (1)",
                    "Mindlinked", "Multiple Arms", "Natural Armour (2)", "Size (4)",
                    "Tyranid", "Unnatural Strength (x2)", "Unnatural Toughness (x2)",
                    "Unnatural Agility (x2)" },
    },

    -- IV.9 §13.8.4 — Ripper (Troop): WS 25 BS 00 S 20 T 15 Ag 40 Int 10 Per 30 WP 30 Fel 00, Wounds 2
    {
        chance  = { Tyranid = 26 },
        glyph   = string.byte("r"),
        name    = "Ripper",
        color   = "crimson",
        hp      = 2.0,
        defense = 0.0,
        corpse  = "crushed Ripper",
        xp      = 6,
        power   = 2.0,
        skill   = 25,                    -- WS (Mandibles melee; BS 00 -> 0)
        ws = 25, bs = 0, s = 20, t = 15, ag = 40, int = 10, per = 30, wp = 30, fel = 0,
        equipment = {},
        skills  = { Athletics = 0, Awareness = 0, Stealth = 0, Survival = 0 },
        talents = { "Heightened Senses (Smell)" },
        traits  = { "Burrower (1)", "Deadly Natural Weapons (Mandibles)", "Dark-Sight",
                    "Natural Armour (2)", "Size (2)",
                    "Tyranid (Instinctive Behaviour [Feed])" },
    },

    -- IV.9 §13.8.4 — Ripper Swarm (Troop): WS 25 BS 00 S 20 T 15 Ag 40 Int 10 Per 30 WP 30 Fel 00, Wounds 13
    {
        chance  = { Tyranid = 34 },
        glyph   = string.byte("r"),
        name    = "Ripper Swarm",
        color   = "purple",
        hp      = 13.0,
        defense = 0.0,
        corpse  = "scattered Ripper Swarm",
        xp      = 24,
        power   = 2.0,
        skill   = 25,                    -- WS (Mandibles melee; BS 00 -> 0)
        ws = 25, bs = 0, s = 20, t = 15, ag = 40, int = 10, per = 30, wp = 30, fel = 0,
        equipment = {},
        skills  = { Athletics = 0, Awareness = 0, Stealth = 0, Survival = 0 },
        talents = { "Furious Assault", "Heightened Senses (Smell)" },
        traits  = { "Burrower (4)", "Deadly Natural Weapons (Mandibles)", "Dark-Sight",
                    "Natural Armour (2)", "Size (5)", "Swarm",
                    "Tyranid (Instinctive Behaviour [Feed])" },
    },

    -- IV.9 §13.8.4 — Hormagaunt (Troop): WS 45 BS 20 S 35 T 30 Ag 55 Int 10 Per 40 WP 30 Fel 00, Wounds 9
    {
        chance  = { Tyranid = 50 },
        glyph   = string.byte("h"),
        name    = "Hormagaunt",
        color   = "crimson",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Hormagaunt",
        xp      = 28,
        power   = 3.0,
        skill   = 45,                    -- WS (Scything Talons melee-primary)
        ws = 45, bs = 20, s = 35, t = 30, ag = 55, int = 10, per = 40, wp = 30, fel = 0,
        equipment = {},
        skills  = { Acrobatics = 2, Awareness = 0, Dodge = 1, Stealth = 0 },
        talents = { "Leap Up", "Leaping Strike", "Swift Attack" },
        traits  = { "Dark-Sight", "Deadly Natural Weapons (Bio-Weapons)",
                    "Natural Armour (3)", "Tyranid (Instinctive Behaviour [Feed])" },
    },

    -- IV.9 §13.8.4 — Termagant (Troop): WS 30 BS 35 S 30 T 30 Ag 40 Int 10 Per 40 WP 30 Fel 00, Wounds 9
    {
        chance  = { Tyranid = 64 },
        glyph   = string.byte("t"),
        name    = "Termagant",
        color   = "crimson",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Termagant",
        xp      = 26,
        power   = 3.0,
        skill   = 35,                    -- BS (Fleshborer ranged-primary)
        ws = 30, bs = 35, s = 30, t = 30, ag = 40, int = 10, per = 40, wp = 30, fel = 0,
        equipment = {},
        skills  = { Athletics = 0, Dodge = 0, Stealth = 0 },
        talents = { "Leap Up" },
        traits  = { "Dark-Sight", "Deadly Natural Weapons (Bio-Weapons)",
                    "Natural Armour (3)", "Tyranid (Instinctive Behaviour [Lurk])" },
    },

    -- IV.9 §13.8.7 — Mucolid Spore (Troop): WS 00 BS 00 S 05 T 45 Ag 10 Int 00 Per 40 WP 10 Fel 00, Wounds 32
    {
        chance  = { Tyranid = 70 },
        glyph   = string.byte("o"),
        name    = "Mucolid Spore",
        color   = "purple",
        hp      = 32.0,
        defense = 0.0,
        corpse  = "burst Mucolid Spore",
        xp      = 30,
        power   = 2.0,
        skill   = 40,                    -- Per (detonation; WS/BS 00 -> 0, uses senses)
        ws = 0, bs = 0, s = 5, t = 45, ag = 10, int = 0, per = 40, wp = 10, fel = 0,
        equipment = {},
        skills  = { Awareness = 2 },
        talents = { "Sprint" },
        traits  = { "Blind", "Flyer (3)", "From Beyond", "Natural Armour (2)",
                    "Size (6)", "Tyranid", "Unnatural Senses (40)" },
    },

    -- IV.9 §13.8.7 — Mieotic Spore (Troop): WS 00 BS 00 S 15 T 35 Ag 15 Int 00 Per 45 WP 10 Fel 00, Wounds 19
    {
        chance  = { Tyranid = 76 },
        glyph   = string.byte("o"),
        name    = "Mieotic Spore",
        color   = "magenta",
        hp      = 19.0,
        defense = 0.0,
        corpse  = "burst Mieotic Spore",
        xp      = 24,
        power   = 2.0,
        skill   = 45,                    -- Per (detonation; WS/BS 00 -> 0, uses senses)
        ws = 0, bs = 0, s = 15, t = 35, ag = 15, int = 0, per = 45, wp = 10, fel = 0,
        equipment = {},
        skills  = { Awareness = 2 },
        talents = { "Sprint" },
        traits  = { "Blind", "From Beyond", "Hoverer (4)", "Natural Armour (2)",
                    "Size (5)", "Tyranid", "Unnatural Senses (45)" },
    },

    -- IV.9 §13.8.8 — Biovore (Troop): WS 35 BS 40 S 40(6) T 40(6) Ag 25 Int 15 Per 45 WP 30 Fel 00, Wounds 28
    {
        chance  = { Tyranid = 82 },
        glyph   = string.byte("b"),
        name    = "Biovore",
        color   = "crimson",
        hp      = 28.0,
        defense = 0.0,
        corpse  = "dead Biovore",
        xp      = 56,
        power   = 4.0,
        skill   = 40,                    -- BS (Spore Mine Launcher ranged-primary)
        ws = 35, bs = 40, s = 40, t = 40, ag = 25, int = 15, per = 45, wp = 30, fel = 0,
        equipment = {},
        skills  = { Athletics = 1, Awareness = 1, Fortitude = 0 },
        talents = { "Sudden Attack", "Vigilant" },
        traits  = { "Dark-Sight", "Natural Armour (6)", "Size (5)",
                    "Tyranid (Instinctive Behaviour [Lurk])", "Unnatural Strength (x2)",
                    "Unnatural Toughness (x2)" },
    },

    -- IV.9 §13.8.8 — Pyrovore (Troop): WS 35 BS 40 S 40(6) T 40(6) Ag 25 Int 15 Per 45 WP 30 Fel 00, Wounds 28
    {
        chance  = { Tyranid = 88 },
        glyph   = string.byte("P"),
        name    = "Pyrovore",
        color   = "crimson",
        hp      = 28.0,
        defense = 0.0,
        corpse  = "charred Pyrovore",
        xp      = 58,
        power   = 4.0,
        skill   = 40,                    -- BS (Flamespurt ranged-primary)
        ws = 35, bs = 40, s = 40, t = 40, ag = 25, int = 15, per = 45, wp = 30, fel = 0,
        equipment = {},
        skills  = { Athletics = 1, Awareness = 1, Fortitude = 0 },
        talents = { "Hip Shooting" },
        traits  = { "Auto-Stabilized", "Dark-Sight", "Natural Armour (6)", "Quadruped",
                    "Size (5)", "Tyranid (Instinctive Behaviour [Lurk])",
                    "Unnatural Strength (x2)", "Unnatural Toughness (x2)" },
    },

    -- IV.9 §13.8.10 — Gargoyle (Troop): WS 30 BS 35 S 25 T 25 Ag 40 Int 10 Per 40 WP 30 Fel 00, Wounds 6
    {
        chance  = { Tyranid = 94 },
        glyph   = string.byte("G"),
        name    = "Gargoyle",
        color   = "crimson",
        hp      = 6.0,
        defense = 0.0,
        corpse  = "dead Gargoyle",
        xp      = 22,
        power   = 3.0,
        skill   = 35,                    -- BS (Fleshborer ranged-primary)
        ws = 30, bs = 35, s = 25, t = 25, ag = 40, int = 10, per = 40, wp = 30, fel = 0,
        equipment = {},
        skills  = { Acrobatics = 0, Athletics = 0, Awareness = 0, Dodge = 0 },
        talents = { "Raptor", "Leap Up" },
        traits  = { "Dark-Sight", "Deadly Natural Weapons (Bio-Weapons)", "Flyer (8)",
                    "Natural Armour (3)", "Tyranid (Instinctive Behaviour [Lurk])" },
    },

    -- IV.9 §13.8.10 — Mycetic Spore (Troop): WS 40 BS 40 S 45(14) T 45(14) Ag 30 Int 00 Per 20 WP 15 Fel 00, Wounds 66
    -- NOTE: source St/T characteristic line prints (14) while the Traits line prints
    -- Unnatural Strength/Toughness (x10); recorded verbatim from source (base 45).
    {
        chance  = { Tyranid = 100 },
        glyph   = string.byte("M"),
        name    = "Mycetic Spore",
        color   = "purple",
        hp      = 66.0,
        defense = 0.0,
        corpse  = "spent Mycetic Spore",
        xp      = 90,
        power   = 4.0,
        skill   = 40,                    -- BS (Twin Deathspitter ranged-primary; Int 00 -> 0)
        ws = 40, bs = 40, s = 45, t = 45, ag = 30, int = 0, per = 20, wp = 15, fel = 0,
        equipment = {},
        skills  = { Awareness = 0, Dodge = 1 },
        talents = { "Hip Shooting", "Sprint", "Whirlwind of Death" },
        traits  = { "Blind", "Deadly Natural Weapons (Bio-Weapons)", "Flyer (14)",
                    "From Beyond", "Size (8)", "Tyranid", "Undying",
                    "Unnatural Senses (50)", "Unnatural Strength (x10)",
                    "Unnatural Toughness (x10)" },
    },

    -- ─────────────────────────────────────────────────────────────────────────
    -- ImperialHuman Faction_Region column (Bestiary §5.1 Imperial Humans). 11 Troop
    -- entries: the Colonist base template, its six Minor NPC variants (Adept,
    -- Bloodskinner, Entertainer, Hired Gun, Scum, Voidfarer) built as base-plus-
    -- overrides, and four officers (Mutant Outcast, Oathsworn Bodyguard, Renegade,
    -- Warp Witch). Cumulative chance strictly ascending in declaration order,
    -- terminal value 100. Characteristics copied verbatim as the base integer.
    -- hp = profile Wounds (Colonist base 9; Hired Gun override 12). Each variant's
    -- skills/talents/traits are the Colonist base entries plus its cited additions
    -- (Requirement 4.4). Each (glyph, color) pair is distinct from every other entry
    -- across the Bestiary, using colors defined in Headers/Colors.hpp. Equipment
    -- left empty for now; weapon/armour wiring is handled by tasks 12.3/12.4.
    -- ─────────────────────────────────────────────────────────────────────────

    -- §5.1 — Colonist (base template) (Troop): WS 25 BS 20 S 30 T 30 Ag 30 Int 25 Per 25 WP 25 Fel 30, Wounds 9
    {
        chance  = { ImperialHuman = 18 },
        glyph   = string.byte("C"),
        name    = "Colonist",
        color   = "white",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Colonist",
        xp      = 12,
        power   = 2.0,
        skill   = 25,                    -- WS (Primitive melee / SP)
        ws = 25, bs = 20, s = 30, t = 30, ag = 30, int = 25, per = 25, wp = 25, fel = 30,
        equipment = {},
        skills  = { Awareness = 0, ["Common Lore (Imperium)"] = 0,
                    ["Drive (Ground Vehicle)"] = 0,
                    ["Speak Language (Low Gothic)"] = 0, ["Trade (Labourer)"] = 0 },
        talents = { "Basic Weapon Training (SP)", "Melee Weapon Training (Primitive)" },
        traits  = {},
    },

    -- §5.1 — Adept (Colonist variant): base + Int 30; Wounds 9
    {
        chance  = { ImperialHuman = 30 },
        glyph   = string.byte("a"),
        name    = "Adept",
        color   = "lightYellow",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Adept",
        xp      = 14,
        power   = 2.0,
        skill   = 25,                    -- WS (base)
        ws = 25, bs = 20, s = 30, t = 30, ag = 30, int = 30, per = 25, wp = 25, fel = 30,
        equipment = {},
        -- Colonist base skills + Adept additions.
        skills  = { Awareness = 0, ["Common Lore (Imperium)"] = 0,
                    ["Drive (Ground Vehicle)"] = 0,
                    ["Speak Language (Low Gothic)"] = 0, ["Trade (Labourer)"] = 0,
                    ["Common Knowledge (Imperium)"] = 1, Literacy = 1,
                    ["Speak Language (High Gothic)"] = 0 },
        talents = { "Basic Weapon Training (SP)", "Melee Weapon Training (Primitive)" },
        traits  = {},
    },

    -- §5.1 — Bloodskinner (Colonist variant): base + WS 35, BS 30, Per 35; Wounds 9
    {
        chance  = { ImperialHuman = 40 },
        glyph   = string.byte("b"),
        name    = "Bloodskinner",
        color   = "brown",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Bloodskinner",
        xp      = 20,
        power   = 3.0,
        skill   = 35,                    -- WS (Chainaxe / knife melee-primary)
        ws = 35, bs = 30, s = 30, t = 30, ag = 30, int = 25, per = 35, wp = 25, fel = 30,
        equipment = {},
        -- Colonist base skills + Bloodskinner additions.
        skills  = { Awareness = 0, ["Common Lore (Imperium)"] = 0,
                    ["Drive (Ground Vehicle)"] = 0,
                    ["Speak Language (Low Gothic)"] = 0, ["Trade (Labourer)"] = 0,
                    ["Navigation (Surface)"] = 0, Survival = 0, Tracking = 0,
                    Wrangling = 0 },
        -- Colonist base talents + Bloodskinner additions.
        talents = { "Basic Weapon Training (SP)", "Melee Weapon Training (Primitive)",
                    "Basic Weapon Training (Primitive)", "Melee Weapon Training (Chain)" },
        traits  = {},
    },

    -- §5.1 — Entertainer (Colonist variant): base + Fel 35; Wounds 9
    {
        chance  = { ImperialHuman = 50 },
        glyph   = string.byte("e"),
        name    = "Entertainer",
        color   = "magenta",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Entertainer",
        xp      = 14,
        power   = 2.0,
        skill   = 25,                    -- WS (base)
        ws = 25, bs = 20, s = 30, t = 30, ag = 30, int = 25, per = 25, wp = 25, fel = 35,
        equipment = {},
        -- Colonist base skills + Entertainer additions.
        skills  = { Awareness = 0, ["Common Lore (Imperium)"] = 0,
                    ["Drive (Ground Vehicle)"] = 0,
                    ["Speak Language (Low Gothic)"] = 0, ["Trade (Labourer)"] = 0,
                    Carouse = 0, Charm = 0, Deceive = 0, Performer = 0 },
        talents = { "Basic Weapon Training (SP)", "Melee Weapon Training (Primitive)" },
        traits  = {},
    },

    -- §5.1 — Hired Gun (Colonist variant): base + BS 35, Wounds 12
    {
        chance  = { ImperialHuman = 62 },
        glyph   = string.byte("h"),
        name    = "Hired Gun",
        color   = "tan",
        hp      = 12.0,                  -- Wounds override 12
        defense = 0.0,
        corpse  = "dead Hired Gun",
        xp      = 24,
        power   = 2.0,
        skill   = 35,                    -- BS (Lasgun / stub automatic ranged-primary)
        ws = 25, bs = 35, s = 30, t = 30, ag = 30, int = 25, per = 25, wp = 25, fel = 30,
        equipment = {},
        -- Colonist base skills + Hired Gun additions.
        skills  = { Awareness = 0, ["Common Lore (Imperium)"] = 0,
                    ["Drive (Ground Vehicle)"] = 0,
                    ["Speak Language (Low Gothic)"] = 0, ["Trade (Labourer)"] = 0,
                    Climb = 0, Intimidate = 0 },
        -- Colonist base talents + Hired Gun additions.
        talents = { "Basic Weapon Training (SP)", "Melee Weapon Training (Primitive)",
                    "Basic Weapon Training (Universal)",
                    "Pistol Weapon Training (Universal)" },
        traits  = {},
    },

    -- §5.1 — Scum (Colonist variant): base + WS 30, Per 30; Wounds 9
    {
        chance  = { ImperialHuman = 74 },
        glyph   = string.byte("s"),
        name    = "Scum",
        color   = "brown",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Scum",
        xp      = 16,
        power   = 2.0,
        skill   = 30,                    -- WS (base override) / stub revolver
        ws = 30, bs = 20, s = 30, t = 30, ag = 30, int = 25, per = 30, wp = 25, fel = 30,
        equipment = {},
        -- Colonist base skills + Scum additions.
        skills  = { Awareness = 0, ["Common Lore (Imperium)"] = 0,
                    ["Drive (Ground Vehicle)"] = 0,
                    ["Speak Language (Low Gothic)"] = 0, ["Trade (Labourer)"] = 0,
                    Carouse = 0, ["Chem-Use"] = 0, Deceive = 0, Gamble = 0,
                    ["Silent Move"] = 0 },
        -- Colonist base talents + Scum additions.
        talents = { "Basic Weapon Training (SP)", "Melee Weapon Training (Primitive)",
                    "Jaded", "Pistol Weapon Training (Universal)" },
        traits  = {},
    },

    -- §5.1 — Voidfarer (Colonist variant): base + S 38, T 38; Wounds 9
    {
        chance  = { ImperialHuman = 82 },
        glyph   = string.byte("v"),
        name    = "Voidfarer",
        color   = "steelBlue",
        hp      = 9.0,
        defense = 0.0,
        corpse  = "dead Voidfarer",
        xp      = 16,
        power   = 3.0,
        skill   = 25,                    -- WS (base)
        ws = 25, bs = 20, s = 38, t = 38, ag = 30, int = 25, per = 25, wp = 25, fel = 30,
        equipment = {},
        -- Colonist base skills + Voidfarer additions.
        skills  = { Awareness = 0, ["Common Lore (Imperium)"] = 0,
                    ["Drive (Ground Vehicle)"] = 0,
                    ["Speak Language (Low Gothic)"] = 0, ["Trade (Labourer)"] = 0,
                    ["Speak Language (Void Cant)"] = 0, ["Tech-Use"] = 0 },
        talents = { "Basic Weapon Training (SP)", "Melee Weapon Training (Primitive)" },
        traits  = {},
    },

    -- §5.1 — Mutant Outcast (Troop): WS 28 BS 22 S 35 T 35 Ag 22 Int 18 Per 25 WP 18 Fel 15, Wounds 12
    {
        chance  = { ImperialHuman = 88 },
        glyph   = string.byte("M"),
        name    = "Mutant Outcast",
        color   = "brown",
        hp      = 12.0,
        defense = 0.0,
        corpse  = "dead Mutant Outcast",
        xp      = 22,
        power   = 3.0,
        skill   = 28,                    -- WS (Improvised club melee-primary)
        ws = 28, bs = 22, s = 35, t = 35, ag = 22, int = 18, per = 25, wp = 18, fel = 15,
        equipment = {},
        skills  = { Climb = 0, Intimidate = 0, Survival = 0, ["Trade (Scavenger)"] = 0 },
        talents = { "Basic Weapon Training (Primitive)", "Frenzy", "Jaded",
                    "Resistance (Poisons)" },
        traits  = { "Mutation" },
    },

    -- §5.1 — Oathsworn Bodyguard (Troop): WS 42 BS 35 S 35 T 40 Ag 35 Int 35 Per 38 WP 35 Fel 28, Wounds 18
    {
        chance  = { ImperialHuman = 93 },
        glyph   = string.byte("O"),
        name    = "Oathsworn Bodyguard",
        color   = "white",
        hp      = 18.0,
        defense = 1.0,
        corpse  = "dead Oathsworn Bodyguard",
        xp      = 48,
        power   = 3.0,
        skill   = 42,                    -- WS (melee-primary; strong ranged too)
        ws = 42, bs = 35, s = 35, t = 40, ag = 35, int = 35, per = 38, wp = 35, fel = 28,
        equipment = {},
        skills  = { Awareness = 1, ["Common Lore (Imperium)"] = 0, Dodge = 1,
                    ["Drive (Land Vehicle)"] = 0, Inquiry = 0, Intimidate = 1,
                    Scrutiny = 1, Security = 1, Shadowing = 1, ["Silent Move"] = 0 },
        talents = { "Basic Weapon Training (Las)", "Basic Weapon Training (SP)",
                    "Basic Weapon Training (Bolt)", "Crack Shot", "Disarm",
                    "Melee Weapon Training (Primitive)",
                    "Melee Weapon Training (Universal)", "Nerves of Steel",
                    "Pistol Weapon Training (Universal)", "Quick Draw" },
        traits  = {},
    },

    -- §5.1 — Renegade (Troop): WS 38 BS 28 S 35 T 40 Ag 30 Int 25 Per 33 WP 35 Fel 22, Wounds 12
    -- NOTE: this is the Chapter V Imperial-Human "Renegade" officer (§5.1), a DIFFERENT
    -- entry from the Chaos "Renegade Soldier"/"Renegade Veteran" (IV.1). Its name is
    -- exactly "Renegade".
    {
        chance  = { ImperialHuman = 97 },
        glyph   = string.byte("r"),
        name    = "Renegade",
        color   = "tan",
        hp      = 12.0,
        defense = 0.0,
        corpse  = "dead Renegade",
        xp      = 32,
        power   = 3.0,
        skill   = 38,                    -- WS (Chainsword / hand cannon)
        ws = 38, bs = 28, s = 35, t = 40, ag = 30, int = 25, per = 33, wp = 35, fel = 22,
        equipment = {},
        skills  = { Awareness = 0, ["Common Lore (Imperium)"] = 0, ["Chem-Use"] = 0,
                    Intimidate = 1, ["Speak Language (Low Gothic)"] = 0, Deceive = 0,
                    ["Tech-Use"] = 0 },
        talents = { "Basic Weapon Training (SP)", "Jaded",
                    "Melee Weapon Training (Chain)", "Melee Weapon Training (Primitive)",
                    "Melee Weapon Training (Thrown)", "Pistol Training (Las)",
                    "Pistol Training (SP)", "Peer (Renegade)" },
        traits  = {},
    },

    -- §5.1 — Warp Witch (Troop): WS 28 BS 28 S 30 T 40 Ag 36 Int 28 Per 37 WP 45 Fel 23, Wounds 13
    {
        chance  = { ImperialHuman = 100 },
        glyph   = string.byte("W"),
        name    = "Warp Witch",
        color   = "purple",
        hp      = 13.0,
        defense = 0.0,
        corpse  = "dead Warp Witch",
        xp      = 44,
        power   = 2.0,
        skill   = 28,                    -- WS (Sacrificial blade melee)
        ws = 28, bs = 28, s = 30, t = 40, ag = 36, int = 28, per = 37, wp = 45, fel = 23,
        equipment = {},
        skills  = { Awareness = 0, ["Ciphers (Occult)"] = 0,
                    ["Common Lore (Imperium)"] = 0, Command = 0, Deceive = 1,
                    ["Forbidden Lore (Warp)"] = 1, Intimidate = 1, Invocation = 1,
                    Psyniscience = 0, ["Secret Tongue (Cult)"] = 0,
                    ["Speak Language (Low Gothic)"] = 0, ["Trade (Seer)"] = 0 },
        talents = { "Dark Soul", "Fearless", "Jaded",
                    "Melee Weapon Training (Primitive)", "Peer (Renegade)",
                    "Pistol Weapon Training (SP)", "Psy Rating (6)",
                    "Resistance (Psychic Techniques)" },
        traits  = { "Dark Pact", "Mutation" },
    },

    -- ─────────────────────────────────────────────────────────────────────────
    -- Servitor Faction_Region column (Bestiary §5.2 Servitors). 4 Troop entries,
    -- cumulative chance strictly ascending in declaration order, terminal value 100.
    -- Characteristics copied verbatim as the base integer: em-dash Fel/BS -> 0
    -- (Req 2.2); Unnatural parentheticals (S/T "(8)") excluded from the stored
    -- integer and recorded as `Unnatural X (xN)` traits (Req 2.3/2.4). hp = profile
    -- Wounds (Req 3.1). Machine / Flyer / Size traits recorded as trait strings
    -- (Req 3.3). Each (glyph, color) pair is distinct from every other entry across
    -- the Bestiary, using colors defined in Headers/Colors.hpp (Req 1.12). Equipment
    -- left empty for now; weapon/armour wiring is handled by tasks 12.3/12.4.
    -- ─────────────────────────────────────────────────────────────────────────

    -- §5.2 — Battle Servitor (Charron-Pattern) (Troop): WS 30 BS 30 S 40(8) T 40(8) Ag 20 Int 20 Per 30 WP 40 Fel —, Wounds 15
    {
        chance  = { Servitor = 25 },
        glyph   = string.byte("B"),
        name    = "Battle Servitor (Charron-Pattern)",
        color   = "metalGrey",
        hp      = 15.0,
        defense = 1.0,
        corpse  = "wrecked Battle Servitor",
        xp      = 40,
        power   = 4.0,
        skill   = 30,                    -- WS (servo-fists / heavy weapon platform; Fel em-dash -> 0)
        ws = 30, bs = 30, s = 40, t = 40, ag = 20, int = 20, per = 30, wp = 40, fel = 0,
        equipment = {},
        skills  = { Awareness = 0 },
        talents = { "Two-Weapon Wielder" },
        traits  = { "Armour Plated", "Auto-Stabilised", "Dark-Sight", "Flyer (2)",
                    "Machine (4)", "Natural Weapon (Servo-Fists)", "Sturdy",
                    "Unnatural Strength (x2)", "Unnatural Toughness (x2)" },
    },

    -- §5.2 — Grapplehawk (Falax-Pattern) (Troop): WS 40 BS — S 35 T 35 Ag 48 Int 18 Per 40 WP 30 Fel —, Wounds 8
    {
        chance  = { Servitor = 50 },
        glyph   = string.byte("g"),
        name    = "Grapplehawk (Falax-Pattern)",
        color   = "metalGrey",
        hp      = 8.0,
        defense = 1.0,
        corpse  = "wrecked Grapplehawk",
        xp      = 30,
        power   = 3.0,
        skill   = 40,                    -- WS (Shock-pulse claws melee; BS/Fel em-dash -> 0)
        ws = 40, bs = 0, s = 35, t = 35, ag = 48, int = 18, per = 40, wp = 30, fel = 0,
        equipment = {},
        skills  = { Awareness = 1, Dodge = 0 },
        talents = { "Fearless", "Swift Attack" },
        traits  = { "Armour Plating", "Dark-Sight", "Flyer (20)", "Machine (4)",
                    "Size (Scrawny)" },
    },

    -- §5.2 — Servitor Drone (Troop): WS 15 BS 15 S 50 T 40 Ag 15 Int 10 Per 20 WP 30 Fel 05, Wounds 10
    {
        chance  = { Servitor = 75 },
        glyph   = string.byte("d"),
        name    = "Servitor Drone",
        color   = "metalGrey",
        hp      = 10.0,
        defense = 1.0,
        corpse  = "wrecked Servitor Drone",
        xp      = 20,
        power   = 3.0,
        skill   = 15,                    -- WS (Fist natural weapon melee)
        ws = 15, bs = 15, s = 50, t = 40, ag = 15, int = 10, per = 20, wp = 30, fel = 5,
        equipment = {},
        skills  = { ["Trade (Any One)"] = 1 },
        talents = {},
        traits  = { "Machine (4)", "Natural Weapon (Fist)" },
    },

    -- §5.2 — Servo Skull (Troop): WS 15 BS 15 S 10 T 20 Ag 30 Int 15 Per 35 WP 20 Fel —, Wounds 4
    {
        chance  = { Servitor = 100 },
        glyph   = string.byte("x"),
        name    = "Servo Skull",
        color   = "boneWhite",
        hp      = 4.0,
        defense = 0.0,
        corpse  = "wrecked Servo Skull",
        xp      = 10,
        power   = 1.0,
        skill   = 15,                    -- WS (unarmed; recon drone; Fel em-dash -> 0)
        ws = 15, bs = 15, s = 10, t = 20, ag = 30, int = 15, per = 35, wp = 20, fel = 0,
        equipment = {},
        skills  = { Awareness = 1, Concealment = 1, Dodge = 0, ["Silent Move"] = 1 },
        talents = { "Fearless" },
        traits  = { "Dark-Sight", "Flyer (6)", "Machine (2)", "Size (Puny)" },
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
