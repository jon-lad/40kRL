-- Decorations.lua
-- Defines decoration templates loaded at game initialization.
-- Each entry is validated by the C++ loader; invalid entries are skipped with a warning.
-- Required fields: glyph, name, color, description, blocks
-- Optional fields: cover_value (integer 0-100, default 0)

decorations = {
    {
        glyph       = "X",
        name        = "ammo crate",
        color       = "lightYellow",
        description = "A battered ammunition crate stamped with Aquila markings.",
        blocks      = false,
        cover_value = 25,
    },
    {
        glyph       = "0",
        name        = "fuel barrel",
        color       = "orange",
        description = "A corroded promethium barrel leaking fumes.",
        blocks      = false,
        cover_value = 20,
    },
    {
        glyph       = "&",
        name        = "cogitator console",
        color       = "lightGreen",
        description = "A flickering cogitator terminal trailing severed data-cables.",
        blocks      = true,
        cover_value = 40,
    },
    {
        glyph       = "O",
        name        = "rockcrete pillar",
        color       = "lightGrey",
        description = "A load-bearing pillar of reinforced rockcrete, scarred by bolt impacts.",
        blocks      = true,
        cover_value = 75,
    },
    {
        glyph       = ";",
        name        = "rubble pile",
        color       = "darkGrey",
        description = "A heap of broken ferrocrete and twisted rebar.",
        blocks      = false,
        cover_value = 15,
    },
    {
        glyph       = "T",
        name        = "comm-relay",
        color       = "lightBlue",
        description = "A vox-relay tower crackling with static interference.",
        blocks      = true,
        cover_value = 30,
    },
    {
        glyph       = "A",
        name        = "aquila shrine",
        color       = "lighterOrange",
        description = "A small devotional shrine bearing the Imperial Aquila.",
        blocks      = true,
        cover_value = 50,
    },
    {
        glyph       = "=",
        name        = "barricade",
        color       = "lightGrey",
        description = "A hastily-erected barricade of sandbags and scrap metal.",
        blocks      = false,
        cover_value = 40,
    },
}
