#include "ActionRegistry.h"

#include <cassert>

static constexpr std::array<ActionMeta, static_cast<int>(ActionId::COUNT)> ACTIONS = {{
    { ActionId::MOVE,                   1, ActionType::HALF,     "Move" },
    { ActionId::STANDARD_ATTACK_MELEE,  1, ActionType::HALF,     "Standard Attack (Melee)" },
    { ActionId::STANDARD_ATTACK_RANGED, 1, ActionType::HALF,     "Standard Attack (Ranged)" },
    { ActionId::AIM,                    1, ActionType::HALF,     "Aim" },
    { ActionId::RELOAD,                 1, ActionType::HALF,     "Reload" },
    { ActionId::OPEN_DOOR,              1, ActionType::HALF,     "Open Door" },
    { ActionId::PICK_UP_ITEM,           1, ActionType::HALF,     "Pick Up Item" },
    { ActionId::USE_ITEM,               1, ActionType::HALF,     "Use Item" },
    { ActionId::CHARGE,                 2, ActionType::FULL,     "Charge" },
    { ActionId::ALL_OUT_ATTACK,         2, ActionType::FULL,     "All-Out Attack" },
    { ActionId::RUN,                    2, ActionType::FULL,     "Run" },
    { ActionId::END_TURN,               0, ActionType::FREE,     "End Turn" },
    { ActionId::DODGE,                  0, ActionType::REACTION, "Dodge" },
    { ActionId::PARRY,                  0, ActionType::REACTION, "Parry" },
}};

const ActionMeta& ActionRegistry::get(ActionId id) {
    int idx = static_cast<int>(id);
    assert(idx >= 0 && idx < static_cast<int>(ActionId::COUNT));
    return ACTIONS[idx];
}

bool ActionRegistry::canAfford(ActionId id, int currentAP) {
    int idx = static_cast<int>(id);
    assert(idx >= 0 && idx < static_cast<int>(ActionId::COUNT));
    return currentAP >= ACTIONS[idx].apCost;
}

bool ActionRegistry::validateRegistry() {
    for (int i = 0; i < static_cast<int>(ActionId::COUNT); ++i) {
        const auto& meta = ACTIONS[i];

        // Verify array ordering matches enum
        if (static_cast<int>(meta.id) != i) return false;

        // Verify cost matches declared type
        switch (meta.type) {
            case ActionType::HALF:
                if (meta.apCost != 1) return false;
                break;
            case ActionType::FULL:
                if (meta.apCost != 2) return false;
                break;
            case ActionType::FREE:
                if (meta.apCost != 0) return false;
                break;
            case ActionType::REACTION:
                if (meta.apCost != 0) return false;
                break;
        }
    }
    return true;
}
