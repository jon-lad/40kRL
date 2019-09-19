#pragma once
class Actor {
private:
	int x, y;
	int ch;
	TCODColor col;
public:
	Actor(int x, int y, int ch, const TCODColor& col);
	void render() const;

	int getX();
	void setX(int x);

	int getY();
	void setY(int y);
	
	int getCh();
	void setCh(int ch);

	TCODColor getColor();
	void setColor(const TCODColor &col);

};