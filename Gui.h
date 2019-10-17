#pragma once


class Gui: public Persistent {
public:
	Gui();

	void render();
	void message(const TCODColor& col, const std::string_view text, ...);
	void load(TCODZip& zip);
	void save(TCODZip& zip);
protected:
	std::unique_ptr<TCODConsole> con;

	void renderBar(int x, int y, int width, const std::string_view name, 
			float value, float maxValue, const TCODColor& barColor, 
			const TCODColor& backColor);
	void renderMouseLook();
	

	struct Message {
		std::string text;
		TCODColor col;
		Message(std::string& text, const TCODColor& col);
		
	};
	std::list<std::unique_ptr<Message>> log;

};
