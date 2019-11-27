#pragma once

class TargetSelector;
class Effect;
class Actor;


using targetPtr_t = std::unique_ptr<TargetSelector>;
using effectPtr_t = std::unique_ptr<Effect>;
using actorPtr_t = std::unique_ptr<Actor>;
using equipLoc_t = Equipment::EquipLocation;

/*Target selector*/

class TargetSelector : public Persistent {
public:
	enum class SelectorType {
		SELF,
		CLOSEST_MONSTER,
		SELECTED_MONSTER,
		WEARER_RANGE,
		SELECTED_RANGE
	};
	TargetSelector(SelectorType type, double range);
	void selectTargets(Actor* wearer, TCODList<Actor*>& list);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
protected:
	SelectorType type;
	double range;
};

class Effect : public Persistent {
public:
	enum class EffectType {
		HEALTH,
		CHANGE_AI,
		CHANGE_STAT
	};
	virtual bool applyTo(Actor* actor) = 0;
	static effectPtr_t create(TCODZip& zip);
};



class Pickable : public Persistent {
public:
	Pickable(targetPtr_t selector, effectPtr_t effect, equipLoc_t equipLocation);
	bool pick(actorPtr_t owner, Actor* wearer);
	bool equip(actorPtr_t owner, Actor* wearer);
	bool use(Actor* owner, Actor* wearer);
	void drop(Actor* owner, Actor* wearer);
	void save(TCODZip& zip);
	void load(TCODZip& zip);

	equipLoc_t equipLocation;
	

protected:
	targetPtr_t selector;
	effectPtr_t effect;
	
	
};






class HealthEffect : public Effect {
public:
	double amount;
	std::string message;
	TCODColor textCol;
	HealthEffect(double amount, std::string_view message, const TCODColor& textCol);
	bool applyTo(Actor* owner);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
};


class AiChangeEffect : public Effect {
public:
	std::unique_ptr<TemporaryAi> newAi;
	std::string message;
	TCODColor textCol;
	

	AiChangeEffect(std::unique_ptr<TemporaryAi> newAi, std::string_view message, const TCODColor& textCol);
	bool applyTo(Actor* actor);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
};