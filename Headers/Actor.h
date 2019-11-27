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
	std::shared_ptr<Equipment> equipment;//something that can equip items
	Actor(int x, int y, int ch,std::string_view name, const TCODColor& col);
	void update();// update the actor each turn
	
	void render() const;//render the actors onto the map

	void save(TCODZip& zip); // Save actor details to zip
	void load(TCODZip& zip); // load actor details from zip
	

	double getDistance(int cx, int cy);//retuens distace from actor to tile

	//Getter and setter for x co-ord
	int getX() const;
	void setX(int x);


	//Getter and setter for x co-ord
	int getY() const; 
	void setY(int y);
	
	//Getter and setter for ascii character
	int getCh() const;
	void setCh(int ch);

	//Getter and setter for character color
	TCODColor getColor() const;
	void setColor(const TCODColor &col);

};