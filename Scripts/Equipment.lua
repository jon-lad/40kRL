-- Equipment.lua
-- Defines equippable item templates loaded at game initialization.
-- Each entry is validated by the C++ loader; invalid entries are skipped with a warning.
-- Required fields: name, glyph, color, slot, weight
-- Optional fields (default to 0): value, power, defense, maxHp, skill
-- Optional field: tier ("common", "uncommon", "rare") — defaults to "common" if omitted.

equipment = {
    -- ===== Existing Player-Oriented Equipment =====

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
        tier    = "common",
        region  = { Universal = 100 },  -- primitive/civilian knife (RT-Weapons Primitive)
        sizeClass   = "Melee",
        weaponGroup = "Primitive",
        damageType  = "R",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
    },
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
        tier    = "uncommon",
        region  = { ImperialHuman = 100 },  -- RT-Weapons Chain: ImperialHuman
        sizeClass   = "Melee",
        weaponGroup = "Primitive",
        damageType  = "R",
        melee = {
            damageDice = "1d10",
            penetration = 2,
            qualities = {"Tearing", "Balanced"},
        },
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
        tier    = "rare",
        region  = { ImperialHuman = 100 },  -- RT-Weapons Power: ImperialHuman
        sizeClass   = "Melee",
        weaponGroup = "Primitive",
        damageType  = "E",
        melee = {
            damageDice = "1d10",
            penetration = 5,
            qualities = {"Power Field", "Balanced"},
        },
    },
    {
        name    = "Laspistol",
        glyph   = ")",
        color   = "lightRed",
        slot    = "weapon",
        weight  = 1.5,
        value   = 20,
        power   = 1.0,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = 0,
        tier    = "common",
        region  = { ImperialHuman = 100 },  -- RT-Weapons Las: ImperialHuman
        sizeClass   = "Pistol",
        weaponGroup = "Las",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10",
            penetration = 0,
            range       = 30,
            rateOfFire  = 1,
            clipSize    = 30,
            reloadTime  = 1,
        },
    },
    {
        name    = "Autogun",
        glyph   = "}",
        color   = "lightGrey",
        slot    = "weapon",
        weight  = 4.5,
        value   = 35,
        power   = 1.0,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = 0,
        tier    = "uncommon",
        region  = { Universal = 100 },  -- RT-Weapons SP: Autogun is Universal
        sizeClass   = "Basic",
        weaponGroup = "SP",
        damageType  = "I",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10",
            penetration = 0,
            range       = 40,
            rateOfFire  = 3,
            clipSize    = 24,
            reloadTime  = 1,
        },
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
        tier    = "uncommon",
        region  = { ImperialHuman = 100 },  -- RT-Equipment Flak Armour: ImperialHuman
        armourLocations = {
            head     = 0,
            body     = 3,
            leftArm  = 3,
            rightArm = 3,
            leftLeg  = 3,
            rightLeg = 3,
        },
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
        tier    = "uncommon",
        region  = { ImperialHuman = 100 },  -- RT-Equipment Carapace: ImperialHuman
        armourLocations = {
            head     = 4,
            body     = 0,
            leftArm  = 0,
            rightArm = 0,
            leftLeg  = 0,
            rightLeg = 0,
        },
    },

    -- ===== Ork Equipment (Enemy-Appropriate) =====

    -- Common tier: basic Ork weapons and armor, weak stats
    {
        name    = "Choppa",
        glyph   = "/",
        color   = "desaturatedGreen",
        slot    = "weapon",
        weight  = 4.0,
        value   = 10,
        power   = 2.0,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = -5,
        tier    = "common",
        region  = { Ork = 100 },  -- RT-Weapons Exotic/Xenos: Choppa (Ork)
        sizeClass   = "Melee",
        weaponGroup = "Primitive",
        damageType  = "R",
        melee = {
            damageDice = "1d10",
            penetration = 0,
            qualities = {"Unbalanced"},
        },
    },
    {
        name    = "Slugga",
        glyph   = ")",
        color   = "desaturatedGreen",
        slot    = "weapon",
        weight  = 2.5,
        value   = 12,
        power   = 1.5,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = 0,
        tier    = "common",
        region  = { Ork = 100 },  -- RT-Weapons SP: Ork Slugga
        sizeClass   = "Pistol",
        weaponGroup = "SP",
        damageType  = "I",
        melee = {
            damageDice = "1d10",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10",
            penetration = 0,
            range       = 15,
            rateOfFire  = 1,
            clipSize    = 6,
            reloadTime  = 1,
        },
    },
    {
        name    = "Scrap Shield",
        glyph   = "(",
        color   = "lightYellow",
        slot    = "offhand",
        weight  = 5.0,
        value   = 8,
        power   = 0.0,
        defense = 1.0,
        maxHp   = 0.0,
        skill   = -3,
        tier    = "common",
        region  = { Ork = 100 },  -- Ork gear (xenos faction)
    },

    -- Uncommon tier: better Ork gear, moderate stats
    {
        name    = "Shoota",
        glyph   = "}",
        color   = "desaturatedGreen",
        slot    = "weapon",
        weight  = 5.5,
        value   = 35,
        power   = 2.5,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = 5,
        tier    = "uncommon",
        region  = { Ork = 100 },  -- RT-Weapons SP: Ork Shoota
        sizeClass   = "Basic",
        weaponGroup = "SP",
        damageType  = "I",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10",
            penetration = 0,
            range       = 30,
            rateOfFire  = 3,
            clipSize    = 18,
            reloadTime  = 1,
        },
    },
    {
        name    = "Big Choppa",
        glyph   = "/",
        color   = "lightGreen",
        slot    = "weapon",
        weight  = 7.0,
        value   = 40,
        power   = 3.5,
        defense = 0.0,
        maxHp   = 0.0,
        skill   = -8,
        tier    = "uncommon",
        region  = { Ork = 100 },  -- RT-Weapons Exotic/Xenos: Big Choppa (Ork)
        sizeClass   = "Melee",
        weaponGroup = "Primitive",
        damageType  = "I",
        melee = {
            damageDice = "2d5",
            penetration = 2,
            qualities = {"Unbalanced"},
        },
    },
    {
        name    = "Ork Armor",
        glyph   = "[",
        color   = "desaturatedGreen",
        slot    = "body",
        weight  = 10.0,
        value   = 25,
        power   = 0.0,
        defense = 1.5,
        maxHp   = 3.0,
        skill   = -8,
        tier    = "uncommon",
        region  = { Ork = 100 },  -- Ork gear (xenos faction)
        armourLocations = {
            head     = 0,
            body     = 2,
            leftArm  = 2,
            rightArm = 2,
            leftLeg  = 1,
            rightLeg = 1,
        },
    },

    -- Rare tier: powerful Ork weapon, strong stats
    {
        name    = "Power Klaw",
        glyph   = "{",
        color   = "lightYellow",
        slot    = "weapon",
        weight  = 9.0,
        value   = 90,
        power   = 5.0,
        defense = 0.5,
        maxHp   = 0.0,
        skill   = -10,
        tier    = "rare",
        region  = { Ork = 100 },  -- RT-Weapons Exotic/Xenos: Power Klaw (Ork)
        sizeClass   = "Melee",
        weaponGroup = "Exotic",
        damageType  = "I",
        melee = {
            damageDice = "2d10",
            penetration = 7,
            qualities = {"Power Field", "Unwieldy"},
        },
    },

    -- ===== New Bestiary Equipment (bestiary-npcs feature) =====
    -- Weapons/armour referenced by the new Enemy_Entries. Reference profiles
    -- cited from Reference/RT-Bestiary.md; parsing per design.md §5.

    -- Lasgun — Hired Gun profile: 1d10+3 E; Pen 0; Basic 30m; S/3/-; Clip 60;
    -- Reload Full; Reliable. (design.md §5 example)
    {
        name    = "Lasgun",
        glyph   = "}",
        color   = "lightRed",
        slot    = "weapon",
        weight  = 4.0,
        tier    = "common",
        -- NOTE: region intentionally omitted to exercise the ImperialHuman
        -- default at load (Lasgun is ImperialHuman per RT-Weapons Las).
        sizeClass   = "Basic",
        weaponGroup = "Las",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+3",
            penetration = 0,
            range       = 30,
            rateOfFire  = 3,
            clipSize    = 60,
            reloadTime  = 1,
        },
    },

    -- Eldar Chainsword — Eldar Guardian profile (RT-Bestiary IV.4 Troop):
    -- 1d10+5 R; Pen 2; Melee; Balanced, Razor Sharp, Tearing.
    {
        name    = "Eldar Chainsword",
        glyph   = "/",
        color   = "lightBlue",
        slot    = "weapon",
        weight  = 3.0,
        tier    = "uncommon",
        region  = { Eldar = 100 },  -- Eldar Guardian weapon (xenos faction)
        sizeClass   = "Melee",
        weaponGroup = "Primitive",
        damageType  = "R",
        melee = {
            damageDice = "1d10+5",
            penetration = 2,
            qualities = {"Balanced", "Razor Sharp", "Tearing"},
        },
    },

    -- Light Flak Coat — Hired Gun profile: Arms 2, Body 2, Legs 2.
    -- (design.md §5 example; head uncited -> 0)
    {
        name    = "Light Flak Coat",
        glyph   = "[",
        color   = "lighterOrange",
        slot    = "body",
        weight  = 6.0,
        tier    = "common",
        region  = { ImperialHuman = 100 },  -- RT-Equipment Flak Armour: ImperialHuman
        armourLocations = {
            head     = 0,
            body     = 2,
            leftArm  = 2,
            rightArm = 2,
            leftLeg  = 2,
            rightLeg = 2,
        },
    },
}
