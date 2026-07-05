#pragma once

#include "Pickable.h"

// Stores all state needed while the Engine is in TARGETING mode.
// Populated when the player uses a targeted item; consumed when
// the player confirms or cancels.
struct TargetingContext {
	Actor* item;                          // the item being used (still in inventory)
	Actor* owner;                         // the actor using the item (always the player)
	float maxRange;                       // 0 = unlimited
	TargetSelector::SelectorType type;    // SELECTED_MONSTER or SELECTED_RANGE
	Effect* effect;                       // the effect to apply on confirmation
	float aoeRange;                       // for SELECTED_RANGE: radius of the AoE
};
