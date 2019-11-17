#pragma once

static constexpr int SCENT_THRESHOLD = 20; //amount of turns monser can smell scent

//Actor self updates
class Ai : public Persistent {
public:

	virtual void update(Actor* owner) = 0; //Updates Actor each turn
	static std::unique_ptr<Ai> create(TCODZip& zip); //factory function for loading Ai's
protected:
	//enum for derived classes for factory
	enum class AiType {
		PLAYER = 0,
		MONSTER,
		CONFUSED_MONSTER
	};

};

//parent class for Ai which replaces another Ai
class TemporaryAi : public Ai {
public:
	TemporaryAi(int nbTurns);
	void update(Actor* owner); // Action to take each turn
	virtual void applyTo(Actor* actor); // apply to actor to have it ai changed
	static auto create(TCODZip& zip); //factory function loading temp ai's
	virtual void save(TCODZip& zip) {}; //save Ai data to zip
	virtual void load(TCODZip& zip) {}; //load temp Ai data from Zi[
protected:
	int nbTurns;  // nubmer of turns tempory ai is active for
	std::shared_ptr<Ai> oldAi; //Ai which this Ai replaced
};

//handles user input and player movment
class PlayerAi : public Ai {
public:
	int xpLevel; //level of player character
	PlayerAi();
	int getNextLevelXp(); // return amount of xp required for next level
	void update(Actor* owner); //update player each turn
	void handleActionKey(Actor* owner, int ascii); //handle user input
	void save(TCODZip& zip); //save Ai to zip
	void load(TCODZip& zip); //load Ai from zip

protected:
	bool moveOrAttack(Actor* owner, int targetx, int targety);//handles player movement
	Actor* chooseFromInventory(Actor* owner); //opens up inventory screen and alows item to be used
};

//Tracks player scent then attacks
class MonsterAi : public Ai {
public:
	void update(Actor* owner); //update monster each turn
	void save(TCODZip& zip); //save monster Ai to zip
	void load(TCODZip& zip); ////save monster Ai to zip
protected:
	void moveOrAttack(Actor* owner, int targetx, int targety); //handles enemy movement
};

//Temporary Ai to make moster move randomly and attack anything in its path
class ConfusedMonsterAi : public TemporaryAi {
public:
	ConfusedMonsterAi(int nbTurns);
	void update(Actor* owner);
	void save(TCODZip& zip);  //save monster Ai to zip
	void load(TCODZip& zip); //load monster Ai to zip
};

