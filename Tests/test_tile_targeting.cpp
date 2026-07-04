#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <algorithm>
#include <vector>
#include <memory>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: tile-targeting — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 8: Inventory key mapping ───────────────────────────────────────
// **Validates: Requirements 8.3, 8.4**
//
// For any inventory of size N (0 <= N <= 26) and any key press:
//   - If the key is 'a' + i where 0 <= i < N, the i-th item is selected
//     (gameStatus transitions away from INVENTORY, inventoryState cleared)
//   - If the key is ESC, no item is selected (gameStatus -> IDLE, inventoryState cleared)
//   - If the key is 'a' + i where i >= N, no item is selected (stays in INVENTORY)

TEST_CASE("PBT: Property 8 — inventory key mapping", "[property][tile-targeting]")
{
    rc::prop("key 'a'+i where i < N selects the i-th item; ESC cancels; i >= N is ignored", []() {
        // Generate random inventory size [0, 26]
        const int inventorySize = *rc::gen::inRange(0, 27);

        // Generate a key scenario:
        // 0 = ESC key, 1..26 = 'a'..'z' (so keyChoice-1 is the item index)
        const int keyChoice = *rc::gen::inRange(0, 28); // 0=ESC, 1-27 = keys a-z + one beyond

        // --- Setup: populate the player's inventory with N equippable items ---
        // We use equippable items because equipping always succeeds and transitions
        // to NEW_TURN, making the key mapping result deterministic and observable.
        Actor* owner = engine.player;
        RC_ASSERT(owner != nullptr);
        RC_ASSERT(owner->container != nullptr);

        // Reset equipment to a fresh instance (avoids stale pointers from prior iterations)
        owner->equipment = std::make_unique<Equipment>();
        // Ensure player has attacker (needed for equip's skill modifier path)
        if (!owner->attacker) {
            owner->attacker = std::make_shared<Attacker>(5.0f, 50);
        }

        // Clear any existing inventory items
        owner->container->inventory.clear();

        // Add inventorySize equippable items (each in a different slot so equips succeed)
        std::vector<Actor*> itemPtrs;
        for (int i = 0; i < inventorySize; ++i) {
            auto item = std::make_unique<Actor>(0, 0, '/', "Item" + std::to_string(i), Colors::white);
            // Cycle through slots to avoid needing complex swap logic
            EquipmentSlot slot = static_cast<EquipmentSlot>(i % static_cast<int>(EquipmentSlot::COUNT));
            StatModifiers mods{ 1.0f, 0.0f, 0.0f, 0 };
            item->equippable = std::make_shared<Equippable>(slot, mods, 1.0f, 10);
            Actor* ptr = item.get();
            owner->container->inventory.push_back(std::move(item));
            itemPtrs.push_back(ptr);
        }

        // Set up INVENTORY state
        engine.inventoryState = InventoryState{ owner, InventoryState::Action::USE };
        engine.gameStatus = Engine::INVENTORY;

        // Configure input state based on keyChoice
        engine.inputState = InputState{}; // reset
        if (keyChoice == 0) {
            // ESC key
            engine.inputState.key.key = SDLK_ESCAPE;
            engine.inputState.key.pressed = true;
            engine.inputState.key.c = 0;
        } else {
            // Letter key: 'a' + (keyChoice - 1)
            int letterIndex = keyChoice - 1; // 0-based index into alphabet
            if (letterIndex < 26) {
                engine.inputState.key.key = SDLK_UNKNOWN;
                engine.inputState.key.pressed = true;
                engine.inputState.key.c = static_cast<char>('a' + letterIndex);
            } else {
                // Beyond 'z' — simulate a non-alphabet key press (no valid selection)
                engine.inputState.key.key = SDLK_UNKNOWN;
                engine.inputState.key.pressed = true;
                engine.inputState.key.c = '{'; // character after 'z' in ASCII
            }
        }

        // Record inventory size before the call
        const int sizeBefore = static_cast<int>(owner->container->inventory.size());

        // --- Act: call updateInventory ---
        engine.updateInventory();

        // --- Assert based on the scenario ---
        if (keyChoice == 0) {
            // ESC: cancellation — no item selected, gameStatus -> IDLE
            RC_ASSERT(engine.gameStatus == Engine::IDLE);
            RC_ASSERT(!engine.inventoryState.has_value());
            // Inventory unchanged
            RC_ASSERT(static_cast<int>(owner->container->inventory.size()) == sizeBefore);
        } else {
            int letterIndex = keyChoice - 1;
            if (letterIndex < 26 && letterIndex < inventorySize) {
                // Valid key: item at letterIndex is selected
                // After selection, inventoryState is cleared
                RC_ASSERT(!engine.inventoryState.has_value());
                // gameStatus changes to NEW_TURN (equippable items always succeed)
                RC_ASSERT(engine.gameStatus == Engine::NEW_TURN);
            } else {
                // Invalid key (index >= inventorySize or beyond 'z'):
                // stays in INVENTORY, inventoryState still active
                RC_ASSERT(engine.gameStatus == Engine::INVENTORY);
                RC_ASSERT(engine.inventoryState.has_value());
                // Inventory unchanged
                RC_ASSERT(static_cast<int>(owner->container->inventory.size()) == sizeBefore);
            }
        }

        // --- Cleanup: restore engine to a sane state ---
        // Reset equipment slots first (before clearing inventory that owns the items)
        if (owner->equipment) {
            for (int s = 0; s < static_cast<int>(EquipmentSlot::COUNT); ++s) {
                EquipmentSlot slot = static_cast<EquipmentSlot>(s);
                if (owner->equipment->getSlot(slot) != nullptr) {
                    owner->equipment->unequip(slot, *owner->container, owner->attacker.get());
                }
            }
        }
        owner->container->inventory.clear();
        engine.inventoryState = std::nullopt;
        engine.gameStatus = Engine::IDLE;
    });
}
