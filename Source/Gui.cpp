
#include <list>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <tuple>
#include "main.h"




Gui::Gui() {
	con = std::make_unique<TCODConsole>(engine.screenWidth, constants::PANEL_HEIGHT);
}

void Gui::render() {
	//clear the gui console
	con->setDefaultBackground(TCODColor::black);
	con->clear();
	//draw the health bar
	renderBar(1, 1, constants::BAR_WIDTH, "HP", engine.player->destructible->hp, engine.player->destructible->maxHp, TCOD_light_red, TCOD_darker_red);
	//draw xp bar
	PlayerAi* ai = (PlayerAi*) engine.player->ai.get();
	std::stringstream ss;
	if (ai) {
		ss << "XP(" << ai->xpLevel << ")";
		renderBar(1, 5, constants::BAR_WIDTH, ss.str(), (float)engine.player->destructible->xp, (float)ai->getNextLevelXp(), TCOD_light_violet, TCOD_darker_violet);
	}
	// draw the message log
	int y = 1;
	float colorCoef = 0.4f;
	for (const auto& message : log) {
		con->setDefaultForeground(message->col* colorCoef);
		con->printf(constants::MSG_X, y, message->text.c_str());
		y++;
		colorCoef += 0.3f;
	}
	//render mouse look
	renderMouseLook();
	//dungeon level
	con->setDefaultForeground(TCOD_white);
	ss.str(std::string());
	ss << "Dungeon level " << engine.level;
	con->printf(3, 3, ss.str().c_str());
	//blit the the GUI console on the root console
	TCODConsole::blit(con.get(), 0, 0, engine.screenWidth, constants::PANEL_HEIGHT, TCODConsole::root, 0, engine.screenHeight - constants::PANEL_HEIGHT);
}
void Gui::renderBar(int x, int y, int width, std::string_view name, float value, float maxValue, const TCODColor& barColor, const TCODColor& backColor) {
	//fill the background
	con->setDefaultBackground(backColor);
	con->rect(x, y, width, 1, false, TCOD_BKGND_SET);
	//compute how much of bar is filled
	int barWidth = (int)(value / maxValue * width);
	//draw the bar
	con->setDefaultBackground(barColor);
	con->rect(x, y, barWidth, 1, false, TCOD_BKGND_SET);
	//print text on top of bar
	con->setDefaultForeground(TCOD_white);
	std::stringstream barText;
	barText << name << " : " << value << "/" << maxValue;
	con->printf(x + width / 2, y, TCOD_BKGND_NONE, TCOD_CENTER, barText.str().c_str());
}

void Gui::renderMouseLook() {
	if (!engine.map->isInFOV(engine.mouse.cx, engine.mouse.cy)) {
		//if mouse is out of fov, nothing to render
		return;
	}
	std::string buf = "";
	bool first = true;
	for (std::list<std::unique_ptr<Actor>>::iterator i = engine.actors.begin(); i != engine.actors.end(); ++i) {
		//find actor under mouse cursor
		if (i->get()->getX() == engine.mouse.cx && i->get()->getY() == engine.mouse.cy) {
			if (!first) {
				buf += ", ";
			}
			else {
				first = false;
			}
			buf += i->get()->name;
		}
	}
	//Display the list of actors under the mouse cursor
	con->setDefaultForeground(TCODColor::lightGrey);
	con->printf(1, 0, buf.c_str());
}

bool Gui::replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos) {
		return false;
	}
	str.replace(start_pos, from.length(), to);
	return true;
}

Gui::Message::Message(std::string_view text, const TCODColor& col): text{ text.data() }, col{ col } 
{}




void Gui::clear() {
	log.clear();
}

void Menu::clear() {
	items.clear();
}

void Menu::addItem(MenuItemCode code, std::string_view label) {
	auto item = std::make_unique<MenuItem>();
	item -> code = code;
	item->label = label;
	items.emplace_back(std::move(item));
}

Menu::MenuItemCode Menu::pick(DisplayMode mode){
	int selectedItem = 0;
	int menux, menuy;
	if (mode == DisplayMode::PAUSE) {
		menux = engine.screenWidth / 2 - constants::PAUSE_MENU_WIDTH / 2;
		menuy = engine.screenHeight / 2 - constants::PAUSE_MENU_HEIGHT / 2;
		TCODConsole::root->setDefaultForeground(TCODColor(200, 180, 50));
		TCODConsole::root->printFrame(menux, menuy, constants::PAUSE_MENU_WIDTH, constants::PAUSE_MENU_HEIGHT, true,
			TCOD_BKGND_ALPHA(70), "menu");
		menux += 2;
		menuy += 3;
	}
	else {
		static TCODImage img("menu_background1.png");
		img.blit2x(TCODConsole::root, 0, 0);
		menux = 10;
		menuy = TCODConsole::root->getHeight() / 3;
	}
	while (!TCODConsole::isWindowClosed()) {
		int currentItem = 0;
		for (auto i = items.begin(); i != items.end(); ++i) {
			if (currentItem == selectedItem) {
				TCODConsole::root->setDefaultForeground(TCOD_lighter_orange);
			}
			else {
				TCODConsole::root->setDefaultForeground(TCOD_light_grey);
			}
			TCODConsole::root->print(menux, menuy + currentItem * 3, i->get()->label.c_str());
			currentItem++;
		}
		TCODConsole::flush();
		//check for key presses
		auto chooseItem = items.begin();
		TCOD_key_t key;
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL);
		switch (key.vk) {
		case TCODK_UP:
			selectedItem--;
			if (selectedItem < 0) {
				selectedItem = (int)items.size() - 1;
			}
			break;
		case TCODK_DOWN:
			selectedItem = selectedItem + 1 % items.size();
			break;
		case TCODK_ENTER:
			advance(chooseItem, selectedItem);
			return chooseItem->get()->code;
		default: break;
		}
	}
	return MenuItemCode::NONE;
}

Camera::Camera(int x, int y, int width, int height, int mapWidth, int mapHeight) : 
	x{ x }, y{ y }, width{ width }, height{ height }, mapWidth{ mapWidth }, mapHeight{ mapHeight }{}

std::tuple<int, int> Camera::apply(int x, int y) {
	x += this->x;
	y += this->y;
	return std::make_tuple(x, y);
}

std::tuple<int, int> Camera::getWorldLocation(int x, int y) {
	x -= this->x;
	y -= this->y;
	return std::make_tuple(x, y);
}



void Camera::update(Actor* actor) {
	x = -actor->getX() + (int)(width / 2);
	y = -actor->getY() + (int)(height / 2);

	//TODO add limits to stop camera near map edges
}