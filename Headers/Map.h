#pragma once

// Tile stores per-cell state that libtcod's TCODMap doesn't track.
struct Tile {
	bool explored = false; // true once this tile has ever been inside the player's FOV
	unsigned int scent = 0; // stamped with (currentScentValue - distance) each time the tile is in FOV
};

// Owns the dungeon floor: the libtcod walkability/FOV map, the tile state array,
// and the seeded RNG used for deterministic BSP room generation.
class Map : public Persistent {
private:
	int width  = 0;
	int height = 0;

public:
	// Monotonically increasing counter incremented once per NEW_TURN.
	// A tile's scent is considered fresh if: tile.scent > currentScentValue - SCENT_THRESHOLD
	unsigned int currentScentValue;

	Map(int width, int height);

	// Generates the map layout using BSP. If withActors is true, populates rooms with monsters and items.
	void init(bool withActors);

	// Returns true if the tile is impassable (not walkable in the libtcod map).
	bool isWall(int x, int y) const;

	// Draws all visible and explored tiles using box-drawing wall characters.
	void render() const;

	// Returns true if (x, y) is currently inside the player's FOV. Also marks the tile as explored.
	bool isInFOV(int x, int y) const;

	// Returns true if this tile has previously been inside the player's FOV.
	bool isExplored(int x, int y) const;

	// Returns true if (x, y) has at least one walkable cardinal or diagonal neighbour.
	// Used to decide whether a wall tile should be drawn with a box-drawing character.
	bool isExplorable(int x, int y) const;

	// Recomputes the libtcod FOV from the player's position and updates scent values on in-FOV tiles.
	void computeFOV();

	// Returns true if (x, y) is not a wall and no blocking actor occupies the tile.
	bool canWalk(int x, int y) const;

	void addMonster(int x, int y);
	void addItem(int x, int y);

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

	int getWidth() const;
	void setWidth(int width);

	int getHeight() const;
	void setHeight(int height);

	unsigned int getScent(int x, int y) const;

protected:
	mutable std::vector<Tile> tiles; // flat array indexed as x + y * width
	std::unique_ptr<TCODMap>     map;
	friend class BspListener;

	// Marks the rectangle from (x1,y1) to (x2,y2) as transparent and walkable.
	void dig(int x1, int y1, int x2, int y2);

	// Digs a room and optionally places the player, stairs, monsters, and items inside it.
	void createRoom(bool isFirstRoom, int x1, int y1, int x2, int y2, bool withActors);

	long seed;
	std::unique_ptr<TCODRandom> rng;
};


