#pragma once

class Menu {
public:
	enum class MenuItemCode {
		NONE = 0,
		NEW_GAME,
		CONTINUE,
		EXIT,
		CONSTITUTION,
		STRENGTH,
		AGILITY
	};
	enum class DisplayMode {
		MAIN,
		PAUSE
	};
	void clear();
	void addItem(MenuItemCode, std::string_view label);
	MenuItemCode pick(DisplayMode mode = DisplayMode::MAIN);
protected:
	struct MenuItem {
		MenuItemCode code;
		std::string label;
	};
	std::list<std::unique_ptr<MenuItem>> items;

};


class Gui: public Persistent {
public:
	Menu menu;

	Gui();

	void render();
	void message(const TCODColor& col, const std::string_view text, ...);
	void load(TCODZip& zip);
	void save(TCODZip& zip);
	void clear();
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

