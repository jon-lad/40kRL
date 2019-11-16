#pragma once


class Actor : public Persistent{
private:
	int x, y;//x and y position in world
	int ch; //symbol for actor
	TCODColor col; //actor color
public:
	std::string name; //Actors name
	bool blocks; //does actor dtop movement
	bool fovOnly;// can you only see the actor when in Fov (after explored)
	std::shared_ptr<Attacker> attacker; //something which attacks
	std::shared_ptr<Destructible> destructible; //something which can be destroyed
	std::shared_ptr<Ai> ai;// something self updating
	std::shared_ptr<Pickable> pickable;// something that can be picked and used
	std::shared_ptr<Container> container;//somthing that can contain actors
	Actor(int x, int y, int ch,std::string_view name, const TCODColor& col);
	void update();
	
	void render() const;

	void save(TCODZip& zip);
	void load(TCODZip& zip);
	

	float getDistance(int cx, int cy);

	int getX() const;
	void setX(int x);

	int getY() const;
	void setY(int y);
	
	int getCh() const;
	void setCh(int ch);

	TCODColor getColor() const;
	void setColor(const TCODColor &col);

};