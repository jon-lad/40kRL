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

    -- Non-combat skills (added for new homeworlds and careers)
    { name = "Tech-Use",                          isCombat = false },
    { name = "Logic",                             isCombat = false },
    { name = "Charm",                             isCombat = false },
    { name = "Common Lore (Imperium)",            isCombat = false },
    { name = "Literacy",                          isCombat = false },
    { name = "Navigation (Surface)",              isCombat = false },
    { name = "Medicae",                           isCombat = false },
    { name = "Chem-Use",                          isCombat = false },
    { name = "Forbidden Lore (Archeotech)",       isCombat = false },
    { name = "Security",                          isCombat = false },
    { name = "Scholastic Lore (Imperial Creed)",  isCombat = false },
    { name = "Evaluate",                          isCombat = false },
    { name = "Inquiry",                           isCombat = false },
    { name = "Blather",                           isCombat = false },
    { name = "Scrutiny",                          isCombat = false },

    -- Combat skills
    { name = "Dodge",                  isCombat = true },
    { name = "Intimidate",             isCombat = true },
    { name = "Parry",                  isCombat = true },
}
