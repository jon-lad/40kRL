#pragma once
class Engine {
public:
	std::list<std::unique_ptr<Actor>> actors;
	Actor* player;
	std::unique_ptr<Map> map;

	Engine();
	void update();
	void render();

};

extern Engine engine;