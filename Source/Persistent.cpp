#include <sstream>
#include <filesystem>
#include "main.h"

/*Engine*/
void Engine::save() {
	if (player->destructible->isDead()) {
		remove("game.sav");
	}
	else {
		TCODZip zip;
		//save the map first
		zip.putInt(map->getWidth());
		zip.putInt(map->getHeight());
		map->save(zip);
		//then the camera
		camera->save(zip);
		//then the player
		player->save(zip);
		//then the stair
		stairs->save(zip);
		//then the rest of the actors
		zip.putInt((int)actors.size() - 2);
		for (auto i = actors.begin(); i != actors.end(); i++) {
			if (i->get() != player && i->get() != stairs) {
				i->get()->save(zip);
			}
		}
		//finally the message log
		gui->save(zip);
		zip.saveToFile("game.sav");
	}
}

void Engine::load() {
	engine.gui->menu.clear();
	engine.gui->menu.addItem(Menu::MenuItemCode::NEW_GAME, "New Game");
	if (std::filesystem::exists("game.sav")) {
		engine.gui->menu.addItem(Menu::MenuItemCode::CONTINUE, "Continue");
	}
	engine.gui->menu.addItem(Menu::MenuItemCode::EXIT, "Exit");
	Menu::MenuItemCode menuItem = engine.gui->menu.pick();
	if (menuItem == Menu::MenuItemCode::EXIT || menuItem == Menu::MenuItemCode::NONE) {
		//Exit or window closed
		exit(0);
	}
	else if (menuItem == Menu::MenuItemCode::NEW_GAME) {
		//New Game
		engine.term();
		engine.init();
	}
	else {
		TCODZip zip;
		//continue from saved game
		engine.term();
		zip.loadFromFile("game.sav");
		//load the map
		int width = zip.getInt();
		int height = zip.getInt();
		map = std::make_unique<Map>(width, height);
		map->load(zip);
		//then the camera
		camera = std::make_unique<Camera>(0, 0, 80, 43, map->getWidth(), map->getHeight());
		camera->load(zip);
		//then the player
		std::unique_ptr<Actor> newPlayer = std::make_unique<Actor>(0, 0, 0, "", TCOD_white);
		player = newPlayer.get();
		actors.emplace_front(std::move(newPlayer));
		player->load(zip);
		//the stairs
		std::unique_ptr<Actor> newStair = std::make_unique<Actor>(0, 0, 0, "", TCOD_white);
		stairs = newStair.get();
		actors.emplace_front(std::move(newStair));
		stairs->load(zip);
		//then all the other actors
		int nbActors = zip.getInt();
		while (nbActors > 0) {
			std::unique_ptr<Actor> actor = std::make_unique<Actor>(0, 0, 0, "", TCOD_white);
			actor->load(zip);
			actors.emplace_front(std::move(actor));
			nbActors--;
		}
		//finaly the log
		gui->load(zip);
		// to force Fov recomputation
		gameStatus = STARTUP;
	}
	
	
}

/*Map*/
void Map::save(TCODZip& zip) {
	zip.putInt(seed);
	for (auto i = tiles.begin(); i != tiles.end(); ++i) {
		zip.putInt(i->explored);
		zip.putInt(i->scent);
		
	}
	zip.putInt(currentScentValue);
}

void Map::load(TCODZip& zip) {
	seed = zip.getInt();
	init(false);
	for (auto i = tiles.begin(); i != tiles.end(); ++i) {
		i->explored = (bool)zip.getInt();
		i->scent = zip.getInt();
		
	}
	currentScentValue = zip.getInt();
}

/*Actors*/
void Actor::save(TCODZip& zip)
{
	//basic Properties
	zip.putInt(x);
	zip.putInt(y);
	zip.putInt(ch);
	zip.putColor(&col);
	zip.putString(name.c_str());
	zip.putInt(blocks);
	zip.putInt(fovOnly);
	//bool for each feature
	zip.putInt(attacker != 0);
	zip.putInt(destructible != 0);
	zip.putInt(ai != 0);
	zip.putInt(pickable != 0);
	zip.putInt(container != 0);
	//save the features themselves
	if (attacker) attacker->save(zip);
	if (destructible) destructible->save(zip);
	if (ai) ai->save(zip);
	if (pickable) pickable->save(zip);
	if (container) container->save(zip);
}

void Actor::load(TCODZip& zip)
{
	//load properties
	x = zip.getInt();
	y = zip.getInt();
	ch = zip.getInt();
	col = zip.getColor();
	name = zip.getString();
	blocks = zip.getInt();
	fovOnly = zip.getInt();
	//load feature booleans
	bool hasAttacker = zip.getInt();
	bool hasDestructible = zip.getInt();
	bool hasAi = zip.getInt();
	bool hasPickable = zip.getInt();
	bool hasContainer = zip.getInt();
	//load features
	if (hasAttacker) {
		attacker = std::make_unique<Attacker>(0.0f);
		attacker->load(zip);
	}
	if (hasDestructible) {
		destructible = std::move(Destructible::create(zip));
	}
	if (hasAi) {
		ai = std::move(Ai::create(zip));
	}
	if (hasPickable) {
		pickable = std::make_unique<Pickable>(std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF,0.0f), Effect::create(zip));
		pickable->load(zip);
	}
	if (hasContainer) {
		container = std::make_unique<Container>(0);
		container->load(zip);
	}
}

/*attacker*/

void Attacker::save(TCODZip& zip) {
	zip.putFloat(power);
}

void Attacker::load(TCODZip& zip) {
	power = zip.getFloat();
}

/*Detructable*/
void Destructible::save(TCODZip& zip) {
	zip.putFloat(maxHp);
	zip.putFloat(hp);
	zip.putFloat(defense);
	zip.putString(corpseName.c_str());
	zip.putInt(xp);
}

void Destructible::load(TCODZip& zip) {
	maxHp = zip.getFloat();
	hp = zip.getFloat();
	defense = zip.getFloat();
	corpseName = zip.getString();
	xp = zip.getInt();
}

std::unique_ptr<Destructible> Destructible::create(TCODZip& zip) {
	DestructibleType type = (Destructible::DestructibleType)zip.getInt();
	std::unique_ptr<Destructible> destructible;
	switch (type)
	{
	case DestructibleType::MONSTER:
		destructible = std::make_unique<MonsterDestructible>(0, 0.0f, "", 0);
		break;
	case DestructibleType::PLAYER:
		destructible = std::make_unique<PlayerDestructible>(0, 0, "", 0);
		break;
	}
	destructible->load(zip);
	return std::move(destructible);
}

void PlayerDestructible::save(TCODZip& zip) {
	zip.putInt((int)DestructibleType::PLAYER);
	Destructible::save(zip);
}

void MonsterDestructible::save(TCODZip& zip) {
	zip.putInt((int)DestructibleType::MONSTER);
	Destructible::save(zip);
}

/*Ai*/
std::unique_ptr<Ai> Ai::create(TCODZip& zip) {
	const auto& type = (Ai::AiType)zip.getInt();
	std::unique_ptr<Ai> ai;
	switch (type) {
	case(AiType::PLAYER):
		ai = std::make_unique<PlayerAi>(); break;
	case(AiType::MONSTER):
		ai = std::make_unique<MonsterAi>(); break;
	default: //case(AiType::CONFUSED_MONSTER):
		ai = std::make_unique<ConfusedMonsterAi>(0); break;
	}
	ai->load(zip);
	return std::move(ai);
}

auto TemporaryAi::create(TCODZip& zip) {
	const auto& type = (Ai::AiType)zip.getInt();
	std::unique_ptr<TemporaryAi> ai;
	switch (type) {
	case(Ai::AiType::CONFUSED_MONSTER):
		ai = std::make_unique<ConfusedMonsterAi>(0); break;
	}
	ai->load(zip);
	return std::move(ai);
}

void PlayerAi::save(TCODZip& zip) {
	zip.putInt((int)AiType::PLAYER);
	zip.putInt(xpLevel);
}

void PlayerAi::load(TCODZip& zip) {
	xpLevel = zip.getInt();
}

void MonsterAi::save(TCODZip& zip) {
	zip.putInt((int)AiType::MONSTER);
	
}

void MonsterAi::load(TCODZip& zip) {
	
}

void ConfusedMonsterAi::save(TCODZip& zip) {
	zip.putInt((int)Ai::AiType::CONFUSED_MONSTER);
	zip.putInt(nbTurns);
	zip.putInt(oldAi != 0);
	if (oldAi) {
		oldAi->save(zip);
	}
}

void ConfusedMonsterAi::load(TCODZip& zip) {
	nbTurns = zip.getInt();
	bool hasOldAi = zip.getInt();
	if (hasOldAi) {
	oldAi = std::move(Ai::create(zip));
	}
}

/*Pickable*/
void Pickable::save(TCODZip& zip) {
	effect->save(zip);
	zip.putInt(selector != 0);
	if (selector) {
		selector->save(zip);
	}
	
}

void Pickable::load(TCODZip& zip) {
	bool hasSelector = zip.getInt();
	if (hasSelector) {
		 
		selector = std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0);
		selector->load(zip);
	}
	

}

void TargetSelector::save(TCODZip& zip) {
	zip.putInt((int)type);
	zip.putFloat(range);
}

void TargetSelector::load(TCODZip& zip) {
	type = (SelectorType)zip.getInt();
	range = zip.getFloat();
}

std::unique_ptr<Effect> Effect::create(TCODZip& zip) {
	const auto& type = (EffectType)zip.getInt();
	std::unique_ptr<Effect> effect;
	switch (type) {
	case EffectType::HEALTH:
		effect = std::make_unique<HealthEffect>(0, "", TCOD_light_grey);
		break;
	case EffectType::CHANGE_AI:
		effect = std::make_unique<AiChangeEffect>(std::make_unique<TemporaryAi>(0), " ", TCOD_light_grey);
		break;
	}
	effect->load(zip);
	return std::move(effect);
}

void HealthEffect::save(TCODZip& zip) {
	zip.putInt((int)EffectType::HEALTH);
	zip.putFloat(amount);
	zip.putString(message.c_str());
	zip.putColor(&textCol);
}

void HealthEffect::load(TCODZip& zip) {
	amount = zip.getFloat();
	message = zip.getString();
	textCol = zip.getColor();
}

void AiChangeEffect::save(TCODZip& zip) {
	zip.putInt((int)EffectType::CHANGE_AI);
	
	newAi->save(zip);
	zip.putString(message.c_str());
	zip.putColor(&textCol);
}

void AiChangeEffect::load(TCODZip& zip) {
	
	newAi = std::move(TemporaryAi::create(zip));
	message = zip.getString();
	textCol = zip.getColor();
}

/*Container*/
void Container::save(TCODZip& zip) {
	zip.putInt(size);
	zip.putInt((int)inventory.size());
	for (auto i = inventory.begin(); i != inventory.end(); ++i) {
		i->get()->save(zip);
	}
}

void Container::load(TCODZip& zip) {
	size = zip.getInt();
	int nbActors = zip.getInt();
	while (nbActors > 0) {
		auto actor = std::make_unique<Actor>(0, 0, 0, " ", TCOD_white);
		actor->load(zip);
		inventory.emplace_back(std::move(actor));
		nbActors--;
	}

}

/*Gui*/

void Gui::save(TCODZip& zip) {
	zip.putInt((int)log.size());
	for (auto i = log.begin(); i != log.end(); i++) {
		zip.putString(i->get()->text.c_str());
		zip.putColor(&i->get()->col);
	}
}

void Gui::load(TCODZip& zip) {
	int nbMessages = zip.getInt();
	while (nbMessages > 0) {
		std::string text = zip.getString();
		TCODColor col = zip.getColor();
		message(col, text);
		nbMessages--;
	}

}

void Camera::save(TCODZip& zip) {
	zip.putInt(x); 
	zip.putInt(y);
	zip.putInt(width);
	zip.putInt(height);
	zip.putInt(mapWidth);
	zip.putInt(mapHeight);
}

void Camera::load(TCODZip& zip) {
	x = zip.getInt();
	y = zip.getInt();
	width = zip.getInt(); 
	height = zip.getInt();
	mapWidth = zip.getInt();
	mapHeight = zip.getInt();
}