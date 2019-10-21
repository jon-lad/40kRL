#pragma once


class Actor : public Persistent{
private:
	int x, y;
	int ch;
	TCODColor col;
public:
	std::string name; //Actors name
	bool blocks;
	bool fovOnly;
	std::unique_ptr<Attacker> attacker;
	std::unique_ptr<Destructible> destructible;
	std::unique_ptr<Ai> ai;// something self updating
	std::unique_ptr<Pickable> pickable;// something that can be picked and used
	std::unique_ptr<Container> container;//somthing that can contain actors
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