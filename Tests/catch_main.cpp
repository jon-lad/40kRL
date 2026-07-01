// Catch2 amalgamated implementation — compiled once, linked into the test binary.
// Do NOT include this in any other translation unit.
#define CATCH_CONFIG_RUNNER
#include "lib/catch_amalgamated.hpp"
#include "main.h"

// Global engine instance for tests. The real game defines this in main.cpp which
// we exclude from the test binary to avoid duplicate main(). The Engine constructor
// calls TCODConsole::initRoot, so the test binary requires libtcod at runtime.
Engine engine(80, 50);

int main(int argc, char* argv[])
{
    // Initialise the engine so tests that need player/stairs/map/gui can run.
    engine.init();
    int result = Catch::Session().run(argc, argv);
    engine.term();
    return result;
}
