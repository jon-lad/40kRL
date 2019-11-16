#pragma once

static constexpr int SCENT_THRESHOLD = 20; //amount of turns monser can smell scent


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

class TemporaryAi : public Ai {
public:
	TemporaryAi(int nbTurns);
	void update(Actor* owner);
	virtual void applyTo(Actor* actor);
	static auto create(TCODZip& zip);
	virtual void save(TCODZip& zip) {};
	virtual void load(TCODZip& zip) {};
protected:
	int nbTurns;
	std::shared_ptr<Ai> oldAi;
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
	static const auto TRACKING_TURNS = 3;
	void save(TCODZip& zip);
	void load(TCODZip& zip);
protected:
	void moveOrAttack(Actor* owner, int targetx, int targety);
};

class ConfusedMonsterAi : public TemporaryAi {
public:
	ConfusedMonsterAi(int nbTurns);
	void update(Actor* owner);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
};

