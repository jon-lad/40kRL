-- Homeworlds.lua
-- Defines homeworld templates for character generation.
-- Loaded by Engine at startup via sol2.
--
-- Each entry has:
--   name           : string  (display name)
--   charMods       : table   (ws, bs, s, t, ag, int, per, wp, fel as signed integers)
--   startingSkills : array   (skill name strings granted at Trained rank)
--   startingTraits : array   (trait name strings recorded on character)

homeworlds = {
    {
        name = "Void Born",
        charMods = { ws = 0, bs = 0, s = -5, t = 0, ag = 0, int = 0, per = 0, wp = 5, fel = 0 },
        startingSkills = { "Navigation (Stellar)", "Pilot (Spacecraft)" },
        startingTraits = { "Void Accustomed", "Charmed" },
    },
    {
        name = "Hive World",
        charMods = { ws = 0, bs = 0, s = 0, t = 0, ag = 5, int = 0, per = 0, wp = 0, fel = -5 },
        startingSkills = { "Awareness", "Deceive" },
        startingTraits = { "Hivebound", "Wary" },
    },
    {
        name = "Feral World",
        charMods = { ws = 5, bs = 0, s = 5, t = 0, ag = 0, int = -5, per = 0, wp = 0, fel = -5 },
        startingSkills = { "Survival", "Tracking" },
        startingTraits = { "Iron Stomach", "Primitive" },
    },
}
