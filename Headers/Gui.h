#pragma once

#include <sstream>
#include <vector>

struct SkillBarEntry;

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

	// Message log capacity — matches layout::MSG_LOG_HEIGHT (6)
	static constexpr int MSG_LOG_CAPACITY = layout::MSG_LOG_HEIGHT;

	Gui();

	void render();
	void renderSkillBar(const std::vector<SkillBarEntry>& skills);
	void renderRightSidebar();
	
	template<typename Color, typename T, typename...Args>
	void message(const Color& col, const T& text, Args&&...args) {

		std::vector<std::string> splitString;
		std::string input = text;
		std::string token;
		std::vector<std::string> stringSubs = makeStringList(std::forward<Args>(args)...);
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
			if (string.empty()) continue; // silently discard empty messages
			if (static_cast<int>(log.size()) >= MSG_LOG_CAPACITY) {
				log.pop_front();
			}
			log.emplace_back(std::make_unique<Message>(string, col));
		}
	}


	void load(TCODZip& zip);
	void save(TCODZip& zip);
	void clear();

	// Returns the text of the most recently added message, or empty string if log is empty.
	// Primarily used by tests to verify GUI feedback.
	std::string getLastMessage() const {
		if (log.empty()) return "";
		return log.back()->text;
	}
protected:
	std::unique_ptr<TCODConsole> hudConsole;
	std::unique_ptr<TCODConsole> msgLogConsole; // dedicated message log sub-console
	std::unique_ptr<TCODConsole> rightSidebarConsole; // right sidebar sub-console (24 x SCREEN_HEIGHT)
	std::unique_ptr<TCODConsole> leftSidebarConsole; // nullptr when LEFT_SIDEBAR_ENABLED is false

	void renderBar(int x, int y, int width, const std::string_view name, 
			float value, float maxValue, const TCODColor& barColor, 
			const TCODColor& backColor);
	void renderMouseLook();
	void renderMessageLog(); // renders the message log into its dedicated HUD region
	void renderLeftSidebar(); // renders left sidebar (no-op when disabled)

	template<typename T>
	std::string makeString(const T& val) {
		std::stringstream ss;
		ss << val;
		return ss.str().data();
	}

	template<typename... Args>
	std::vector<std::string> makeStringList(Args&&... args) {
		std::vector<std::string> result;
		(result.push_back(makeString(std::forward<Args>(args))), ...);
		return result;
	}

	bool replace(std::string& str, const std::string& from, const std::string& to);
	
	

	struct Message {
		std::string text;
		TCODColor col;
		Message(std::string_view text, const TCODColor& col);
		
	};
	std::list<std::unique_ptr<Message>> log;
};

// Converts between world coordinates and screen coordinates by maintaining a
// signed offset that keeps the player centred in the viewport.
// Transform: screen = world + offset,  world = screen - offset
// Offset update: x = -(player.x) + width/2,  y = -(player.y) + height/2
class Camera : public Persistent {
public:
	int x, y;
	int width, height;
	int mapWidth, mapHeight;
	int scrollMargin = 20;
	Camera(int x, int y, int width, int height, int mapWidth, int mapHeight);
	std::tuple<int, int> apply(int x, int y);
	std::tuple<int, int> getWorldLocation(int x, int y);
	void update(Actor* actor, bool isOutdoor = false);
	void save(TCODZip& zip);
	void load(TCODZip& zip);
};

