#include "libtcod.h"
#include "Actor.h"

Actor::Actor(int x, int y, int ch, const TCODColor& col) :
	x{ x }, y{ y }, ch{ ch }, col{ col }
{
}

void Actor::render() const
{
	TCODConsole::root->setChar(x, y, ch);
	TCODConsole::root->setCharForeground(x, y, col);
}

int Actor::getX()
{
	return x;
}
void Actor::setX(int x) {
	this->x = x;
}

int Actor::getY() 
{
	return y;
}
void Actor::setY(int y)
{
	this->y = y;
}

int Actor::getCh()
{
	return ch;
}
void Actor::setCh(int ch)
{
	this->ch = ch;
}

TCODColor Actor::getColor()
{
	return col;
}
void Actor::setColor(const TCODColor &col)
{
	this->col = col;
}