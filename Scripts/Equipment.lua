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

    -- ===== Expanded Imperial/Universal Armoury (equipment-catalog-expansion) =====
    -- Curated spread of player-appropriate weapons and armour ported from
    -- Reference/RT-Weapons.md and Reference/RT-Equipment.md. ImperialHuman + Universal
    -- only (xenos items already exist elsewhere). Tier derived from Availability;
    -- rateOfFire uses the semi-auto burst (or full-auto burst / 1 for single-shot),
    -- consistent with the existing Autogun (S/3/10 -> 3). RoF footnotes in comments.

    -- ---- Las Weapons ----

    -- Long-Las | Heavy | 150m | S/-/- | 1d10+3 E | Pen 1 | Clip 40 | Reload Full |
    -- Accurate, Deadly, Felling(2), Reliable, Variable | 5kg | Scarce -> uncommon
    {
        name    = "Long-Las",
        glyph   = "}",
        color   = "lightRed",
        slot    = "weapon",
        weight  = 5.0,
        value   = 60,
        tier    = "uncommon",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Heavy",
        weaponGroup = "Las",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+3",
            penetration = 1,
            range       = 150,
            rateOfFire  = 1,   -- S/-/- single-shot
            clipSize    = 40,
            reloadTime  = 2,
        },
    },
    -- Hot-Shot Laspistol | Pistol | 20m | S/2/- | 1d10+4 E | Pen 7 | Clip 40 |
    -- Reload 2 Full | Variable | 4kg | Rare -> rare
    {
        name    = "Hot-Shot Laspistol",
        glyph   = ")",
        color   = "lightRed",
        slot    = "weapon",
        weight  = 4.0,
        value   = 85,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Pistol",
        weaponGroup = "Las",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+4",
            penetration = 7,
            range       = 20,
            rateOfFire  = 2,   -- S/2/- semi-auto burst
            clipSize    = 40,
            reloadTime  = 2,
        },
    },

    -- ---- Solid Projectile (SP) Weapons ----

    -- Stub Automatic | Pistol | 30m | S/2/- | 1d10+3 I | Pen 0 | Clip 9 | Reload Full |
    -- Reliable | 2kg | Common -> common
    {
        name    = "Stub Automatic",
        glyph   = ")",
        color   = "lightGrey",
        slot    = "weapon",
        weight  = 2.0,
        value   = 15,
        tier    = "common",
        region  = { Universal = 100 },
        sizeClass   = "Pistol",
        weaponGroup = "SP",
        damageType  = "I",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+3",
            penetration = 0,
            range       = 30,
            rateOfFire  = 2,   -- S/2/-
            clipSize    = 9,
            reloadTime  = 1,
        },
    },
    -- Stub Revolver | Pistol | 30m | S/-/- | 1d10+3 I | Pen 0 | Clip 6 | Reload 2 Full |
    -- Reliable | 2kg | Plentiful -> common
    {
        name    = "Stub Revolver",
        glyph   = ")",
        color   = "lightGrey",
        slot    = "weapon",
        weight  = 2.0,
        value   = 12,
        tier    = "common",
        region  = { Universal = 100 },
        sizeClass   = "Pistol",
        weaponGroup = "SP",
        damageType  = "I",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+3",
            penetration = 0,
            range       = 30,
            rateOfFire  = 1,   -- S/-/- single-shot
            clipSize    = 6,
            reloadTime  = 2,
        },
    },
    -- Autopistol | Pistol | 30m | S/3/6 | 1d10+2 I | Pen 0 | Clip 18 | Reload Full | 2kg |
    -- Average -> uncommon
    {
        name    = "Autopistol",
        glyph   = ")",
        color   = "lightGrey",
        slot    = "weapon",
        weight  = 2.0,
        value   = 20,
        tier    = "uncommon",
        region  = { Universal = 100 },
        sizeClass   = "Pistol",
        weaponGroup = "SP",
        damageType  = "I",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+2",
            penetration = 0,
            range       = 30,
            rateOfFire  = 3,   -- S/3/6 semi-auto burst
            clipSize    = 18,
            reloadTime  = 1,
        },
    },
    -- Combat Shotgun | Basic | 30m | S/3/- | 1d10+4 I | Pen 0 | Clip 18 | Reload Full |
    -- Reliable, Scatter | 5kg | Scarce -> uncommon
    {
        name    = "Combat Shotgun",
        glyph   = "}",
        color   = "lightGrey",
        slot    = "weapon",
        weight  = 5.0,
        value   = 45,
        tier    = "uncommon",
        region  = { Universal = 100 },
        sizeClass   = "Basic",
        weaponGroup = "SP",
        damageType  = "I",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+4",
            penetration = 0,
            range       = 30,
            rateOfFire  = 3,   -- S/3/-
            clipSize    = 18,
            reloadTime  = 1,
        },
    },
    -- Heavy Stubber | Heavy | 100m | -/-/8 | 1d10+4 I | Pen 3 | Clip 80 | Reload 2 Full |
    -- Reliable | 30kg | Rare -> rare
    {
        name    = "Heavy Stubber",
        glyph   = "]",
        color   = "lightGrey",
        slot    = "weapon",
        weight  = 30.0,
        value   = 120,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Heavy",
        weaponGroup = "SP",
        damageType  = "I",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+4",
            penetration = 3,
            range       = 100,
            rateOfFire  = 8,   -- -/-/8 full-auto only
            clipSize    = 80,
            reloadTime  = 2,
        },
    },

    -- ---- Bolt Weapons ----

    -- Bolt Pistol | Pistol | 30m | S/2/- | 1d10+5 X | Pen 4 | Clip 8 | Reload Full |
    -- Tearing | 4kg | Very Rare -> rare
    {
        name    = "Bolt Pistol",
        glyph   = ")",
        color   = "lightBlue",
        slot    = "weapon",
        weight  = 4.0,
        value   = 110,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Pistol",
        weaponGroup = "Bolt",
        damageType  = "X",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+5",
            penetration = 4,
            range       = 30,
            rateOfFire  = 2,   -- S/2/-
            clipSize    = 8,
            reloadTime  = 1,
        },
    },
    -- Boltgun | Basic | 100m | S/2/3 | 1d10+5 X | Pen 4 | Clip 24 | Reload Full |
    -- Tearing | 7kg | Very Rare -> rare
    {
        name    = "Boltgun",
        glyph   = "}",
        color   = "lightBlue",
        slot    = "weapon",
        weight  = 7.0,
        value   = 150,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Basic",
        weaponGroup = "Bolt",
        damageType  = "X",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+5",
            penetration = 4,
            range       = 100,
            rateOfFire  = 2,   -- S/2/3 semi-auto burst
            clipSize    = 24,
            reloadTime  = 1,
        },
    },

    -- ---- Plasma Weapons ----

    -- Plasma Pistol | Pistol | 30m | S/2/- | 1d10+6 E | Pen 8 | Clip 10 | Reload 3 Full |
    -- Maximal, Overheat | 4kg | Very Rare -> rare
    {
        name    = "Plasma Pistol",
        glyph   = ")",
        color   = "lightBlue",
        slot    = "weapon",
        weight  = 4.0,
        value   = 200,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Pistol",
        weaponGroup = "Plasma",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+6",
            penetration = 8,
            range       = 30,
            rateOfFire  = 2,   -- S/2/-
            clipSize    = 10,
            reloadTime  = 3,
        },
    },
    -- Plasma Gun | Basic | 90m | S/2/- | 1d10+7 E | Pen 8 | Clip 40 | Reload 5 Full |
    -- Maximal, Overheat | 18kg | Very Rare -> rare
    {
        name    = "Plasma Gun",
        glyph   = "}",
        color   = "lightBlue",
        slot    = "weapon",
        weight  = 18.0,
        value   = 260,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Basic",
        weaponGroup = "Plasma",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+7",
            penetration = 8,
            range       = 90,
            rateOfFire  = 2,   -- S/2/-
            clipSize    = 40,
            reloadTime  = 5,
        },
    },

    -- ---- Melta Weapons ----

    -- Inferno Pistol | Pistol | 10m | S/-/- | 2d10+10 E | Pen 12 | Clip 3 | Reload Full |
    -- Melta | 3kg | Near Unique -> rare
    {
        name    = "Inferno Pistol",
        glyph   = ")",
        color   = "lightRed",
        slot    = "weapon",
        weight  = 3.0,
        value   = 320,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Pistol",
        weaponGroup = "Melta",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "2d10+10",
            penetration = 12,
            range       = 10,
            rateOfFire  = 1,   -- S/-/-
            clipSize    = 3,
            reloadTime  = 1,
        },
    },
    -- Meltagun | Basic | 20m | S/-/- | 2d10+10 E | Pen 12 | Clip 5 | Reload Full |
    -- Melta | 15kg | Very Rare -> rare
    {
        name    = "Meltagun",
        glyph   = "}",
        color   = "lightRed",
        slot    = "weapon",
        weight  = 15.0,
        value   = 300,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Basic",
        weaponGroup = "Melta",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "2d10+10",
            penetration = 12,
            range       = 20,
            rateOfFire  = 1,   -- S/-/-
            clipSize    = 5,
            reloadTime  = 1,
        },
    },

    -- ---- Flame Weapons ----

    -- Hand Flamer | Pistol | 10m | S/-/- | 1d10+4 E | Pen 2 | Clip 2 | Reload 2 Full |
    -- Flame, Spray | 4kg | Rare -> rare
    {
        name    = "Hand Flamer",
        glyph   = ")",
        color   = "lighterOrange",
        slot    = "weapon",
        weight  = 4.0,
        value   = 90,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Pistol",
        weaponGroup = "Flame",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+4",
            penetration = 2,
            range       = 10,
            rateOfFire  = 1,   -- S/-/-
            clipSize    = 2,
            reloadTime  = 2,
        },
    },
    -- Flamer | Basic | 20m | S/-/- | 1d10+4 E | Pen 2 | Clip 6 | Reload 2 Full |
    -- Flame, Spray | 6kg | Scarce -> uncommon
    {
        name    = "Flamer",
        glyph   = "}",
        color   = "lighterOrange",
        slot    = "weapon",
        weight  = 6.0,
        value   = 70,
        tier    = "uncommon",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Basic",
        weaponGroup = "Flame",
        damageType  = "E",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {},
        },
        ranged = {
            damageDice  = "1d10+4",
            penetration = 2,
            range       = 20,
            rateOfFire  = 1,   -- S/-/-
            clipSize    = 6,
            reloadTime  = 2,
        },
    },

    -- ---- Melee: Chain / Power / Primitive ----

    -- Chain Axe | Melee | 1d10+4(+SB) R | Pen 2 | Felling(1), Tearing, Two-Handed,
    -- Unbalanced | 13kg | Scarce -> uncommon
    {
        name    = "Chain Axe",
        glyph   = "/",
        color   = "lightBlue",
        slot    = "weapon",
        weight  = 13.0,
        value   = 65,
        tier    = "uncommon",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Melee",
        weaponGroup = "Chain",
        damageType  = "R",
        melee = {
            damageDice = "1d10+4",
            penetration = 2,
            qualities = {"Felling(1)", "Tearing", "Two-Handed", "Unbalanced"},
        },
    },
    -- Power Axe | Melee | 1d10+7(+SB) E | Pen 6 | Felling(1), Power Field, Unbalanced,
    -- Two-Handed | 6kg | Very Rare -> rare
    {
        name    = "Power Axe",
        glyph   = "/",
        color   = "lightGrey",
        slot    = "weapon",
        weight  = 6.0,
        value   = 180,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Melee",
        weaponGroup = "Power",
        damageType  = "E",
        melee = {
            damageDice = "1d10+7",
            penetration = 6,
            qualities = {"Felling(1)", "Power Field", "Unbalanced", "Two-Handed"},
        },
    },
    -- Eviscerator | Melee | 2d10+2(+SB) R | Pen 8 | Tearing, Razor Sharp, Two-Handed,
    -- Unwieldy, Vengeful(9) | 15kg | Very Rare -> rare
    {
        name    = "Eviscerator",
        glyph   = "\\",
        color   = "lightBlue",
        slot    = "weapon",
        weight  = 15.0,
        value   = 190,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        sizeClass   = "Melee",
        weaponGroup = "Chain",
        damageType  = "R",
        melee = {
            damageDice = "2d10+2",
            penetration = 8,
            qualities = {"Tearing", "Razor Sharp", "Two-Handed", "Unwieldy", "Vengeful(9)"},
        },
    },
    -- Knife | Melee | 1d5(+SB) R | Pen 0 | Piercing | 1kg | Plentiful -> common
    {
        name    = "Knife",
        glyph   = "-",
        color   = "white",
        slot    = "weapon",
        weight  = 1.0,
        value   = 8,
        tier    = "common",
        region  = { Universal = 100 },
        sizeClass   = "Melee",
        weaponGroup = "Primitive",
        damageType  = "R",
        melee = {
            damageDice = "1d5",
            penetration = 0,
            qualities = {"Piercing"},
        },
    },
    -- Sword | Melee | 1d10(+SB) R | Pen 0 | Balanced, Primitive(7) | 3kg | Common -> common
    {
        name    = "Sword",
        glyph   = "|",
        color   = "white",
        slot    = "weapon",
        weight  = 3.0,
        value   = 18,
        tier    = "common",
        region  = { Universal = 100 },
        sizeClass   = "Melee",
        weaponGroup = "Primitive",
        damageType  = "R",
        melee = {
            damageDice = "1d10",
            penetration = 0,
            qualities = {"Balanced", "Primitive(7)"},
        },
    },

    -- ---- Armour: Flak ----

    -- Flak Cloak | Arms, Body, Legs | AP 3 | 8kg | Scarce -> uncommon
    {
        name    = "Flak Cloak",
        glyph   = "[",
        color   = "lighterOrange",
        slot    = "body",
        weight  = 8.0,
        value   = 35,
        defense = 3.0,
        skill   = -3,
        tier    = "uncommon",
        region  = { ImperialHuman = 100 },
        armourLocations = {
            head     = 0,
            body     = 3,
            leftArm  = 3,
            rightArm = 3,
            leftLeg  = 3,
            rightLeg = 3,
        },
    },
    -- Guard Flak Armour | All | AP 4 | 11kg | Scarce -> uncommon
    {
        name    = "Guard Flak Armour",
        glyph   = "[",
        color   = "desaturatedGreen",
        slot    = "body",
        weight  = 11.0,
        value   = 55,
        defense = 4.0,
        skill   = -5,
        tier    = "uncommon",
        region  = { ImperialHuman = 100 },
        armourLocations = {
            head     = 4,
            body     = 4,
            leftArm  = 4,
            rightArm = 4,
            leftLeg  = 4,
            rightLeg = 4,
        },
    },

    -- ---- Armour: Mesh ----

    -- Mesh Vest | Body | AP 4 | 2kg | Rare -> rare
    {
        name    = "Mesh Vest",
        glyph   = "[",
        color   = "lightGrey",
        slot    = "body",
        weight  = 2.0,
        value   = 90,
        defense = 4.0,
        skill   = -1,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        armourLocations = {
            head     = 0,
            body     = 4,
            leftArm  = 0,
            rightArm = 0,
            leftLeg  = 0,
            rightLeg = 0,
        },
    },

    -- ---- Armour: Carapace ----

    -- Carapace Chestplate | Body | AP 6 | 7kg | Rare -> rare
    {
        name    = "Carapace Chestplate",
        glyph   = "[",
        color   = "lightGrey",
        slot    = "body",
        weight  = 7.0,
        value   = 130,
        defense = 6.0,
        skill   = -4,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        armourLocations = {
            head     = 0,
            body     = 6,
            leftArm  = 0,
            rightArm = 0,
            leftLeg  = 0,
            rightLeg = 0,
        },
    },
    -- Storm Trooper Carapace | All | AP 6 | 15kg | Very Rare -> rare
    {
        name    = "Storm Trooper Carapace",
        glyph   = "[",
        color   = "lightBlue",
        slot    = "body",
        weight  = 15.0,
        value   = 220,
        defense = 6.0,
        skill   = -6,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        armourLocations = {
            head     = 6,
            body     = 6,
            leftArm  = 6,
            rightArm = 6,
            leftLeg  = 6,
            rightLeg = 6,
        },
    },

    -- ---- Armour: Power ----

    -- Light Power Armour | All | AP 7 | 40kg | Very Rare -> rare
    {
        name    = "Light Power Armour",
        glyph   = "[",
        color   = "lightBlue",
        slot    = "body",
        weight  = 40.0,
        value   = 400,
        defense = 7.0,
        maxHp   = 5.0,
        skill   = -8,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        armourLocations = {
            head     = 7,
            body     = 7,
            leftArm  = 7,
            rightArm = 7,
            leftLeg  = 7,
            rightLeg = 7,
        },
    },
    -- Power Armour | All | AP 8 | 65kg | Very Rare -> rare
    {
        name    = "Power Armour",
        glyph   = "[",
        color   = "lightBlue",
        slot    = "body",
        weight  = 65.0,
        value   = 550,
        defense = 8.0,
        maxHp   = 8.0,
        skill   = -10,
        tier    = "rare",
        region  = { ImperialHuman = 100 },
        armourLocations = {
            head     = 8,
            body     = 8,
            leftArm  = 8,
            rightArm = 8,
            leftLeg  = 8,
            rightLeg = 8,
        },
    },

    -- ---- Armour: Primitive ----

    -- Robes | All | AP 0 | 4kg | Plentiful -> common (civilian/flavour body slot)
    {
        name    = "Robes",
        glyph   = "[",
        color   = "white",
        slot    = "body",
        weight  = 4.0,
        value   = 5,
        defense = 0.0,
        tier    = "common",
        region  = { Universal = 100 },
        armourLocations = {
            head     = 0,
            body     = 0,
            leftArm  = 0,
            rightArm = 0,
            leftLeg  = 0,
            rightLeg = 0,
        },
    },
    -- Heavy Leathers/Furs | Arms, Body, Legs | AP 2 | 7kg | Common -> common
    {
        name    = "Heavy Leathers",
        glyph   = "[",
        color   = "lighterOrange",
        slot    = "body",
        weight  = 7.0,
        value   = 12,
        defense = 2.0,
        skill   = -2,
        tier    = "common",
        region  = { Universal = 100 },
        armourLocations = {
            head     = 0,
            body     = 2,
            leftArm  = 2,
            rightArm = 2,
            leftLeg  = 2,
            rightLeg = 2,
        },
    },
    -- Flak Helmet | Head | AP 2 | 2kg | Average -> uncommon
    {
        name    = "Flak Helmet",
        glyph   = "^",
        color   = "lighterOrange",
        slot    = "head",
        weight  = 2.0,
        value   = 18,
        defense = 2.0,
        tier    = "uncommon",
        region  = { ImperialHuman = 100 },
        armourLocations = {
            head     = 2,
            body     = 0,
            leftArm  = 0,
            rightArm = 0,
            leftLeg  = 0,
            rightLeg = 0,
        },
    },
}
