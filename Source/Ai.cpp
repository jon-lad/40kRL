
#include <memory>
#include <list>
#include "main.h"
#include "ChargeResolver.h"

// ─── PlayerAi ────────────────────────────────────────────────────────────────

PlayerAi::PlayerAi() {}

void PlayerAi::update(Actor* owner)
{
	if (owner->destructible && owner->destructible->isDead()) { return; }
	if (!owner->actionBudget) return;

	// Handle pending door direction selection
	if (waitingForDoorDirection) {
		int dx = 0, dy = 0;
		switch (engine.inputState.key.key) {
			case SDLK_UP:    dy = -1; break;
			case SDLK_DOWN:  dy =  1; break;
			case SDLK_LEFT:  dx = -1; break;
			case SDLK_RIGHT: dx =  1; break;
			case SDLK_KP_7:  dx = -1; dy = -1; break;
			case SDLK_KP_8:  dy = -1; break;
			case SDLK_KP_9:  dx =  1; dy = -1; break;
			case SDLK_KP_4:  dx = -1; break;
			case SDLK_KP_6:  dx =  1; break;
			case SDLK_KP_1:  dx = -1; dy =  1; break;
			case SDLK_KP_2:  dy =  1; break;
			case SDLK_KP_3:  dx =  1; dy =  1; break;
			case SDLK_ESCAPE:
				// Cancel door direction selection
				waitingForDoorDirection = false;
				pendingDoors.clear();
				engine.gui->message(Colors::lightGrey, "Cancelled.");
				return;
			default: return; // Ignore other keys while waiting for direction
		}
		if (dx != 0 || dy != 0) {
			int targetX = owner->getX() + dx;
			int targetY = owner->getY() + dy;
			// Find a door at the chosen direction
			for (Actor* door : pendingDoors) {
				if (door->getX() == targetX && door->getY() == targetY) {
					if (!owner->actionBudget->canAfford(1)) {
						engine.gui->message(Colors::lightGrey, "Not enough AP.");
						waitingForDoorDirection = false;
						pendingDoors.clear();
						return;
					}
					owner->actionBudget->spend(1);
					door->openable->open(door);
					waitingForDoorDirection = false;
					pendingDoors.clear();
					return;
				}
			}
			engine.gui->message(Colors::lightGrey, "There is no door in that direction.");
			waitingForDoorDirection = false;
			pendingDoors.clear();
		}
		return;
	}

	// Handle pending run direction selection
	if (pendingRun) {
		int dx = 0, dy = 0;
		switch (engine.inputState.key.key) {
			case SDLK_UP:    dy = -1; break;
			case SDLK_DOWN:  dy =  1; break;
			case SDLK_LEFT:  dx = -1; break;
			case SDLK_RIGHT: dx =  1; break;
			case SDLK_KP_7:  dx = -1; dy = -1; break;
			case SDLK_KP_8:  dy = -1; break;
			case SDLK_KP_9:  dx =  1; dy = -1; break;
			case SDLK_KP_4:  dx = -1; break;
			case SDLK_KP_6:  dx =  1; break;
			case SDLK_KP_1:  dx = -1; dy =  1; break;
			case SDLK_KP_2:  dy =  1; break;
			case SDLK_KP_3:  dx =  1; dy =  1; break;
			case SDLK_ESCAPE:
				// Cancel run
				pendingRun = false;
				pendingRunDistance = 0;
				engine.gui->message(Colors::lightGrey, "Cancelled.");
				return;
			default: return; // Ignore other keys while waiting for direction
		}
		if (dx != 0 || dy != 0) {
			// Spend 2 AP for Run (Full Action)
			if (!owner->actionBudget->canAfford(2)) {
				engine.gui->message(Colors::lightGrey, "Not enough AP to run.");
				pendingRun = false;
				pendingRunDistance = 0;
				return;
			}
			owner->actionBudget->spend(2);

			// Move up to pendingRunDistance tiles in the chosen direction
			int tilesMoved = 0;
			for (int i = 0; i < pendingRunDistance; ++i) {
				int nextX = owner->getX() + dx;
				int nextY = owner->getY() + dy;
				if (!engine.map->canWalk(nextX, nextY)) {
					break; // blocked
				}
				owner->setX(nextX);
				owner->setY(nextY);
				tilesMoved++;
			}

			if (tilesMoved > 0) {
				engine.map->computeFOV();
				engine.camera->update(owner, engine.map->getLevelType() == LevelType::OUTDOOR);
				engine.gui->message(Colors::playerAction, "You run %d tiles!", tilesMoved);
			} else {
				engine.gui->message(Colors::lightGrey, "You can't run in that direction.");
			}

			pendingRun = false;
			pendingRunDistance = 0;
		}
		return;
	}

	int dx = 0, dy = 0;
	switch (engine.inputState.key.key) {
		// Arrow keys
		case SDLK_UP:    dy = -1; break;
		case SDLK_DOWN:  dy =  1; break;
		case SDLK_LEFT:  dx = -1; break;
		case SDLK_RIGHT: dx =  1; break;

		// Numpad 8-directional movement
		case SDLK_KP_7:  dx = -1; dy = -1; break;
		case SDLK_KP_8:  dy = -1; break;
		case SDLK_KP_9:  dx =  1; dy = -1; break;
		case SDLK_KP_4:  dx = -1; break;
		case SDLK_KP_6:  dx =  1; break;
		case SDLK_KP_1:  dx = -1; dy =  1; break;
		case SDLK_KP_2:  dy =  1; break;
		case SDLK_KP_3:  dx =  1; dy =  1; break;

		// Numpad 5 = End Turn (free action)
		case SDLK_KP_5:
			owner->actionBudget->setAP(0);
			engine.gui->message(Colors::lightGrey, "You end your turn.");
			return;

		case SDLK_F12:
			engine.debugMode = !engine.debugMode;
			engine.gui->message(Colors::yellow,
				engine.debugMode ? "Debug mode ON" : "Debug mode OFF");
			break;
		case SDLK_PAGEDOWN:
			if (engine.debugMode) {
				engine.gui->message(Colors::yellow, "DEBUG: Skipping to next level...");
				engine.nextLevel(StairDirection::DOWN);
			}
			break;
		case SDLK_PAGEUP:
			if (engine.debugMode) {
				engine.gui->message(Colors::yellow, "DEBUG: Skipping to previous level...");
				engine.nextLevel(StairDirection::UP);
			}
			break;
		default:
			if (engine.inputState.key.c != 0) {
				handleActionKey(owner, engine.inputState.key.c);
			}
			break;
	}

	if (dx != 0 || dy != 0) {
		if (!owner->actionBudget->canAfford(1)) {
			engine.gui->message(Colors::lightGrey, "Not enough AP.");
			return;
		}
		owner->actionBudget->spend(1);
		if (moveOrAttack(owner, owner->getX() + dx, owner->getY() + dy)) {
			engine.map->computeFOV();
		}
	}
}

bool PlayerAi::moveOrAttack(Actor* owner, int targetX, int targetY)
{
	if (engine.map->isWall(targetX, targetY)) {
		// Check if there's a closed door blocking the way
		for (auto& actorPtr : engine.actors) {
			Actor* actor = actorPtr.get();
			if (actor->openable && !actor->openable->isOpen()
				&& actor->getX() == targetX && actor->getY() == targetY)
			{
				engine.gui->message(Colors::lightGrey, "The door is closed.");
				return false;
			}
		}
		return false;
	}

	// Attack the first living actor on the target tile.
	for (auto& actorPtr : engine.actors) {
		Actor* actor = actorPtr.get();
		if (actor->destructible && !actor->destructible->isDead()
			&& actor->getX() == targetX && actor->getY() == targetY)
		{
			// Standard Attack (melee): +10 WS modifier per RT-CoreMechanics §4
			owner->attacker->addModifier(10);
			owner->attacker->attack(owner, actor);
			owner->attacker->removeModifier(10);
			return false;
		}
	}

	// Notify the player about any corpses or items on the tile.
	for (auto& actorPtr : engine.actors) {
		Actor* actor = actorPtr.get();
		const bool isCorpse = actor->destructible && actor->destructible->isDead();
		const bool isItem   = actor->pickable != nullptr;
		if ((isCorpse || isItem) && actor->getX() == targetX && actor->getY() == targetY) {
			engine.gui->message(Colors::uiText, "Theres a # here", actor->name);
		}
	}

	owner->setX(targetX);
	owner->setY(targetY);
	engine.camera->update(owner, engine.map->getLevelType() == LevelType::OUTDOOR);
	return true;
}

void PlayerAi::handleActionKey(Actor* owner, int ascii)
{
	switch (ascii) {
	case 'g': // pick up item on current tile
	{
		// Collect all pickable items at the player's position.
		std::vector<Actor*> pickableItems;
		for (auto& actorPtr : engine.actors) {
			Actor* item = actorPtr.get();
			if (item->pickable && item->getX() == owner->getX() && item->getY() == owner->getY()) {
				pickableItems.push_back(item);
			}
		}

		if (pickableItems.empty()) {
			// No items on tile.
			engine.gui->message(Colors::uiText, "There is nothing here to pick up.");
		} else if (pickableItems.size() == 1) {
			// Single item — check AP, then auto-pickup.
			if (!owner->actionBudget->canAfford(1)) {
				engine.gui->message(Colors::lightGrey, "Not enough AP.");
				return;
			}
			Actor* item = pickableItems[0];
			// Find the owning unique_ptr in the actors list.
			for (auto it = engine.actors.begin(); it != engine.actors.end(); ++it) {
				if (it->get() == item) {
					if (item->pickable->pick(std::move(*it), owner)) {
						engine.gui->message(Colors::uiText, "You pick up the #.", item->name);
						// Erase the now-null slot left by the move.
						engine.actors.erase(it);
						owner->actionBudget->spend(1);
					} else {
						engine.gui->message(Colors::damage, "Your inventory is full!");
					}
					break;
				}
			}
		} else {
			// Multiple items — open the pickup menu.
			engine.beginPickupMenu(pickableItems);
			return;
		}
		break;
	}

	case 'i': // open inventory (tabbed menu on Inventory tab)
		engine.beginTabbedMenu(TabbedMenuState::Tab::INVENTORY);
		return;

	case 'd': // drop an item from inventory
		engine.beginInventory(owner, InventoryState::Action::DROP);
		return;

	case 'l': // enter look mode
		engine.beginLook();
		return;

	case 'c': // open character sheet (tabbed menu on Equipment tab)
		engine.beginTabbedMenu(TabbedMenuState::Tab::EQUIPMENT);
		return;

	case 'm': // open world map
		engine.beginWorldMap();
		return;

	case 'x': // open advance purchase overlay
		engine.beginAdvances();
		return;

	case '?': // open help overlay
		engine.beginHelp();
		return;

	case '<': // ascend stairs
		if (engine.stairsUp
			&& engine.stairsUp->getX() == owner->getX()
			&& engine.stairsUp->getY() == owner->getY())
		{
			engine.nextLevel(StairDirection::UP);
		} else {
			engine.gui->message(Colors::uiText, "There are no stairs here.");
		}
		break;

	case '>': // descend stairs
		if (engine.stairsDown
			&& engine.stairsDown->getX() == owner->getX()
			&& engine.stairsDown->getY() == owner->getY())
		{
			engine.nextLevel(StairDirection::DOWN);
		} else {
			engine.gui->message(Colors::uiText, "There are no stairs here.");
		}
		break;

	case 's': // shoot ranged weapon
	{
		// Check if player has a ranged weapon equipped in weapon slot.
		Actor* weaponItem = owner->equipment ? owner->equipment->getSlot(EquipmentSlot::WEAPON) : nullptr;
		if (!weaponItem || !weaponItem->equippable || !weaponItem->equippable->rangedStats) {
			engine.gui->message(Colors::uiText, "You have no ranged weapon equipped.");
			return;
		}
		// Check ammo.
		if (weaponItem->equippable->currentAmmo <= 0) {
			engine.gui->message(Colors::uiText, "Your weapon is empty. Press 'r' to reload.");
			return;
		}
		// Check AP before entering targeting.
		if (!owner->actionBudget->canAfford(1)) {
			engine.gui->message(Colors::lightGrey, "Not enough AP.");
			return;
		}
		// Enter targeting mode for ranged attack.
		float weaponRange = static_cast<float>(weaponItem->equippable->rangedStats->range);
		engine.targetingCtx = TargetingContext{
			weaponItem,                                    // item (the weapon)
			owner,                                         // owner
			weaponRange,                                   // maxRange
			TargetSelector::SelectorType::SELECTED_MONSTER, // type
			nullptr,                                       // effect (unused for ranged)
			0.0f,                                          // aoeRange
			true                                           // isRangedAttack
		};
		engine.gameStatus = Engine::TARGETING;
		engine.gui->message(Colors::uiText, "Left-click to confirm, right-click or ESC to cancel.");
		return;
	}

	case 'r': // reload ranged weapon
	{
		// Check if player has a ranged weapon equipped in weapon slot.
		Actor* weaponItem = owner->equipment ? owner->equipment->getSlot(EquipmentSlot::WEAPON) : nullptr;
		if (!weaponItem || !weaponItem->equippable || !weaponItem->equippable->rangedStats) {
			engine.gui->message(Colors::uiText, "You have no ranged weapon to reload.");
			return;
		}
		// Check if ammo is already full.
		if (weaponItem->equippable->currentAmmo >= weaponItem->equippable->rangedStats->clipSize) {
			engine.gui->message(Colors::uiText, "Your weapon is already fully loaded.");
			return;
		}
		// Check AP before performing action.
		if (!owner->actionBudget->canAfford(1)) {
			engine.gui->message(Colors::lightGrey, "Not enough AP.");
			return;
		}
		// Perform reload: set currentAmmo to clipSize, display message, spend AP.
		weaponItem->equippable->currentAmmo = weaponItem->equippable->rangedStats->clipSize;
		engine.gui->message(Colors::uiText, "# reloads #.", owner->name, weaponItem->name);
		owner->actionBudget->spend(1);
		return;
	}

	case 'o': // open an adjacent door
	{
		// Scan cardinal neighbours for closed doors
		static constexpr int cardinalDX[4] = { 0, 0, 1, -1 };
		static constexpr int cardinalDY[4] = { -1, 1, 0, 0 };

		std::vector<Actor*> closedDoors;
		for (int i = 0; i < 4; ++i) {
			const int nx = owner->getX() + cardinalDX[i];
			const int ny = owner->getY() + cardinalDY[i];
			for (auto& actorPtr : engine.actors) {
				Actor* actor = actorPtr.get();
				if (actor->openable && !actor->openable->isOpen()
					&& actor->getX() == nx && actor->getY() == ny)
				{
					closedDoors.push_back(actor);
				}
			}
		}

		if (closedDoors.empty()) {
			engine.gui->message(Colors::lightGrey, "There is no door to open.");
		} else if (closedDoors.size() == 1) {
			if (!owner->actionBudget->canAfford(1)) {
				engine.gui->message(Colors::lightGrey, "Not enough AP.");
				return;
			}
			owner->actionBudget->spend(1);
			closedDoors[0]->openable->open(closedDoors[0]);
		} else {
			// Multiple adjacent closed doors — prompt for direction.
			PlayerAi* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
			if (playerAi) {
				playerAi->waitingForDoorDirection = true;
				playerAi->pendingDoors = closedDoors;
				engine.gui->message(Colors::lightGrey, "Which direction? (use arrow keys)");
			}
		}
		break;
	}

	case 'a': // aim action (Half Action, 1 AP)
	{
		if (!owner->actionBudget || !owner->actionBudget->canAfford(1)) {
			engine.gui->message(Colors::lightGrey, "Not enough AP to aim.");
			return;
		}
		owner->actionBudget->spend(1);
		owner->actionBudget->addAimBonus();
		engine.gui->message(Colors::playerAction, "You take aim. (+%d to next attack)",
			owner->actionBudget->getAimBonus());
		return;
	}

	case 'A': // All-Out Attack (Full Action, 2 AP)
	{
		if (!owner->actionBudget || !owner->actionBudget->canAfford(2)) {
			engine.gui->message(Colors::lightGrey, "Not enough AP for All-Out Attack (requires 2 AP).");
			return;
		}
		// Find adjacent enemy to attack
		Actor* target = nullptr;
		for (int ddx = -1; ddx <= 1; ++ddx) {
			for (int ddy = -1; ddy <= 1; ++ddy) {
				if (ddx == 0 && ddy == 0) continue;
				Actor* adj = engine.getActorAt(owner->getX() + ddx, owner->getY() + ddy);
				if (adj && adj != owner && adj->destructible && !adj->destructible->isDead()) {
					target = adj;
					break;
				}
			}
			if (target) break;
		}
		if (!target) {
			engine.gui->message(Colors::lightGrey, "No adjacent enemy for All-Out Attack.");
			return;
		}
		owner->actionBudget->spend(2);
		owner->actionBudget->forfeitReaction();
		if (owner->attacker) {
			owner->attacker->addModifier(30);
			owner->attacker->attack(owner, target);
			owner->attacker->removeModifier(30);
		}
		engine.gui->message(Colors::playerAction, "You unleash an All-Out Attack!");
		return;
	}

	case 'R': // Run (Full Action, 2 AP)
	{
		if (!owner->actionBudget || !owner->actionBudget->canAfford(2)) {
			engine.gui->message(Colors::lightGrey, "Not enough AP to run (requires 2 AP).");
			return;
		}
		// Compute max run distance: AgB × 6
		int agB = 3; // default
		if (owner->characteristics) {
			agB = owner->characteristics->bonus(CharId::Ag);
		}
		int maxTiles = agB * 6;

		// Enter pending run mode — wait for direction key, then move
		// AP is deducted in the pendingRun handler after direction is chosen
		PlayerAi* playerAi = dynamic_cast<PlayerAi*>(owner->ai.get());
		if (playerAi) {
			playerAi->pendingRun = true;
			playerAi->pendingRunDistance = maxTiles;
			engine.gui->message(Colors::lightGrey, "Run in which direction? (up to %d tiles)", maxTiles);
		}
		return;
	}

	case 'C': // Charge (Full Action, 2 AP)
	{
		if (!owner->actionBudget || !owner->actionBudget->canAfford(2)) {
			engine.gui->message(Colors::lightGrey, "Not enough AP to charge (requires 2 AP).");
			return;
		}
		// For now, charge toward the closest visible enemy
		Actor* target = engine.getClosestMonster(owner->getX(), owner->getY(), 0.0f);
		if (!target) {
			engine.gui->message(Colors::lightGrey, "No target to charge.");
			return;
		}
		int agB = 3;
		if (owner->characteristics) {
			agB = owner->characteristics->bonus(CharId::Ag);
		}
		ChargeResult charge = ChargeResolver::compute(
			owner->getX(), owner->getY(),
			target->getX(), target->getY(),
			agB, *engine.map);
		if (!charge.valid) {
			engine.gui->message(Colors::lightGrey, "Charge path is blocked or target is out of range.");
			return;
		}
		owner->actionBudget->spend(2);
		owner->setX(charge.endX);
		owner->setY(charge.endY);
		engine.map->computeFOV();
		if (owner->attacker) {
			owner->attacker->addModifier(20);
			owner->attacker->attack(owner, target);
			owner->attacker->removeModifier(20);
		}
		engine.gui->message(Colors::playerAction, "You charge!");
		engine.camera->update(owner, engine.map->getLevelType() == LevelType::OUTDOOR);
		return;
	}


	case 'e': // open equipment menu
	{
		static constexpr int EQUIP_WIDTH = 50;
		static constexpr int EQUIP_HEIGHT = 10;
		static TCODConsole equipConsole(EQUIP_WIDTH, EQUIP_HEIGHT);

		equipConsole.setDefaultForeground(Colors::menuFrame);
		equipConsole.printFrame(0, 0, EQUIP_WIDTH, EQUIP_HEIGHT, true, TCOD_BKGND_DEFAULT, "equipment");
		equipConsole.setDefaultForeground(Colors::white);

		const char* slotNames[] = { "Weapon", "Offhand", "Head", "Body" };
		for (int i = 0; i < 4; i++) {
			Actor* equipped = owner->equipment->getSlot(static_cast<EquipmentSlot>(i));
			std::string line = std::string("[") + slotNames[i] + "]  ";
			if (equipped) {
				line += equipped->name;
				auto& mods = equipped->equippable->modifiers;
				if (mods.power != 0) line += " (+" + std::to_string((int)mods.power) + " pow)";
				if (mods.defense != 0) line += " (+" + std::to_string((int)mods.defense) + " def)";
				if (mods.skill != 0) line += " (" + std::to_string(mods.skill) + " skill)";
			} else {
				line += "empty";
			}
			equipConsole.printf(2, i + 2, "%c) %s", 'a' + i, line.c_str());
		}

		TCODConsole::blit(&equipConsole, 0, 0, EQUIP_WIDTH, EQUIP_HEIGHT,
			TCODConsole::root,
			engine.screenWidth / 2 - EQUIP_WIDTH / 2,
			engine.screenHeight / 2 - EQUIP_HEIGHT / 2);
		TCODConsole::flush();

		// Wait for input: a-d to unequip a slot, ESC to close
		TCOD_key_t key;
		TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, nullptr, true);
		if (key.vk == TCODK_CHAR && key.c >= 'a' && key.c <= 'd') {
			int slotIndex = key.c - 'a';
			EquipmentSlot slot = static_cast<EquipmentSlot>(slotIndex);
			if (owner->equipment->getSlot(slot)) {
				if (!owner->actionBudget->canAfford(1)) {
					engine.gui->message(Colors::lightGrey, "Not enough AP.");
					return;
				}
				owner->equipment->unequip(slot, *owner->container, owner->attacker.get());
				engine.gui->message(Colors::uiText, "Item unequipped.");
				owner->actionBudget->spend(1);
			}
		}
		break;
	}
	}
}

// ─── MonsterAi ───────────────────────────────────────────────────────────────

void MonsterAi::update(Actor* owner)
{
	if (owner->destructible && owner->destructible->isDead()) { return; }

	// Legacy path: if no ActionBudget, use old 1-action behaviour
	if (!owner->actionBudget) {
		moveOrAttack(owner, engine.player->getX(), engine.player->getY());
		return;
	}

	// AP-based action loop: spend full budget
	while (owner->actionBudget->getAP() > 0) {
		if (!selectAndExecuteAction(owner)) {
			break; // no valid action available
		}
	}
}

bool MonsterAi::selectAndExecuteAction(Actor* owner)
{
	const int ap = owner->actionBudget->getAP();
	const int dx = engine.player->getX() - owner->getX();
	const int dy = engine.player->getY() - owner->getY();
	const float distance = sqrtf(static_cast<float>(dx * dx + dy * dy));

	// Adjacent to player — consider All-Out Attack (Full Action, 2 AP) or standard attack (1 AP)
	if (distance < 2.0f && ap >= 1) {
		// TODO: All-Out Attack (2 AP, +30 WS, forfeit reaction) — enable for elite/boss enemies
		// when they have a significant tactical advantage (e.g., high WS, target is wounded).
		// For now, basic enemies always use the standard 1 AP melee attack.

		owner->actionBudget->spend(1);
		if (owner->attacker) {
			// Standard Attack (melee): +10 WS modifier per RT-CoreMechanics §4
			owner->attacker->addModifier(10);
			owner->attacker->attack(owner, engine.player);
			owner->attacker->removeModifier(10);
		}
		return true;
	}

	// Consider Charge (Full Action, 2 AP) when far enough to benefit
	if (ap >= 2 && distance >= 2.0f) {
		int agB = 3; // default AgB if no characteristics
		if (owner->characteristics) {
			agB = owner->characteristics->bonus(CharId::Ag);
		}
		ChargeResult charge = ChargeResolver::compute(
			owner->getX(), owner->getY(),
			engine.player->getX(), engine.player->getY(),
			agB, *engine.map);
		if (charge.valid) {
			owner->actionBudget->spend(2);
			owner->setX(charge.endX);
			owner->setY(charge.endY);
			// Log charge if visible to player
			if (engine.map->isInFOV(owner->getX(), owner->getY())) {
				engine.gui->message(Colors::enemyAction, "The # charges!", owner->name);
			}
			// Resolve melee attack with +20 WS modifier
			if (owner->attacker) {
				owner->attacker->addModifier(20);
				owner->attacker->attack(owner, engine.player);
				owner->attacker->removeModifier(20);
			}
			return true;
		}
	}

	// Move toward player (1 AP)
	if (ap >= 1) {
		owner->actionBudget->spend(1);
		moveToward(owner, engine.player->getX(), engine.player->getY());
		return true;
	}

	return false;
}

void MonsterAi::moveToward(Actor* owner, int targetX, int targetY)
{
	const int dx = targetX - owner->getX();
	const int dy = targetY - owner->getY();
	const float distance = sqrtf(static_cast<float>(dx * dx + dy * dy));
	if (distance < 1.0f) return;

	const int stepX = static_cast<int>(std::round(dx / distance));
	const int stepY = static_cast<int>(std::round(dy / distance));
	const int nextX = owner->getX() + stepX;
	const int nextY = owner->getY() + stepY;

	// Check for closed doors — open them and consume the action
	for (auto& actorPtr : engine.actors) {
		if (actorPtr->openable && !actorPtr->openable->isOpen()
			&& actorPtr->getX() == nextX && actorPtr->getY() == nextY) {
			actorPtr->openable->open(actorPtr.get());
			if (engine.map->isInFOV(owner->getX(), owner->getY())) {
				engine.gui->message(Colors::enemyAction, "The # opens the door.", owner->name);
			}
			return;
		}
	}

	if (engine.map->isInFOV(owner->getX(), owner->getY())) {
		// Player is visible — step directly toward them.
		if (engine.map->canWalk(nextX, nextY)) {
			owner->setX(nextX);
			owner->setY(nextY);
			return;
		}
	}

	// Player not visible — follow the strongest scent trail in the 8 neighbours.
	static constexpr int neighbourDX[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	static constexpr int neighbourDY[8] = { -1,-1,-1,  0, 0,  1, 1, 1 };

	unsigned int bestScent     = 0;
	int          bestNeighbour = -1;

	for (int i = 0; i < 8; ++i) {
		const int cellX = owner->getX() + neighbourDX[i];
		const int cellY = owner->getY() + neighbourDY[i];
		if (engine.map->canWalk(cellX, cellY)) {
			const unsigned int cellScent = engine.map->getScent(cellX, cellY);
			const bool scentIsFresh = cellScent > engine.map->currentScentValue - SCENT_THRESHOLD;
			if (scentIsFresh && cellScent > bestScent) {
				bestScent     = cellScent;
				bestNeighbour = i;
			}
		}
	}

	if (bestNeighbour != -1) {
		const int scentNextX = owner->getX() + neighbourDX[bestNeighbour];
		const int scentNextY = owner->getY() + neighbourDY[bestNeighbour];

		// Check if there's a closed door at the scent target — open it and consume the action.
		for (auto& actorPtr : engine.actors) {
			if (actorPtr->openable && !actorPtr->openable->isOpen()
				&& actorPtr->getX() == scentNextX && actorPtr->getY() == scentNextY) {
				actorPtr->openable->open(actorPtr.get());
				if (engine.map->isInFOV(owner->getX(), owner->getY())) {
					engine.gui->message(Colors::enemyAction, "The # opens the door.", owner->name);
				}
				return;
			}
		}

		owner->setX(scentNextX);
		owner->setY(scentNextY);
	}
}

void MonsterAi::moveOrAttack(Actor* owner, int targetX, int targetY)
{
	const int dx = targetX - owner->getX();
	const int dy = targetY - owner->getY();
	const float distance = sqrtf(static_cast<float>(dx * dx + dy * dy));

	if (distance < 2.0f) {
		// Adjacent — standard attack with +10 WS modifier.
		// Only attack if visible to player (FOV-gated logging handled in Attacker)
		if (owner->attacker) {
			owner->attacker->addModifier(10);
			owner->attacker->attack(owner, engine.player);
			owner->attacker->removeModifier(10);
		}
		return;
	}

	// Before any movement: check if the step toward the target has a closed door.
	// If so, open it and consume the turn (do not move).
	const int stepX = static_cast<int>(std::round(dx / distance));
	const int stepY = static_cast<int>(std::round(dy / distance));
	const int nextX = owner->getX() + stepX;
	const int nextY = owner->getY() + stepY;

	for (auto& actorPtr : engine.actors) {
		if (actorPtr->openable && !actorPtr->openable->isOpen()
			&& actorPtr->getX() == nextX && actorPtr->getY() == nextY) {
			actorPtr->openable->open(actorPtr.get());
			if (engine.map->isInFOV(owner->getX(), owner->getY())) {
				engine.gui->message(Colors::enemyAction, "The # opens the door.", owner->name);
			}
			return; // turn consumed
		}
	}

	if (engine.map->isInFOV(owner->getX(), owner->getY())) {
		// Player is visible — step directly toward them.
		if (engine.map->canWalk(nextX, nextY)) {
			owner->setX(nextX);
			owner->setY(nextY);
			return;
		}
	}

	// Player not visible — follow the strongest scent trail in the 8 neighbours.
	static constexpr int neighbourDX[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	static constexpr int neighbourDY[8] = { -1,-1,-1,  0, 0,  1, 1, 1 };

	unsigned int bestScent     = 0;
	int          bestNeighbour = -1;

	for (int i = 0; i < 8; ++i) {
		const int cellX = owner->getX() + neighbourDX[i];
		const int cellY = owner->getY() + neighbourDY[i];
		if (engine.map->canWalk(cellX, cellY)) {
			const unsigned int cellScent = engine.map->getScent(cellX, cellY);
			const bool scentIsFresh = cellScent > engine.map->currentScentValue - SCENT_THRESHOLD;
			if (scentIsFresh && cellScent > bestScent) {
				bestScent     = cellScent;
				bestNeighbour = i;
			}
		}
	}

	if (bestNeighbour != -1) {
		const int scentNextX = owner->getX() + neighbourDX[bestNeighbour];
		const int scentNextY = owner->getY() + neighbourDY[bestNeighbour];

		// Check if there's a closed door at the scent target — open it and consume the turn.
		for (auto& actorPtr : engine.actors) {
			if (actorPtr->openable && !actorPtr->openable->isOpen()
				&& actorPtr->getX() == scentNextX && actorPtr->getY() == scentNextY) {
				actorPtr->openable->open(actorPtr.get());
				if (engine.map->isInFOV(owner->getX(), owner->getY())) {
					engine.gui->message(Colors::enemyAction, "The # opens the door.", owner->name);
				}
				return; // turn consumed
			}
		}

		owner->setX(scentNextX);
		owner->setY(scentNextY);
	}
}

// ─── RangedAi ────────────────────────────────────────────────────────────────

void RangedAi::update(Actor* owner)
{
	if (owner->destructible && owner->destructible->isDead()) { return; }

	const int dx = engine.player->getX() - owner->getX();
	const int dy = engine.player->getY() - owner->getY();
	const float distance = sqrtf(static_cast<float>(dx * dx + dy * dy));

	// 1. Adjacent to player → melee attack.
	if (distance < 2.0f) {
		if (owner->attacker) {
			owner->attacker->attack(owner, engine.player);
		}
		return;
	}

	// Determine weapon stats for range/ammo checks.
	Actor* weaponItem = owner->equipment ? owner->equipment->getSlot(EquipmentSlot::WEAPON) : nullptr;
	const bool hasRangedWeapon = weaponItem && weaponItem->equippable && weaponItem->equippable->rangedStats;
	const int currentAmmo = hasRangedWeapon ? weaponItem->equippable->currentAmmo : 0;
	const int weaponRange = hasRangedWeapon ? weaponItem->equippable->rangedStats->range : 0;

	// 2. Has LoS (monster is in player's FOV) + within weapon range + has ammo → shoot.
	if (engine.map->isInFOV(owner->getX(), owner->getY())) {
		if (hasRangedWeapon && currentAmmo > 0 && distance <= static_cast<float>(weaponRange)) {
			shoot(owner, engine.player);
			return;
		}

		// 5. Has LoS but zero ammo and not adjacent → reload.
		if (hasRangedWeapon && currentAmmo <= 0) {
			reload(owner);
			return;
		}

		// 3. Has LoS but beyond weapon range → move toward player.
		moveToward(owner, engine.player->getX(), engine.player->getY());
		return;
	}

	// 4. No LoS → follow scent trail.
	followScent(owner);
}

void RangedAi::shoot(Actor* owner, Actor* target)
{
	// Standard Attack (ranged): +10 BS modifier per RT-CoreMechanics §4
	if (owner->attacker) {
		owner->attacker->addModifier(10);
	}
	RangedCombat::resolve(owner, target);
	if (owner->attacker) {
		owner->attacker->removeModifier(10);
	}
}

void RangedAi::reload(Actor* owner)
{
	Actor* weaponItem = owner->equipment ? owner->equipment->getSlot(EquipmentSlot::WEAPON) : nullptr;
	if (weaponItem && weaponItem->equippable && weaponItem->equippable->rangedStats) {
		weaponItem->equippable->currentAmmo = weaponItem->equippable->rangedStats->clipSize;
		if (engine.map->isInFOV(owner->getX(), owner->getY())) {
			engine.gui->message(Colors::enemyAction, "# reloads #.", owner->name, weaponItem->name);
		}
	}
}

void RangedAi::moveToward(Actor* owner, int targetX, int targetY)
{
	const int dx = targetX - owner->getX();
	const int dy = targetY - owner->getY();
	const float distance = sqrtf(static_cast<float>(dx * dx + dy * dy));

	const int stepX = static_cast<int>(std::round(dx / distance));
	const int stepY = static_cast<int>(std::round(dy / distance));
	if (engine.map->canWalk(owner->getX() + stepX, owner->getY() + stepY)) {
		owner->setX(owner->getX() + stepX);
		owner->setY(owner->getY() + stepY);
	}
}

void RangedAi::followScent(Actor* owner)
{
	static constexpr int neighbourDX[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	static constexpr int neighbourDY[8] = { -1,-1,-1,  0, 0,  1, 1, 1 };

	unsigned int bestScent     = 0;
	int          bestNeighbour = -1;

	for (int i = 0; i < 8; ++i) {
		const int cellX = owner->getX() + neighbourDX[i];
		const int cellY = owner->getY() + neighbourDY[i];
		if (engine.map->canWalk(cellX, cellY)) {
			const unsigned int cellScent = engine.map->getScent(cellX, cellY);
			const bool scentIsFresh = cellScent > engine.map->currentScentValue - SCENT_THRESHOLD;
			if (scentIsFresh && cellScent > bestScent) {
				bestScent     = cellScent;
				bestNeighbour = i;
			}
		}
	}

	if (bestNeighbour != -1) {
		owner->setX(owner->getX() + neighbourDX[bestNeighbour]);
		owner->setY(owner->getY() + neighbourDY[bestNeighbour]);
	}
}

// ─── TemporaryAi ─────────────────────────────────────────────────────────────

TemporaryAi::TemporaryAi(int turnsRemaining) : turnsRemaining{ turnsRemaining } {}

void TemporaryAi::update(Actor* owner)
{
	turnsRemaining--;
	if (turnsRemaining == 0) {
		owner->ai = std::move(oldAi); // restore the original AI
	}
}

void TemporaryAi::applyTo(Actor* actor)
{
	// Legacy path — only safe when the caller immediately moves this into actor->ai.
	oldAi = std::move(actor->ai);
}

void TemporaryAi::applyToActor(std::unique_ptr<TemporaryAi> self, Actor* actor)
{
	self->oldAi = std::move(actor->ai);
	actor->ai   = std::move(self);
}

// ─── ConfusedMonsterAi ───────────────────────────────────────────────────────

ConfusedMonsterAi::ConfusedMonsterAi(int turnsRemaining) : TemporaryAi(turnsRemaining) {}

void ConfusedMonsterAi::update(Actor* owner)
{
	TCODRandom* rng = TCODRandom::getInstance();
	const int dx = rng->getInt(-1, 1);
	const int dy = rng->getInt(-1, 1);

	if (dx != 0 || dy != 0) {
		const int destX = owner->getX() + dx;
		const int destY = owner->getY() + dy;
		if (engine.map->canWalk(destX, destY)) {
			owner->setX(destX);
			owner->setY(destY);
		} else {
			// Bumped into something — attack it if possible.
			Actor* blocker = engine.getActorAt(destX, destY);
			if (blocker && owner->attacker) {
				owner->attacker->attack(owner, blocker);
			}
		}
	}

	TemporaryAi::update(owner); // decrement counter and restore AI if expired
}
