-- Equipment.lua
-- Defines equippable item templates loaded at game initialization.
-- Each entry is validated by the C++ loader; invalid entries are skipped with a warning.
-- Required fields: name, glyph, color, slot, weight
-- Optional fields (default to 0): value, power, defense, maxHp, skill

equipment = {
    {
        name    = "Chainsword",
        glyph   = "/",
        color   = "lightBlue",
        slot    = "weapon",
        weight  = 3.5,
        value   = 50,
        power   = 3.0,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = 0,
    },
    {
        name    = "Power Sword",
        glyph   = "|",
        color   = "lightGrey",
        slot    = "weapon",
        weight  = 5.0,
        value   = 80,
        power   = 4.0,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = 10,
    },
    {
        name    = "Flak Armor",
        glyph   = "[",
        color   = "lighterOrange",
        slot    = "body",
        weight  = 8.0,
        value   = 30,
        power   = 0.0,
        defense = 2.0,
        maxHp   = 0.0,
        skill   = -5,
    },
    {
        name    = "Combat Knife",
        glyph   = "-",
        color   = "white",
        slot    = "weapon",
        weight  = 1.0,
        value   = 15,
        power   = 1.0,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = 5,
    },
    {
        name    = "Carapace Helm",
        glyph   = "^",
        color   = "lightGrey",
        slot    = "head",
        weight  = 4.0,
        value   = 40,
        power   = 0.0,
        defense = 1.0,
        maxHp   = 5.0,
        skill   = -2,
    },
}
