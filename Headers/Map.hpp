#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <memory>
#include <string>

// Forward declaration — full definition in Headers/WfcGenerator.h (created by task 1.1)
struct WfcTileset;

// Sentinel appended at the very end of the Map::save stream to mark the presence
// of the persisted region field. "RGNM" in ASCII. Because TCODZip::getInt()
// returns 0 on an exhausted archive, this non-zero constant cannot be mistaken
// for end-of-stream in a pre-region save. Namespace-scope so tests can reference
// it directly (see Tests/test_region_persistence.cpp).
inline constexpr int REGION_SENTINEL = 0x52474E4D; // "RGNM"

// Identifies the generation algorithm used for a map level.
enum class LevelType : int {
	BSP     = 0, // Binary Space Partitioning (rooms + corridors)
	OUTDOOR = 1, // Perlin-noise open terrain (ground, trees, water)
	WFC     = 2  // Wave Function Collapse (hive city interiors)
};

// Terrain classification for outdoor tiles, derived from Perlin noise thresholds.
enum class TerrainType : uint8_t {
	GROUND = 0, // walkable, transparent
	TREE   = 1, // not walkable, not transparent
	WATER  = 2  // not walkable, transparent
};

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

	LevelType levelType = LevelType::BSP;
	std::string regionName; // whole-level region; empty until assigned, then treated as Default_Region
	std::vector<TerrainType> terrainTypes; // only populated for OUTDOOR levels
	std::vector<std::pair<int,int>> outdoorRegion; // largest connected ground component

	// Outdoor generation and rendering (implemented in later tasks).
	void initOutdoor(bool withActors);
	void renderOutdoor() const;
	void placeOutdoorActors();
	void addOutdoorDecorations();
	std::vector<std::pair<int,int>> findLargestGroundRegion() const;

	// WFC generation, rendering, and actor placement.
	std::vector<int> wfcTileIds;                  // resolved tile index per cell
	std::shared_ptr<WfcTileset> wfcTileset;       // cached tileset for rendering
	void initWfc(bool withActors);
	void renderWfc() const;
	void placeWfcActors();

	// BSP generation extracted from init for dispatch clarity.
	void initBsp(bool withActors);

public:
	// Monotonically increasing counter incremented once per NEW_TURN.
	// A tile's scent is considered fresh if: tile.scent > currentScentValue - SCENT_THRESHOLD
	unsigned int currentScentValue;

	Map(int width, int height);

	// Generates the map layout. If withActors is true, populates with monsters and items.
	// The level type determines which generation algorithm is used.
	void init(bool withActors, LevelType type = LevelType::BSP);

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
	void addDecorations(int x1, int y1, int x2, int y2);

	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

	int getWidth() const;
	void setWidth(int width);

	int getHeight() const;
	void setHeight(int height);

	unsigned int getScent(int x, int y) const;

	LevelType getLevelType() const { return levelType; }

	// Returns the terrain type at (x, y). Only valid for OUTDOOR levels.
	TerrainType getTerrainType(int x, int y) const { return terrainTypes[x + y * width]; }

	// Sets the transparency and walkability of the tile at (x, y) in the TCODMap.
	// Used by Openable to update door tiles on state transitions.
	void setTileProperties(int x, int y, bool transparent, bool walkable);

	// Sets the internal seed used for deterministic BSP generation.
	// Must be called BEFORE init() to take effect.
	void setSeed(long newSeed) { seed = newSeed; }

	// Returns the WFC tile description at (x, y). Only valid for WFC levels.
	std::string getWfcTileDescription(int x, int y) const;

	// Whole-level region name (a Region_Name string). Assigned once at level
	// creation and retained for the level's lifetime; every spawn on the level
	// uses this single value (whole-level granularity).
	const std::string& getRegionName() const { return regionName; }
	void setRegionName(const std::string& region) { regionName = region; }

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


