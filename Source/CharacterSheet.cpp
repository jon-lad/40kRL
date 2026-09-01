#include "main.hpp"

void CharacterSheet::save(TCODZip& zip) {
	characteristics.save(zip);
	career.save(zip);
}

void CharacterSheet::load(TCODZip& zip) {
	characteristics.load(zip);
	career.load(zip);
}
