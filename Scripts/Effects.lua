-- Effects.lua
-- Called by HealthEffect::applyTo when amount > 0.
-- Globals injected by C++:
--   act    : Actor usertype  (the target being healed)
--   amount : float           (the configured heal amount)
-- Must return the actual number of HP restored.

local healAmount = act.destructible:heal(amount)
return healAmount
