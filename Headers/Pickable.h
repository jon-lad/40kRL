#pragma once

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
	TargetSelector(SelectorType type, float range);
	void selectTargets(Actor* wearer, TCODList<Actor*>& list);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
protected:
	SelectorType type;
	float range;
};

class Effect : public Persistent {
public:
	enum class EffectType {
		HEALTH,
		CHANGE_AI
	};
	virtual bool applyTo(Actor* actor) = 0;
	static std::unique_ptr<Effect> create(TCODZip& zip);
};



class Pickable : public Persistent {
public:
	Pickable(std::unique_ptr<TargetSelector> selector, std::unique_ptr<Effect> effect, Equipment::EquipLocation equipLocation);
	bool pick(std::unique_ptr<Actor> owner, Actor* wearer);
	bool equip(std::unique_ptr<Actor>, Actor* wearer);
	bool use(Actor* owner, Actor* wearer);
	void drop(Actor* owner, Actor* wearer);
	void save(TCODZip& zip);
	void load(TCODZip& zip);

	Equipment::EquipLocation equipLocation;

protected:
	std::unique_ptr<TargetSelector> selector;
	std::unique_ptr<Effect> effect;
	
};






class HealthEffect : public Effect {
public:
	float amount;
	std::string message;
	TCODColor textCol;
	HealthEffect(float amount, std::string_view message, const TCODColor& textCol);
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