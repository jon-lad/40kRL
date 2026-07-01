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

    -- Player starting stats
    playerHp      = 30.0,
    playerDefense = 2.0,
    playerPower   = 5.0,
    playerInvSize = 26,

    -- FOV
    fovRadius = 10,

    -- Level-up costs
    levelUpBase   = 200,
    levelUpFactor = 150,
}
