#include "TurnFlow.hpp"

bool shouldEndTurnToIdle(bool playerDead, Engine::GameStatus current)
{
    if (playerDead) return false;                 // never reset a dead player (Property 1)
    if (current == Engine::DEFEAT) return false;   // never leave DEFEAT (Property 1)
    return true;                                   // living player: reset as before (Property 2)
}
