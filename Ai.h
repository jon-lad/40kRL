#pragma once

class Ai : public Persistent {
public:
	virtual void update(Actor* owner) = 0;
	static std::unique_ptr<Ai> create(TCODZip& zip);
protected:
	enum class AiType {
		PLAYER = 0,
		MONSTER,
		CONFUSED_MONSTER
	};

};

class PlayerAi : public Ai {
public:
	int xpLevel;
	PlayerAi();
	int getNextLevelXp();
	void update(Actor* owner);
	void handleActionKey(Actor* owner, int ascii);
	void save(TCODZip& zip);
	void load(TCODZip& zip);

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
	void save(TCODZip& zip);
	void load(TCODZip& zip);
protected:
	void moveOrAttack(Actor* owner, int targetx, int targety);
	int moveCount;
};

class ConfusedMonsterAi : public Ai {
public:
	ConfusedMonsterAi(int nbTurns, std::unique_ptr<Ai> oldAi);
	void update(Actor* owner);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
protected:
	int nbTurns;
	std::unique_ptr<Ai> oldAi;
};

