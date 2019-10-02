#pragma once
#include <list>

class Gui {
public:
	Gui();

	void render();
	void message(const TCODColor& col, const char* text, ...);
protected:
	std::unique_ptr<TCODConsole> con;

	void renderBar(int x, int y, int width, const char* name, 
			float value, float maxValue, const TCODColor& barColor, 
			const TCODColor& backColor);
	void renderMouseLook();

	struct Message {
		char *text;
		TCODColor col;
		Message(const char* text, const TCODColor& col);
		~Message();
	};
	std::list<std::unique_ptr<Message>> log;

};
