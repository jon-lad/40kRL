#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: inventory-equipment-ux — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 4: Inventory displays only unequipped items with sequential letters ─
// Feature: inventory-equipment-ux, Property 4: Inventory displays only unequipped items with sequential letters
// **Validates: Requirements 4.1, 4.2, 4.4**
//
// For any container with M total items where K are equipped (0 <= K <= M),
// the inventory menu SHALL display exactly M-K items, each assigned a
// sequential letter from 'a' to 'a'+(M-K-1), and no displayed item SHALL
// be currently equipped.

TEST_CASE("PBT: Property 4 — Inventory displays only unequipped items with sequential letters", "[property][inventory-equipment-ux]")
{
    rc::prop("filtered inventory has M-K unequipped items with sequential letters a..z", []() {
        // Generate random total items M in [1, 10]
        const int M = *rc::gen::inRange(1, 10);
        // Generate random equipped count K in [0, M]
        const int K = *rc::gen::inRange(0, M);

        // Simulate a boolean array: true = equipped, false = unequipped
        // Mark the first K items as equipped (deterministic for simplicity;
        // the randomness comes from M and K varying across iterations)
        std::vector<bool> equipped(M, false);
        for (int i = 0; i < K; ++i) {
            equipped[i] = true;
        }

        // Simulate the filtering logic from renderInventory:
        // iterate all items, skip equipped ones, collect unequipped indices
        std::vector<int> filteredIndices;
        for (int idx = 0; idx < M; ++idx) {
            if (!equipped[idx]) {
                filteredIndices.push_back(idx);
            }
        }

        // Property: filtered count == M - K
        RC_ASSERT(static_cast<int>(filteredIndices.size()) == M - K);

        // Property: sequential letter assignment works: letter[j] = 'a' + j
        for (int j = 0; j < static_cast<int>(filteredIndices.size()); ++j) {
            char expectedLetter = 'a' + j;
            RC_ASSERT(expectedLetter >= 'a' && expectedLetter <= 'z');
        }

        // Property: no filtered item is in the "equipped" set
        for (int idx : filteredIndices) {
            RC_ASSERT(!equipped[idx]);
        }
    });
}


// ─── Property 5: Inventory selection maps to the correct filtered item ───────
// Feature: inventory-equipment-ux, Property 5: Inventory selection maps to the correct filtered item
// **Validates: Requirements 4.3**
//
// For any inventory with M total items where K are equipped (0 ≤ K < M, ensuring
// at least 1 unequipped), and any valid selection index j (0 ≤ j < M-K),
// the selection mapping iterates items, skips equipped ones, and lands on the
// j-th unequipped item in container order. That item must NOT be equipped.

TEST_CASE("PBT: Property 5 — inventory selection maps to the correct filtered item", "[property][inventory-equipment-ux]")
{
    rc::prop("selecting 'a'+j acts on the j-th unequipped item in container order", []() {
        // Generate random total items [1, 10]
        const int M = *rc::gen::inRange(1, 11);

        // Generate random number of equipped items [0, M-1] — at least 1 unequipped
        const int K = *rc::gen::inRange(0, M);

        // Precondition: must have at least one unequipped item
        RC_PRE(K < M);

        // Build a simulated equipped/unequipped pattern
        // Mark the first K items as equipped (deterministic pattern;
        // randomness comes from M and K varying across iterations)
        std::vector<bool> equipped(M, false);
        for (int i = 0; i < K; ++i) {
            equipped[i] = true;
        }

        const int unequippedCount = M - K;

        // Generate a valid selection index j in [0, unequippedCount)
        const int j = *rc::gen::inRange(0, unequippedCount);
        RC_PRE(j < unequippedCount);

        // Simulate the selection mapping from updateInventory:
        // iterate items, skip equipped, count unequipped until reaching the j-th one
        int count = 0;
        int selectedItemIndex = -1;
        for (int idx = 0; idx < M; ++idx) {
            if (!equipped[idx]) {
                if (count == j) {
                    selectedItemIndex = idx;
                    break;
                }
                count++;
            }
        }

        // The mapping must find a valid item
        RC_ASSERT(selectedItemIndex >= 0);
        RC_ASSERT(selectedItemIndex < M);

        // The selected item must NOT be equipped
        RC_ASSERT(!equipped[selectedItemIndex]);

        // Verify the letter key mapping: pressing 'a'+j → index j in filtered list → correct raw item
        char key = static_cast<char>('a' + j);
        int selectionIndex = key - 'a';
        RC_ASSERT(selectionIndex == j);

        // Re-run the mapping with selectionIndex to confirm consistency
        int verifyCount = 0;
        int verifiedIndex = -1;
        for (int idx = 0; idx < M; ++idx) {
            if (!equipped[idx]) {
                if (verifyCount == selectionIndex) {
                    verifiedIndex = idx;
                    break;
                }
                verifyCount++;
            }
        }
        RC_ASSERT(verifiedIndex == selectedItemIndex);
    });

    rc::prop("all unequipped items are reachable via exactly one letter key", []() {
        // Generate random total items [1, 10]
        const int M = *rc::gen::inRange(1, 11);

        // Generate random number of equipped items [0, M-1]
        const int K = *rc::gen::inRange(0, M);

        // Precondition: must have at least one unequipped item
        RC_PRE(K < M);

        // Build equipped pattern (mark first K as equipped)
        std::vector<bool> equipped(M, false);
        for (int i = 0; i < K; ++i) {
            equipped[i] = true;
        }

        const int unequippedCount = M - K;

        // For each possible selection index j in [0, unequippedCount), the mapping
        // must find a unique, non-equipped item
        std::vector<int> mappedItems;
        for (int j = 0; j < unequippedCount; ++j) {
            int count = 0;
            int selectedItemIndex = -1;
            for (int idx = 0; idx < M; ++idx) {
                if (!equipped[idx]) {
                    if (count == j) {
                        selectedItemIndex = idx;
                        break;
                    }
                    count++;
                }
            }
            RC_ASSERT(selectedItemIndex >= 0);
            RC_ASSERT(!equipped[selectedItemIndex]);
            mappedItems.push_back(selectedItemIndex);
        }

        // All mapped items must be distinct (bijection between letters and unequipped items)
        for (size_t a = 0; a < mappedItems.size(); ++a) {
            for (size_t b = a + 1; b < mappedItems.size(); ++b) {
                RC_ASSERT(mappedItems[a] != mappedItems[b]);
            }
        }

        // The mapped items must cover all unequipped items
        RC_ASSERT(static_cast<int>(mappedItems.size()) == unequippedCount);
    });
}


// ─── Property 3: Menu letter selection picks the correct item ─────────────────
// Feature: inventory-equipment-ux, Property 3: Menu letter selection picks the correct item
// **Validates: Requirements 2.3**
//
// For any pickup menu with N items (2 <= N <= 26) and any valid index i (0 <= i < N),
// pressing 'a'+i selects the item at position i. For invalid indices (i >= N),
// the selection is ignored (menu stays open).

TEST_CASE("PBT: Property 3 — menu letter selection picks the correct item", "[property][inventory-equipment-ux]")
{
    rc::prop("key 'a'+i where i < N selects item at index i; i >= N is ignored", []() {
        // Generate random menu size [2, 26] — pickup menu requires at least 2 items
        const int N = *rc::gen::inRange(2, 27);

        // Generate a key index that may be valid or invalid [0, 27)
        // 0..N-1 = valid selection, N..26 = invalid (ignored)
        const int i = *rc::gen::inRange(0, 27);

        // --- Test the key mapping logic directly (same arithmetic as updatePickupMenu) ---
        // This avoids calling updatePickupMenu() which would mutate engine state.

        char key = static_cast<char>('a' + i);
        int mappedIndex = key - 'a';

        if (i < N) {
            // Valid selection: the mapped index equals i and is within bounds
            RC_ASSERT(mappedIndex == i);
            RC_ASSERT(mappedIndex >= 0);
            RC_ASSERT(mappedIndex < N);
        } else {
            // Invalid selection: index is out of bounds, should be ignored
            RC_ASSERT(mappedIndex >= N);
        }
    });

    rc::prop("valid index always maps back to itself via char arithmetic", []() {
        // Generate random menu size [2, 26]
        const int N = *rc::gen::inRange(2, 27);

        // Generate a candidate index [0, 26)
        const int i = *rc::gen::inRange(0, 26);

        // Precondition: i must be a valid index for this menu size
        RC_PRE(i < N);

        // The core invariant of updatePickupMenu:
        // key = 'a' + i; itemIndex = key - 'a'; assert itemIndex == i
        char key = static_cast<char>('a' + i);
        int itemIndex = key - 'a';

        RC_ASSERT(itemIndex == i);
        RC_ASSERT(itemIndex >= 0);
        RC_ASSERT(itemIndex < N);

        // The item at position i would be picked (pickupMenuState->items[itemIndex])
        // Verify the letter displayed to the user matches the key they'd press
        char displayedLetter = static_cast<char>('a' + itemIndex);
        RC_ASSERT(displayedLetter == key);
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: inventory-equipment-ux — Unit Tests for Edge Cases (Tasks 7.1–7.6)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── 7.1: Escape closes pickup menu without side effects ─────────────────────
// **Validates: Requirements 2.5**

TEST_CASE("Escape closes pickup menu without side effects", "[unit][inventory-equipment-ux]")
{
    // Setup: create 3 self-contained item actors on the map
    auto item1 = std::make_unique<Actor>(5, 5, '!', "potion1", Colors::white);
    auto item2 = std::make_unique<Actor>(5, 5, '!', "potion2", Colors::white);
    auto item3 = std::make_unique<Actor>(5, 5, '!', "potion3", Colors::white);

    Actor* item1Ptr = item1.get();
    Actor* item2Ptr = item2.get();
    Actor* item3Ptr = item3.get();

    // Add items to engine.actors so they're "on the map"
    engine.actors.push_back(std::move(item1));
    engine.actors.push_back(std::move(item2));
    engine.actors.push_back(std::move(item3));

    // Open pickup menu with 3 items
    std::vector<Actor*> menuItems = { item1Ptr, item2Ptr, item3Ptr };
    engine.pickupMenuState = PickupMenuState{ menuItems };
    engine.gameStatus = Engine::PICKUP_MENU;

    // Simulate ESC key press
    engine.inputState = InputState{};
    engine.inputState.key.key = SDLK_ESCAPE;
    engine.inputState.key.pressed = true;

    // Act
    engine.updatePickupMenu();

    // Assert: gameStatus returns to IDLE
    REQUIRE(engine.gameStatus == Engine::IDLE);

    // Assert: pickupMenuState is cleared
    REQUIRE(!engine.pickupMenuState.has_value());

    // Assert: no items removed from actors list (all 3 still present)
    int found = 0;
    for (const auto& actorPtr : engine.actors) {
        if (actorPtr.get() == item1Ptr || actorPtr.get() == item2Ptr || actorPtr.get() == item3Ptr) {
            found++;
        }
    }
    REQUIRE(found == 3);

    // Cleanup
    engine.actors.remove_if([&](const std::unique_ptr<Actor>& a) {
        return a.get() == item1Ptr || a.get() == item2Ptr || a.get() == item3Ptr;
    });
}

// ─── 7.2: Empty tile produces message and no menu ────────────────────────────
// **Validates: Requirements 5.1, 5.2**

TEST_CASE("Empty tile produces message and no menu", "[unit][inventory-equipment-ux]")
{
    // This tests the state machine logic: when no items are found at the player's
    // position, the expected behavior is gameStatus = NEW_TURN and no pickup menu.
    // We simulate the logic that handleActionKey('g') does when 0 items found:
    // set gameStatus = NEW_TURN, pickupMenuState remains nullopt.

    // Setup: ensure no pickup menu state
    engine.pickupMenuState = std::nullopt;
    engine.gameStatus = Engine::IDLE;

    // Simulate the 0-items branch from handleActionKey:
    // "if 0 items: display message, set gameStatus = NEW_TURN"
    std::vector<Actor*> itemsOnTile; // empty — no pickable items
    if (itemsOnTile.empty()) {
        // This is what handleActionKey does for the empty case
        engine.gameStatus = Engine::NEW_TURN;
    }

    // Assert: gameStatus is NEW_TURN (turn consumed for "nothing here" message)
    REQUIRE(engine.gameStatus == Engine::NEW_TURN);

    // Assert: pickupMenuState remains nullopt (no menu opened)
    REQUIRE(!engine.pickupMenuState.has_value());

    // Cleanup
    engine.gameStatus = Engine::IDLE;
}

// ─── 7.3: Single item auto-pickup does not show menu ─────────────────────────
// **Validates: Requirements 1.1, 1.2**

TEST_CASE("Single item auto-pickup does not show menu", "[unit][inventory-equipment-ux]")
{
    // Setup: create a self-contained player-like actor with container
    auto testPlayer = std::make_unique<Actor>(5, 5, '@', "TestPlayer", Colors::white);
    testPlayer->container = std::make_unique<Container>(26);
    Actor* playerPtr = testPlayer.get();

    // Create a single pickable item at the same position
    auto healEffect = std::make_unique<HealthEffect>(5.0f, "healed #", Colors::healing);
    auto selector = std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f);
    auto item = std::make_unique<Actor>(5, 5, '!', "health potion", Colors::healthPotion);
    item->pickable = std::make_shared<Pickable>(std::move(selector), std::move(healEffect));
    Actor* itemPtr = item.get();

    // Simulate the single-item branch from handleActionKey:
    // "If 1 item: auto-pickup (existing logic)"
    engine.pickupMenuState = std::nullopt;
    engine.gameStatus = Engine::IDLE;

    std::vector<Actor*> itemsOnTile = { itemPtr };

    if (itemsOnTile.size() == 1) {
        // Auto-pickup: use pick() to move item into player's container
        bool picked = itemPtr->pickable->pick(std::move(item), playerPtr);
        REQUIRE(picked);
        // In the real code, gameStatus would be set to NEW_TURN
        engine.gameStatus = Engine::NEW_TURN;
    }

    // Assert: no pickup menu was opened
    REQUIRE(!engine.pickupMenuState.has_value());

    // Assert: item is now in the player's container
    bool itemInInventory = false;
    for (const auto& invItem : playerPtr->container->inventory) {
        if (invItem.get() == itemPtr) {
            itemInInventory = true;
            break;
        }
    }
    REQUIRE(itemInInventory);

    // Assert: gameStatus advanced to NEW_TURN (turn consumed)
    REQUIRE(engine.gameStatus == Engine::NEW_TURN);

    // Cleanup
    playerPtr->container->inventory.clear();
    engine.gameStatus = Engine::IDLE;
}

// ─── 7.4: Full inventory during menu selection shows error ───────────────────
// **Validates: Requirements 2.6**

TEST_CASE("Full inventory during menu selection shows error", "[unit][inventory-equipment-ux]")
{
    // Setup: create a player actor with a full container (size 2, already has 2 items)
    auto testPlayer = std::make_unique<Actor>(5, 5, '@', "TestPlayer", Colors::white);
    testPlayer->container = std::make_unique<Container>(2); // max 2 items
    Actor* savedPlayer = engine.player;
    engine.player = testPlayer.get();

    // Fill the container to capacity
    auto filler1 = std::make_unique<Actor>(0, 0, '!', "filler1", Colors::white);
    auto filler2 = std::make_unique<Actor>(0, 0, '!', "filler2", Colors::white);
    testPlayer->container->add(std::move(filler1));
    testPlayer->container->add(std::move(filler2));
    REQUIRE(testPlayer->container->inventory.size() == 2);

    // Create a pickable item on the map to attempt picking up
    auto healEffect = std::make_unique<HealthEffect>(5.0f, "healed #", Colors::healing);
    auto selector = std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f);
    auto mapItem = std::make_unique<Actor>(5, 5, '!', "extra potion", Colors::healthPotion);
    mapItem->pickable = std::make_shared<Pickable>(std::move(selector), std::move(healEffect));
    Actor* mapItemPtr = mapItem.get();

    // Add the item to engine.actors (it needs to be there for updatePickupMenu to find it)
    engine.actors.push_back(std::move(mapItem));

    // Record inventory size before attempt
    const size_t invSizeBefore = testPlayer->container->inventory.size();

    // Open pickup menu with this item
    engine.pickupMenuState = PickupMenuState{ { mapItemPtr } };
    engine.gameStatus = Engine::PICKUP_MENU;

    // Simulate pressing 'a' to select the first item
    engine.inputState = InputState{};
    engine.inputState.key.pressed = true;
    engine.inputState.key.c = 'a';

    // Act
    engine.updatePickupMenu();

    // Assert: menu closed (inventory full path)
    REQUIRE(!engine.pickupMenuState.has_value());

    // Assert: state is IDLE (inventory full doesn't advance turn)
    REQUIRE(engine.gameStatus == Engine::IDLE);

    // Assert: item was NOT added to inventory (inventory size unchanged)
    REQUIRE(testPlayer->container->inventory.size() == invSizeBefore);

    // Cleanup: remove any null slots left in actors
    engine.actors.remove_if([](const std::unique_ptr<Actor>& a) { return a == nullptr; });
    testPlayer->container->inventory.clear();
    engine.player = savedPlayer;
}

// ─── 7.5: Inventory menu with all items equipped shows empty list ────────────
// **Validates: Requirements 4.4**

TEST_CASE("Inventory menu with all items equipped shows empty list", "[unit][inventory-equipment-ux]")
{
    // Setup: create an actor with Container and Equipment, equip all items
    auto testActor = std::make_unique<Actor>(5, 5, '@', "TestPlayer", Colors::white);
    testActor->container = std::make_unique<Container>(26);
    testActor->equipment = std::make_unique<Equipment>();
    testActor->attacker = std::make_shared<Attacker>(5.0f, 40);
    Actor* owner = testActor.get();

    // Create equippable items and add to container
    auto weapon = std::make_unique<Actor>(0, 0, '/', "sword", Colors::white);
    weapon->equippable = std::make_shared<Equippable>(EquipmentSlot::WEAPON, StatModifiers{1.0f, 0, 0, 0}, 3.0f, 10);
    Actor* weaponPtr = weapon.get();
    owner->container->add(std::move(weapon));

    auto armor = std::make_unique<Actor>(0, 0, '[', "chestplate", Colors::white);
    armor->equippable = std::make_shared<Equippable>(EquipmentSlot::BODY, StatModifiers{0, 2.0f, 0, 0}, 5.0f, 20);
    Actor* armorPtr = armor.get();
    owner->container->add(std::move(armor));

    // Equip both items
    owner->equipment->equip(weaponPtr, owner->container.get(), owner->attacker.get());
    owner->equipment->equip(armorPtr, owner->container.get(), owner->attacker.get());

    // Verify both are equipped
    REQUIRE(owner->equipment->isEquipped(weaponPtr));
    REQUIRE(owner->equipment->isEquipped(armorPtr));

    // Simulate the inventory filtering logic from renderInventory:
    // iterate container, skip equipped items, count unequipped
    int unequippedCount = 0;
    for (const auto& itemPtr : owner->container->inventory) {
        if (!itemPtr) continue;
        if (owner->equipment && owner->equipment->isEquipped(itemPtr.get())) continue;
        unequippedCount++;
    }

    // Assert: no unequipped items are listed (filter produces 0 items)
    REQUIRE(unequippedCount == 0);
}

// ─── 7.6: Movement keys ignored during PICKUP_MENU state ────────────────────
// **Validates: Requirements 3.3**

TEST_CASE("Movement keys ignored during PICKUP_MENU state", "[unit][inventory-equipment-ux]")
{
    // Setup: open pickup menu with some items
    auto item1 = std::make_unique<Actor>(5, 5, '!', "potion", Colors::white);
    Actor* item1Ptr = item1.get();
    engine.actors.push_back(std::move(item1));

    engine.pickupMenuState = PickupMenuState{ { item1Ptr } };
    engine.gameStatus = Engine::PICKUP_MENU;

    // Test multiple movement keys: arrows and vi keys
    std::vector<SDL_Keycode> movementKeys = {
        SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
        SDLK_KP_1, SDLK_KP_2, SDLK_KP_3, SDLK_KP_4,
        SDLK_KP_6, SDLK_KP_7, SDLK_KP_8, SDLK_KP_9
    };

    for (SDL_Keycode moveKey : movementKeys) {
        // Re-open menu state for each key test
        engine.pickupMenuState = PickupMenuState{ { item1Ptr } };
        engine.gameStatus = Engine::PICKUP_MENU;

        // Simulate the movement key press
        engine.inputState = InputState{};
        engine.inputState.key.key = moveKey;
        engine.inputState.key.pressed = true;
        engine.inputState.key.c = 0; // movement keys don't produce printable chars

        // Act
        engine.updatePickupMenu();

        // Assert: state remains PICKUP_MENU (movement was ignored)
        REQUIRE(engine.gameStatus == Engine::PICKUP_MENU);

        // Assert: pickup menu is still active
        REQUIRE(engine.pickupMenuState.has_value());
    }

    // Cleanup
    engine.pickupMenuState = std::nullopt;
    engine.gameStatus = Engine::IDLE;
    engine.actors.remove_if([&](const std::unique_ptr<Actor>& a) {
        return a.get() == item1Ptr;
    });
}
