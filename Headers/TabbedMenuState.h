#pragma once
#include "Paginator.h"
#include <array>

// Forward-declare InventoryState::Action to avoid pulling in Engine.h
// The actual enum is defined in Engine.h as InventoryState::Action { USE, DROP }.
// We replicate the enum here to keep the header lightweight and avoid circular deps.

struct TabbedMenuState {
    enum class Tab { INVENTORY = 0, EQUIPMENT, SKILLS, COUNT };

    Tab activeTab = Tab::INVENTORY;

    // Per-tab scroll state preserved across tab switches
    std::array<Paginator, static_cast<int>(Tab::COUNT)> paginators;

    // Inventory tab action (use/drop) — mirrors InventoryState::Action
    enum class Action { USE, DROP };
    Action pendingAction = Action::USE;

    void cycleTab();              // activeTab = (activeTab + 1) % COUNT
    Paginator& activePaginator(); // paginators[static_cast<int>(activeTab)]
};
