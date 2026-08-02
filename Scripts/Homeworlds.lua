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
    {
        name = "Death World",
        charMods = { ws = 5, bs = 0, s = 0, t = 5, ag = -5, int = 0, per = 0, wp = 0, fel = -5 },
        startingSkills = { "Survival", "Tracking" },
        startingTraits = { "Iron Stomach", "Paranoid" },
    },
    {
        name = "Forge World",
        charMods = { ws = 0, bs = 0, s = -5, t = 0, ag = 0, int = 5, per = 0, wp = 5, fel = -5 },
        startingSkills = { "Tech-Use", "Logic" },
        startingTraits = { "Mechanicus Implants", "Cog Boy" },
    },
    {
        name = "Noble Born",
        charMods = { ws = 0, bs = 0, s = 0, t = -5, ag = 0, int = 0, per = 0, wp = -5, fel = 10 },
        startingSkills = { "Charm", "Command" },
        startingTraits = { "Supremely Connected", "Vendetta" },
    },
    {
        name = "Imperial World",
        charMods = { ws = 0, bs = 0, s = -5, t = 0, ag = -5, int = 0, per = 5, wp = 5, fel = 0 },
        startingSkills = { "Common Lore (Imperium)", "Literacy" },
        startingTraits = { "Blessed Ignorance", "Liturgical Familiarity" },
    },
    {
        name = "Frontier World",
        charMods = { ws = 0, bs = 5, s = 0, t = 0, ag = 0, int = -5, per = 5, wp = 0, fel = -5 },
        startingSkills = { "Survival", "Navigation (Surface)" },
        startingTraits = { "Frontier Hardiness", "Wilderness Savvy" },
    },
}
