#pragma once

// Engine.hpp is not self-contained — it relies on the include order established
// by main.hpp (libtcod + component headers declared before Engine). Including
// main.hpp here brings in Engine::GameStatus with all its prerequisites, with no
// engine initialization required (per test-isolation.md).
#include "main.hpp"  // for Engine::GameStatus (via Engine.hpp)

// Pure decision predicate for the end-of-turn "reset to IDLE" sites in
// Engine::update(). Engine-free and side-effect-free so it is safe to unit
// test without initializing the global Engine (see test-isolation.md).
//
// Returns true  => the turn loop MAY set gameStatus = IDLE.
// Returns false => the turn loop MUST NOT reset (a dead player / DEFEAT stands).
bool shouldEndTurnToIdle(bool playerDead, Engine::GameStatus current);
