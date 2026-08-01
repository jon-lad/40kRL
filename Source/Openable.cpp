#include "main.h"
#include "Openable.h"

// STUB implementation for TDD — tests are expected to FAIL until task 1.2 implements
// the real logic.

Openable::Openable() : opened(false) {}

bool Openable::isOpen() const { return opened; }

void Openable::open(Actor* owner) {
    // TODO: task 1.2 — implement state transition logic
    (void)owner;
}

void Openable::close(Actor* owner) {
    // TODO: task 1.2 — implement state transition logic
    (void)owner;
}

void Openable::save(TCODZip& zip) {
    // TODO: task 3.2 — implement serialization
    (void)zip;
}

void Openable::load(TCODZip& zip) {
    // TODO: task 3.2 — implement serialization
    (void)zip;
}
