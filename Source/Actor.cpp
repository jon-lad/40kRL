
#include <memory>
#include <string>
#include <sstream>
#include <list>
#include <cmath>
#include "main.hpp"
#include "CP437.hpp"

Actor::Actor(int x, int y, int glyph, std::string_view name, const TCODColor& color)
	: x{ x }, y{ y }, glyph{ cp437::sanitizeGlyph(glyph) }, name{ name }, description{}, color{ color }, blocks{ true }, fovOnly{ true }
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

void Actor::assignRenderLayer()
{
	if (ai || name == "player") {
		renderLayer = RenderLayers::LIVING;
	} else if (openable) {
		renderLayer = RenderLayers::DOOR;
	} else if (pickable) {
		renderLayer = RenderLayers::ITEM;
	} else {
		renderLayer = RenderLayers::DECORATION;
	}
}

float Actor::getDistance(int cx, int cy) const
{
	int dx = x - cx;
	int dy = y - cy;
	return std::sqrtf(static_cast<float>(dx * dx + dy * dy));
}

float Actor::getWeight() const
{
	if (equippable) { return equippable->weight; }
	if (pickable) { return pickable->weight; }
	return 0.0f;
}

int Actor::getValue() const
{
	if (equippable) { return equippable->value; }
	if (pickable) { return pickable->value; }
	return 0;
}

int Actor::getX() const { return x; }
void Actor::setX(int newX) { x = newX; }

int Actor::getY() const { return y; }
void Actor::setY(int newY) { y = newY; }

int Actor::getGlyph() const { return glyph; }
void Actor::setGlyph(int newGlyph) { glyph = cp437::sanitizeGlyph(newGlyph); }

TCODColor Actor::getColor() const { return color; }
void Actor::setColor(const TCODColor& newColor) { color = newColor; }
