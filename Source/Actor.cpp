
#include <memory>
#include <string>
#include <sstream>
#include <list>
#include <cmath>
#include "main.h"

Actor::Actor(int x, int y, int glyph, std::string_view name, const TCODColor& color)
	: x{ x }, y{ y }, glyph{ glyph }, name{ name }, color{ color }, blocks{ true }, fovOnly{ true }
{
	// All component slots start empty; callers assign components after construction.
}

void Actor::render() const
{
	auto [screenX, screenY] = engine.camera->apply(x, y);
	renderPutChar(TCODConsole::root->get_data(), screenX, screenY, glyph, {color.r, color.g, color.b});
}

void Actor::update()
{
	if (ai) { ai->update(this); }
}

float Actor::getDistance(int cx, int cy) const
{
	int dx = x - cx;
	int dy = y - cy;
	return std::sqrtf(static_cast<float>(dx * dx + dy * dy));
}

int Actor::getX() const { return x; }
void Actor::setX(int newX) { x = newX; }

int Actor::getY() const { return y; }
void Actor::setY(int newY) { y = newY; }

int Actor::getGlyph() const { return glyph; }
void Actor::setGlyph(int newGlyph) { glyph = newGlyph; }

TCODColor Actor::getColor() const { return color; }
void Actor::setColor(const TCODColor& newColor) { color = newColor; }
