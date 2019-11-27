#pragma once
//actor can take damage and be destroyed
class Destructible : public Persistent {
public:
	double maxHp; // maximum Health points
	double hp; // current health points
	double defense; //hit point deflected
	std::string corpseName; // the actors name once dead
	int xp; //xp for killing monster or player xp

	Destructible(double maxHp, double defense, std::string_view corpseName, int xp);
	
	//return if actor has died
	inline bool isDead() {
		return hp <= 0;
	}


	double takeDamage(Actor* owner, double damage); //if actor is attacked work out how much hp is lost
	double heal(int amount); //restore hp to actor

	virtual void die(Actor* owner); //what actor does when it dies

	virtual void save(TCODZip& zip); //save destructible data to zip
	virtual void load(TCODZip& zip); //load destructible data from zip
	static std::unique_ptr<Destructible> create(TCODZip& zip); //factory function for loading destructibles
	
protected:
	//enum for factory function
	enum class DestructibleType {
		MONSTER = 0,
		PLAYER
	};
};

//monster destructible dies and turns into corpse
class MonsterDestructible : public Destructible {
public:
	MonsterDestructible(double maxHp, double defense, std::string_view corpseName, int xp);
	void die(Actor* owner);//turns actor into corpse
	void save(TCODZip& zip);//save destructible data to zip
	
};

//player dies and game ends
class PlayerDestructible : public Destructible {
public:
	PlayerDestructible(double maxHp, double defense, std::string_view corpseName, int xp);
	void die(Actor* owner);//ends game
	void save(TCODZip& zip);//save destructible data to zip
	
};


