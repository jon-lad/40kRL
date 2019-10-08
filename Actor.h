#pragma once


class Actor {
private:
	int x, y;
	int ch;
	TCODColor col;
public:
	char name[20]; //Actors name
	bool blocks;
	std::unique_ptr<Attacker> attacker;
	std::unique_ptr<Destructible> destructible;
	std::unique_ptr<Ai> ai;// something self updating
	std::unique_ptr<Pickable> pickable;// something that can be picked and used
	std::unique_ptr<Container> container;//somthing that can contain actors
	Actor(int x, int y, int ch,const char*name, const TCODColor& col);
	void update();
	
	void render() const;

	int getX() const;
	void setX(int x);

	int getY() const;
	void setY(int y);
	
	int getCh() const;
	void setCh(int ch);

	TCODColor getColor() const;
	void setColor(const TCODColor &col);

};