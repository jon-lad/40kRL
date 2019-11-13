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
	Actor* stairs;
	int FOVRadius;
	int screenWidth;
	int screenHeight;
	std::unique_ptr<Camera> camera;
	TCOD_key_t lastKey;
	TCOD_mouse_t mouse;
	std::unique_ptr<Gui> gui;
	int level;
	

	Engine(int screenWidth, int screenHeigh);
	void update();
	void render();
	void sendToBack(Actor* actor);
	Actor* getClosestMonster(int x, int y, float range);
	Actor* getActor(int x, int y) const;
	bool pickAtTile(int* x, int* y, float maxRange = 0.0f);
	void nextLevel();
	void init();
	void term();
	void save();
	void load();
private:
	
};


	extern Engine engine;
