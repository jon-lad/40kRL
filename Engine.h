#pragma once


class Engine {
public:
	enum GameStatus {
		STARTUP,
		IDLE,
		NEW_TURN,
		VICTORY,
		DEFEAT
	} gameStatus;
	std::list<std::unique_ptr<Actor>> actors;
	std::unique_ptr<Map> map;
	Actor* player;
	int FOVRadius;
	int screenWidth;
	int screenHeight;
	TCOD_key_t lastKey;
	TCOD_mouse_t mouse;
	std::unique_ptr<Gui> gui;

	Engine(int screenWidth, int screenHeigh);
	void update();
	void render();
	void sendToBack(Actor* actor);
private:
	
};


	extern Engine engine;
