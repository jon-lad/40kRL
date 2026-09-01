#include "main.hpp"
#include "Openable.hpp"

Openable::Openable() : opened(false) {}

bool Openable::isOpen() const { return opened; }

void Openable::open(Actor* owner) {
	if (opened) { return; } // guard: no-op if already open

	opened = true;
	owner->setGlyph('/');  // CP437 codepoint 47: open door
	owner->setColor(TCODColor{100, 65, 30});
	owner->blocks = false;

	// Update TCODMap and recompute FOV if engine map is available
	if (engine.map) {
		engine.map->setTileProperties(owner->getX(), owner->getY(), true, true);
		engine.map->computeFOV();
	}
}

void Openable::close(Actor* owner) {
	if (!opened) { return; } // guard: no-op if already closed

	opened = false;
	owner->setGlyph('+');
	owner->setColor(TCODColor{150, 100, 50});
	owner->blocks = true;

	// Update TCODMap and recompute FOV if engine map is available
	if (engine.map) {
		engine.map->setTileProperties(owner->getX(), owner->getY(), false, false);
		engine.map->computeFOV();
	}
}

void Openable::save(TCODZip& zip) {
	zip.putInt(opened ? 1 : 0);
}

void Openable::load(TCODZip& zip) {
	opened = (zip.getInt() != 0);
}
