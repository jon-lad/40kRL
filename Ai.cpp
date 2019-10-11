#include <memory>
#include <list>
#include "main.h"

void PlayerAi::update(Actor* owner) {
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
		case TCODK_CHAR: handleActionKey(owner, engine.lastKey.c); break;
		default: break;
		}
		if (dx != 0 || dy != 0) {
			engine.gameStatus = Engine::NEW_TURN;
			if (moveOrAttack(owner, owner->getX() + dx, owner->getY() + dy)) {
				engine.map->computeFOV();
			}
		}
}


void MonsterAi::update(Actor* owner) {
		if (owner->destructible && owner->destructible->isDead()) {
			return;
		}
		if (engine.map->isInFOV(owner->getX(), owner->getY())) {
			// we can see the player. move towards him
			moveCount = TRACKING_TURNS;
		}
		else {
			moveCount--;
		}
		if (moveCount > 0) {
			moveOrAttack(owner, engine.player->getX(), engine.player->getY());
		}
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
			engine.gui->message(TCODColor::lightGrey, "There's a %s here.", i->get()->name);
		}
	}
	owner->setX(targetx);
	owner->setY(targety);
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
			con.printf(2, y, "(%c) %s", shortcut, i->get()->name);
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
		if (actorIndex >= 0 && actorIndex < owner->container->inventory.size()) {
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
		for (std::list<std::unique_ptr<Actor>>::iterator i = engine.actors.begin(); i != engine.actors.end(); ++i) {
			if (i->get()->pickable && i->get()->getX() == owner->getX() && i->get()->getY() == owner->getY()) {
				const char* itemName = i->get()->name;
				Actor* item = i->get();
				if (item->pickable->pick(std::move(*i), owner)) {
					found = true;
					engine.gui->message(TCODColor::lightGrey, "You pick up the %s", itemName);
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
	}
}

void MonsterAi::moveOrAttack(Actor * owner, int targetx, int targety) {
	int dx = targetx - owner->getX();
	int dy = targety - owner->getY();
	int stepdx = (dx > 0 ? 1 : -1);
	int stepdy = (dy > 0 ? 1 : -1);
	float distance = sqrtf((float)dx * (float)dx + (float)dy * (float)dy);
	if (distance >= 2) {
		dx = (int)(round(dx / distance));
		dy = (int)(round(dy / distance));
		if (engine.map->canWalk(owner->getX() + dx, owner->getY() + dy)) {
			owner->setX(owner->getX() + dx);
			owner->setY(owner->getY() + dy);
		}
		else if (engine.map->canWalk(owner->getX() + stepdx, owner->getY())) {
			owner->setX(owner->getX() + stepdx);
		}
		else if (engine.map->canWalk(owner->getX(), owner->getY() + stepdy)) {
			owner->setY(owner->getY() + stepdy);
		}
	}
	else if (owner->attacker) {
		owner->attacker->attack(owner, engine.player);
	}
}

ConfusedMonsterAi::ConfusedMonsterAi(int nbTurns, std::unique_ptr<Ai> oldAi): nbTurns{ nbTurns }, oldAi{ std::move(oldAi) } {
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
	nbTurns--;
	if (nbTurns==0) {
		owner->ai = std::move(oldAi);
	}

}