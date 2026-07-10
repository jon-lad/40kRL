-- Talents.lua
-- Defines all talent templates. Loaded by Engine::init via loadTalentDefinitions().
-- Each entry has: name (string), description (string).

talents = {
    {
        name = "Pistol Weapon Training (Universal)",
        description = "May use any pistol weapon without penalty.",
    },
    {
        name = "Basic Weapon Training (Universal)",
        description = "May use any basic weapon without penalty.",
    },
    {
        name = "Air of Authority",
        description = "Affect more targets with Command tests.",
    },
    {
        name = "Iron Discipline",
        description = "Followers may re-roll failed Willpower tests.",
    },
    {
        name = "Swift Attack",
        description = "May make multiple melee attacks in a single turn.",
    },
    {
        name = "Crushing Blow",
        description = "Add half Weapon Skill bonus to melee damage.",
    },
}
