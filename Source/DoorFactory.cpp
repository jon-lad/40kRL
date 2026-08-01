#include "main.h"
#include "Openable.h"
#include "DoorFactory.h"

// STUB implementation for TDD — creates a door Actor with the correct invariants.
// This stub provides just enough to make Property 1 pass once task 1.3 refines it.
std::unique_ptr<Actor> createDoor(int x, int y) {
    auto door = std::make_unique<Actor>(x, y, '+', "door", TCODColor{150, 100, 50});
    door->blocks  = true;
    door->fovOnly = false;
    door->openable = std::make_shared<Openable>();
    return door;
}
