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

    -- Decoration spawning
    maxRoomDecorations     = 3,
    outdoorDecorationCount = 8,

    -- World Map generation parameters
    worldMapNoiseScale       = 0.03,
    worldMapOctaves          = 4,
    worldMapLacunarity       = 2.0,
    worldMapSwampThreshold   = -0.4,
    worldMapForestThreshold  = -0.1,
    worldMapDesertThreshold  = 0.2,
    worldMapCityCount        = 3,
    worldMapCitySeparation   = 15,
    worldMapCityNames        = {"Hive Primus", "Hive Secundus", "Hive Tertius", "Hive Quartus", "Hive Quintus"},

    -- WFC hive city generation parameters
    wfcMaxRestarts         = 10,
    wfcMinWalkablePercent  = 0.20,
    wfcMinStairDistance    = 20,
    wfcMinMonsters         = 8,
    wfcMaxMonsters         = 16,
    wfcMinItems            = 3,
    wfcMaxItems            = 7,

    -- Carrying capacity (max total weight the player can carry; 0 = unlimited)
    carryingCapacity = 50.0,

    -- Maximum number of previously-visited levels kept in memory.
    -- Minimum: 2, Maximum: 200, Default: 30.
    maxCachedLevels = 30,
}
