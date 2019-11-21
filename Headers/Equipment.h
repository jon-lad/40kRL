#pragma once

//Actor can equip items
class Equipment : public Persistent{
protected:
	struct HumanoidLocations {
		std::unique_ptr<Actor> head;
		std::unique_ptr<Actor> body;
		std::unique_ptr<Actor> hands;
		std::unique_ptr<Actor> legs;
	};
public:
	Equipment();
	HumanoidLocations human;
	void save(TCODZip& zip);
	void load(TCODZip& zip);
	

	enum class EquipLocation {
		NONE,
		HEAD,
		BODY,
		HANDS,
		LEGS
	};

	bool equip(std::unique_ptr<Actor> owner, Actor* wearer);

};
