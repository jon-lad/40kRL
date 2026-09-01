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
        chance  = 50,
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
        chance  = 75,
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
        chance  = 90,
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
        chance  = 100,
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

function spawnEnemy(roll, x, y)
    for _, e in ipairs(enemies) do
        if roll < e.chance then
            addActor(x, y, e)
            return
        end
    end
end
