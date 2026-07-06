-- Config.lua
-- Global game configuration loaded once during Engine::init.
-- Values here override the C++ defaults.

config = {
    -- Map dimensions
    mapWidth    = 160,
    mapHeight   = 86,

    -- Viewport
    viewportWidth  = 80,
    viewportHeight = 43,

    -- FOV
    fovRadius = 10,

    -- Level-up costs
    levelUpBase   = 200,
    levelUpFactor = 150,

    -- Outdoor generation (Perlin noise parameters)
    outdoorGroundThreshold = -0.1,
    outdoorWaterThreshold  = -0.5,
    outdoorOctaves         = 4,
    outdoorLacunarity      = 2.0,
    outdoorNoiseScale      = 0.05,

    -- Outdoor actor counts
    outdoorMinMonsters = 6,
    outdoorMaxMonsters = 12,
    outdoorMinItems    = 2,
    outdoorMaxItems    = 5,

    -- Outdoor level transition and camera
    startingDepth          = 20,   -- player starts at this depth, ascends to 0 (surface)
    outdoorScrollMargin    = 20,

    -- Carrying capacity (max total weight the player can carry; 0 = unlimited)
    carryingCapacity = 50.0,

    -- Maximum number of previously-visited levels kept in memory.
    -- Minimum: 2, Maximum: 200, Default: 30.
    maxCachedLevels = 30,
}
