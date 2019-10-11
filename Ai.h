#pragma once

class Ai {
public:
	virtual void update(Actor* owner) = 0;

};

class PlayerAi : public Ai {
public:
	void update(Actor* owner);
	void handleActionKey(Actor* owner, int ascii);
protected:
	bool moveOrAttack(Actor* owner, int targetx, int targety);
	Actor* chooseFromInventory(Actor* owner);
};

class MonsterAi : public Ai {
public:
	void update(Actor* owner);
	// how many turns the monster chases the player
	// after losing his sight
	static const int TRACKING_TURNS = 3;
protected:
	void moveOrAttack(Actor* owner, int targetx, int targety);
	int moveCount;
};

class ConfusedMonsterAi : public Ai {
public:
	ConfusedMonsterAi(int nbTurns, std::unique_ptr<Ai> oldAi);
	void update(Actor* owner);
protected:
	int nbTurns;
	std::unique_ptr<Ai> oldAi;
};

