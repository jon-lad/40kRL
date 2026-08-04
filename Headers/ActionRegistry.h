#pragma once

#include <array>
#include <string_view>

enum class ActionType { HALF, FULL, FREE, REACTION };

enum class ActionId {
    // Half Actions (cost 1)
    MOVE,
    STANDARD_ATTACK_MELEE,
    STANDARD_ATTACK_RANGED,
    AIM,
    RELOAD,
    OPEN_DOOR,
    PICK_UP_ITEM,
    USE_ITEM,

    // Full Actions (cost 2)
    CHARGE,
    ALL_OUT_ATTACK,
    RUN,

    // Free Actions (cost 0)
    END_TURN,

    // Reactions (cost 0 AP, cost 1 Reaction)
    DODGE,
    PARRY,

    COUNT
};

struct ActionMeta {
    ActionId         id;
    int              apCost;
    ActionType       type;
    std::string_view name;  // display name for log messages
};

namespace ActionRegistry {
    // Returns metadata for the given action. Asserts on out-of-bounds.
    const ActionMeta& get(ActionId id);

    // Returns true if the actor can afford this action given current AP.
    bool canAfford(ActionId id, int currentAP);

    // Validates that cost matches type (Half=1, Full=2, Free=0, Reaction=0).
    // Used at startup to assert registry consistency.
    bool validateRegistry();
}
