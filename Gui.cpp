#include <list>
#include <memory>
#include <string>
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
	for (std::list<std::unique_ptr<Message>>::iterator i = log.begin(); i != log.end(); ++i) {
		con->setDefaultForeground(i->get()->col* colorCoef);
		con->printf(MSG_X, y, i->get()->text);
		y++;
		colorCoef += 0.3f;
	}
	//render mouse look
	renderMouseLook();
	//blit the the GUI console on the root console
	TCODConsole::blit(con.get(), 0, 0, engine.screenWidth, PANEL_HEIGHT, TCODConsole::root, 0, engine.screenHeight - PANEL_HEIGHT);
}
void Gui::renderBar(int x, int y, int width, const char* name, float value, float maxValue, const TCODColor& barColor, const TCODColor& backColor) {
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
	con->printf(x + width / 2, y, TCOD_BKGND_NONE, TCOD_CENTER, "%s : %g/%g", name, value, maxValue);
}

void Gui::renderMouseLook() {
	if (!engine.map->isInFOV(engine.mouse.cx, engine.mouse.cy)) {
		//if mouse is out of fov, nothing to render
		return;
	}
	char buf[128] = "";
	bool first = true;
	for (std::list<std::unique_ptr<Actor>>::iterator i = engine.actors.begin(); i != engine.actors.end(); ++i) {
		//find actor under mouse cursor
		if (i->get()->getX() == engine.mouse.cx && i->get()->getY() == engine.mouse.cy) {
			if (!first) {
				strcat_s(buf, ", ");
			}
			else {
				first = false;
			}
			strcat_s(buf, i->get()->name);
		}
	}
	//Display the list of actors under the mouse cursor
	con->setDefaultForeground(TCODColor::lightGrey);
	con->printf(1, 0, buf);
}

Gui::Message::Message(const char* text, const TCODColor& col) :text{ _strdup(text) }, col{ col } {
}

Gui::Message::~Message() {
	free(text);
}

void Gui::message(const TCODColor &col, const char *text, ...){
	//build the text
	va_list ap;
	char buf[128];
	va_start(ap, text);
	vsprintf_s(buf, text, ap);
	va_end(ap);
	char *lineBegin = buf;
	char *lineEnd;
	 do {
		if (log.size() == MSG_HEIGHT) {
			log.pop_front();
		}
	//detect end of line
		lineEnd = strchr(lineBegin, '\n');
		if (lineEnd) {
			*lineEnd = '\0';
		}
		//add message to log
		
		log.emplace_back(std::make_unique<Message>(lineBegin, col));
	} while (lineEnd);
}