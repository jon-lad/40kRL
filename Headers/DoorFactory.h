#pragma once

#include <memory>

class Actor;

// Factory function for creating door actors. Encapsulates all invariants:
// glyph '+', colour {150,100,50}, blocks=true, fovOnly=false, Openable component attached.
//
// STUB: Declaration only for TDD. Implementation in task 1.3.
std::unique_ptr<Actor> createDoor(int x, int y);
