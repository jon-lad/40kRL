
#include <algorithm>
#include <memory>
#include <queue>
#include <vector>
#include <list>
#include <sstream>
#include <sol/sol.hpp>
#include "main.h"

static constexpr int ROOM_MAX_SIZE     = 12;
static constexpr int ROOM_MIN_SIZE     = 6;
static constexpr int MAX_ROOM_MONSTERS = 3;
static constexpr int MAX_ROOM_ITEMS    = 2;
static constexpr int BSP_DEPTH         = 8;
static constexpr char GROUND_GLYPH     = '.';
static constexpr int MIN_PLAYABLE_AREA = 200;
static constexpr int MAX_RETRIES       = 10;

// ─── BspListener ─────────────────────────────────────────────────────────────

// Receives callbacks from the libtcod BSP traversal and digs rooms + corridors.
class BspListener : public ITCODBspCallback {
private:
	Map& map;
	int  roomCount = 0;
	int  lastRoomCentreX = 0;
	int  lastRoomCentreY = 0;

public:
	explicit BspListener(Map& map) : map{ map } {}

	bool visitNode(TCODBsp* node, void* userData) override
	{
		if (!node->isLeaf()) { return true; }

		const bool withActors = static_cast<bool>(userData);

		// Pick random room dimensions within the BSP partition.
		const int w = map.rng->getInt(ROOM_MIN_SIZE, node->w - 2);
		const int h = map.rng->getInt(ROOM_MIN_SIZE, node->h - 2);
		const int x = map.rng->getInt(node->x + 1, node->x + node->w - w - 1);
		const int y = map.rng->getInt(node->y + 1, node->y + node->h - h - 1);

		map.createRoom(roomCount == 0, x, y, x + w - 1, y + h - 1, withActors);

		if (roomCount > 0) {
			// Connect this room to the previous one with an L-shaped corridor.
			const int centreX = x + w / 2;
			const int centreY = y + h / 2;
			map.dig(lastRoomCentreX, lastRoomCentreY, centreX, lastRoomCentreY);
			map.dig(centreX, lastRoomCentreY, centreX, centreY);
			lastRoomCentreX = centreX;
			lastRoomCentreY = centreY;
		} else {
			lastRoomCentreX = x + w / 2;
			lastRoomCentreY = y + h / 2;
		}

		roomCount++;
		return true;
	}
};

// ─── Map ─────────────────────────────────────────────────────────────────────

Map::Map(int width, int height)
	: width{ width }, height{ height }
	, currentScentValue{ static_cast<unsigned int>(SCENT_THRESHOLD) }
{
	seed = TCODRandom::getInstance()->getInt(0, 0x7FFFFFFF);
}

void Map::init(bool withActors, LevelType type)
{
	rng   = std::make_unique<TCODRandom>(seed, TCOD_RNG_CMWC);
	tiles = std::vector<Tile>(width * height);
	map   = std::make_unique<TCODMap>(width, height);
	levelType = type;

	if (type == LevelType::OUTDOOR) {
		initOutdoor(withActors);
	} else {
		initBsp(withActors);
	}
}

void Map::initBsp(bool withActors)
{
	TCODBsp bsp(0, 0, width, height);
	bsp.splitRecursive(rng.get(), BSP_DEPTH, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);

	BspListener listener(*this);
	bsp.traverseInvertedLevelOrder(&listener, reinterpret_cast<void*>(withActors));
}

void Map::initOutdoor(bool withActors)
{
	// ── 1. Load noise/threshold parameters from Config.lua (with defaults) ──

	float groundThreshold = -0.1f;
	float waterThreshold  = -0.5f;
	int   octaves         = 4;
	float lacunarity      = 2.0f;
	float noiseScale      = 0.05f;

	try {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		lua.script_file("Scripts/Config.lua");

		sol::table cfg = lua["config"];
		if (cfg.valid()) {
			groundThreshold = cfg.get_or("outdoorGroundThreshold", groundThreshold);
			waterThreshold  = cfg.get_or("outdoorWaterThreshold",  waterThreshold);
			octaves         = cfg.get_or("outdoorOctaves",         octaves);
			lacunarity      = cfg.get_or("outdoorLacunarity",      lacunarity);
			noiseScale      = cfg.get_or("outdoorNoiseScale",      noiseScale);
		}
	} catch (const sol::error&) {
		// Config load failed — use compiled defaults silently.
	}

	// ── 2. Clamp out-of-range values and log warnings via Gui ──

	bool clamped = false;

	if (groundThreshold < -1.0f || groundThreshold > 1.0f) {
		groundThreshold = std::max(-1.0f, std::min(1.0f, groundThreshold));
		clamped = true;
	}
	if (waterThreshold < -1.0f || waterThreshold > 1.0f) {
		waterThreshold = std::max(-1.0f, std::min(1.0f, waterThreshold));
		clamped = true;
	}
	if (octaves < 1 || octaves > 8) {
		octaves = std::max(1, std::min(8, octaves));
		clamped = true;
	}
	if (lacunarity < 1.0f || lacunarity > 4.0f) {
		lacunarity = std::max(1.0f, std::min(4.0f, lacunarity));
		clamped = true;
	}
	if (noiseScale <= 0.0f || noiseScale >= 1.0f) {
		noiseScale = std::max(0.001f, std::min(0.999f, noiseScale));
		clamped = true;
	}

	if (clamped) {
		engine.gui->message(Colors::damage,
			"Warning: outdoor config values clamped to valid ranges.");
	}

	// ── 3. Create TCODNoise with 2 dimensions ──

	// ── Retry loop for connectivity guarantee ──
	for (int attempt = 0; attempt <= MAX_RETRIES; ++attempt) {

		TCODNoise noise(2, lacunarity, 1.0f / lacunarity, rng.get());

		// ── 4 & 5. Sample noise, classify terrain, store in terrainTypes and set TCODMap ──

		terrainTypes = std::vector<TerrainType>(width * height);

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				float coords[2] = { x * noiseScale, y * noiseScale };
				float value = noise.get(coords, TCOD_NOISE_PERLIN);

				TerrainType terrain;
				bool walkable;
				bool transparent;

				if (value > groundThreshold) {
					terrain     = TerrainType::GROUND;
					walkable    = true;
					transparent = true;
				} else if (value > waterThreshold) {
					terrain     = TerrainType::TREE;
					walkable    = false;
					transparent = false;
				} else {
					terrain     = TerrainType::WATER;
					walkable    = false;
					transparent = true;
				}

				terrainTypes[x + y * width] = terrain;
				map->setProperties(x, y, transparent, walkable);
			}
		}

		// ── 4b. Connectivity guarantee: find largest connected ground region ──
		outdoorRegion = findLargestGroundRegion();

		if (static_cast<int>(outdoorRegion.size()) >= MIN_PLAYABLE_AREA) {
			break; // Region is large enough — proceed with generation
		}

		// Region too small — increment seed and retry
		if (attempt < MAX_RETRIES) {
			seed++;
			rng = std::make_unique<TCODRandom>(seed, TCOD_RNG_CMWC);
		}
	}

	// Actor placement is handled by placeOutdoorActors (task 2.6).
	if (withActors) {
		placeOutdoorActors();
	}
}

void Map::renderOutdoor() const
{
	static const TCODColor LIGHT_OUTDOOR_GROUND{ 100, 160, 60 };
	static const TCODColor DARK_OUTDOOR_GROUND { 40,  80,  25 };
	static const TCODColor LIGHT_TREE          { 0,   180, 0  };
	static const TCODColor DARK_TREE           { 0,   90,  0  };
	static const TCODColor LIGHT_WATER         { 60,  100, 200 };
	static const TCODColor DARK_WATER          { 25,  50,  100 };

	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			auto [screenX, screenY] = engine.camera->apply(x, y);
			TerrainType terrain = terrainTypes[x + y * width];

			if (isInFOV(x, y)) {
				switch (terrain) {
				case TerrainType::GROUND:
					renderPutChar(TCODConsole::root->get_data(), screenX, screenY, '.', {LIGHT_OUTDOOR_GROUND.r, LIGHT_OUTDOOR_GROUND.g, LIGHT_OUTDOOR_GROUND.b});
					break;
				case TerrainType::TREE:
					renderPutChar(TCODConsole::root->get_data(), screenX, screenY, CharConst::SPADE, {LIGHT_TREE.r, LIGHT_TREE.g, LIGHT_TREE.b});
					break;
				case TerrainType::WATER:
					renderPutChar(TCODConsole::root->get_data(), screenX, screenY, '~', {LIGHT_WATER.r, LIGHT_WATER.g, LIGHT_WATER.b});
					break;
				}
			} else if (isExplored(x, y)) {
				switch (terrain) {
				case TerrainType::GROUND:
					renderPutChar(TCODConsole::root->get_data(), screenX, screenY, '.', {DARK_OUTDOOR_GROUND.r, DARK_OUTDOOR_GROUND.g, DARK_OUTDOOR_GROUND.b});
					break;
				case TerrainType::TREE:
					renderPutChar(TCODConsole::root->get_data(), screenX, screenY, CharConst::SPADE, {DARK_TREE.r, DARK_TREE.g, DARK_TREE.b});
					break;
				case TerrainType::WATER:
					renderPutChar(TCODConsole::root->get_data(), screenX, screenY, '~', {DARK_WATER.r, DARK_WATER.g, DARK_WATER.b});
					break;
				}
			}
		}
	}
}

void Map::placeOutdoorActors()
{
	if (outdoorRegion.empty()) { return; }

	// ── 1. Load actor-count config from Config.lua ──

	int minMonsters = 6;
	int maxMonsters = 12;
	int minItems    = 2;
	int maxItems    = 5;

	try {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		lua.script_file("Scripts/Config.lua");

		sol::table cfg = lua["config"];
		if (cfg.valid()) {
			minMonsters = cfg.get_or("outdoorMinMonsters", minMonsters);
			maxMonsters = cfg.get_or("outdoorMaxMonsters", maxMonsters);
			minItems    = cfg.get_or("outdoorMinItems",    minItems);
			maxItems    = cfg.get_or("outdoorMaxItems",    maxItems);
		}
	} catch (const sol::error&) {
		// Config load failed — use compiled defaults.
	}

	// ── 2. Place player on a random ground tile in the largest connected component ──

	int playerIdx = rng->getInt(0, static_cast<int>(outdoorRegion.size()) - 1);
	auto [playerX, playerY] = outdoorRegion[playerIdx];
	engine.player->setX(playerX);
	engine.player->setY(playerY);

	// ── 3. Place down-stairs (dungeon entrance) at Euclidean distance ≥ 40 from player ──
	//       Fallback: furthest ground tile in the region.
	//       On the surface there are no up-stairs (can't go higher than depth 0).

	int bestStairsIdx = 0;
	float bestDist    = 0.0f;

	for (int i = 0; i < static_cast<int>(outdoorRegion.size()); ++i) {
		auto [tx, ty] = outdoorRegion[i];
		float dx = static_cast<float>(tx - playerX);
		float dy = static_cast<float>(ty - playerY);
		float dist = std::sqrt(dx * dx + dy * dy);
		if (dist > bestDist) {
			bestDist = dist;
			bestStairsIdx = i;
		}
	}

	// Use the furthest tile (which satisfies ≥ 40 if possible; if no tile is ≥ 40,
	// we still use the furthest — this IS the fallback behaviour).
	auto [stairsX, stairsY] = outdoorRegion[bestStairsIdx];
	if (engine.stairsDown) {
		engine.stairsDown->setX(stairsX);
		engine.stairsDown->setY(stairsY);
	}

	// ── 4. Scatter enemies on unoccupied ground tiles ──

	int monsterCount = rng->getInt(minMonsters, maxMonsters);
	static constexpr int MAX_PLACEMENT_ATTEMPTS = 100;

	for (int m = 0; m < monsterCount; ++m) {
		for (int attempt = 0; attempt < MAX_PLACEMENT_ATTEMPTS; ++attempt) {
			int idx = rng->getInt(0, static_cast<int>(outdoorRegion.size()) - 1);
			auto [mx, my] = outdoorRegion[idx];
			if (canWalk(mx, my)) {
				addMonster(mx, my);
				break;
			}
		}
	}

	// ── 5. Scatter items on unoccupied ground tiles ──

	int itemCount = rng->getInt(minItems, maxItems);

	for (int i = 0; i < itemCount; ++i) {
		for (int attempt = 0; attempt < MAX_PLACEMENT_ATTEMPTS; ++attempt) {
			int idx = rng->getInt(0, static_cast<int>(outdoorRegion.size()) - 1);
			auto [ix, iy] = outdoorRegion[idx];
			if (canWalk(ix, iy)) {
				addItem(ix, iy);
				break;
			}
		}
	}

	// ── 6. Place decorations on ground tiles ──
	addOutdoorDecorations();
}

void Map::addOutdoorDecorations()
{
	if (engine.decorationTemplates.empty()) { return; }
	if (outdoorRegion.empty()) { return; }

	// Load outdoor decoration count from Config.lua (default 8)
	int decorationCount = 8;

	try {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		lua.script_file("Scripts/Config.lua");

		sol::table cfg = lua["config"];
		if (cfg.valid()) {
			decorationCount = cfg.get_or("outdoorDecorationCount", decorationCount);
		}
	} catch (const sol::error&) {
		// Config load failed — use compiled default.
	}

	if (decorationCount <= 0) { return; }

	static constexpr int MAX_PLACEMENT_ATTEMPTS = 100;
	TCODRandom* globalRng = TCODRandom::getInstance();

	for (int d = 0; d < decorationCount; ++d) {
		// Select a random template
		int templateIdx = globalRng->getInt(0, static_cast<int>(engine.decorationTemplates.size()) - 1);
		const auto& tmpl = engine.decorationTemplates[templateIdx];

		// Find a random walkable unoccupied ground tile in outdoorRegion
		bool placed = false;
		for (int attempt = 0; attempt < MAX_PLACEMENT_ATTEMPTS; ++attempt) {
			int regionIdx = globalRng->getInt(0, static_cast<int>(outdoorRegion.size()) - 1);
			auto [dx, dy] = outdoorRegion[regionIdx];
			if (canWalk(dx, dy)) {
				// Spawn decoration Actor
				auto decoration = std::make_unique<Actor>(dx, dy, tmpl.glyph, tmpl.name, tmpl.color);
				decoration->blocks      = tmpl.blocks;
				decoration->fovOnly     = true;
				decoration->description = tmpl.description;
				decoration->coverValue  = tmpl.coverValue;
				engine.actors.push_back(std::move(decoration));
				placed = true;
				break;
			}
		}
		// If no valid tile found after all attempts, skip this placement silently.
	}
}

// BFS flood-fill over ground tiles using 4-connectivity (cardinal directions).
// Returns the largest connected component as a vector of (x,y) coordinate pairs.
std::vector<std::pair<int,int>> Map::findLargestGroundRegion() const
{
	std::vector<bool> visited(width * height, false);
	std::vector<std::pair<int,int>> largestComponent;

	static constexpr int dx[4] = { 0, 0, -1, 1 };
	static constexpr int dy[4] = { -1, 1, 0, 0 };

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int idx = x + y * width;
			if (visited[idx] || terrainTypes[idx] != TerrainType::GROUND) {
				continue;
			}

			// BFS from this unvisited ground tile
			std::vector<std::pair<int,int>> component;
			std::queue<std::pair<int,int>> frontier;
			frontier.push({ x, y });
			visited[idx] = true;

			while (!frontier.empty()) {
				auto [cx, cy] = frontier.front();
				frontier.pop();
				component.push_back({ cx, cy });

				for (int d = 0; d < 4; ++d) {
					int nx = cx + dx[d];
					int ny = cy + dy[d];
					if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
					int nIdx = nx + ny * width;
					if (!visited[nIdx] && terrainTypes[nIdx] == TerrainType::GROUND) {
						visited[nIdx] = true;
						frontier.push({ nx, ny });
					}
				}
			}

			if (component.size() > largestComponent.size()) {
				largestComponent = std::move(component);
			}
		}
	}

	return largestComponent;
}

bool Map::isWall(int x, int y) const
{
	return !map->isWalkable(x, y);
}

bool Map::isExplored(int x, int y) const
{
	const int index = x + y * width;
	if (index < 0 || index >= static_cast<int>(tiles.size())) { return false; }
	return tiles[index].explored;
}

bool Map::isExplorable(int x, int y) const
{
	// A wall tile is "explorable" (worth drawing with a box-drawing character)
	// if at least one of its 8 neighbours is a floor tile.
	static constexpr int dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	static constexpr int dy[8] = { -1,-1,-1,  0, 0,  1, 1, 1 };
	for (int i = 0; i < 8; ++i) {
		if (!engine.map->isWall(x + dx[i], y + dy[i])) { return true; }
	}
	return false;
}

bool Map::isInFOV(int x, int y) const
{
	if (x < 0 || x >= width || y < 0 || y >= height) { return false; }
	if (map->isInFov(x, y)) {
		tiles[x + y * width].explored = true; // tiles is mutable
		return true;
	}
	return false;
}

void Map::computeFOV()
{
	map->computeFov(engine.player->getX(), engine.player->getY(), engine.fovRadius);

	// Stamp scent on every tile now in FOV, attenuated by distance.
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			if (isInFOV(x, y)) {
				const int  dx         = x - engine.player->getX();
				const int  dy         = y - engine.player->getY();
				const int  distance   = static_cast<int>(std::sqrt(dx * dx + dy * dy));
				const unsigned int newScent = currentScentValue - distance;
				if (newScent > tiles[x + y * width].scent) {
					tiles[x + y * width].scent = newScent;
				}
			}
		}
	}
}

bool Map::canWalk(int x, int y) const
{
	if (isWall(x, y)) { return false; }
	for (const auto& actorPtr : engine.actors) {
		if (actorPtr->blocks && actorPtr->getX() == x && actorPtr->getY() == y) {
			return false;
		}
	}
	return true;
}

// ─── Rendering helpers ───────────────────────────────────────────────────────

// Selects the appropriate double-line box-drawing character for a wall tile
// based on which of its four cardinal neighbours are also wall tiles.
static int chooseWallGlyph(bool top, bool bottom, bool left, bool right)
{
	if ( top &&  bottom &&  left &&  right) return CharConst::DCROSS;
	if (!top &&  bottom &&  left &&  right) return CharConst::DTEES;
	if ( top && !bottom &&  left &&  right) return CharConst::DTEEN;
	if ( top &&  bottom && !left &&  right) return CharConst::DTEEE;
	if ( top &&  bottom &&  left && !right) return CharConst::DTEEW;
	if ( top && !bottom && !left &&  right) return CharConst::DNE;
	if ( top && !bottom &&  left && !right) return CharConst::DNW;
	if (!top &&  bottom &&  left && !right) return CharConst::DSE;
	if (!top &&  bottom && !left &&  right) return CharConst::DSW;
	if ( top &&  bottom && !left && !right) return CharConst::DVLINE;
	if (!top && !bottom &&  left &&  right) return CharConst::DHLINE;
	return CharConst::RADIO_UNSET;
}

static void renderWallTile(int screenX, int screenY, int worldX, int worldY,
	const TCODColor& wallColor, const Map* mapPtr)
{
	const bool top    = mapPtr->isWall(worldX, worldY - 1) && mapPtr->isExplorable(worldX, worldY - 1);
	const bool bottom = mapPtr->isWall(worldX, worldY + 1) && mapPtr->isExplorable(worldX, worldY + 1);
	const bool left   = mapPtr->isWall(worldX - 1, worldY) && mapPtr->isExplorable(worldX - 1, worldY);
	const bool right  = mapPtr->isWall(worldX + 1, worldY) && mapPtr->isExplorable(worldX + 1, worldY);

	renderPutChar(TCODConsole::root->get_data(), screenX, screenY, chooseWallGlyph(top, bottom, left, right), {wallColor.r, wallColor.g, wallColor.b});
}

void Map::render() const
{
	if (levelType == LevelType::OUTDOOR) {
		renderOutdoor();
		return;
	}

	static const TCODColor DARK_WALL   { 0,   0,   100 };
	static const TCODColor DARK_GROUND { 50,  50,  150 };
	static const TCODColor LIGHT_WALL  { 130, 110, 50  };
	static const TCODColor LIGHT_GROUND{ 200, 180, 50  };

	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			auto [screenX, screenY] = engine.camera->apply(x, y);

			if (isInFOV(x, y)) {
				if (isWall(x, y)) {
					renderWallTile(screenX, screenY, x, y, LIGHT_WALL, this);
				} else {
					renderPutChar(TCODConsole::root->get_data(), screenX, screenY, GROUND_GLYPH, {LIGHT_GROUND.r, LIGHT_GROUND.g, LIGHT_GROUND.b});
				}
			} else if (isExplored(x, y)) {
				if (isWall(x, y)) {
					renderWallTile(screenX, screenY, x, y, DARK_WALL, this);
				} else {
					renderPutChar(TCODConsole::root->get_data(), screenX, screenY, GROUND_GLYPH, {DARK_GROUND.r, DARK_GROUND.g, DARK_GROUND.b});
				}
			}
		}
	}
}

// ─── Map generation ──────────────────────────────────────────────────────────

void Map::dig(int x1, int y1, int x2, int y2)
{
	if (x2 < x1) { std::swap(x1, x2); }
	if (y2 < y1) { std::swap(y1, y2); }
	for (int tx = x1; tx <= x2; ++tx) {
		for (int ty = y1; ty <= y2; ++ty) {
			map->setProperties(tx, ty, true, true);
		}
	}
}

void Map::createRoom(bool isFirstRoom, int x1, int y1, int x2, int y2, bool withActors)
{
	dig(x1, y1, x2, y2);
	if (!withActors) { return; }

	const int centreX = (x1 + x2) / 2;
	const int centreY = (y1 + y2) / 2;

	if (isFirstRoom) {
		engine.player->setX(centreX);
		engine.player->setY(centreY);
		if (engine.stairsUp) {
			engine.stairsUp->setX(centreX);
			engine.stairsUp->setY(centreY);
		}
	} else {
		// Every non-first room updates stairsDown position — last room wins.
		if (engine.stairsDown) {
			engine.stairsDown->setX(centreX);
			engine.stairsDown->setY(centreY);
		}

		TCODRandom* rng = TCODRandom::getInstance();
		int monstersToPlace = rng->getInt(0, MAX_ROOM_MONSTERS);
		int itemsToPlace    = rng->getInt(0, MAX_ROOM_ITEMS);

		while (monstersToPlace-- > 0) {
			const int mx = rng->getInt(x1, x2);
			const int my = rng->getInt(y1, y2);
			if (canWalk(mx, my)) { addMonster(mx, my); }
		}
		while (itemsToPlace-- > 0) {
			const int ix = rng->getInt(x1, x2);
			const int iy = rng->getInt(y1, y2);
			if (canWalk(ix, iy)) { addItem(ix, iy); }
		}

		addDecorations(x1, y1, x2, y2);
	}
}

// ─── Lua-driven spawn helpers ────────────────────────────────────────────────

// Maps Lua colour names to TCODColor values.
// Use the shared colour resolver from Colors.h.
static TCODColor colorFromName(const std::string& name)
{
	return Colors::colorFromName(name);
}

// Maps Lua selector-type strings to the enum.
static TargetSelector::SelectorType selectorFromName(const std::string& name)
{
	if (name == "SELF")             return TargetSelector::SelectorType::SELF;
	if (name == "CLOSEST_MONSTER")  return TargetSelector::SelectorType::CLOSEST_MONSTER;
	if (name == "SELECTED_MONSTER") return TargetSelector::SelectorType::SELECTED_MONSTER;
	if (name == "WEARER_RANGE")     return TargetSelector::SelectorType::WEARER_RANGE;
	if (name == "SELECTED_RANGE")   return TargetSelector::SelectorType::SELECTED_RANGE;
	return TargetSelector::SelectorType::SELF;
}

void Map::addMonster(int x, int y)
{
	TCODRandom* rng = TCODRandom::getInstance();
	const int roll = rng->getInt(0, 100);

	try {
		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::string);

		// Inject C++ callback that Lua calls to actually create the actor.
		// Receives (x, y, entry_table) so C++ can read all fields including equipment config.
		lua["addActor"] = [](int ax, int ay, sol::table entry)
		{
			// ── Core enemy fields ──
			std::string name      = entry.get_or(std::string("name"), std::string(""));
			std::string colorName = entry.get_or(std::string("color"), std::string(""));
			int glyph             = entry.get_or(std::string("glyph"), static_cast<int>('?'));
			float hp              = entry.get_or(std::string("hp"), 10.0f);
			float defense         = entry.get_or(std::string("defense"), 0.0f);
			std::string corpse    = entry.get_or(std::string("corpse"), std::string("remains"));
			int xp                = entry.get_or(std::string("xp"), 0);
			float power           = entry.get_or(std::string("power"), 1.0f);
			int skill             = entry.get_or(std::string("skill"), 40);

			// ── Characteristic fields (default 20 for enemies) ──
			int charWS  = entry.get_or(std::string("ws"),  20);
			int charBS  = entry.get_or(std::string("bs"),  20);
			int charS   = entry.get_or(std::string("s"),   20);
			int charT   = entry.get_or(std::string("t"),   20);
			int charAg  = entry.get_or(std::string("ag"),  20);
			int charInt = entry.get_or(std::string("int"), 20);
			int charPer = entry.get_or(std::string("per"), 20);
			int charWP  = entry.get_or(std::string("wp"),  20);
			int charFel = entry.get_or(std::string("fel"), 20);

			TCODColor col = colorFromName(colorName);
			auto monster = std::make_unique<Actor>(ax, ay, glyph, name, col);
			monster->destructible = std::make_unique<MonsterDestructible>(hp, defense, corpse, xp);
			monster->attacker     = std::make_unique<Attacker>(power, skill);
			monster->ai           = std::make_unique<MonsterAi>();

			// ── Attach Characteristics component ──
			auto chars = std::make_shared<Characteristics>(20);
			chars->set(CharId::WS,  charWS);
			chars->set(CharId::BS,  charBS);
			chars->set(CharId::S,   charS);
			chars->set(CharId::T,   charT);
			chars->set(CharId::Ag,  charAg);
			chars->set(CharId::Int, charInt);
			chars->set(CharId::Per, charPer);
			chars->set(CharId::WP,  charWP);
			chars->set(CharId::Fel, charFel);
			monster->characteristics = chars;

			// ── Parse equipment config from the Lua table ──
			EnemyEquipmentConfig equipConfig;
			bool hasEquipmentField = false;
			bool hasEquipTierField = false;

			// Read "equipment" field (list of strings)
			sol::optional<sol::table> equipList = entry["equipment"];
			if (equipList) {
				hasEquipmentField = true;
				for (size_t i = 1; i <= equipList->size(); i++) {
					sol::optional<std::string> itemName = (*equipList)[i];
					if (itemName) {
						equipConfig.equipmentNames.push_back(*itemName);
					}
				}
			}

			// Read "dropChance" field (float)
			sol::optional<float> dropChanceOpt = entry["dropChance"];
			if (dropChanceOpt) {
				float dc = *dropChanceOpt;
				if (dc < 0.0f) {
					engine.gui->message(Colors::damage,
						"Warning: # dropChance clamped to 0.0 (was #)", name, dc);
					dc = 0.0f;
				} else if (dc > 1.0f) {
					engine.gui->message(Colors::damage,
						"Warning: # dropChance clamped to 1.0 (was #)", name, dc);
					dc = 1.0f;
				}
				equipConfig.dropChance = dc;
			}

			// Read "equipTier" field (table with common/uncommon/rare weights)
			sol::optional<sol::table> tierTable = entry["equipTier"];
			if (tierTable) {
				hasEquipTierField = true;
				float defaultCommon = 70.0f;
				float defaultUncommon = 25.0f;
				float defaultRare = 5.0f;
				equipConfig.tierWeights.common   = tierTable->get_or(std::string("common"), defaultCommon);
				equipConfig.tierWeights.uncommon = tierTable->get_or(std::string("uncommon"), defaultUncommon);
				equipConfig.tierWeights.rare     = tierTable->get_or(std::string("rare"), defaultRare);
			}

			// Determine useTierSelection logic:
			// If "equipTier" is present and "equipment" is absent → use tier selection
			// If both are present → named list takes precedence, ignore equipTier
			if (hasEquipTierField && !hasEquipmentField) {
				equipConfig.useTierSelection = true;
			}

			// Store the parsed config on the monster for later use by the equipment
			// resolution step. Only store if the enemy actually has equipment-related fields.
			if (hasEquipmentField || hasEquipTierField) {
				monster->equipConfig = std::make_unique<EnemyEquipmentConfig>(std::move(equipConfig));
			}

			engine.actors.push_back(std::move(monster));

			// ── Equipment resolution (task 2.2) ──
			// Resolve equipment config and attach items to the newly created enemy.
			Actor* spawned = engine.actors.back().get();
			if (spawned->equipConfig && !engine.equipmentTemplates.empty()) {
				const auto& cfg = *spawned->equipConfig;

				// Create an Equipment instance on the enemy
				spawned->equipment = std::make_unique<Equipment>();
				spawned->equipment->dropChance = cfg.dropChance;

				// Collect resolved templates
				std::vector<const EquipmentTemplate*> resolvedTemplates;

				if (!cfg.equipmentNames.empty()) {
					// Named equipment: look up each name in engine.equipmentTemplates
					for (const auto& itemName : cfg.equipmentNames) {
						const EquipmentTemplate* found = nullptr;
						for (const auto& tmpl : engine.equipmentTemplates) {
							if (tmpl.name == itemName) {
								found = &tmpl;
								break;
							}
						}
						if (!found) {
							engine.gui->message(Colors::damage,
								"Warning: equipment template '#' not found for #", itemName, name);
							continue;
						}
						resolvedTemplates.push_back(found);
					}
				} else if (cfg.useTierSelection) {
					// Tier-based random selection using the reusable helper
					// For each slot, use the helper to pick a template based on tier weights
					for (int slotIdx = 0; slotIdx < static_cast<int>(EquipmentSlot::COUNT); ++slotIdx) {
						EquipmentSlot targetSlot = static_cast<EquipmentSlot>(slotIdx);
						const EquipmentTemplate* selected = engine.selectEquipmentByTier(targetSlot, cfg.tierWeights);
						if (selected) {
							resolvedTemplates.push_back(selected);
						}
					}
				}

				// Create item Actors from resolved templates, equip them
				// Track which slots have been filled to detect conflicts
				std::array<bool, static_cast<int>(EquipmentSlot::COUNT)> slotFilled = {};

				for (const auto* tmpl : resolvedTemplates) {
					int slotIdx = static_cast<int>(tmpl->slot);

					// Warn about slot conflicts (multiple items for same slot)
					if (slotFilled[slotIdx]) {
						engine.gui->message(Colors::damage,
							"Warning: # has multiple items for same slot, equipping '#' last",
							name, tmpl->name);
					}
					slotFilled[slotIdx] = true;

					// Create item Actor with Equippable component
					auto item = std::make_unique<Actor>(0, 0, tmpl->glyph, tmpl->name, tmpl->color);
					item->blocks = false;
					item->equippable = std::make_shared<Equippable>(
						tmpl->slot, tmpl->modifiers, tmpl->weight, tmpl->value);

					// Equip the item (pass nullptr for Container since enemies don't use inventory)
					Actor* itemPtr = item.get();
					spawned->equipment->ownedItems.push_back(std::move(item));
					spawned->equipment->equip(itemPtr, nullptr, spawned->attacker.get());
				}

				// Consume the equipConfig — it's no longer needed
				spawned->equipConfig.reset();
			}
		};

		lua.script_file("Scripts/Enemies.lua");
		lua["spawnEnemy"](roll, x, y);

	} catch (const sol::error& /*e*/) {
		// Lua script failed — fall back to hard-coded spawn so the game still works.
		if (roll < 80) {
			auto ork = std::make_unique<Actor>(x, y, 'o', "Ork", Colors::orkSkin);
			ork->destructible = std::make_unique<MonsterDestructible>(10.0f, 0.0f, "dead Ork", 35);
			ork->attacker     = std::make_unique<Attacker>(3.0f, 40);
			ork->ai           = std::make_unique<MonsterAi>();
			engine.actors.push_back(std::move(ork));
		} else {
			auto nob = std::make_unique<Actor>(x, y, 'N', "Nob", Colors::nobArmour);
			nob->destructible = std::make_unique<MonsterDestructible>(16.0f, 1.0f, "Nob carcass", 100);
			nob->attacker     = std::make_unique<Attacker>(4.0f, 40);
			nob->ai           = std::make_unique<MonsterAi>();
			engine.actors.push_back(std::move(nob));
		}
	}
}

void Map::addItem(int x, int y)
{
	TCODRandom* rng  = TCODRandom::getInstance();

	// 25% chance to spawn equipment if templates are available
	if (!engine.equipmentTemplates.empty() && rng != nullptr && rng->getInt(0, 99) < 25) {
		int templateIndex = rng->getInt(0, static_cast<int>(engine.equipmentTemplates.size()) - 1);
		const auto& tmpl = engine.equipmentTemplates[templateIndex];

		auto item = std::make_unique<Actor>(x, y, tmpl.glyph, tmpl.name, tmpl.color);
		item->blocks = false;
		item->pickable = std::make_shared<Pickable>(nullptr, nullptr);
		item->pickable->weight = tmpl.weight;
		item->pickable->value = tmpl.value;
		item->equippable = std::make_shared<Equippable>(tmpl.slot, tmpl.modifiers, tmpl.weight, tmpl.value);
		engine.actors.emplace_front(std::move(item));
		return;
	}

	const int   roll = rng->getInt(0, 100);

	try {
		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::string);

		// Inject C++ factory callbacks that Lua's spawnItem function will call.
		lua["spawnHealthPotion"] = [](int ix, int iy, float healAmount) {
			auto potion = std::make_unique<Actor>(ix, iy, '!', "health potion", Colors::healthPotion);
			potion->blocks   = false;
			potion->pickable = std::make_unique<Pickable>(
				std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f),
				std::make_unique<HealthEffect>(healAmount, "", Colors::uiText));
			engine.actors.emplace_front(std::move(potion));
		};

		lua["spawnDamageScroll"] = [](int ix, int iy, float damage, const std::string& name,
			const std::string& message, const std::string& colorName,
			const std::string& selectorName, float range)
		{
			TCODColor col = colorFromName(colorName);
			auto scroll = std::make_unique<Actor>(ix, iy, '#', name, Colors::scroll);
			scroll->blocks   = false;
			scroll->pickable = std::make_unique<Pickable>(
				std::make_unique<TargetSelector>(selectorFromName(selectorName), range),
				std::make_unique<HealthEffect>(damage, message, col));
			engine.actors.emplace_front(std::move(scroll));
		};

		lua["spawnConfusionScroll"] = [](int ix, int iy, int turns, const std::string& name,
			const std::string& message, const std::string& colorName, float range)
		{
			TCODColor col = colorFromName(colorName);
			auto scroll = std::make_unique<Actor>(ix, iy, '#', name, Colors::scroll);
			scroll->blocks   = false;
			scroll->pickable = std::make_unique<Pickable>(
				std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELECTED_MONSTER, range),
				std::make_unique<AiChangeEffect>(
					std::make_unique<ConfusedMonsterAi>(turns), message, col));
			engine.actors.emplace_front(std::move(scroll));
		};

		lua.script_file("Scripts/Items.lua");
		lua["spawnItem"](roll, x, y);

	} catch (const sol::error& /*e*/) {
		// Lua script failed — fall back to a simple health potion.
		auto potion = std::make_unique<Actor>(x, y, '!', "health potion", Colors::healthPotion);
		potion->blocks   = false;
		potion->pickable = std::make_unique<Pickable>(
			std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f),
			std::make_unique<HealthEffect>(4.0f, "", Colors::uiText));
		engine.actors.emplace_front(std::move(potion));
	}
}

// ─── Decoration spawning ─────────────────────────────────────────────────────

void Map::addDecorations(int x1, int y1, int x2, int y2)
{
	// Skip if no templates available
	if (engine.decorationTemplates.empty()) { return; }

	// Load maxRoomDecorations from Config.lua (default 3), treat < 0 as 0
	int maxDecorations = 3;
	try {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		lua.script_file("Scripts/Config.lua");

		sol::table cfg = lua["config"];
		if (cfg.valid()) {
			maxDecorations = cfg.get_or("maxRoomDecorations", 3);
		}
	} catch (const sol::error&) {
		// Config load failed — use default
	}

	if (maxDecorations < 0) { maxDecorations = 0; }
	if (maxDecorations == 0) { return; }

	// Pick random count in [0, maxDecorations]
	TCODRandom* rng = TCODRandom::getInstance();
	int count = rng->getInt(0, maxDecorations);

	static constexpr int MAX_PLACEMENT_ATTEMPTS = 20;

	for (int i = 0; i < count; ++i) {
		// Select a random template
		int templateIdx = rng->getInt(0, static_cast<int>(engine.decorationTemplates.size()) - 1);
		const auto& tmpl = engine.decorationTemplates[templateIdx];

		// Find a walkable unoccupied tile within the room bounds
		bool placed = false;
		for (int attempt = 0; attempt < MAX_PLACEMENT_ATTEMPTS; ++attempt) {
			int dx = rng->getInt(x1, x2);
			int dy = rng->getInt(y1, y2);
			if (canWalk(dx, dy)) {
				auto decoration = std::make_unique<Actor>(dx, dy, tmpl.glyph, tmpl.name, tmpl.color);
				decoration->blocks      = tmpl.blocks;
				decoration->fovOnly     = true;
				decoration->description = tmpl.description;
				decoration->coverValue  = tmpl.coverValue;
				// No Ai, Attacker, Destructible, or Pickable — they default to nullptr
				engine.actors.push_back(std::move(decoration));
				placed = true;
				break;
			}
		}
		// If no valid tile found, skip this placement silently
	}
}

// ─── Accessors ───────────────────────────────────────────────────────────────

int Map::getWidth()  const { return width;  }
int Map::getHeight() const { return height; }
void Map::setWidth(int w)  { width  = w; }
void Map::setHeight(int h) { height = h; }

unsigned int Map::getScent(int x, int y) const
{
	return tiles[x + y * width].scent;
}
