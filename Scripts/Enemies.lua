-- Enemies.lua
-- Defines enemy templates used by Map::addMonster.
-- C++ calls: spawnEnemy(roll, x, y)
--   roll : int  (0-99 random roll)
--   x, y : int  (world position)
-- The function calls back into C++ via the injected addActor(actor_def) function.

-- Enemy definitions table.
-- Fields: chance (cumulative %), glyph, name, color, hp, defense, corpse, xp, power
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
    },
}

function spawnEnemy(roll, x, y)
    for _, e in ipairs(enemies) do
        if roll < e.chance then
            addActor(x, y, e.glyph, e.name, e.color, e.hp, e.defense, e.corpse, e.xp, e.power)
            return
        end
    end
end
