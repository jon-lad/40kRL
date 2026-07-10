#pragma once

#include <memory>

class Equippable;
class Equipment;
class CareerProgression;
struct EnemyEquipmentConfig;

// An Actor is any entity that exists on the map: the player, monsters, items, corpses, stairs.
// Capabilities are defined by which optional components are non-null.
class Actor : public Persistent{
private:
	int x, y;       // world position
	int glyph;      // ASCII/libtcod character used to render this actor
	TCODColor color;
public:
	std::string name;
	std::string description; // optional description shown in look mode (max 200 chars)
	bool blocks;    // true if this actor prevents other actors from entering its tile
	bool fovOnly;   // if true, only render when the tile is currently in the player's FOV
	int coverValue = 0; // cover bonus (0-100) for future cover system; populated from DecorationTemplate

	// Optional components — null means "this actor does not have this capability"
	std::shared_ptr<Attacker>    attacker;
	std::shared_ptr<Destructible> destructible;
	std::shared_ptr<Ai>          ai;
	std::shared_ptr<Pickable>    pickable;
	std::shared_ptr<Container>   container;
	std::shared_ptr<Equippable>  equippable;
	std::shared_ptr<Characteristics> characteristics;
	std::shared_ptr<CareerProgression> career;  // non-null for player after chargen
	std::unique_ptr<Equipment>   equipment;  // non-null on any actor with equipment

	// Parsed equipment config from Lua (used during spawn to resolve equipment in task 2.2).
	// Null for actors without equipment configuration. Consumed and cleared after equipment is attached.
	std::unique_ptr<EnemyEquipmentConfig> equipConfig;

	Actor(int x, int y, int glyph, std::string_view name, const TCODColor& color);

	// Delegates to ai->update if the ai component is present; otherwise a no-op.
	void update();

	// Draws this actor at its world position, translated through the camera.
	void render() const;

	void save(TCODZip& zip);
	void load(TCODZip& zip);

	// Returns the Euclidean distance from this actor to world coordinate (cx, cy).
	float getDistance(int cx, int cy) const;

	// Returns equippable->weight if equippable exists, else pickable->weight if pickable exists, else 0.
	float getWeight() const;

	// Returns equippable->value if equippable exists, else pickable->value if pickable exists, else 0.
	int getValue() const;

	int getX() const;
	void setX(int newX);

	int getY() const;
	void setY(int newY);

	int getGlyph() const;
	void setGlyph(int newGlyph);

	TCODColor getColor() const;
	void setColor(const TCODColor& newColor);
};