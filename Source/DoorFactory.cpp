#include "main.h"
#include "Colors.h"
#include "Openable.h"
#include "DoorFactory.h"

// Creates a door Actor with all construction invariants satisfied:
// glyph '+', name "door", colour doorClosed, blocks=true, fovOnly=false, Openable attached.
std::unique_ptr<Actor> createDoor(int x, int y) {
    auto door = std::make_unique<Actor>(x, y, '+', "door", Colors::doorClosed);
    door->blocks  = true;
    door->fovOnly = false;
    door->openable = std::make_shared<Openable>();
    return door;
}
