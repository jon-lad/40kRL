
#include <list>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <stdarg.h>
#include "main.h"


static constexpr int PANEL_HEIGHT = 7;
static constexpr int BAR_WIDTH = 20;
static constexpr int MSG_X = BAR_WIDTH + 2;
static constexpr int MSG_HEIGHT = PANEL_HEIGHT - 2;

Gui::Gui() {
	con = std::make_unique<TCODConsole>(engine.screenWidth, PANEL_HEIGHT);
}

void Gui::render() {
	//clear the gui console
	con->setDefaultBackground(TCODColor::black);
	con->clear();
	//draw the health bar
	renderBar(1, 1, BAR_WIDTH, "HP", engine.player->destructible->hp, engine.player->destructible->maxHp, TCODColor::lightRed, TCODColor::darkerRed);
	// draw the message log
	int y = 1;
	float colorCoef = 0.4f;
	for (auto i = log.begin(); i != log.end(); ++i) {
		con->setDefaultForeground(i->get()->col* colorCoef);
		con->printf(MSG_X, y, i->get()->text.c_str());
		y++;
		colorCoef += 0.3f;
	}
	//render mouse look
	renderMouseLook();
	//blit the the GUI console on the root console
	TCODConsole::blit(con.get(), 0, 0, engine.screenWidth, PANEL_HEIGHT, TCODConsole::root, 0, engine.screenHeight - PANEL_HEIGHT);
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

Gui::Message::Message(std::string& text, const TCODColor& col): text{ text }, col{ col } 
{}



void Gui::message(const TCODColor &col, std::string_view text, ...){
	std::vector<std::string> splitString;
	std::string input = text.data();
	std::istringstream ss(input);
	std::string token;
	while(std::getline(ss, token, '\n')) {

		splitString.emplace_back(token);
	}
		//add message to log
	for(auto i = splitString.begin(); i != splitString.end(); ++i) {
		if (log.size() == MSG_HEIGHT) {
			log.pop_front();
		}
		log.emplace_back(std::make_unique<Message>(*i, col));
	}

}

