#pragma once

using actorPtr_t = std::unique_ptr<Actor>;

class Engine {
public:
	enum GameStatus {
		STARTUP,
		IDLE,
		NEW_TURN,
		VICTORY,
		DEFEAT
	} gameStatus;
	std::list<actorPtr_t> actors;
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
	Actor* getClosestMonster(int x, int y, double range);
	Actor* getActor(int x, int y) const;
	bool pickAtTile(int* x, int* y, double maxRange = 0.0);
	void nextLevel();
	void init();
	void term();
	void save();
	void load();
	
};


	extern Engine engine;
