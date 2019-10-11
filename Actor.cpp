#include <memory>
#include <list>
#include <cmath>
#include "main.h"



Actor::Actor(int x, int y, int ch, const char* name, const TCODColor& col) :
	x{ x }, y{ y }, ch{ ch }, col{ col }, blocks{ true }
{
	strcpy_s(this->name, name);
	attacker.reset();
	destructible.reset();
	ai.reset();
	pickable.reset();
	container.reset();
}

void Actor::render() const
{
	TCODConsole::root->setChar(x, y, ch);
	TCODConsole::root->setCharForeground(x, y, col);
}

void Actor::update() 
{
	if (ai) { ai->update(this); }
}



float Actor::getDistance(int cx, int cy){
	int dx = x - cx;
	int dy = y - cy;
	return std::sqrtf(dx * dx + dy * dy);
}

int Actor::getX() const
{
	return x;
}
void Actor::setX(int x) {
	this->x = x;
}

int Actor::getY() const
{
	return y;
}
void Actor::setY(int y)
{
	this->y = y;
}

int Actor::getCh() const
{
	return ch;
}
void Actor::setCh(int ch)
{
	this->ch = ch;
}

TCODColor Actor::getColor() const
{
	return col;
}
void Actor::setColor(const TCODColor &col)
{
	this->col = col;
}