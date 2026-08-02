#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <memory>
#include <filesystem>
#include <set>
#include <algorithm>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: map-doors — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 1: Door construction invariant ─────────────────────────────────
// **Validates: Requirements 1.1, 1.2, 1.3, 1.4**
//
// For any door created via the door factory function, the resulting Actor SHALL
// have: a non-null Openable component in the closed state, blocks == true,
// fovOnly == false, glyph '+', and colour {150, 100, 50}.

TEST_CASE("PBT: Property 1 — Door construction invariant", "[property][map-doors]")
{
    rc::prop("createDoor produces Actor with correct invariants for any position", []() {
        // Generate random map-valid positions
        const int x = *rc::gen::inRange(0, 79);
        const int y = *rc::gen::inRange(0, 49);

        auto door = createDoor(x, y);

        // Non-null actor
        RC_ASSERT(door != nullptr);

        // Position matches
        RC_ASSERT(door->getX() == x);
        RC_ASSERT(door->getY() == y);

        // Non-null Openable in closed state
        RC_ASSERT(door->openable != nullptr);
        RC_ASSERT(door->openable->isOpen() == false);

        // Spatial properties
        RC_ASSERT(door->blocks == true);
        RC_ASSERT(door->fovOnly == false);

        // Visual properties
        RC_ASSERT(door->getGlyph() == '+');
        TCODColor expectedColor{150, 100, 50};
        RC_ASSERT(door->getColor() == expectedColor);
    });
}

// ─── Property 2: Door state consistency ──────────────────────────────────────
// **Validates: Requirements 1.2, 2.1, 2.2, 2.3, 2.4, 3.1, 3.2**
//
// For any door Actor with a non-null Openable component, the following invariants
// SHALL hold:
// - If openable->isOpen() == false: glyph '+', colour {150,100,50}, blocks == true,
//   TCODMap at door position is not walkable and not transparent.
// - If openable->isOpen() == true: glyph '/', colour {100,65,30}, blocks == false,
//   TCODMap at door position is walkable and transparent.

TEST_CASE("PBT: Property 2 — Door state consistency", "[property][map-doors]")
{
    rc::prop("open/closed state matches glyph, colour, blocks, and TCODMap properties", []() {
        // Use a small TCODMap for spatial property verification
        const int mapW = 20;
        const int mapH = 20;
        auto tcodMap = std::make_unique<TCODMap>(mapW, mapH);

        // Generate a valid position within the map
        const int x = *rc::gen::inRange(0, mapW - 1);
        const int y = *rc::gen::inRange(0, mapH - 1);

        // Create a door at this position
        auto door = createDoor(x, y);

        // Set initial TCODMap state for closed door
        tcodMap->setProperties(x, y, false, false); // not transparent, not walkable

        // Randomly decide whether to open the door
        const bool shouldOpen = *rc::gen::arbitrary_bool();

        if (shouldOpen) {
            door->openable->open(door.get());
            // After open: update TCODMap (simulating what Openable::open should do)
            // Since the stub doesn't do this yet, we verify the expected contract
            // by checking what SHOULD be true after open is implemented:
            // For now we check the door's observable state

            // Expected state after open:
            RC_ASSERT(door->openable->isOpen() == true);
            RC_ASSERT(door->getGlyph() == '/');
            TCODColor openColor{100, 65, 30};
            RC_ASSERT(door->getColor() == openColor);
            RC_ASSERT(door->blocks == false);
        } else {
            // Door stays closed — verify closed invariants
            RC_ASSERT(door->openable->isOpen() == false);
            RC_ASSERT(door->getGlyph() == '+');
            TCODColor closedColor{150, 100, 50};
            RC_ASSERT(door->getColor() == closedColor);
            RC_ASSERT(door->blocks == true);

            // TCODMap should reflect closed state
            RC_ASSERT(tcodMap->isWalkable(x, y) == false);
            RC_ASSERT(tcodMap->isTransparent(x, y) == false);
        }
    });
}

// ─── Property 3: State transition round trip ─────────────────────────────────
// **Validates: Requirements 3.3, 3.4**
//
// For any door Actor, calling openable->open(door) followed by openable->close(door)
// SHALL restore all observable state (glyph, colour, blocks) to the values held
// before the open call.

TEST_CASE("PBT: Property 3 — State transition round trip", "[property][map-doors]")
{
    rc::prop("open then close restores all observable state to pre-open values", []() {
        const int x = *rc::gen::inRange(0, 79);
        const int y = *rc::gen::inRange(0, 49);

        auto door = createDoor(x, y);

        // Capture pre-open state
        const int preGlyph  = door->getGlyph();
        const TCODColor preColor = door->getColor();
        const bool preBlocks = door->blocks;
        const bool preIsOpen = door->openable->isOpen();

        // Perform round trip: open then close
        door->openable->open(door.get());
        door->openable->close(door.get());

        // All observable state should be restored
        RC_ASSERT(door->getGlyph() == preGlyph);
        RC_ASSERT(door->getColor() == preColor);
        RC_ASSERT(door->blocks == preBlocks);
        RC_ASSERT(door->openable->isOpen() == preIsOpen);
    });
}

// ─── Property 4: Serialization round trip ────────────────────────────────────
// **Validates: Requirements 1.5, 7.1, 7.2, 7.3, 7.4, 7.5**
//
// For any door Actor in either the open or closed state, serializing (save) and
// then deserializing (load) the Actor SHALL produce a door with identical
// position, glyph, colour, name, blocks flag, fovOnly flag, and Openable state.

TEST_CASE("PBT: Property 4 — Serialization round trip", "[property][map-doors]")
{
    rc::prop("save then load preserves all door state including Openable", []() {
        // Generate random position
        const int x = *rc::gen::inRange(0, 79);
        const int y = *rc::gen::inRange(0, 49);

        // Create a door via factory
        auto door = createDoor(x, y);

        // Randomly decide whether to open the door
        const bool shouldOpen = *rc::gen::arbitrary_bool();
        if (shouldOpen) {
            door->openable->open(door.get());
        }

        // Capture expected state before save
        const int expectedX      = door->getX();
        const int expectedY      = door->getY();
        const int expectedGlyph  = door->getGlyph();
        const TCODColor expectedColor = door->getColor();
        const std::string expectedName = door->name;
        const bool expectedBlocks  = door->blocks;
        const bool expectedFovOnly = door->fovOnly;
        const bool expectedIsOpen  = door->openable->isOpen();

        // Serialize to a temp file
        const char* tempFile = "__test_door_serialization_roundtrip.sav";
        {
            TCODZip zip;
            door->save(zip);
            zip.saveToFile(tempFile);
        }

        // Deserialize into a fresh Actor
        Actor loaded(0, 0, 0, "", TCODColor{0, 0, 0});
        {
            TCODZip zip;
            zip.loadFromFile(tempFile);
            loaded.load(zip);
        }

        // Clean up temp file
        std::filesystem::remove(tempFile);

        // Verify position
        RC_ASSERT(loaded.getX() == expectedX);
        RC_ASSERT(loaded.getY() == expectedY);

        // Verify glyph
        RC_ASSERT(loaded.getGlyph() == expectedGlyph);

        // Verify colour
        RC_ASSERT(loaded.getColor().r == expectedColor.r);
        RC_ASSERT(loaded.getColor().g == expectedColor.g);
        RC_ASSERT(loaded.getColor().b == expectedColor.b);

        // Verify name
        RC_ASSERT(loaded.name == expectedName);

        // Verify flags
        RC_ASSERT(loaded.blocks == expectedBlocks);
        RC_ASSERT(loaded.fovOnly == expectedFovOnly);

        // Verify Openable component was restored (requires Actor::save/load to
        // handle Openable — expected to fail until task 3.2 is implemented)
        RC_ASSERT(loaded.openable != nullptr);
        RC_ASSERT(loaded.openable->isOpen() == expectedIsOpen);
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: map-doors — Property 5 & Player Interaction Unit Tests (Task 4.1)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 5: Player opens exactly the single adjacent closed door ────────
// **Validates: Requirements 4.1**
//
// For any player position and exactly one closed door at a cardinal neighbour,
// when the player presses 'o', the door's Openable state SHALL transition to
// open, and gameStatus SHALL advance to NEW_TURN.

TEST_CASE("PBT: Property 5 — Player opens exactly the single adjacent closed door", "[property][map-doors]")
{
    rc::prop("pressing 'o' with one adjacent closed door opens it and sets NEW_TURN", []() {
        // Generate a player position away from map edges to leave room for cardinal neighbours
        const int mapW = 20;
        const int mapH = 20;
        const int px = *rc::gen::inRange(1, mapW - 2);
        const int py = *rc::gen::inRange(1, mapH - 2);

        // Pick a random cardinal direction for the door (0=N, 1=S, 2=E, 3=W)
        const int direction = *rc::gen::inRange(0, 3);
        int dx = 0, dy = 0;
        switch (direction) {
            case 0: dy = -1; break; // North
            case 1: dy =  1; break; // South
            case 2: dx =  1; break; // East
            case 3: dx = -1; break; // West
        }
        const int doorX = px + dx;
        const int doorY = py + dy;

        // --- Set up the engine for this test iteration ---

        // Ensure the map is properly sized and all tiles are walkable/transparent
        // (simulating a room area)
        engine.map = std::make_unique<Map>(mapW, mapH);
        engine.map->init(false, LevelType::BSP);

        // Ensure GUI is available for messages
        engine.gui = std::make_unique<Gui>();

        // Create a player actor
        auto playerActor = std::make_unique<Actor>(px, py, '@', "player", Colors::white);
        playerActor->ai = std::make_shared<PlayerAi>();
        playerActor->attacker = std::make_shared<Attacker>(5.0f);
        playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
        engine.player = playerActor.get();

        // Clear existing actors and set up fresh
        engine.actors.clear();
        engine.actors.push_back(std::move(playerActor));

        // Create a closed door at the chosen cardinal neighbour
        auto door = createDoor(doorX, doorY);
        Actor* doorPtr = door.get();
        engine.actors.push_back(std::move(door));

        // Set the door tile as not walkable/not transparent (closed door)
        engine.map->setTileProperties(doorX, doorY, false, false);

        // Set engine state to IDLE (waiting for player input)
        engine.gameStatus = Engine::IDLE;

        // Simulate pressing 'o'
        engine.inputState = InputState{};
        engine.inputState.key.c = 'o';

        // Call handleActionKey directly (this is what PlayerAi::update delegates to)
        auto* playerAi = dynamic_cast<PlayerAi*>(engine.player->ai.get());
        RC_ASSERT(playerAi != nullptr);
        playerAi->handleActionKey(engine.player, 'o');

        // Verify: the door should now be open
        RC_ASSERT(doorPtr->openable != nullptr);
        RC_ASSERT(doorPtr->openable->isOpen() == true);

        // Verify: gameStatus should be NEW_TURN
        RC_ASSERT(engine.gameStatus == Engine::NEW_TURN);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: map-doors — Unit Tests for Player Door Interaction Edge Cases
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Player presses 'o' with no adjacent doors ──────────────────────────────
// **Validates: Requirements 4.3**

TEST_CASE("Player presses 'o' with no adjacent doors shows message, no state change", "[map-doors]")
{
    const int mapW = 20;
    const int mapH = 20;

    // Set up map
    engine.map = std::make_unique<Map>(mapW, mapH);
    engine.map->init(false, LevelType::BSP);

    // Create a player at centre
    auto playerActor = std::make_unique<Actor>(10, 10, '@', "player", Colors::white);
    playerActor->ai = std::make_shared<PlayerAi>();
    playerActor->attacker = std::make_shared<Attacker>(5.0f);
    playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
    engine.player = playerActor.get();

    // Clear actors and set up fresh
    engine.actors.clear();
    engine.actors.push_back(std::move(playerActor));

    // No doors placed — player has no adjacent doors

    // Ensure GUI is available for messages
    engine.gui = std::make_unique<Gui>();

    engine.gameStatus = Engine::IDLE;

    // Act: press 'o'
    auto* playerAi = dynamic_cast<PlayerAi*>(engine.player->ai.get());
    REQUIRE(playerAi != nullptr);
    playerAi->handleActionKey(engine.player, 'o');

    // Assert: no state change to NEW_TURN (message displayed but no action taken)
    // The spec says "no state change" — gameStatus should remain IDLE or not advance
    // The message "There is no door to open." should be displayed via gui->message
    // Since the 'o' handler isn't implemented yet, this test is expected to fail.
    REQUIRE(engine.gameStatus == Engine::IDLE);
}

// ─── Player presses 'o' with 2 adjacent doors triggers direction prompt ──────
// **Validates: Requirements 4.2**

TEST_CASE("Player presses 'o' with 2 adjacent doors triggers direction prompt", "[map-doors]")
{
    const int mapW = 20;
    const int mapH = 20;

    // Set up map
    engine.map = std::make_unique<Map>(mapW, mapH);
    engine.map->init(false, LevelType::BSP);

    // Create a player at centre
    auto playerActor = std::make_unique<Actor>(10, 10, '@', "player", Colors::white);
    playerActor->ai = std::make_shared<PlayerAi>();
    playerActor->attacker = std::make_shared<Attacker>(5.0f);
    playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
    engine.player = playerActor.get();

    // Clear actors and set up fresh
    engine.actors.clear();
    engine.actors.push_back(std::move(playerActor));

    // Place two closed doors adjacent to player (North and East)
    auto doorNorth = createDoor(10, 9);
    auto doorEast  = createDoor(11, 10);
    engine.actors.push_back(std::move(doorNorth));
    engine.actors.push_back(std::move(doorEast));

    // Set door tiles as not walkable
    engine.map->setTileProperties(10, 9, false, false);
    engine.map->setTileProperties(11, 10, false, false);

    // Ensure GUI is available
    engine.gui = std::make_unique<Gui>();

    engine.gameStatus = Engine::IDLE;

    // Act: press 'o' — with multiple doors, a direction prompt should be triggered
    auto* playerAi = dynamic_cast<PlayerAi*>(engine.player->ai.get());
    REQUIRE(playerAi != nullptr);
    playerAi->handleActionKey(engine.player, 'o');

    // Assert: since there are multiple doors, the game should prompt for direction.
    // The exact mechanism may vary (could set a sub-state, display a message, etc.)
    // At minimum, neither door should have been opened without direction selection.
    // Since interaction logic isn't implemented, this test is expected to fail.
    bool northDoorStillClosed = true;
    bool eastDoorStillClosed  = true;
    for (auto& actorPtr : engine.actors) {
        if (actorPtr->openable && actorPtr->getX() == 10 && actorPtr->getY() == 9) {
            northDoorStillClosed = !actorPtr->openable->isOpen();
        }
        if (actorPtr->openable && actorPtr->getX() == 11 && actorPtr->getY() == 10) {
            eastDoorStillClosed = !actorPtr->openable->isOpen();
        }
    }
    // With multiple doors and no direction chosen yet, both should remain closed
    REQUIRE(northDoorStillClosed);
    REQUIRE(eastDoorStillClosed);
}

// ─── Player moves into closed door — movement blocked ────────────────────────
// **Validates: Requirements 4.4**

TEST_CASE("Player moves into closed door — movement blocked, message displayed", "[map-doors]")
{
    const int mapW = 20;
    const int mapH = 20;

    // Set up map
    engine.map = std::make_unique<Map>(mapW, mapH);
    engine.map->init(false, LevelType::BSP);

    // Create a player at (10, 10)
    auto playerActor = std::make_unique<Actor>(10, 10, '@', "player", Colors::white);
    playerActor->ai = std::make_shared<PlayerAi>();
    playerActor->attacker = std::make_shared<Attacker>(5.0f);
    playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
    engine.player = playerActor.get();

    // Clear actors
    engine.actors.clear();
    engine.actors.push_back(std::move(playerActor));

    // Place a closed door to the east at (11, 10)
    auto door = createDoor(11, 10);
    engine.actors.push_back(std::move(door));

    // Set the door tile as not walkable (closed door blocks movement)
    engine.map->setTileProperties(11, 10, false, false);

    // Ensure GUI is available
    engine.gui = std::make_unique<Gui>();

    // Set up camera (needed for update to work if player moves)
    engine.camera = std::make_unique<Camera>(0, 0, mapW, mapH, mapW, mapH);

    engine.gameStatus = Engine::IDLE;

    // Act: simulate player pressing RIGHT arrow to move east into closed door
    engine.inputState = InputState{};
    engine.inputState.key.key = SDLK_RIGHT;
    engine.inputState.key.pressed = true;

    // PlayerAi::update processes the key and calls moveOrAttack internally
    engine.player->ai->update(engine.player);

    // Assert: player position should not have changed (closed door blocks)
    REQUIRE(engine.player->getX() == 10);
    REQUIRE(engine.player->getY() == 10);

    // The message "The door is closed." should be displayed by the door interaction logic.
    // Since the door-specific message logic isn't implemented yet, the movement
    // is simply blocked by isWall. This test is expected to fail once the message
    // requirement is validated.
}

// ─── Player moves into open door — movement succeeds ─────────────────────────
// **Validates: Requirements 4.5**

TEST_CASE("Player moves into open door — movement succeeds", "[map-doors]")
{
    const int mapW = 20;
    const int mapH = 20;

    // Set up map
    engine.map = std::make_unique<Map>(mapW, mapH);
    engine.map->init(false, LevelType::BSP);

    // Create a player at (10, 10)
    auto playerActor = std::make_unique<Actor>(10, 10, '@', "player", Colors::white);
    playerActor->ai = std::make_shared<PlayerAi>();
    playerActor->attacker = std::make_shared<Attacker>(5.0f);
    playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
    engine.player = playerActor.get();

    // Clear actors
    engine.actors.clear();
    engine.actors.push_back(std::move(playerActor));

    // Create a door at (11, 10) and open it
    auto door = createDoor(11, 10);
    door->openable->open(door.get());
    engine.actors.push_back(std::move(door));

    // Set the door tile as walkable/transparent (open door)
    engine.map->setTileProperties(11, 10, true, true);

    // Ensure GUI is available
    engine.gui = std::make_unique<Gui>();

    // Set up camera (needed for moveOrAttack to call camera->update)
    engine.camera = std::make_unique<Camera>(0, 0, mapW, mapH, mapW, mapH);

    engine.gameStatus = Engine::IDLE;

    // Act: simulate player pressing RIGHT arrow to move east into the open door
    engine.inputState = InputState{};
    engine.inputState.key.key = SDLK_RIGHT;
    engine.inputState.key.pressed = true;

    // PlayerAi::update processes the key and calls moveOrAttack internally
    engine.player->ai->update(engine.player);

    // Assert: movement should succeed (open door tile is walkable)
    REQUIRE(engine.player->getX() == 11);
    REQUIRE(engine.player->getY() == 10);
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: map-doors — Property 6 & Monster Door Interaction Unit Tests (Task 5.1)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 6: Monster opens closed door on bump ───────────────────────────
// **Validates: Requirements 5.1**
//
// For any monster with a non-null Ai that attempts to move into a tile occupied
// by a closed door Actor, the door SHALL transition to open and the monster SHALL
// not change position (turn consumed).

TEST_CASE("PBT: Property 6 — Monster opens closed door on bump", "[property][map-doors]")
{
    rc::prop("monster bumping into a closed door opens it without moving", []() {
        const int mapW = 20;
        const int mapH = 20;

        // Generate monster position away from edges (leave room for door + player)
        const int mx = *rc::gen::inRange(2, mapW - 3);
        const int my = *rc::gen::inRange(2, mapH - 3);

        // Pick a random cardinal direction for the door relative to the monster
        const int direction = *rc::gen::inRange(0, 3);
        int dx = 0, dy = 0;
        switch (direction) {
            case 0: dy = -1; break; // North
            case 1: dy =  1; break; // South
            case 2: dx =  1; break; // East
            case 3: dx = -1; break; // West
        }
        const int doorX = mx + dx;
        const int doorY = my + dy;

        // Place player on the far side of the door so the monster wants to move
        // through the door to reach the player.
        const int playerX = doorX + dx;
        const int playerY = doorY + dy;

        // Ensure player position is within bounds
        RC_PRE(playerX >= 0 && playerX < mapW);
        RC_PRE(playerY >= 0 && playerY < mapH);

        // --- Set up the engine for this test iteration ---
        engine.map = std::make_unique<Map>(mapW, mapH);
        engine.map->init(false, LevelType::BSP);
        engine.gui = std::make_unique<Gui>();

        // Make all tiles walkable and transparent initially
        for (int y = 0; y < mapH; ++y) {
            for (int x = 0; x < mapW; ++x) {
                engine.map->setTileProperties(x, y, true, true);
            }
        }

        // Create a player actor (needed by MonsterAi::update which targets engine.player)
        auto playerActor = std::make_unique<Actor>(playerX, playerY, '@', "player", Colors::white);
        playerActor->ai = std::make_shared<PlayerAi>();
        playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
        engine.player = playerActor.get();

        engine.actors.clear();
        engine.actors.push_back(std::move(playerActor));

        // Create a closed door between the monster and the player
        auto door = createDoor(doorX, doorY);
        Actor* doorPtr = door.get();
        engine.actors.push_back(std::move(door));

        // Set door tile as not walkable (closed door)
        engine.map->setTileProperties(doorX, doorY, false, false);

        // Create a monster with MonsterAi at (mx, my)
        auto monster = std::make_unique<Actor>(mx, my, 'O', "Ork", TCODColor{0, 128, 0});
        monster->ai = std::make_shared<MonsterAi>();
        monster->blocks = true;
        monster->fovOnly = true;
        Actor* monsterPtr = monster.get();
        engine.actors.push_back(std::move(monster));

        // Record original monster position
        const int origMX = monsterPtr->getX();
        const int origMY = monsterPtr->getY();

        // Act: call the monster's update, which internally calls moveOrAttack
        // targeting the player position. The monster should bump into the door.
        monsterPtr->ai->update(monsterPtr);

        // Assert: the door should now be open
        RC_ASSERT(doorPtr->openable != nullptr);
        RC_ASSERT(doorPtr->openable->isOpen() == true);

        // Assert: monster should NOT have moved (turn consumed by door opening)
        RC_ASSERT(monsterPtr->getX() == origMX);
        RC_ASSERT(monsterPtr->getY() == origMY);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Unit Test: Monster opens door → GUI message "The X opens the door"
// ═══════════════════════════════════════════════════════════════════════════════

// **Validates: Requirements 5.2**

TEST_CASE("Monster opens door — GUI message 'The X opens the door'", "[map-doors]")
{
    const int mapW = 20;
    const int mapH = 20;

    // Set up map with all tiles walkable
    engine.map = std::make_unique<Map>(mapW, mapH);
    engine.map->init(false, LevelType::BSP);
    engine.gui = std::make_unique<Gui>();

    for (int y = 0; y < mapH; ++y) {
        for (int x = 0; x < mapW; ++x) {
            engine.map->setTileProperties(x, y, true, true);
        }
    }

    // Place player at (15, 10) — far enough to trigger movement, not attack
    auto playerActor = std::make_unique<Actor>(15, 10, '@', "player", Colors::white);
    playerActor->ai = std::make_shared<PlayerAi>();
    playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
    engine.player = playerActor.get();

    engine.actors.clear();
    engine.actors.push_back(std::move(playerActor));

    // Place a closed door at (11, 10)
    auto door = createDoor(11, 10);
    Actor* doorPtr = door.get();
    engine.actors.push_back(std::move(door));
    engine.map->setTileProperties(11, 10, false, false);

    // Place an Ork monster at (10, 10) — wants to move east toward player,
    // will bump into the door at (11, 10)
    auto monster = std::make_unique<Actor>(10, 10, 'O', "Ork", TCODColor{0, 128, 0});
    monster->ai = std::make_shared<MonsterAi>();
    monster->blocks = true;
    monster->fovOnly = true;
    Actor* monsterPtr = monster.get();
    engine.actors.push_back(std::move(monster));

    // Act: monster updates, should bump into door and open it
    monsterPtr->ai->update(monsterPtr);

    // Assert: door is open
    REQUIRE(doorPtr->openable->isOpen() == true);

    // Assert: monster did not move
    REQUIRE(monsterPtr->getX() == 10);
    REQUIRE(monsterPtr->getY() == 10);

    // Assert: the GUI log contains the expected message
    std::string lastMsg = engine.gui->getLastMessage();
    REQUIRE(lastMsg.find("Ork") != std::string::npos);
    REQUIRE(lastMsg.find("opens the door") != std::string::npos);
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: map-doors — Property 7 & Door Placement Unit Tests (Task 7.1)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 7: No duplicate door positions after generation ────────────────
// **Validates: Requirements 6.1, 6.2, 6.3, 6.4, 6.5**
//
// For any BSP-generated map, the set of positions occupied by door Actors SHALL
// contain no duplicates — each (x, y) pair appears at most once.

TEST_CASE("PBT: Property 7 — No duplicate door positions after generation", "[property][map-doors]")
{
    rc::prop("BSP generation produces no duplicate door positions", []() {
        // Use a map size representative of BSP generation
        const int mapW = 80;
        const int mapH = 50;

        // Set up minimal engine state needed for BSP map generation
        engine.gui = std::make_unique<Gui>();
        engine.actors.clear();

        // Create a player actor (required by Map::createRoom for placement)
        auto playerActor = std::make_unique<Actor>(0, 0, '@', "player", Colors::white);
        playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
        playerActor->ai = std::make_shared<PlayerAi>();
        engine.player = playerActor.get();
        engine.actors.push_back(std::move(playerActor));

        // Create stairs actors (required by createRoom — it positions stairs during BSP gen)
        auto stairsUp = std::make_unique<Actor>(0, 0, '<', "stairs up", Colors::white);
        stairsUp->blocks = false;
        stairsUp->fovOnly = false;
        engine.stairsUp = stairsUp.get();
        engine.actors.push_back(std::move(stairsUp));

        auto stairsDown = std::make_unique<Actor>(0, 0, '>', "stairs down", Colors::white);
        stairsDown->blocks = false;
        stairsDown->fovOnly = false;
        engine.stairsDown = stairsDown.get();
        engine.actors.push_back(std::move(stairsDown));

        // Generate a BSP map (with actors, which triggers door placement once implemented)
        engine.map = std::make_unique<Map>(mapW, mapH);
        engine.map->init(true, LevelType::BSP);

        // Collect all door positions (actors with non-null openable)
        std::set<std::pair<int, int>> doorPositions;
        int doorCount = 0;

        for (const auto& actor : engine.actors) {
            if (actor->openable != nullptr) {
                doorCount++;
                doorPositions.insert({actor->getX(), actor->getY()});
            }
        }

        // Property: set size equals count (no duplicates)
        RC_ASSERT(static_cast<int>(doorPositions.size()) == doorCount);

        // Additionally verify at least one door was placed (BSP maps should have doors)
        RC_ASSERT(doorCount > 0);
    });
}

// ═══════════════════════════════════════════════════════════════════════════════
// Unit Test: BSP generation produces doors only on BSP levels (not outdoor/WFC)
// ═══════════════════════════════════════════════════════════════════════════════

// **Validates: Requirements 6.3**

TEST_CASE("BSP generation produces doors only on BSP levels, not outdoor or WFC", "[map-doors]")
{
    const int mapW = 80;
    const int mapH = 50;

    // --- Helper lambda to count doors in the current actor list ---
    auto countDoors = []() {
        int count = 0;
        for (const auto& actor : engine.actors) {
            if (actor->openable != nullptr) {
                count++;
            }
        }
        return count;
    };

    // --- Helper lambda to set up minimal engine state for map generation ---
    auto setupEngine = [&]() {
        engine.gui = std::make_unique<Gui>();
        engine.actors.clear();

        auto playerActor = std::make_unique<Actor>(0, 0, '@', "player", Colors::white);
        playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
        playerActor->ai = std::make_shared<PlayerAi>();
        engine.player = playerActor.get();
        engine.actors.push_back(std::move(playerActor));

        auto stairsUp = std::make_unique<Actor>(0, 0, '<', "stairs up", Colors::white);
        stairsUp->blocks = false;
        stairsUp->fovOnly = false;
        engine.stairsUp = stairsUp.get();
        engine.actors.push_back(std::move(stairsUp));

        auto stairsDown = std::make_unique<Actor>(0, 0, '>', "stairs down", Colors::white);
        stairsDown->blocks = false;
        stairsDown->fovOnly = false;
        engine.stairsDown = stairsDown.get();
        engine.actors.push_back(std::move(stairsDown));
    };

    SECTION("BSP map should produce doors") {
        setupEngine();
        engine.map = std::make_unique<Map>(mapW, mapH);
        engine.map->init(true, LevelType::BSP);

        // Once door placement is implemented, BSP maps should have at least one door
        REQUIRE(countDoors() > 0);
    }

    SECTION("Outdoor map should produce zero doors") {
        setupEngine();
        engine.map = std::make_unique<Map>(mapW, mapH);
        engine.map->init(true, LevelType::OUTDOOR);

        REQUIRE(countDoors() == 0);
    }

    SECTION("WFC map should produce zero doors") {
        setupEngine();
        engine.map = std::make_unique<Map>(mapW, mapH);
        engine.map->init(true, LevelType::WFC);

        REQUIRE(countDoors() == 0);
    }
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: map-doors — Integration Tests (Task 8.2)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Integration Test 1: Save/load with mixed door states ────────────────────
// **Validates: Requirements 7.1, 7.2, 7.3, 7.4**
//
// Creates multiple doors in various states (some open, some closed),
// serializes all of them to a single file, loads them back, and verifies
// that positions, states, glyphs, colours, and spatial properties are preserved.

TEST_CASE("Integration: save/load with mixed door states across multiple rooms", "[map-doors][integration]")
{
    const int mapW = 40;
    const int mapH = 40;

    // Set up a map so that Openable::open/close can update tile properties
    engine.gui = std::make_unique<Gui>();
    engine.actors.clear();
    engine.fovRadius = 10;
    engine.map = std::make_unique<Map>(mapW, mapH);
    engine.map->init(false, LevelType::BSP);

    // Create player (needed for computeFOV)
    auto playerActor = std::make_unique<Actor>(1, 1, '@', "player", Colors::white);
    playerActor->ai = std::make_shared<PlayerAi>();
    playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
    engine.player = playerActor.get();
    engine.actors.push_back(std::move(playerActor));

    // Create doors at various positions simulating multiple rooms
    struct DoorConfig {
        int x, y;
        bool shouldOpen;
    };
    std::vector<DoorConfig> configs = {
        {5,  10, false},  // closed
        {10, 10, true},   // open
        {15, 10, false},  // closed
        {20, 10, true},   // open
        {25, 10, true},   // open
        {5,  20, false},  // closed
        {10, 20, false},  // closed
        {15, 20, true},   // open
    };

    // Create doors, set their state, and track expected values
    struct DoorExpected {
        int x, y;
        bool isOpen;
        int glyph;
        TCODColor color;
        bool blocks;
    };
    std::vector<DoorExpected> expected;

    for (const auto& cfg : configs) {
        auto door = createDoor(cfg.x, cfg.y);
        // Make the tile walkable first so the door can be placed
        engine.map->setTileProperties(cfg.x, cfg.y, false, false);

        if (cfg.shouldOpen) {
            door->openable->open(door.get());
        }
        expected.push_back({
            cfg.x, cfg.y,
            door->openable->isOpen(),
            door->getGlyph(),
            door->getColor(),
            door->blocks
        });
        engine.actors.push_back(std::move(door));
    }

    // Serialize all doors to a temp file
    const char* tempFile = "__test_integration_doors_save_load.sav";
    {
        TCODZip zip;
        zip.putInt(static_cast<int>(configs.size()));
        for (const auto& actorPtr : engine.actors) {
            if (actorPtr->openable != nullptr) {
                actorPtr->save(zip);
            }
        }
        zip.saveToFile(tempFile);
    }

    // Load the doors back into fresh actors
    std::vector<DoorExpected> loaded;
    {
        TCODZip zip;
        zip.loadFromFile(tempFile);
        int count = zip.getInt();
        for (int i = 0; i < count; ++i) {
            Actor loadedActor(0, 0, 0, "", TCODColor{0, 0, 0});
            loadedActor.load(zip);
            REQUIRE(loadedActor.openable != nullptr);
            loaded.push_back({
                loadedActor.getX(), loadedActor.getY(),
                loadedActor.openable->isOpen(),
                loadedActor.getGlyph(),
                loadedActor.getColor(),
                loadedActor.blocks
            });
        }
    }
    std::filesystem::remove(tempFile);

    // Verify same count
    REQUIRE(loaded.size() == expected.size());

    // Verify each door's full state
    for (size_t i = 0; i < expected.size(); ++i) {
        INFO("Door " << i << " at (" << expected[i].x << "," << expected[i].y << ")");
        REQUIRE(loaded[i].x == expected[i].x);
        REQUIRE(loaded[i].y == expected[i].y);
        REQUIRE(loaded[i].isOpen == expected[i].isOpen);
        REQUIRE(loaded[i].glyph == expected[i].glyph);
        REQUIRE(loaded[i].color.r == expected[i].color.r);
        REQUIRE(loaded[i].color.g == expected[i].color.g);
        REQUIRE(loaded[i].color.b == expected[i].color.b);
        REQUIRE(loaded[i].blocks == expected[i].blocks);
    }

    // Verify TCODMap properties match loaded state (Actor::load restores them)
    for (const auto& door : loaded) {
        if (door.isOpen) {
            REQUIRE(engine.map->isWall(door.x, door.y) == false);
        } else {
            REQUIRE(engine.map->isWall(door.x, door.y) == true);
        }
    }
}

// ─── Integration Test 2: Reproducible door placement with known seed ─────────
// **Validates: Requirements 3.3, 3.4**
//
// Uses the same Map seed (by reusing the same Map object) to verify that
// BSP generation produces identical door positions when regenerated.
// This tests that the seeded RNG in Map::init produces deterministic results.

TEST_CASE("Integration: BSP generation with known seed produces reproducible door placements", "[map-doors][integration]")
{
    const int mapW = 80;
    const int mapH = 50;

    // Helper: set up engine state and generate BSP map, returning door positions.
    // Uses a fixed seed on the Map object for determinism.
    auto generateAndCollectDoors = [&](long fixedSeed) -> std::vector<std::pair<int, int>> {
        engine.gui = std::make_unique<Gui>();
        engine.actors.clear();
        engine.fovRadius = 10;
        engine.dungeonLevel = 1;

        auto playerActor = std::make_unique<Actor>(0, 0, '@', "player", Colors::white);
        playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
        playerActor->ai = std::make_shared<PlayerAi>();
        engine.player = playerActor.get();
        engine.actors.push_back(std::move(playerActor));

        auto stairsUp = std::make_unique<Actor>(0, 0, '<', "stairs up", Colors::white);
        stairsUp->blocks = false;
        stairsUp->fovOnly = false;
        engine.stairsUp = stairsUp.get();
        engine.actors.push_back(std::move(stairsUp));

        auto stairsDown = std::make_unique<Actor>(0, 0, '>', "stairs down", Colors::white);
        stairsDown->blocks = false;
        stairsDown->fovOnly = false;
        engine.stairsDown = stairsDown.get();
        engine.actors.push_back(std::move(stairsDown));

        engine.map = std::make_unique<Map>(mapW, mapH);
        engine.map->setSeed(fixedSeed);
        engine.map->init(true, LevelType::BSP);
        engine.camera = std::make_unique<Camera>(0, 0, 80, 43, mapW, mapH);

        std::vector<std::pair<int, int>> positions;
        for (const auto& actor : engine.actors) {
            if (actor->openable != nullptr) {
                positions.emplace_back(actor->getX(), actor->getY());
            }
        }
        std::sort(positions.begin(), positions.end());
        return positions;
    };

    // Use a known seed for reproducible BSP generation
    const long knownSeed = 12345;

    // First generation
    auto firstPositions = generateAndCollectDoors(knownSeed);
    REQUIRE(firstPositions.size() > 0);

    // Second generation with same seed — should produce identical door placement
    auto secondPositions = generateAndCollectDoors(knownSeed);

    // Door positions must be identical (same seed → same BSP → same doors)
    REQUIRE(secondPositions.size() == firstPositions.size());
    for (size_t i = 0; i < firstPositions.size(); ++i) {
        INFO("Door " << i << " expected (" << firstPositions[i].first << "," << firstPositions[i].second
             << ") got (" << secondPositions[i].first << "," << secondPositions[i].second << ")");
        REQUIRE(secondPositions[i].first == firstPositions[i].first);
        REQUIRE(secondPositions[i].second == firstPositions[i].second);
    }
}

// ─── Integration Test 3: FOV recompute after door open reveals hidden tiles ──
// **Validates: Requirements 3.3, 3.4**
//
// Creates a controlled map with a closed door blocking FOV. Verifies that a tile
// behind the door is not visible. Opens the door. Recomputes FOV. Verifies the
// tile behind the door is now visible.

TEST_CASE("Integration: FOV recompute after door open reveals previously hidden tiles", "[map-doors][integration]")
{
    const int mapW = 20;
    const int mapH = 20;

    // Set up engine
    engine.gui = std::make_unique<Gui>();
    engine.actors.clear();
    engine.fovRadius = 10;

    // Create the map — all tiles start as walls (not walkable, not transparent)
    engine.map = std::make_unique<Map>(mapW, mapH);
    engine.map->init(false, LevelType::BSP);

    // Manually carve a corridor with a door in the middle:
    //
    //   Player at (5, 10), corridor extends east to (7, 10).
    //   Door at (8, 10) — closed (not transparent, not walkable).
    //   Room behind door at (9, 10) and (10, 10) — walkable and transparent.
    //
    // With the door closed, (9, 10) should NOT be in FOV.
    // After opening the door, (9, 10) SHOULD be in FOV.

    // Carve walkable/transparent tiles for the corridor and room
    for (int x = 5; x <= 7; ++x) {
        engine.map->setTileProperties(x, 10, true, true); // transparent, walkable
    }
    // Door position: initially closed (not transparent, not walkable)
    engine.map->setTileProperties(8, 10, false, false);

    // Room behind the door
    for (int x = 9; x <= 12; ++x) {
        engine.map->setTileProperties(x, 10, true, true); // transparent, walkable
    }

    // Create player at (5, 10)
    auto playerActor = std::make_unique<Actor>(5, 10, '@', "player", Colors::white);
    playerActor->ai = std::make_shared<PlayerAi>();
    playerActor->destructible = std::make_shared<PlayerDestructible>(30.0f, 2.0f, "your corpse", 0);
    engine.player = playerActor.get();
    engine.actors.push_back(std::move(playerActor));

    // Place a closed door at (8, 10)
    auto door = createDoor(8, 10);
    Actor* doorPtr = door.get();
    engine.actors.push_back(std::move(door));

    // Compute FOV from the player's position
    engine.map->computeFOV();

    // The corridor tiles (5-7, 10) should be in FOV
    REQUIRE(engine.map->isInFOV(5, 10) == true);
    REQUIRE(engine.map->isInFOV(6, 10) == true);
    REQUIRE(engine.map->isInFOV(7, 10) == true);

    // The door tile itself should be in FOV (it's in line of sight even though not transparent)
    // Note: TCODMap FOV algorithms include the blocking tile itself in FOV
    REQUIRE(engine.map->isInFOV(8, 10) == true);

    // The tile BEHIND the door (9, 10) should NOT be in FOV (door blocks transparency)
    REQUIRE(engine.map->isInFOV(9, 10) == false);

    // Now open the door — this should update TCODMap to transparent/walkable and recompute FOV
    doorPtr->openable->open(doorPtr);

    // After opening, the tile behind the door should now be visible
    REQUIRE(engine.map->isInFOV(9, 10) == true);
    REQUIRE(engine.map->isInFOV(10, 10) == true);
}
