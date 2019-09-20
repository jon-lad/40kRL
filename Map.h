#pragma once

struct Tile {
	bool canWalk {}; // is tile passable
	Tile() :canWalk{ false } {}
};  

class Map {
private:
	int width {}; //Map width
	int height {}; //Map Height
public:
	
	Map(int width, int height);
	bool isWall(int x, int y) const;
	void render() const;

	int getWidth() const;
	void setWidth(int width);

	int getHeight() const;
	void setHeight(int height);
protected:
	std::vector<Tile> tiles{};
	void setWall(int x, int y);
};