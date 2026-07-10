-- Careers.lua
-- Defines career path templates with rank-gated advance tables.
-- Each career has multiple ranks; each rank has an XP threshold and a table of purchasable advances.
-- Loaded by Engine::init() via sol2.

careers = {
    {
        name = "Rogue Trader",
        ranks = {
            {
                rankNumber = 1,
                rankTitle = "Rogue Trader",
                xpThreshold = 0,
                startingSkills = { "Command", "Commerce" },
                startingTalents = { "Pistol Weapon Training (Universal)" },
                advances = {
                    { type = "characteristic", name = "WS",  cost = 100, amount = 5 },
                    { type = "characteristic", name = "BS",  cost = 100, amount = 5 },
                    { type = "characteristic", name = "Fel", cost = 100, amount = 5 },
                    { type = "characteristic", name = "WP",  cost = 100, amount = 5 },
                    { type = "characteristic", name = "Int", cost = 250, amount = 5 },
                    { type = "skill",          name = "Command",   cost = 100 },
                    { type = "skill",          name = "Commerce",  cost = 100 },
                    { type = "skill",          name = "Charm",     cost = 100 },
                    { type = "talent",         name = "Air of Authority", cost = 200 },
                    { type = "talent",         name = "Exceptional Leader", cost = 200 },
                },
            },
            {
                rankNumber = 2,
                rankTitle = "Master Trader",
                xpThreshold = 500,
                startingSkills = {},
                startingTalents = {},
                advances = {
                    { type = "characteristic", name = "Fel", cost = 250, amount = 5 },
                    { type = "characteristic", name = "WP",  cost = 250, amount = 5 },
                    { type = "characteristic", name = "Int", cost = 250, amount = 5 },
                    { type = "skill",          name = "Barter",    cost = 200 },
                    { type = "skill",          name = "Evaluate",  cost = 200 },
                    { type = "talent",         name = "Iron Discipline", cost = 300 },
                    { type = "talent",         name = "Master Orator",   cost = 300 },
                },
            },
            {
                rankNumber = 3,
                rankTitle = "Warrant Holder",
                xpThreshold = 1500,
                startingSkills = {},
                startingTalents = {},
                advances = {
                    { type = "characteristic", name = "Fel", cost = 500, amount = 5 },
                    { type = "characteristic", name = "WS",  cost = 500, amount = 5 },
                    { type = "characteristic", name = "BS",  cost = 500, amount = 5 },
                    { type = "skill",          name = "Command",   cost = 300 },
                    { type = "skill",          name = "Intimidate", cost = 300 },
                    { type = "talent",         name = "Into the Jaws of Hell", cost = 500 },
                },
            },
        },
    },
    {
        name = "Arch-Militant",
        ranks = {
            {
                rankNumber = 1,
                rankTitle = "Arch-Militant",
                xpThreshold = 0,
                startingSkills = { "Dodge", "Awareness" },
                startingTalents = { "Basic Weapon Training (Universal)" },
                advances = {
                    { type = "characteristic", name = "WS",  cost = 100, amount = 5 },
                    { type = "characteristic", name = "BS",  cost = 100, amount = 5 },
                    { type = "characteristic", name = "S",   cost = 100, amount = 5 },
                    { type = "characteristic", name = "T",   cost = 250, amount = 5 },
                    { type = "characteristic", name = "Ag",  cost = 100, amount = 5 },
                    { type = "skill",          name = "Dodge",     cost = 100 },
                    { type = "skill",          name = "Awareness", cost = 100 },
                    { type = "skill",          name = "Intimidate", cost = 100 },
                    { type = "talent",         name = "Crushing Blow",   cost = 200 },
                    { type = "talent",         name = "Swift Attack",    cost = 200 },
                },
            },
            {
                rankNumber = 2,
                rankTitle = "Warrior Prime",
                xpThreshold = 500,
                startingSkills = {},
                startingTalents = {},
                advances = {
                    { type = "characteristic", name = "WS",  cost = 250, amount = 5 },
                    { type = "characteristic", name = "BS",  cost = 250, amount = 5 },
                    { type = "characteristic", name = "S",   cost = 250, amount = 5 },
                    { type = "characteristic", name = "T",   cost = 250, amount = 5 },
                    { type = "skill",          name = "Parry",     cost = 200 },
                    { type = "skill",          name = "Scrutiny",  cost = 200 },
                    { type = "talent",         name = "Lightning Attack", cost = 300 },
                    { type = "talent",         name = "Combat Master",    cost = 300 },
                },
            },
            {
                rankNumber = 3,
                rankTitle = "Battlemaster",
                xpThreshold = 1500,
                startingSkills = {},
                startingTalents = {},
                advances = {
                    { type = "characteristic", name = "WS",  cost = 500, amount = 5 },
                    { type = "characteristic", name = "S",   cost = 500, amount = 5 },
                    { type = "characteristic", name = "T",   cost = 500, amount = 5 },
                    { type = "skill",          name = "Dodge",     cost = 300 },
                    { type = "skill",          name = "Command",   cost = 300 },
                    { type = "talent",         name = "Wall of Steel",   cost = 500 },
                    { type = "talent",         name = "Step Aside",      cost = 500 },
                },
            },
        },
    },
}
