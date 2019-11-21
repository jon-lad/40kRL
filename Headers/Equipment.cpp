#include <memory>
#include <sstream>
#include "main.h"

Equipment::Equipment(){}

bool Equipment::equip(std::unique_ptr<Actor> actor, Actor* wearer) {
	switch (actor->pickable->equipLocation){
	case EquipLocation::NONE:
	{
		return false;
	}
	break;
	case EquipLocation::HEAD:
	{
		if (!human.head) {
			human.head = std::move(actor);
		}
		else {
			wearer->container->add(std::move(human.head));
			human.head = std::move(actor);
		}
	}
	break;
	case EquipLocation::BODY:
	{
		human.body = std::move(actor);
	}
	break;
	case EquipLocation::HANDS:
	{
		human.hands = std::move(actor);
	}
	break;
	case EquipLocation::LEGS:
	{
		human.legs = std::move(actor);
	}
	break;
	}
	return true;
}