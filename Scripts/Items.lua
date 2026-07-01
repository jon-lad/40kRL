-- Items.lua
-- Defines item templates used by Map::addItem.
-- C++ calls: spawnItem(roll, x, y)
--   roll : int  (0-99 random roll)
--   x, y : int  (world position)
-- The function calls back into C++ via injected factory functions:
--   spawnHealthPotion(x, y, healAmount)
--   spawnDamageScroll(x, y, damage, name, message, color, selectorType, range)
--   spawnConfusionScroll(x, y, turns, name, message, color, range)

-- Item spawn table. Entries are checked in order; first match wins.
-- chance is the upper bound of the roll range (cumulative).
local items = {
    {
        chance = 70,
        spawn  = function(x, y)
            spawnHealthPotion(x, y, 4.0)
        end,
    },
    {
        chance = 80,
        spawn  = function(x, y)
            spawnDamageScroll(
                x, y,
                -20.0,
                "scroll of lightning bolt",
                "A lightning bolt strikes the #\nwith a crack of loud thunder!\nThe damage is # hit points.",
                "lightBlue",
                "CLOSEST_MONSTER",
                5.0
            )
        end,
    },
    {
        chance = 90,
        spawn  = function(x, y)
            spawnDamageScroll(
                x, y,
                -12.0,
                "scroll of fireball",
                "The # gets burned for # hit points.",
                "orange",
                "SELECTED_RANGE",
                3.0
            )
        end,
    },
    {
        chance = 100,
        spawn  = function(x, y)
            spawnConfusionScroll(
                x, y,
                10,
                "scroll of confusion",
                "The eyes of the # glaze over as he starts to stumble around!",
                "lightGreen",
                5.0
            )
        end,
    },
}

function spawnItem(roll, x, y)
    for _, item in ipairs(items) do
        if roll < item.chance then
            item.spawn(x, y)
            return
        end
    end
end
