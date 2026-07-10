-- Talents.lua
-- Defines all talent templates. Loaded by Engine::init via loadTalentDefinitions().
-- Each entry has: name (string), description (string).

talents = {
    -- Universal weapon trainings
    {
        name = "Pistol Weapon Training (Universal)",
        description = "May use any pistol weapon without penalty.",
    },
    {
        name = "Basic Weapon Training (Universal)",
        description = "May use any basic weapon without penalty.",
    },
    {
        name = "Melee Weapon Training (Primitive)",
        description = "May use primitive melee weapons without penalty.",
    },

    -- Rogue Trader career talents
    {
        name = "Air of Authority",
        description = "Affect more targets with Command tests.",
    },
    {
        name = "Exceptional Leader",
        description = "Followers gain +10 to Willpower tests against Fear.",
    },
    {
        name = "Iron Discipline",
        description = "Followers may re-roll failed Willpower tests.",
    },
    {
        name = "Master Orator",
        description = "Affect large crowds with Charm and Command tests.",
    },
    {
        name = "Into the Jaws of Hell",
        description = "Allies within earshot ignore Fear and Pinning while you lead.",
    },

    -- Arch-Militant career talents
    {
        name = "Swift Attack",
        description = "May make multiple melee attacks in a single turn.",
    },
    {
        name = "Crushing Blow",
        description = "Add half Weapon Skill bonus to melee damage.",
    },
    {
        name = "Lightning Attack",
        description = "May make even more melee attacks than Swift Attack.",
    },
    {
        name = "Combat Master",
        description = "Opponents gain no bonus for outnumbering you in melee.",
    },
    {
        name = "Wall of Steel",
        description = "May make one additional Parry per round.",
    },
    {
        name = "Step Aside",
        description = "May attempt a second Dodge per round.",
    },

    -- Void-Master career talents
    {
        name = "Crack Shot",
        description = "Add degree of success bonus damage to ranged attacks.",
    },
    {
        name = "Lightning Reflexes",
        description = "Double Agility bonus for Initiative rolls.",
    },
    {
        name = "Sharpshooter",
        description = "Reduce penalty for called shots by 10.",
    },
    {
        name = "Hard Target",
        description = "Enemies suffer -20 to hit when you Run.",
    },
    {
        name = "Master Pilot",
        description = "Re-roll failed Pilot tests.",
    },
    {
        name = "Nerves of Steel",
        description = "Re-roll failed Pinning tests.",
    },

    -- Explorator (Tech-Priest) career talents
    {
        name = "Mechadendrite Use (Utility)",
        description = "May use utility mechadendrites in combat and exploration.",
    },
    {
        name = "Binary Chatter",
        description = "+10 to Fellowship tests with tech-priests and servitors.",
    },
    {
        name = "Electrical Succour",
        description = "Restore 1d5 wounds to a machine or cybernetic.",
    },
    {
        name = "Luminen Charge",
        description = "Recharge or power devices by channeling electrical energy.",
    },
    {
        name = "Ferric Lure",
        description = "Summon metallic objects within 20m towards you.",
    },
    {
        name = "Master Enginseer",
        description = "+20 to Tech-Use tests for repairs.",
    },
    {
        name = "Rite of Pure Thought",
        description = "Immune to Fear, Pinning, and mind-affecting psychic powers.",
    },

    -- Missionary career talents
    {
        name = "Frenzy",
        description = "Enter Frenzy for +10 WS, +10 S, immunity to Fear and Stunning.",
    },
    {
        name = "Flagellant",
        description = "Gain +10 WP for resisting pain and fear.",
    },
    {
        name = "Battle Rage",
        description = "May Charge while Frenzied.",
    },
    {
        name = "True Grit",
        description = "Halve Critical damage taken (round up).",
    },
    {
        name = "Iron Will",
        description = "+10 to resist psychic powers.",
    },
    {
        name = "Touched by the Fates",
        description = "Gain one additional Fate Point.",
    },

    -- Seneschal career talents
    {
        name = "Peer (Underworld)",
        description = "+10 to Fellowship tests with criminals and black marketeers.",
    },
    {
        name = "Cover-Up",
        description = "Automatically succeed at concealing minor crimes.",
    },
    {
        name = "Unremarkable",
        description = "Enemies must pass Awareness to notice you in crowds.",
    },
    {
        name = "Master of All Trades",
        description = "Count as Trained in all untrained Basic skills.",
    },
    {
        name = "Infused Knowledge",
        description = "Count as having Scholastic Lore for all fields.",
    },
}
