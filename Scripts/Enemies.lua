-- Enemies.lua
-- Defines enemy templates used by Map::addMonster.
-- C++ calls: spawnEnemy(roll, x, y)
--   roll : int  (0-99 random roll)
--   x, y : int  (world position)
-- The function calls back into C++ via the injected addActor(x, y, entry) function,
-- passing the entire enemy table entry so C++ can read all fields including equipment config.

-- Enemy definitions table.
-- Fields: chance (cumulative %), glyph, name, color, hp, defense, corpse, xp, power, skill
-- Optional equipment fields: equipment (list of strings), dropChance (float), equipTier (table)
local enemies = {
    {
        chance  = 60,
        glyph   = string.byte("g"),
        name    = "Gretchin",
        color   = "desaturatedGreen",
        hp      = 5.0,
        defense = 0.0,
        corpse  = "dead Gretchin",
        xp      = 15,
        power   = 2.0,
        skill   = 25,
        ws = 25, bs = 15, s = 20, t = 20, ag = 30, int = 15, per = 25, wp = 15, fel = 10,
        equipment = { "Choppa" },
        dropChance = 0.3,
    },
    {
        chance  = 90,
        glyph   = string.byte("o"),
        name    = "Ork",
        color   = "desaturatedGreen",
        hp      = 10.0,
        defense = 0.0,
        corpse  = "dead Ork",
        xp      = 35,
        power   = 3.0,
        skill   = 35,
        ws = 35, bs = 20, s = 40, t = 40, ag = 25, int = 15, per = 25, wp = 25, fel = 15,
        equipTier = { common = 80, uncommon = 18, rare = 2 },
        dropChance = 0.4,
    },
    {
        chance  = 100,
        glyph   = string.byte("N"),
        name    = "Nob",
        color   = "darkerGreen",
        hp      = 16.0,
        defense = 1.0,
        corpse  = "Nob carcass",
        xp      = 100,
        power   = 4.0,
        skill   = 45,
        ws = 45, bs = 25, s = 50, t = 50, ag = 30, int = 20, per = 30, wp = 35, fel = 25,
        equipment = { "Big Choppa", "Ork Armor" },
        dropChance = 0.5,
    },
}

function spawnEnemy(roll, x, y)
    for _, e in ipairs(enemies) do
        if roll < e.chance then
            addActor(x, y, e)
            return
        end
    end
end
