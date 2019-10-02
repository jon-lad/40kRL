#pragma once

class Ai {
public:
	virtual void update(Actor* owner) = 0;

};

class PlayerAi : public Ai {
public:
	void update(Actor* owner);
protected:
	bool moveOrAttack(Actor* owner, int targetx, int targety);
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

