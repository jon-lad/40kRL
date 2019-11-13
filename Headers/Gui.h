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
	
	template<typename TCODColor, typename T, typename...Args>
	void message(const TCODColor& col, const T& text, const Args&&...args) {

		std::vector<std::string> splitString;
		std::string input = text;
		std::string token;
		std::vector<std::string> stringSubs = makeStringList(args...);
		bool x = true;
		int i = 0;
		while(x){
			if ((int)stringSubs.size() > i) {
				x = replace(input, "#", stringSubs.at(i));
				i++;
			}
			else { x = false; }
		}
		std::stringstream ss(input);
		while (getline(ss, token, '\n')) {
			splitString.emplace_back(token);
		}
	
		//add message to log
		for (const auto& string : splitString) {
			if (log.size() == constants::MSG_HEIGHT) {
				log.pop_front();
			}
			log.emplace_back(std::make_unique<Message>(string, col));
		}
	}


	void load(TCODZip& zip);
	void save(TCODZip& zip);
	void clear();
protected:
	std::unique_ptr<TCODConsole> con;

	void renderBar(int x, int y, int width, const std::string_view name, 
			float value, float maxValue, const TCODColor& barColor, 
			const TCODColor& backColor);
	void renderMouseLook();

	template<typename T>
	std::string makeString(const T& val) {
		std::stringstream ss;
		ss << val;
		return ss.str().data();
	}

	template<typename...Args>
	std::vector<std::string> makeStringList(Args&&...args) {
		std::vector<std::string> v;
		std::string s;
		std::initializer_list<int>{
			(s = makeString(std::move(args)),
				std::clog << "Adding: '" << args << "' [" << typeid(std::move(args)).name() << "]",
				v.push_back(s),
				std::clog << " done - verify: '" << v.back() << "'\n",
				0)...
		};
		return v;
	}

	bool replace(std::string& str, const std::string& from, const std::string& to);
	
	

	struct Message {
		std::string text;
		TCODColor col;
		Message(std::string_view text, const TCODColor& col);
		
	};
	std::list<std::unique_ptr<Message>> log;
};

class Camera : public Persistent {
public:
	int x, y;
	int width, height;
	int mapWidth, mapHeight;
	Camera(int x, int y, int width, int height, int mapWidth, int mapHeight);
	std::tuple<int, int> apply(int x, int y);
	std::tuple<int, int> getWorldLocation(int x, int y);
	void update(Actor* actor);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
};

