-- Skills.lua
-- Defines all available skills and their properties.
-- Each entry has a name and whether it is classified as combat or non-combat.
-- Non-combat skills are recorded on the character but have no mechanical effect this version.

skills = {
    -- Non-combat skills (referenced by Homeworlds.lua)
    { name = "Navigation (Stellar)",   isCombat = false },
    { name = "Pilot (Spacecraft)",     isCombat = false },
    { name = "Awareness",              isCombat = false },
    { name = "Deceive",                isCombat = false },
    { name = "Survival",               isCombat = false },
    { name = "Tracking",               isCombat = false },

    -- Non-combat skills (referenced by Careers.lua)
    { name = "Command",                isCombat = false },
    { name = "Commerce",               isCombat = false },
    { name = "Barter",                 isCombat = false },

    -- Combat skills (referenced by Arch-Militant career path)
    { name = "Dodge",                  isCombat = true },
    { name = "Intimidate",             isCombat = true },
}
