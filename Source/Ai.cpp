
#include <memory>
#include <list>
#include "main.h"

// ─── PlayerAi ────────────────────────────────────────────────────────────────

PlayerAi::PlayerAi() {}

void PlayerAi::update(Actor* owner)
{
	if (owner->destructible && owner->destructible->isDead()) { return; }

	int dx = 0, dy = 0;
	switch (engine.inputState.key.key) {
		case SDLK_UP:    dy = -1; break;
		case SDLK_DOWN:  dy =  1; break;
		case SDLK_LEFT:  dx = -1; break;
		case SDLK_RIGHT: dx =  1; break;
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
		engine.gameStatus = Engine::NEW_TURN;
		if (moveOrAttack(owner, owner->getX() + dx, owner->getY() + dy)) {
			engine.map->computeFOV();
		}
	}
}

bool PlayerAi::moveOrAttack(Actor* owner, int targetX, int targetY)
{
	if (engine.map->isWall(targetX, targetY)) { return false; }

	// Attack the first living actor on the target tile.
	for (auto& actorPtr : engine.actors) {
		Actor* actor = actorPtr.get();
		if (actor->destructible && !actor->destructible->isDead()
			&& actor->getX() == targetX && actor->getY() == targetY)
		{
			owner->attacker->attack(owner, actor);
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
			engine.gameStatus = Engine::NEW_TURN;
		} else if (pickableItems.size() == 1) {
			// Single item — auto-pickup.
			Actor* item = pickableItems[0];
			// Find the owning unique_ptr in the actors list.
			for (auto it = engine.actors.begin(); it != engine.actors.end(); ++it) {
				if (it->get() == item) {
					if (item->pickable->pick(std::move(*it), owner)) {
						engine.gui->message(Colors::uiText, "You pick up the #.", item->name);
						// Erase the now-null slot left by the move.
						engine.actors.erase(it);
					} else {
						engine.gui->message(Colors::damage, "Your inventory is full!");
					}
					break;
				}
			}
			engine.gameStatus = Engine::NEW_TURN;
		} else {
			// Multiple items — open the pickup menu.
			engine.beginPickupMenu(pickableItems);
			return;
		}
		break;
	}

	case 'i': // open inventory to use/equip an item
		engine.beginInventory(owner, InventoryState::Action::USE);
		return;

	case 'd': // drop an item from inventory
		engine.beginInventory(owner, InventoryState::Action::DROP);
		return;

	case 'l': // enter look mode
		engine.beginLook();
		return;

	case 'c': // open character sheet
		engine.beginCharacterSheet();
		return;

	case 'm': // open world map
		engine.beginWorldMap();
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
		// Perform reload: set currentAmmo to clipSize, display message, advance turn.
		weaponItem->equippable->currentAmmo = weaponItem->equippable->rangedStats->clipSize;
		engine.gui->message(Colors::uiText, "# reloads #.", owner->name, weaponItem->name);
		engine.gameStatus = Engine::NEW_TURN;
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
				owner->equipment->unequip(slot, *owner->container, owner->attacker.get());
				engine.gui->message(Colors::uiText, "Item unequipped.");
				engine.gameStatus = Engine::NEW_TURN;
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
	moveOrAttack(owner, engine.player->getX(), engine.player->getY());
}

void MonsterAi::moveOrAttack(Actor* owner, int targetX, int targetY)
{
	const int dx = targetX - owner->getX();
	const int dy = targetY - owner->getY();
	const float distance = sqrtf(static_cast<float>(dx * dx + dy * dy));

	if (distance < 2.0f) {
		// Adjacent — attack.
		if (owner->attacker) {
			owner->attacker->attack(owner, engine.player);
		}
		return;
	}

	if (engine.map->isInFOV(owner->getX(), owner->getY())) {
		// Player is visible — step directly toward them.
		const int stepX = static_cast<int>(std::round(dx / distance));
		const int stepY = static_cast<int>(std::round(dy / distance));
		if (engine.map->canWalk(owner->getX() + stepX, owner->getY() + stepY)) {
			owner->setX(owner->getX() + stepX);
			owner->setY(owner->getY() + stepY);
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
		owner->setX(owner->getX() + neighbourDX[bestNeighbour]);
		owner->setY(owner->getY() + neighbourDY[bestNeighbour]);
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
	RangedCombat::resolve(owner, target);
}

void RangedAi::reload(Actor* owner)
{
	Actor* weaponItem = owner->equipment ? owner->equipment->getSlot(EquipmentSlot::WEAPON) : nullptr;
	if (weaponItem && weaponItem->equippable && weaponItem->equippable->rangedStats) {
		weaponItem->equippable->currentAmmo = weaponItem->equippable->rangedStats->clipSize;
		engine.gui->message(Colors::uiText, "# reloads #.", owner->name, weaponItem->name);
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
