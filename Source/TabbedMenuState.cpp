#include "../Headers/TabbedMenuState.hpp"

void TabbedMenuState::cycleTab() {
    int next = (static_cast<int>(activeTab) + 1) % static_cast<int>(Tab::COUNT);
    activeTab = static_cast<Tab>(next);
}

Paginator& TabbedMenuState::activePaginator() {
    return paginators[static_cast<int>(activeTab)];
}
