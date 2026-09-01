#pragma once
#include "Persistent.hpp"

class Actor;

// Component that makes an Actor into an interactive door.
// Tracks open/closed state and manages the side effects of state transitions
// (glyph, colour, blocks flag, TCODMap properties, FOV recompute).
class Openable : public Persistent {
public:
    Openable();  // initialises in CLOSED state

    bool isOpen() const;

    // Transitions from closed to open. Updates the owning Actor's glyph, colour,
    // blocks flag, and the Map's TCODMap properties. Triggers FOV recompute.
    void open(Actor* owner);

    // Transitions from open to closed. Inverse of open().
    void close(Actor* owner);

    void save(TCODZip& zip) override;
    void load(TCODZip& zip) override;

private:
    bool opened = false;
};
