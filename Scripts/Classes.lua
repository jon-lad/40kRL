-- Classes.lua
-- Defines player class templates. Engine::init loads the default class stats.
-- Later this will support a class selection menu at game start.

classes = {
    {
        name    = "Space Marine",
        hp      = 30.0,
        defense = 2.0,
        power   = 5.0,
        skill   = 50,
        invSize = 26,
        ws      = 40,
        bs      = 45,
        s       = 40,
        t       = 40,
        ag      = 35,
        int     = 30,
        per     = 35,
        wp      = 35,
        fel     = 25,
    },
    -- Future classes:
    -- { name = "Imperial Guard", hp = 20.0, defense = 1.0, power = 3.0, skill = 35, invSize = 30 },
    -- { name = "Tech-Priest",   hp = 25.0, defense = 3.0, power = 4.0, skill = 40, invSize = 20 },
}

-- For now the engine picks the first entry. Later this becomes a menu selection.
defaultClass = classes[1]
