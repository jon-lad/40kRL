#pragma once
struct Tile {
	bool explored {}; // is tile passable
	Tile() :explored{ false } {}
};  

class Map {
private:
	int width {}; //Map width
	int height {}; //Map Height
public:
	
	Map(int width, int height);
	bool isWall(int x, int y) const;
	void render() const;
	bool isInFOV(int x, int y) const;
	bool isExplored(int x, int y) const;
	void computeFOV();
	bool canWalk(int x, int y) const;//takes into account walls and Actors
	void addMonster(int x, int y);

	int getWidth() const;
	void setWidth(int width);

	int getHeight() const;
	void setHeight(int height);
	
	
protected:
	mutable std::vector<Tile> tiles{};
	std::unique_ptr<TCODMap> map;
	friend class BspListener;
	void dig(int x1, int y1, int x2, int y2);
	void createRoom(bool first, int x1, int y1, int x2, int y2);
};


