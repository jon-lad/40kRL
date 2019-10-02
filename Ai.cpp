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
		case TCODK_RIGHT:dx = 1; break;
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
	//look for corpses
	for (std::list<std::unique_ptr<Actor>>::iterator i = engine.actors.begin(); i != engine.actors.end(); ++i) {
		if (i->get()->destructible && i->get()->destructible->isDead() && i->get()->getX() == targetx && i->get()->getY() == targety) {
			engine.gui->message(TCODColor::lightGrey, "There's a %s here.", i->get()->name);
		}
	}
	owner->setX(targetx);
	owner->setY(targety);
	return true;
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