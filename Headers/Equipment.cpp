#include <memory>
#include <sstream>
#include "main.h"

Equipment::Equipment(){}

bool Equipment::equip(std::unique_ptr<Actor> actor, Actor* wearer) {
	switch (actor->pickable->equipLocation){
	case EquipLocation::NONE:
	{
		engine.gui->message(TCODColor::lightGrey, "You cannot equip the #", std::move(actor->name));
		wearer->container->add(std::move(actor));
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
		if (!human.body) {
			human.body = std::move(actor);
		}
		else {
			wearer->container->add(std::move(human.body));
			human.body = std::move(actor);
		}
	}
	break;
	case EquipLocation::HANDS:
	{
		if (!human.hands) {
			human.hands = std::move(actor);
		}
		else {
			wearer->container->add(std::move(human.hands));
			human.hands = std::move(actor);
		}
	}
	break;
	case EquipLocation::LEGS:
	{
		if (!human.legs) {
			human.legs = std::move(actor);
		}
		else {
			wearer->container->add(std::move(human.legs));
			human.legs = std::move(actor);
		}
	}
	break;
	}
	return true;
}