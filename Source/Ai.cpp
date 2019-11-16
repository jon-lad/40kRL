
#include <memory>
#include <list>
#include<sstream>
#include "main.h"

void PlayerAi::update(Actor* owner) {
	int levelUpXp = getNextLevelXp();
	if (owner->destructible->xp >= levelUpXp) {
		xpLevel++;
		owner->destructible->xp -= levelUpXp;
		std::stringstream ss;
		ss << "Your battle skills grow stronger! You reached level " << xpLevel << ".";
		engine.gui->message(TCOD_yellow, ss.str());
		engine.gui->menu.clear();
		engine.gui->menu.addItem(Menu::MenuItemCode::CONSTITUTION, "Constitution (+20HP)");
		engine.gui->menu.addItem(Menu::MenuItemCode::STRENGTH, "Strength (+1 Attack)");
		engine.gui->menu.addItem(Menu::MenuItemCode::AGILITY, "Agility (+1 Defense)");
		Menu::MenuItemCode menuItem = engine.gui->menu.pick(Menu::DisplayMode::PAUSE);
		switch (menuItem) {
			case Menu::MenuItemCode::CONSTITUTION:
				owner->destructible->maxHp += 20;
				owner->destructible->hp += 20;
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
	if (owner->destructible && owner->destructible->isDead()) {
		return;
	}
	int dx = 0, dy = 0;
	switch (engine.lastKey.vk)
	{
	case TCODK_UP: dy = -1; break;
	case TCODK_DOWN: dy = 1; break;
	case TCODK_LEFT: dx = -1; break;
	case TCODK_RIGHT: dx = 1; break;
	case TCODK_TEXT: handleActionKey(owner, *engine.lastKey.text); break;
	default: break;
	}
	if (dx != 0 || dy != 0) {
		engine.gameStatus = Engine::NEW_TURN;
		if (moveOrAttack(owner, owner->getX() + dx, owner->getY() + dy)) {
			engine.map->computeFOV();
		}
	}
}

PlayerAi::PlayerAi(): xpLevel{1}{}

static constexpr int LEVEL_UP_BASE = 200;
static constexpr int LEVEL_UP_FACTOR = 150;

int PlayerAi::getNextLevelXp() {
	return LEVEL_UP_BASE + xpLevel * LEVEL_UP_FACTOR;
}


void MonsterAi::update(Actor* owner) {
		if (owner->destructible && owner->destructible->isDead()) {
			return;
		}
		moveOrAttack(owner, engine.player->getX(), engine.player->getY());
}


bool PlayerAi::moveOrAttack(Actor* owner, int targetx, int targety)
{
	if (engine.map->isWall(targetx, targety)) { return false; }
	//look for living targets
	for (std::list<std::unique_ptr<Actor>>::iterator i = engine.actors.begin(); i != engine.actors.end(); ++i) {
		if (i->get()->destructible && !i->get()->destructible->isDead() && i->get()->getX() == targetx && i->get()->getY() == targety) {
			owner->attacker->attack(owner,i->get());
			return false;
		}
	}
	//look for corpses or items
	for (std::list<std::unique_ptr<Actor>>::iterator i = engine.actors.begin(); i != engine.actors.end(); ++i) {
		bool corpseOrItem = i->get()->destructible && i->get()->destructible->isDead() || i->get()->pickable;
		if (corpseOrItem && i->get()->getX() == targetx && i->get()->getY() == targety) {
			
			engine.gui->message(TCODColor::lightGrey, "Theres a # here", std::move(i->get()->name));
		}
	}
	owner->setX(targetx);
	owner->setY(targety);
	engine.camera->update(owner);
	return true;
}

Actor* PlayerAi::chooseFromInventory(Actor* owner) {
	static constexpr int INVENTORY_WIDTH = 50;
	static constexpr int INVENTORY_HEIGHT = 28;
	static TCODConsole con(INVENTORY_WIDTH, INVENTORY_HEIGHT);

	//Display the inventory frame
	con.setDefaultForeground(TCODColor(200, 180, 50));
	con.printFrame(0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, true, TCOD_BKGND_DEFAULT, "inventory");
	//Display the items with their keyboard shortcuts
	con.setDefaultForeground(TCOD_white);
	int shortcut = 'a';
	int y = 1;
	for (std::list<std::unique_ptr<Actor>>::iterator i = owner->container->inventory.begin(); i != owner->container->inventory.end(); ++i) {
		if (*i) {
			con.printf(2, y, "(%c) %s", shortcut, i->get()->name.c_str());
			y++;
			shortcut++;
		}
	}
	//blit the inventory console on the root console
	TCODConsole::blit(&con, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, TCODConsole::root, engine.screenWidth/2 - INVENTORY_WIDTH/2,
		engine.screenHeight/2 - INVENTORY_HEIGHT/2);
	TCODConsole::flush();
	//wait for a key press
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
	if (key.vk == TCODK_CHAR) {
		int actorIndex = key.c - 'a';
		if (actorIndex >= 0 && actorIndex < (int) owner->container->inventory.size()) {
			std::list<std::unique_ptr<Actor>>::iterator it = owner->container->inventory.begin();
			std::advance(it, actorIndex);
			return it->get();
		}
	}
	return NULL;
}

void PlayerAi::handleActionKey(Actor* owner, int ascii) {
	switch (ascii) {
	case 'g': //pickup item
	{
		bool found = false;
		for (auto actor = engine.actors.begin(); actor != engine.actors.end(); ++actor) {
			if (actor->get()->pickable && actor->get()->getX() == owner->getX() && actor->get()->getY() == owner->getY()) {
				Actor* item = actor->get();
				if (item->pickable->pick(std::move(*actor), owner)) {
					found = true;
					engine.gui->message(TCODColor::lightGrey, "You pick up the #.", std::move(item->name));
					auto i = engine.actors.begin();
					auto e = engine.actors.end();
					while (i != e) {
						if (i->get() == nullptr) {
							i = engine.actors.erase(i);
						}
						else { i++; }
					}
					break;
				}
				else if (!found) {
					found = true;
					engine.gui->message(TCOD_red, "Your inventory is full!");
				}
			}
		}
		if (!found) {
			engine.gui->message(TCODColor::lightGrey, "There is nothing here to pick up.");
		}
		engine.gameStatus = Engine::NEW_TURN;
	}
	break;
	case 'i'://inventory
	{
		Actor* item = chooseFromInventory(owner);
		if (item) {
			for (auto i = owner->container->inventory.begin(); i != owner->container->inventory.end(); ++i) {
				if (i->get() == item) {
					item->pickable->use(i->get(), owner);
					break;
				}
			}
			engine.gameStatus = Engine::NEW_TURN;
		}
	}
	break;
	case 'd'://drop
	{
		Actor* item = chooseFromInventory(owner);
		if (item) {
			item->pickable->drop(item, owner);
			engine.gameStatus = Engine::NEW_TURN;
		}
	}
	break;
	case '>'://go down stairs
		if (engine.stairs->getX() == owner->getX() && engine.stairs->getY() == owner->getY()) {
			engine.nextLevel();
		}
		else {
			engine.gui->message(TCOD_light_grey, "There are no stairs here.");
		}
		break;
	}
}

void MonsterAi::moveOrAttack(Actor * owner, int targetx, int targety) {
	int dx = targetx - owner->getX();
	int dy = targety - owner->getY();
	int stepdx = (dx > 0 ? 1 : -1);
	int stepdy = (dy > 0 ? 1 : -1);
	float distance = sqrtf((float)dx * (float)dx + (float)dy * (float)dy);
	if (distance < 2) {
		//at melee range attack
		if (owner->attacker) {
			owner->attacker->attack(owner, engine.player);
		}
		return;
	}
	else if (engine.map->isInFOV(owner->getX(), owner->getY())) {
		//player in sight go towards him
		dx = (int)std::round(dx / distance);
		dy = (int)std::round(dx / distance);
		if (engine.map->canWalk(owner->getX() + dx, owner->getY() + dy)) {
			owner->setX(owner->getX() + dx);
			owner->setY(owner->getY() + dy);
			return;
		}
	}
	//player not visible use scent tracking
	//find the adjacent cell with highest scent level
	unsigned int bestLevel = 0;
	int bestCellIndex = -1;
	static constexpr int tdx[8] = { -1,0,1,-1,1,-1,0,1 };
	static constexpr int tdy[8] = { -1,-1,-1,0,0,1,1,1 };
	for (int i = 0; i < 8; i++) {
		int cellx = owner->getX() + tdx[i];
		int celly = owner->getY() + tdy[i];
		if (engine.map->canWalk(cellx, celly)) {
			unsigned int cellScent = engine.map->getScent(cellx, celly);
			if (cellScent > engine.map->currentScentValue - SCENT_THRESHOLD && cellScent > bestLevel) {
				bestLevel = cellScent;
				bestCellIndex = i;
			}
		}
	}
	if (bestCellIndex != -1) {
		//the monster smells the player follow the scent
		owner->setX(owner->getX() + tdx[bestCellIndex]);
		owner->setY(owner->getY() + tdy[bestCellIndex]);

	}
	

}

ConfusedMonsterAi::ConfusedMonsterAi(int nbTurns): TemporaryAi(nbTurns) {
}

void ConfusedMonsterAi::update(Actor* owner) {
	TCODRandom* rng = TCODRandom::getInstance();
	int dx = rng->getInt(-1, 1);
	int dy = rng->getInt(-1, 1);
	if (dx != 0 || dy != 0) {
		int destx = owner->getX() + dx;
		int desty = owner->getY() + dy;
		if (engine.map->canWalk(destx, desty)) {
			owner->setX(destx);
			owner->setY(desty);
		}
		else {
			Actor* actor = engine.getActor(destx, desty);
			if (actor) {
				owner->attacker->attack(owner, actor);
			}
		}
	}
	TemporaryAi::update(owner);

}

TemporaryAi::TemporaryAi(int nbTurns) : nbTurns{ nbTurns } {
}

void TemporaryAi::update(Actor* owner) {
	nbTurns--;
	if (nbTurns == 0) {
		owner->ai = std::move(oldAi);

	}
}

void TemporaryAi::applyTo(Actor* actor) {
	oldAi = std::move(actor->ai);
	std::unique_ptr<TemporaryAi> tempai(this);
	actor->ai = std::move(tempai);
}
