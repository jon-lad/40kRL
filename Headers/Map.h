#pragma once



struct Tile {
	bool explored {}; // is tile passable
	unsigned int scent;
	Tile() :explored{ false }, scent{ 0 } {}
	
};  

class Map : public Persistent {
private:
	int width {}; //Map width
	int height {}; //Map Height
public:
	unsigned int currentScentValue;
	
	Map(int width, int height);
	void init(bool withActors);
	bool isWall(int x, int y) const;
	void render() const;
	bool isInFOV(int x, int y) const;
	bool isExplored(int x, int y) const;
	bool isExplorable(int x, int y) const;
	void computeFOV();
	bool canWalk(int x, int y) const;//takes into account walls and Actors
	void addMonster(int x, int y);
	void addItem(int x, int y);
	void save(TCODZip& zip);
	void load(TCODZip& zip);

	int getWidth() const;
	void setWidth(int width);

	int getHeight() const;
	void setHeight(int height);
	
	unsigned int getScent(int x, int y) const;
	
protected:
	mutable std::vector<Tile> tiles{};
	std::unique_ptr<TCODMap> map;
	friend class BspListener;
	void dig(int x1, int y1, int x2, int y2);
	void createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors);
	long seed;
	std::unique_ptr<TCODRandom> rng;
};


