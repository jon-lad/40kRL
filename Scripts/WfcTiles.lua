-- WfcTiles.lua
-- Defines the default hive city WFC tileset loaded at game initialization.
-- Each entry is validated by the C++ loader; invalid entries are skipped with a warning.
-- Required fields: id, glyph, color, walkable, transparent, adjacency (north/south/east/west)
-- Optional fields: weight (float, default 1.0)

-- All tile IDs that corridor can connect to (all walkable + bulkhead-wall for symmetry)
local all_tiles = {
    "corridor", "hab-unit", "wide-corridor", "shaft",
    "market", "manufactorum", "sump-pool", "chapel", "ventilation-duct",
    "bulkhead-wall"
}

wfc_tiles = {
    -- ─── Corridor ───────────────────────────────────────────────────────
    -- Universal connector: permits adjacency with ALL tile types (walkable + wall).
    {
        id          = "corridor",
        glyph       = string.byte("."),
        color       = "lightGrey",
        walkable    = true,
        transparent = true,
        weight      = 3.0,
        adjacency   = {
            north = all_tiles,
            south = all_tiles,
            east  = all_tiles,
            west  = all_tiles,
        },
    },

    -- ─── Hab-Unit ───────────────────────────────────────────────────────
    -- Residential blocks. Connect to corridors or other hab-units.
    {
        id          = "hab-unit",
        glyph       = string.byte("."),
        color       = "brown",
        walkable    = true,
        transparent = true,
        weight      = 3.0,
        adjacency   = {
            north = {"corridor", "hab-unit"},
            south = {"corridor", "hab-unit"},
            east  = {"corridor", "hab-unit"},
            west  = {"corridor", "hab-unit"},
        },
    },

    -- ─── Wide Corridor ──────────────────────────────────────────────────
    -- Major arterial passages linking large functional areas.
    {
        id          = "wide-corridor",
        glyph       = string.byte("#"),
        color       = "lightGrey",
        walkable    = true,
        transparent = true,
        weight      = 1.0,
        adjacency   = {
            north = {"corridor", "wide-corridor", "manufactorum", "chapel", "market", "shaft", "bulkhead-wall"},
            south = {"corridor", "wide-corridor", "manufactorum", "chapel", "market", "shaft", "bulkhead-wall"},
            east  = {"corridor", "wide-corridor", "manufactorum", "chapel", "market", "shaft", "bulkhead-wall"},
            west  = {"corridor", "wide-corridor", "manufactorum", "chapel", "market", "shaft", "bulkhead-wall"},
        },
    },

    -- ─── Shaft ──────────────────────────────────────────────────────────
    -- Vertical access between hive layers.
    {
        id          = "shaft",
        glyph       = string.byte(">"),
        color       = "darkGrey",
        walkable    = true,
        transparent = true,
        weight      = 1.0,
        adjacency   = {
            north = {"corridor", "wide-corridor"},
            south = {"corridor", "wide-corridor"},
            east  = {"corridor", "wide-corridor"},
            west  = {"corridor", "wide-corridor"},
        },
    },

    -- ─── Market ─────────────────────────────────────────────────────────
    -- Trading bazaars and reclamation stalls.
    {
        id          = "market",
        glyph       = string.byte("$"),
        color       = "lightYellow",
        walkable    = true,
        transparent = true,
        weight      = 1.0,
        adjacency   = {
            north = {"corridor", "wide-corridor"},
            south = {"corridor", "wide-corridor"},
            east  = {"corridor", "wide-corridor"},
            west  = {"corridor", "wide-corridor"},
        },
    },

    -- ─── Manufactorum ───────────────────────────────────────────────────
    -- Industrial workshops and forge areas. Never directly adjacent to hab-units.
    {
        id          = "manufactorum",
        glyph       = string.byte("="),
        color       = "orange",
        walkable    = true,
        transparent = true,
        weight      = 1.0,
        adjacency   = {
            north = {"corridor", "wide-corridor"},
            south = {"corridor", "wide-corridor"},
            east  = {"corridor", "wide-corridor"},
            west  = {"corridor", "wide-corridor"},
        },
    },

    -- ─── Sump Pool ──────────────────────────────────────────────────────
    -- Toxic waste reservoirs deep in the underhive.
    {
        id          = "sump-pool",
        glyph       = string.byte("~"),
        color       = "lightGreen",
        walkable    = true,
        transparent = true,
        weight      = 0.5,
        adjacency   = {
            north = {"corridor", "ventilation-duct"},
            south = {"corridor", "ventilation-duct"},
            east  = {"corridor", "ventilation-duct"},
            west  = {"corridor", "ventilation-duct"},
        },
    },

    -- ─── Bulkhead Wall ──────────────────────────────────────────────────
    -- Structural barriers between hive sections. NOT walkable, NOT transparent.
    {
        id          = "bulkhead-wall",
        glyph       = string.byte("#"),
        color       = "darkGrey",
        walkable    = false,
        transparent = false,
        weight      = 1.0,
        adjacency   = {
            north = {"corridor", "wide-corridor", "bulkhead-wall"},
            south = {"corridor", "wide-corridor", "bulkhead-wall"},
            east  = {"corridor", "wide-corridor", "bulkhead-wall"},
            west  = {"corridor", "wide-corridor", "bulkhead-wall"},
        },
    },

    -- ─── Chapel ─────────────────────────────────────────────────────────
    -- Devotional shrines and prayer halls.
    {
        id          = "chapel",
        glyph       = string.byte("+"),
        color       = "gold",
        walkable    = true,
        transparent = true,
        weight      = 0.5,
        adjacency   = {
            north = {"corridor", "wide-corridor"},
            south = {"corridor", "wide-corridor"},
            east  = {"corridor", "wide-corridor"},
            west  = {"corridor", "wide-corridor"},
        },
    },

    -- ─── Ventilation Duct ───────────────────────────────────────────────
    -- Narrow maintenance passages and air circulation shafts.
    {
        id          = "ventilation-duct",
        glyph       = string.byte("."),
        color       = "cyan",
        walkable    = true,
        transparent = true,
        weight      = 1.0,
        adjacency   = {
            north = {"corridor", "sump-pool"},
            south = {"corridor", "sump-pool"},
            east  = {"corridor", "sump-pool"},
            west  = {"corridor", "sump-pool"},
        },
    },
}
