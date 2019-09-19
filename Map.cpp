#include <memory>
#include <vector>
#include "libtcod.h"
#include "Map.h"

Map::Map(int width, int height) : width{ width }, height{ height }
{
	int size = height * width;
	tiles = std::vector<Tile>(size,Tile());
	setWall(30, 22);
	setWall(50, 22);
}

bool Map::isWall(int x, int y) const
{
	int position{};
	
	position = x + (y * width);
	
	return !tiles.at(position).canWalk;
}

void Map::setWall(int x, int y)
{
	tiles.at(x + (y * width)).canWalk = false;
}

void Map::render() const
{
	static const TCODColor darkWall{ 0,0,100 };
	static const TCODColor darkGround{ 50,50,150 };
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? darkWall : darkGround);
		}
	}
}

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