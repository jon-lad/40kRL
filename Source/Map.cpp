
#include <memory>
#include <vector>
#include <list>
#include <sstream>
#include "main.h"

static constexpr int ROOM_MAX_SIZE     = 12;
static constexpr int ROOM_MIN_SIZE     = 6;
static constexpr int MAX_ROOM_MONSTERS = 3;
static constexpr int MAX_ROOM_ITEMS    = 2;
static constexpr int BSP_DEPTH         = 8;
static constexpr char GROUND_GLYPH     = '.';

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

void Map::init(bool withActors)
{
	rng   = std::make_unique<TCODRandom>(seed, TCOD_RNG_CMWC);
	tiles = std::vector<Tile>(width * height);
	map   = std::make_unique<TCODMap>(width, height);

	TCODBsp bsp(0, 0, width, height);
	bsp.splitRecursive(rng.get(), BSP_DEPTH, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);

	BspListener listener(*this);
	bsp.traverseInvertedLevelOrder(&listener, reinterpret_cast<void*>(withActors));
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
static TCOD_chars_t chooseWallGlyph(bool top, bool bottom, bool left, bool right)
{
	if ( top &&  bottom &&  left &&  right) return TCOD_CHAR_DCROSS;
	if (!top &&  bottom &&  left &&  right) return TCOD_CHAR_DTEES;
	if ( top && !bottom &&  left &&  right) return TCOD_CHAR_DTEEN;
	if ( top &&  bottom && !left &&  right) return TCOD_CHAR_DTEEE;
	if ( top &&  bottom &&  left && !right) return TCOD_CHAR_DTEEW;
	if (!top &&  bottom &&  left && !right) return TCOD_CHAR_DNE;
	if (!top &&  bottom && !left &&  right) return TCOD_CHAR_DNW;
	if ( top && !bottom &&  left && !right) return TCOD_CHAR_DSE;
	if ( top && !bottom && !left &&  right) return TCOD_CHAR_DSW;
	if ( top &&  bottom && !left && !right) return TCOD_CHAR_DVLINE;
	if (!top && !bottom &&  left &&  right) return TCOD_CHAR_DHLINE;
	return TCOD_CHAR_RADIO_UNSET;
}

static void renderWallTile(int screenX, int screenY, int worldX, int worldY,
	const TCODColor& wallColor, const Map* mapPtr)
{
	const bool top    = mapPtr->isWall(worldX, worldY - 1) && mapPtr->isExplorable(worldX, worldY - 1);
	const bool bottom = mapPtr->isWall(worldX, worldY + 1) && mapPtr->isExplorable(worldX, worldY + 1);
	const bool left   = mapPtr->isWall(worldX - 1, worldY) && mapPtr->isExplorable(worldX - 1, worldY);
	const bool right  = mapPtr->isWall(worldX + 1, worldY) && mapPtr->isExplorable(worldX + 1, worldY);

	TCODConsole::root->setChar(screenX, screenY, chooseWallGlyph(top, bottom, left, right));
	TCODConsole::root->setCharForeground(screenX, screenY, wallColor);
}

void Map::render() const
{
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
					TCODConsole::root->setChar(screenX, screenY, GROUND_GLYPH);
					TCODConsole::root->setCharForeground(screenX, screenY, LIGHT_GROUND);
				}
			} else if (isExplored(x, y)) {
				if (isWall(x, y)) {
					renderWallTile(screenX, screenY, x, y, DARK_WALL, this);
				} else {
					TCODConsole::root->setChar(screenX, screenY, GROUND_GLYPH);
					TCODConsole::root->setCharForeground(screenX, screenY, DARK_GROUND);
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
	} else {
		engine.stairs->setX(centreX);
		engine.stairs->setY(centreY);

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
	}
}

void Map::addMonster(int x, int y)
{
	TCODRandom* rng = TCODRandom::getInstance();

	if (rng->getInt(0, 100) < 80) {
		auto ork = std::make_unique<Actor>(x, y, 'o', "Ork", TCODColor::desaturatedGreen);
		ork->destructible = std::make_unique<MonsterDestructible>(10.0f, 0.0f, "dead Ork", 35);
		ork->attacker     = std::make_unique<Attacker>(3.0f);
		ork->ai           = std::make_unique<MonsterAi>();
		engine.actors.push_back(std::move(ork));
	} else {
		auto nob = std::make_unique<Actor>(x, y, 'N', "Nob", TCODColor::darkerGreen);
		nob->destructible = std::make_unique<MonsterDestructible>(16.0f, 1.0f, "Nob carcass", 100);
		nob->attacker     = std::make_unique<Attacker>(4.0f);
		nob->ai           = std::make_unique<MonsterAi>();
		engine.actors.push_back(std::move(nob));
	}
}

void Map::addItem(int x, int y)
{
	TCODRandom* rng  = TCODRandom::getInstance();
	const int   roll = rng->getInt(0, 100);

	// Spawn chances: 70% health potion, 10% lightning bolt, 10% fireball, 10% confusion scroll.
	if (roll < 70) {
		auto potion = std::make_unique<Actor>(x, y, '!', "health potion", TCOD_violet);
		potion->blocks   = false;
		potion->pickable = std::make_unique<Pickable>(
			std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f),
			std::make_unique<HealthEffect>(4.0f, "", TCODColor::lightGrey));
		engine.actors.emplace_front(std::move(potion));

	} else if (roll < 80) {
		auto scroll = std::make_unique<Actor>(x, y, '#', "scroll of lightning bolt", TCOD_light_yellow);
		scroll->blocks   = false;
		scroll->pickable = std::make_unique<Pickable>(
			std::make_unique<TargetSelector>(TargetSelector::SelectorType::CLOSEST_MONSTER, 5.0f),
			std::make_unique<HealthEffect>(-20.0f,
				"A lightning bolt strikes the #\nwith a crack of loud thunder!\nThe damage is # hit points.",
				TCODColor::lightBlue));
		engine.actors.emplace_front(std::move(scroll));

	} else if (roll < 90) {
		auto scroll = std::make_unique<Actor>(x, y, '#', "scroll of fireball", TCOD_light_yellow);
		scroll->blocks   = false;
		scroll->pickable = std::make_unique<Pickable>(
			std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELECTED_RANGE, 3.0f),
			std::make_unique<HealthEffect>(-12.0f,
				"The # gets burned for # hit points.",
				TCODColor::orange));
		engine.actors.emplace_front(std::move(scroll));

	} else {
		auto scroll = std::make_unique<Actor>(x, y, '#', "scroll of confusion", TCOD_light_yellow);
		scroll->blocks   = false;
		scroll->pickable = std::make_unique<Pickable>(
			std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELECTED_MONSTER, 5.0f),
			std::make_unique<AiChangeEffect>(
				std::make_unique<ConfusedMonsterAi>(10),
				"The eyes of the # glaze over as he starts to stumble around!",
				TCOD_light_green));
		engine.actors.emplace_front(std::move(scroll));
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
