#include <memory>
#include <vector>
#include <list>
#include "main.h"

static constexpr int ROOM_MAX_SIZE = 12;
static constexpr int ROOM_MIN_SIZE = 6;

static constexpr int MAX_ROOM_MONSTERS = 3;

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
			// dig a room
			TCODRandom* rng = TCODRandom::getInstance();
			w = rng->getInt(ROOM_MIN_SIZE, node->w - 2);
			h = rng->getInt(ROOM_MIN_SIZE, node->h - 2);
			x = rng->getInt(node->x + 1, node->x + node->w - w - 1);
			y = rng->getInt(node->y + 1, node->y + node->h - h - 1);
			map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1);
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



Map::Map(int width, int height) : width{ width }, height{ height }
{
	//tiles is one dimensional so calculates size of the vector
	int size = height * width;
	//Set a vector of tiles for each co ordinate on map to store extra info 
	tiles = std::vector<Tile>(size,Tile());
	//create TCODMAp to calculate FOV and wether co-ordinate is passable
	map = std::make_unique<TCODMap>(width, height);
	//Create Binary Space partittion for Room generation
	TCODBsp bsp(0, 0, width, height);
	//Split the BSP into Partitions which are larger than Room Max size
	bsp.splitRecursive(NULL, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5, 1.5);
	//new BSP listener to create rooms using the generated partitions
	BspListener listener(*this);
	//Traverse the BSP and create the rooms and connect with corridors
	bsp.traverseInvertedLevelOrder(&listener,NULL);
	
}

//returns if position is a wall
bool Map::isWall(int x, int y) const
{
	
	return !map->isWalkable(x,y);
}

//returns ex x, y position has been in FOV
bool Map::isExplored(int x, int y) const {

	return tiles.at(x + y * width).explored;
}

/*Find out if x and y are in FOV  if so returns truesets tiles 
to explored if they have been seen. This is const as is called by render function*/
bool Map::isInFOV(int x, int y) const {
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
}

bool Map::canWalk(int x, int y) const{
	if (isWall(x, y)) {
		return false;
	}
	for (std::list<std::unique_ptr<Actor>>::iterator i = engine.actors.begin(); i != engine.actors.end(); ++i) {
		if (i->get()->blocks && i->get()->getX() == x && i->get()->getY() == y){
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
			//if it is if filed of view reder light
			if (isInFOV(x, y)) {
				TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? lightWall : lightGround);
			}
			//if it has previously been seen render dark
			else if (isExplored(x, y)) {
				TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? darkWall : darkGround);
			}
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
void Map::createRoom(bool first, int x1, int y1, int x2, int y2)
{
	dig(x1, y1, x2, y2);
	if (first) {
		//put the player in the first room
		engine.player->setX((x1 + x2) / 2);
		engine.player->setY((y1 + y2) / 2);
	}
	else {
		TCODRandom* rng = TCODRandom::getInstance();
		int nbMonsters = rng->getInt(0, MAX_ROOM_MONSTERS);
		while (nbMonsters> 0){
			int x = rng->getInt(x1, x2);
			int y = rng->getInt(y1, y2);
			if (canWalk(x, y)) {
				addMonster(x, y);
			}
			nbMonsters--;
		}
	}
}

void Map::addMonster(int x, int y) {
	TCODRandom* rng = TCODRandom::getInstance();
	if (rng->getInt(0, 100) < 80) {
		//create an orc
		std::unique_ptr<Actor> ork = std::make_unique<Actor>(x, y, 'o', "Ork", TCODColor::desaturatedGreen);
		ork->destructible =std::move(std::make_unique<Destructible>(10.0f, 0.0f, "dead Ork"));
		ork->attacker = std::move(std::make_unique<Attacker>(3.0f));
		ork->ai = std::move(std::make_unique<MonsterAi>());
		engine.actors.emplace_back(std::move(ork));
		
	}
	else {
		//create a troll
		std::unique_ptr<Actor> Nob = std::make_unique<Actor>(x, y, 'N', "Nob", TCODColor::darkerGreen);
		Nob->destructible = std::move(std::make_unique<Destructible>(16.0f, 1.0f, "Nob carcass"));
		Nob->attacker = std::move(std::make_unique<Attacker>(4.0f));
		Nob->ai = std::move(std::make_unique<MonsterAi>());
		engine.actors.emplace_back(std::move(Nob));
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



