
#include <memory>
#include <list>
#include <sstream>
#include "main.h"

// ─── PlayerAi ────────────────────────────────────────────────────────────────

PlayerAi::PlayerAi() : xpLevel{ 1 } {}

static constexpr int LEVEL_UP_BASE   = 200;
static constexpr int LEVEL_UP_FACTOR = 150;

int PlayerAi::getNextLevelXp()
{
	return LEVEL_UP_BASE + xpLevel * LEVEL_UP_FACTOR;
}

void PlayerAi::update(Actor* owner)
{
	if (owner->destructible && owner->destructible->isDead()) { return; }

	// Check for level-up before processing movement so the stat bonus applies immediately.
	const int levelUpXp = getNextLevelXp();
	if (owner->destructible->xp >= levelUpXp) {
		xpLevel++;
		owner->destructible->xp -= levelUpXp;

		std::stringstream ss;
		ss << "Your battle skills grow stronger! You reached level " << xpLevel << ".";
		engine.gui->message(TCOD_yellow, ss.str());

		engine.gui->menu.clear();
		engine.gui->menu.addItem(Menu::MenuItemCode::CONSTITUTION, "Constitution (+20HP)");
		engine.gui->menu.addItem(Menu::MenuItemCode::STRENGTH,     "Strength (+1 Attack)");
		engine.gui->menu.addItem(Menu::MenuItemCode::AGILITY,      "Agility (+1 Defense)");
		const Menu::MenuItemCode choice = engine.gui->menu.pick(Menu::DisplayMode::PAUSE);
		switch (choice) {
			case Menu::MenuItemCode::CONSTITUTION:
				owner->destructible->maxHp += 20;
				owner->destructible->hp    += 20;
				break;
			case Menu::MenuItemCode::STRENGTH:
				owner->attacker->power += 1;
				break;
			case Menu::MenuItemCode::AGILITY:
				owner->destructible->defense += 1;
				break;
			default: break;
		}
	}

	int dx = 0, dy = 0;
	switch (engine.lastKey.vk) {
		case TCODK_UP:    dy = -1; break;
		case TCODK_DOWN:  dy =  1; break;
		case TCODK_LEFT:  dx = -1; break;
		case TCODK_RIGHT: dx =  1; break;
		case TCODK_TEXT:  handleActionKey(owner, *engine.lastKey.text); break;
		default: break;
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
			engine.gui->message(TCODColor::lightGrey, "Theres a # here", actor->name);
		}
	}

	owner->setX(targetX);
	owner->setY(targetY);
	engine.camera->update(owner);
	return true;
}

Actor* PlayerAi::chooseFromInventory(Actor* owner)
{
	static constexpr int INVENTORY_WIDTH  = 50;
	static constexpr int INVENTORY_HEIGHT = 28;
	static TCODConsole inventoryConsole(INVENTORY_WIDTH, INVENTORY_HEIGHT);

	inventoryConsole.setDefaultForeground(TCODColor(200, 180, 50));
	inventoryConsole.printFrame(0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, true, TCOD_BKGND_DEFAULT, "inventory");

	inventoryConsole.setDefaultForeground(TCOD_white);
	int shortcutKey = 'a';
	int row = 1;
	for (const auto& itemPtr : owner->container->inventory) {
		if (itemPtr) {
			inventoryConsole.printf(2, row, "(%c) %s", shortcutKey, itemPtr->name.c_str());
			row++;
			shortcutKey++;
		}
	}

	TCODConsole::blit(&inventoryConsole, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT,
		TCODConsole::root,
		engine.screenWidth  / 2 - INVENTORY_WIDTH  / 2,
		engine.screenHeight / 2 - INVENTORY_HEIGHT / 2);
	TCODConsole::flush();

	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, nullptr, true);
	if (key.vk == TCODK_CHAR) {
		const int itemIndex = key.c - 'a';
		if (itemIndex >= 0 && itemIndex < static_cast<int>(owner->container->inventory.size())) {
			auto it = owner->container->inventory.begin();
			std::advance(it, itemIndex);
			return it->get();
		}
	}
	return nullptr;
}

void PlayerAi::handleActionKey(Actor* owner, int ascii)
{
	switch (ascii) {
	case 'g': // pick up item on current tile
	{
		bool pickedUp = false;
		for (auto actor = engine.actors.begin(); actor != engine.actors.end(); ++actor) {
			Actor* item = actor->get();
			if (item->pickable && item->getX() == owner->getX() && item->getY() == owner->getY()) {
				if (item->pickable->pick(std::move(*actor), owner)) {
					pickedUp = true;
					engine.gui->message(TCODColor::lightGrey, "You pick up the #.", item->name);
					// Erase the now-null slot left by the move.
					auto i = engine.actors.begin();
					while (i != engine.actors.end()) {
						i = (i->get() == nullptr) ? engine.actors.erase(i) : std::next(i);
					}
					break;
				} else if (!pickedUp) {
					pickedUp = true; // suppress the "nothing here" message
					engine.gui->message(TCOD_red, "Your inventory is full!");
				}
			}
		}
		if (!pickedUp) {
			engine.gui->message(TCODColor::lightGrey, "There is nothing here to pick up.");
		}
		engine.gameStatus = Engine::NEW_TURN;
		break;
	}

	case 'i': // open inventory to use an item
	{
		Actor* item = chooseFromInventory(owner);
		if (item) {
			for (auto& slot : owner->container->inventory) {
				if (slot.get() == item) {
					item->pickable->use(slot.get(), owner);
					break;
				}
			}
			engine.gameStatus = Engine::NEW_TURN;
		}
		break;
	}

	case 'd': // drop an item from inventory
	{
		Actor* item = chooseFromInventory(owner);
		if (item) {
			item->pickable->drop(item, owner);
			engine.gameStatus = Engine::NEW_TURN;
		}
		break;
	}

	case '>': // descend stairs
		if (engine.stairs->getX() == owner->getX() && engine.stairs->getY() == owner->getY()) {
			engine.nextLevel();
		} else {
			engine.gui->message(TCOD_light_grey, "There are no stairs here.");
		}
		break;
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
