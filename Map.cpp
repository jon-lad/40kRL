
#include <memory>
#include <vector>
#include <list>
#include <sstream>
#include "main.h"

static constexpr auto ROOM_MAX_SIZE = 12;
static constexpr auto ROOM_MIN_SIZE = 6;

static constexpr auto MAX_ROOM_MONSTERS = 3;
static constexpr auto MAX_ROOM_ITEMS = 2;


static constexpr auto ground = '.';



class BspListener : public ITCODBspCallback {
private:
	Map& map; // a map to dig
	int roomNum; // room number
	int lastx, lasty; // center of the last room
public:
	BspListener(Map& map) : map(map), roomNum(0) {}
	bool visitNode(TCODBsp* node, void* userData) {
		if (node->isLeaf()) {
			int x, y, w, h;
			bool withActors = (bool) userData;
			// dig a room

			w = map.rng->getInt(ROOM_MIN_SIZE, node->w - 2);
			h = map.rng->getInt(ROOM_MIN_SIZE, node->h - 2);
			x = map.rng->getInt(node->x + 1, node->x + node->w - w - 1);
			y = map.rng->getInt(node->y + 1, node->y + node->h - h - 1);
			map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1, withActors);
			if (roomNum != 0) {
				// dig a corridor from last room
				map.dig(lastx, lasty, x + w / 2, lasty);
				map.dig(x + w / 2, lasty, x + w / 2, y + h / 2);
			}
			lastx = x + w / 2;
			lasty = y + h / 2;
			roomNum++;
		}
		return true;
	}
};



Map::Map(int width, int height) : width{ width }, height{ height }, currentScentValue{SCENT_THRESHOLD}
{
	seed = TCODRandom::getInstance()->getInt(0, 0x7FFFFFFF);

}

void Map::init(bool withActors) {
	rng = std::make_unique<TCODRandom>(seed, TCOD_RNG_CMWC);
	//tiles is one dimensional so calculates size of the vector
	int size = height * width;
	//Set a vector of tiles for each co ordinate on map to store extra info 
	tiles = std::vector<Tile>(size, Tile());
	//create TCODMAp to calculate FOV and wether co-ordinate is passable
	map = std::make_unique<TCODMap>(width, height);
	//Create Binary Space partittion for Room generation
	TCODBsp bsp(0, 0, width, height);
	//Split the BSP into Partitions which are larger than Room Max size
	bsp.splitRecursive(rng.get(), 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5, 1.5);
	//new BSP listener to create rooms using the generated partitions
	BspListener listener(*this);
	//Traverse the BSP and create the rooms and connect with corridors
	bsp.traverseInvertedLevelOrder(&listener, (void*)withActors);
}

//returns if position is a wall
bool Map::isWall(int x, int y) const
{
	
	return !map->isWalkable(x,y);
}

//returns ex x, y position has been in FOV
bool Map::isExplored(int x, int y) const{
	int tileNo = x + y * width;
	if (tileNo < tiles.size()) {
		return tiles.at(tileNo).explored;
	}
	return false;
}

bool Map::isExplorable(int x, int y)const {
	static constexpr int tdx[8] = { -1,0,1,-1,1,-1,0,1 };
	static constexpr int tdy[8] = { -1,-1,-1,0,0,1,1,1 };
	for (int i = 0; i < 8; i++) {
		int cellx = x + tdx[i];
		int celly = y + tdy[i];
		if (!engine.map->isWall(cellx, celly)) {
			return true;
		}
	}
	return false;
}

/*Find out if x and y are in FOV  if so returns truesets tiles 
to explored if they have been seen. This is const as is called by render function*/
bool Map::isInFOV(int x, int y) const{
	if (x < 0 || x >= width || y < 0 || y >= height) {
		return false;
	}
	if (map->isInFov(x, y)) {
		//tiles is mutable so can be modified.
		tiles.at(x + y * width).explored = true;
		return true;
	}
	return false;
}

void Map::computeFOV() {
	//use TCODMap to compute Field of view for player
	map->computeFov(engine.player->getX(), engine.player->getY(), engine.FOVRadius);
	//update scent field
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			if (isInFOV(x, y)) {
				unsigned int oldScent = getScent(x, y);
				int dx = x - engine.player->getX();
				int dy = y - engine.player->getY();
				long distance = (int)std::sqrt(dx * dx + dy * dy);
				unsigned int newScent = currentScentValue - distance;
				if (newScent > oldScent) {
					tiles.at(x + (y * width)).scent = newScent;
				}
			}
		}
	}
}

bool Map::canWalk(int x, int y) const{
	if (isWall(x, y)) {
		return false;
	}
	for (const auto& actor : engine.actors) {
		if (actor->blocks && actor->getX() == x && actor->getY() == y){
			return false;
		}
	}
	return true;
}

void Map::render() const
{
	//constanst colours to render
	static const TCODColor darkWall{ 0,0,100 };
	static const TCODColor darkGround{ 50,50,150 };
	static const TCODColor lightWall{ 130,110,50 };
	static const TCODColor lightGround{ 200,180,50 };

	//for each position on map
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{

			//render scent
			int scent = SCENT_THRESHOLD - (currentScentValue - getScent(x, y));
			scent = CLAMP(0, 10, scent);
			float sc = scent * 0.1f;

			//if it is if filed of view render light
			if (isInFOV(x, y)) {
				TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? lightWall : TCODColor::lightGrey * sc);
			}
			//if it has previously been seen render dark
			else if (isExplored(x, y)) {
				TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? darkWall : TCODColor::lightGrey * sc);
			}
			else if (!isWall(x, y)) {
				TCODConsole::root->setCharBackground(x, y, TCODColor::white * sc);
			}

			// dont render scent

			//if it is if filed of view render light
			/*if (isInFOV(x, y)) {
				if (isWall(x, y)) {
					bool isTopExploredWall = isWall(x, y - 1) && isExplorable(x, y - 1);
					bool isBottomExploredWall = isWall(x, y + 1) && isExplorable(x, y + 1);
					bool isLeftExploredWall = isWall(x -1, y) && isExplorable(x - 1, y);
					bool isRightExploredWall = isWall(x + 1, y) && isExplorable(x + 1, y);
			
					TCOD_chars_t tileChar = TCOD_CHAR_RADIO_UNSET;

					if (isTopExploredWall && isBottomExploredWall && isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DCROSS;
					}
					else if (!isTopExploredWall && isBottomExploredWall && isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DTEES;
					}
					else if (isTopExploredWall && !isBottomExploredWall && isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DTEEN;
					}
					else if (isTopExploredWall && isBottomExploredWall && !isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DTEEE;
					}
					else if (isTopExploredWall && isBottomExploredWall && isLeftExploredWall && !isRightExploredWall) {
						tileChar = TCOD_CHAR_DTEEW;
					}
					else if (!isTopExploredWall && isBottomExploredWall && isLeftExploredWall && !isRightExploredWall) {
						tileChar = TCOD_CHAR_DNE;
					}
					else if (!isTopExploredWall && isBottomExploredWall && !isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DNW;
					}
					else if (isTopExploredWall && !isBottomExploredWall && isLeftExploredWall && !isRightExploredWall) {
						tileChar = TCOD_CHAR_DSE;
					}
					else if (isTopExploredWall && !isBottomExploredWall && !isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DSW;
					}
					else if (isTopExploredWall && isBottomExploredWall && !isLeftExploredWall && !isRightExploredWall) {
						tileChar = TCOD_CHAR_DVLINE;
					}
					else if (!isTopExploredWall && !isBottomExploredWall && isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DHLINE;
					}
					
					TCODConsole::root->setChar(x, y, tileChar);
					TCODConsole::root->setCharForeground(x, y, lightWall);
				}
				else {
					TCODConsole::root->setChar(x, y, ground);
					TCODConsole::root->setCharForeground(x, y, lightGround); 
				}
				
			}
			//if it has previously been seen render dark
			else if (isExplored(x, y)) {
				if (isWall(x, y)) {
					bool isTopExploredWall = isWall(x, y - 1) && isExplorable(x, y - 1);
					bool isBottomExploredWall = isWall(x, y + 1) && isExplorable(x, y + 1);
					bool isLeftExploredWall = isWall(x - 1, y) && isExplorable(x - 1, y);
					bool isRightExploredWall = isWall(x + 1, y) && isExplorable(x + 1, y);

					TCOD_chars_t tileChar = TCOD_CHAR_RADIO_UNSET;

					if (isTopExploredWall && isBottomExploredWall && isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DCROSS;
					}
					else if (!isTopExploredWall && isBottomExploredWall && isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DTEES;
					}
					else if (isTopExploredWall && !isBottomExploredWall && isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DTEEN;
					}
					else if (isTopExploredWall && isBottomExploredWall && !isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DTEEE;
					}
					else if (isTopExploredWall && isBottomExploredWall && isLeftExploredWall && !isRightExploredWall) {
						tileChar = TCOD_CHAR_DTEEW;
					}
					else if (!isTopExploredWall && isBottomExploredWall && isLeftExploredWall && !isRightExploredWall) {
						tileChar = TCOD_CHAR_DNE;
					}
					else if (!isTopExploredWall && isBottomExploredWall && !isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DNW;
					}
					else if (isTopExploredWall && !isBottomExploredWall && isLeftExploredWall && !isRightExploredWall) {
						tileChar = TCOD_CHAR_DSE;
					}
					else if (isTopExploredWall && !isBottomExploredWall && !isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DSW;
					}
					else if (isTopExploredWall && isBottomExploredWall && !isLeftExploredWall && !isRightExploredWall) {
						tileChar = TCOD_CHAR_DVLINE;
					}
					else if (!isTopExploredWall && !isBottomExploredWall && isLeftExploredWall && isRightExploredWall) {
						tileChar = TCOD_CHAR_DHLINE;
					}
					TCODConsole::root->setChar(x, y, tileChar);
					TCODConsole::root->setCharForeground(x, y, darkWall);
				}
				else {
					TCODConsole::root->setChar(x, y, ground);
					TCODConsole::root->setCharForeground(x, y, darkGround);
				}
			}*/
		}
	}
}

//Dig space out of map
void Map::dig(int x1, int y1, int x2, int y2)
{
	//makes sure x1 is smaller than x2
	if (x2 < x1) {
		int temp = x2;
		x2 = x1;
		x1 = temp;
	}
	//make sure y1 is smaller than y2
	if (y2 < y1) {
		int temp = y2;
		y2 = y1;
		y1 = temp;
	}
	//for tiles in range set transparent and walkable on TCODMAp
	for(int tilex = x1; tilex <= x2; tilex++){ 
		for (int tiley = y1; tiley <= y2; tiley++) {
			map->setProperties(tilex, tiley, true, true);
		}
	}
}

/*Dig a room if its the first put player in the room susequent rooms have 1/4 
chance of spawning an actor*/
void Map::createRoom(bool first, int x1, int y1, int x2, int y2, bool withActors)
{
	dig(x1, y1, x2, y2);
	if (!withActors) {
		return;
	}
	if (first) {
		//put the player in the first room
		engine.player->setX((x1 + x2) / 2);
		engine.player->setY((y1 + y2) / 2);
	}
	else {
		//set starir position
		engine.stairs->setX((x1 + x2) / 2);
		engine.stairs->setY((y1 + y2) / 2);
		TCODRandom* rng = TCODRandom::getInstance();
		int nbMonsters = rng->getInt(0, MAX_ROOM_MONSTERS);
		int nbItems = rng->getInt(0, MAX_ROOM_ITEMS);
		while (nbMonsters> 0){
			int x = rng->getInt(x1, x2);
			int y = rng->getInt(y1, y2);
			if (canWalk(x, y)) {
				addMonster(x, y);
			}
			nbMonsters--;
		}
		while (nbItems > 0) {
			int x = rng->getInt(x1, x2);
			int y = rng->getInt(y1, y2);
			if (canWalk(x, y)) {
				addItem(x, y);
			}
			nbItems--;
		}

	}
}

void Map::addMonster(int x, int y) {
	TCODRandom* rng = TCODRandom::getInstance();
	if (rng->getInt(0, 100) < 80) {
		//create an orc
		auto ork = std::make_unique<Actor>(x, y, 'o', "Ork", TCODColor::desaturatedGreen);
		ork->destructible =std::move(std::make_unique<MonsterDestructible>(10.0f, 0.0f, "dead Ork", 35));
		ork->attacker = std::move(std::make_unique<Attacker>(3.0f));
		ork->ai = std::move(std::make_unique<MonsterAi>());
		engine.actors.push_back(std::move(ork));
		
	}
	else {
		//create a troll
		auto Nob = std::make_unique<Actor>(x, y, 'N', "Nob", TCODColor::darkerGreen);
		Nob->destructible = std::move(std::make_unique<MonsterDestructible>(16.0f, 1.0f, "Nob carcass", 100));
		Nob->attacker = std::move(std::make_unique<Attacker>(4.0f));
		Nob->ai = std::move(std::make_unique<MonsterAi>());
		engine.actors.push_back(std::move(Nob));
	}
	
}

void Map::addItem(int x, int y) {
	TCODRandom* rng = TCODRandom::getInstance();
	auto dice = rng->getInt(0, 100);
	if (dice < 70) {
		//create a health potion
		std::unique_ptr<Actor> healthPotion = std::make_unique<Actor>(x, y, '!', "health potion", TCOD_violet);
		healthPotion->blocks = false;
		healthPotion->pickable = std::make_unique<Pickable>(std::move(std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELF, 0.0f)), std::move(std::make_unique<HealthEffect>(4.0f, "", TCODColor::lightGrey)));
		engine.actors.emplace_front(std::move(healthPotion));
	}
	else if (dice < 70 + 10) {
		//create a scroll of Lightning bolt
		std::unique_ptr<Actor> scrollOfLightningBolt = std::make_unique<Actor>(x, y, '#', "scroll of lightning bolt", TCOD_light_yellow);
		scrollOfLightningBolt->blocks = false;
		scrollOfLightningBolt->pickable = std::make_unique<Pickable>(std::move(std::make_unique<TargetSelector>(TargetSelector::SelectorType::CLOSEST_MONSTER,5.0f)),
			std::move(std::make_unique<HealthEffect>(-20.0f,"A lightning bolt strikes the # \nwith the sound of loud thunder!\nThe damege is # hit points.", TCODColor::lightBlue)));
		engine.actors.emplace_front(std::move(scrollOfLightningBolt));
	}
	else if (dice < 70 + 10+10) {
		//create a scroll of Fireball
		std::unique_ptr<Actor> scrollOfFireball = std::make_unique<Actor>(x, y, '#', "scroll of fireball", TCOD_light_yellow);
		scrollOfFireball->blocks = false;
		scrollOfFireball->pickable = std::make_unique<Pickable>(std::move(std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELECTED_RANGE, 3.0f)),
			std::move(std::make_unique<HealthEffect>(-12.0f, "The # gets burned for # hit points.", TCODColor::orange)));
		engine.actors.emplace_front(std::move(scrollOfFireball));
	}
	else if (dice < 70 + 10 + 10 + 10) {
		//create a scroll of Confusion
		std::unique_ptr<Actor> scrollOfConfusion = std::make_unique<Actor>(x, y, '#', "scroll of confusion", TCOD_light_yellow);
		scrollOfConfusion->blocks = false;
		scrollOfConfusion->pickable = std::make_unique<Pickable>(std::move(std::make_unique<TargetSelector>(TargetSelector::SelectorType::SELECTED_MONSTER, 5.0f)),
			std::move(std::make_unique<AiChangeEffect>(std::make_unique<ConfusedMonsterAi>(10), "The eyes of the # glaze over\nas he starts to stumble around!.", TCOD_light_green)));
		engine.actors.emplace_front(std::move(scrollOfConfusion));
	}
}


//Get and sets for width and height
int Map::getWidth() const
{
	return width;
}
void Map::setWidth(int width)
{
	this->width = width;
}

int Map::getHeight() const
{
	return height;
}
void Map::setHeight(int height)
{
	this->height = height;
}


unsigned int Map::getScent(int x, int y) const {
	int location = x + (y * width);
	return tiles.at(location).scent;
}


