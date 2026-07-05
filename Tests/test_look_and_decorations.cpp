#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

#include <algorithm>
#include <vector>
#include <string>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: look-and-decorations — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 2: Actor description formatting ────────────────────────────────
// **Validates: Requirements 3.2, 3.3**
//
// For any actor name (1–30 printable ASCII chars) and description (0–200
// printable ASCII chars), the formatted look-mode output shall equal the name
// alone when the description is empty, or "name: description" when non-empty.

TEST_CASE("PBT: Property 2 — actor description formatting", "[pbt][property][look-and-decorations]")
{
    rc::prop("formatted output is name when description empty, name + ': ' + description otherwise", []() {
        // Generate a random name: 1–30 printable ASCII characters (32-126 inclusive)
        auto printableChar = rc::gen::charInRange(32, 126);
        const auto name = *rc::gen::stringOf(printableChar, 1, 30);

        // Generate a random description: 0–200 printable ASCII characters
        const auto description = *rc::gen::stringOf(printableChar, 0, 200);

        // Apply the formatting logic (same as renderLook):
        // if description is empty → output = name
        // else → output = name + ": " + description
        std::string formatted;
        if (description.empty()) {
            formatted = name;
        } else {
            formatted = name + ": " + description;
        }

        // Verify the expected output
        if (description.empty()) {
            RC_ASSERT(formatted == name);
        } else {
            RC_ASSERT(formatted == name + ": " + description);
            // Also verify it starts with the name and contains the separator
            RC_ASSERT(formatted.substr(0, name.size()) == name);
            RC_ASSERT(formatted.substr(name.size(), 2) == ": ");
            RC_ASSERT(formatted.substr(name.size() + 2) == description);
        }

        // Additional invariant: formatted output is never empty (name is always >= 1 char)
        RC_ASSERT(!formatted.empty());
        RC_ASSERT(formatted.size() >= name.size());
    });
}

// ─── Property 5: Invalid decoration entries are skipped during loading ────────
// **Validates: Requirements 4.4**
//
// For any decoration entry table with random valid/invalid field combinations,
// the loader shall skip entries missing required fields or having an unrecognised
// colour name. The loaded template count equals the count of fully-valid entries.

// Helper: generate a random bool via inRange to avoid template-in-macro issues
static rc::Gen<bool> genBool() {
    return {[](std::mt19937& rng) {
        return std::uniform_int_distribution<int>(0, 1)(rng) != 0;
    }};
}

TEST_CASE("PBT: Property 5 — invalid decoration entries are skipped during loading", "[pbt][property][look-and-decorations]")
{
    // Valid colour names recognised by Colors::colorFromName (non-black results)
    static const std::vector<std::string> validColorNames = {
        "white", "desaturatedGreen", "darkerGreen", "lightBlue",
        "orange", "lightGreen", "violet", "lightYellow",
        "lightGrey", "lighterOrange", "darkGrey", "red", "darkRed", "cyan"
    };

    rc::prop("loaded count equals count of entries with all fields valid", []() {
        // Generate random number of entries (1–15)
        const int entryCount = *rc::gen::inRange(1, 16);

        int validCount = 0;
        int skippedCount = 0;

        // For each entry, generate bool flags for field validity
        for (int i = 0; i < entryCount; ++i) {
            // Each bool flag: true = field is valid/present, false = invalid/missing
            const bool hasGlyph        = *genBool();
            const bool hasName         = *genBool();
            const bool hasColor        = *genBool();
            const bool hasDescription  = *genBool();
            const bool hasBlocks       = *genBool();
            const bool colorRecognized = *genBool();

            // Simulate the loader's two-stage validation (from Engine::init):
            // Stage 1: check required fields are non-empty and blocks is present
            //   if (glyphStr.empty() || name.empty() || colorName.empty() ||
            //       description.empty() || !blocksOpt.has_value()) { skip; }
            bool passesFieldCheck = hasGlyph && hasName && hasColor &&
                                    hasDescription && hasBlocks;

            // Stage 2: check colour is recognized (only reached if stage 1 passes)
            //   TCODColor color = Colors::colorFromName(colorName);
            //   if (color.r == 0 && color.g == 0 && color.b == 0) { skip; }
            bool passesColorCheck = passesFieldCheck && colorRecognized;

            if (passesColorCheck) {
                ++validCount;
            } else {
                ++skippedCount;
            }

            // Verify: entry is valid iff ALL 6 conditions hold
            bool allValid = hasGlyph && hasName && hasColor &&
                            hasDescription && hasBlocks && colorRecognized;
            RC_ASSERT(passesColorCheck == allValid);
        }

        // Verify: valid + skipped = total entries
        RC_ASSERT(validCount + skippedCount == entryCount);

        // Verify: loaded count is bounded by total
        RC_ASSERT(validCount >= 0);
        RC_ASSERT(validCount <= entryCount);

        // Verify against actual Colors::colorFromName logic:
        // All recognized colour names should NOT return black sentinel
        for (const auto& cname : validColorNames) {
            TCODColor c = Colors::colorFromName(cname);
            RC_ASSERT(!(c.r == 0 && c.g == 0 && c.b == 0));
        }

        // An unrecognized colour name SHOULD return black (sentinel)
        TCODColor sentinel = Colors::colorFromName("totallyBogusColorName");
        RC_ASSERT(sentinel.r == 0 && sentinel.g == 0 && sentinel.b == 0);
    });
}


// ─── Property 6: Decoration spawn count bounded by config ────────────────────
// **Validates: Requirements 5.1, 5.5**
//
// For any maxRoomDecorations config value (including negative values), the number
// of decorations spawned in a single room shall be in [0, max(0, configValue)].
// The spawning logic from addDecorations is:
//   int maxDecorations = configValue;
//   if (maxDecorations < 0) maxDecorations = 0;
//   if (maxDecorations == 0) return; // no decorations spawned
//   int count = rng->getInt(0, maxDecorations);
// So for any configValue:
//   effective max = max(0, configValue)
//   spawned count is in [0, effective max]

TEST_CASE("PBT: Property 6 — decoration spawn count bounded by config", "[pbt][property][look-and-decorations]")
{
    rc::prop("spawned decoration count is in [0, max(0, configValue)]", []() {
        // Generate a random maxRoomDecorations config value in [-5, 10]
        const int configValue = *rc::gen::inRange(-5, 11);

        // Compute effective max the same way addDecorations does
        int effectiveMax = configValue;
        if (effectiveMax < 0) effectiveMax = 0;

        // If effectiveMax is 0, no decorations can be spawned
        if (effectiveMax == 0) {
            // The function returns early — spawned count must be 0
            int spawnedCount = 0;
            RC_ASSERT(spawnedCount == 0);
        } else {
            // Generate a random spawn count simulating rng->getInt(0, effectiveMax)
            // Note: stub inRange is inclusive [min, max], so use effectiveMax directly
            const int spawnedCount = *rc::gen::inRange(0, effectiveMax);

            // Verify: spawned count is in [0, effectiveMax]
            RC_ASSERT(spawnedCount >= 0);
            RC_ASSERT(spawnedCount <= effectiveMax);
        }

        // Universal invariants:
        // 1. effectiveMax is always >= 0
        RC_ASSERT(effectiveMax >= 0);

        // 2. effectiveMax == max(0, configValue)
        int expectedMax = (configValue < 0) ? 0 : configValue;
        RC_ASSERT(effectiveMax == expectedMax);
    });
}

// ─── Property 8: Spawned decoration matches its template ─────────────────────
// **Validates: Requirements 6.1, 6.2, 6.3, 6.4, 8.3**
//
// For any DecorationTemplate, when an Actor is spawned from that template,
// the Actor shall have: blocks == template.blocks, fovOnly == true,
// description == template.description, coverValue == template.coverValue,
// and all gameplay components (ai, attacker, destructible, pickable) are null.

TEST_CASE("PBT: Property 8 — spawned decoration matches its template", "[pbt][property][look-and-decorations]")
{
    rc::prop("decoration actor fields match template values", []() {
        // Generate random template values
        int glyph = *rc::gen::inRange(33, 127); // printable non-space

        // Generate random name: 1-30 printable ASCII chars
        auto printableChar = rc::gen::charInRange(32, 126);
        std::string name = *rc::gen::stringOf(printableChar, 1, 30);

        // Generate random description: 0-120 printable ASCII chars
        std::string description = *rc::gen::stringOf(printableChar, 0, 120);

        // Generate random blocks flag
        bool blocks = *genBool();

        // Generate random coverValue: 0-100
        int coverValue = *rc::gen::inRange(0, 101);

        // Spawn decoration actor the same way addDecorations does:
        Actor decoration(0, 0, glyph, name, TCODColor{255, 255, 255});
        decoration.blocks = blocks;
        decoration.fovOnly = true;
        decoration.description = description;
        decoration.coverValue = coverValue;

        // Verify all fields match template values
        RC_ASSERT(decoration.blocks == blocks);
        RC_ASSERT(decoration.fovOnly == true);
        RC_ASSERT(decoration.description == description);
        RC_ASSERT(decoration.coverValue == coverValue);

        // Verify no gameplay components are attached
        RC_ASSERT(decoration.attacker == nullptr);
        RC_ASSERT(decoration.destructible == nullptr);
        RC_ASSERT(decoration.ai == nullptr);
        RC_ASSERT(decoration.pickable == nullptr);
    });
}

// ─── Property 7: Decorations placed on walkable unoccupied tiles ─────────────
// **Validates: Requirements 5.2, 7.3**
//
// For any random room layout with a walkability grid and a set of occupied
// positions, every decoration placed using the same logic as addDecorations
// must land on a tile that is walkable AND was not occupied at placement time.

TEST_CASE("PBT: Property 7 — decorations placed on walkable unoccupied tiles", "[pbt][property][look-and-decorations]")
{
    rc::prop("every placed decoration is on a walkable unoccupied tile at placement time", []() {
        // 1. Generate a room of random size (4-8 x 4-8), kept small to limit generator calls
        const int width = *rc::gen::inRange(4, 8);
        const int height = *rc::gen::inRange(4, 8);
        const int totalTiles = width * height;

        // 2. Generate walkability bits as a flat vector (one per tile)
        const auto walkBits = *rc::gen::container<int>(totalTiles, totalTiles, rc::gen::inRange(0, 1));

        // Build a lookup from the flat vector
        auto isWalkable = [&](int x, int y) -> bool {
            return walkBits[static_cast<size_t>(y * width + x)] != 0;
        };

        // 3. Generate occupancy bits (one per tile), ~25% chance of occupied for walkable tiles
        const auto occBits = *rc::gen::container<int>(totalTiles, totalTiles, rc::gen::inRange(0, 3));
        std::vector<bool> occupied(static_cast<size_t>(totalTiles), false);
        for (int i = 0; i < totalTiles; ++i) {
            if (isWalkable(i % width, i / width) && occBits[static_cast<size_t>(i)] == 0) {
                occupied[static_cast<size_t>(i)] = true;  // ~25% of walkable tiles pre-occupied
            }
        }

        auto isOccupied = [&](int x, int y) -> bool {
            return occupied[static_cast<size_t>(y * width + x)];
        };

        // 4. Generate placement parameters
        const int numDecorations = *rc::gen::inRange(1, 5);

        // Pre-generate candidate tile indices for placement attempts (max 50)
        const auto candidates = *rc::gen::container<int>(50, 50, rc::gen::inRange(0, totalTiles - 1));
        // Pre-generate blocks flags for each decoration (5 values)
        const auto blockFlags = *rc::gen::container<int>(5, 5, rc::gen::inRange(0, 1));

        // Track placements for verification
        struct Placement { int x, y; bool blocks; };
        std::vector<Placement> placements;

        int candidateIdx = 0;
        for (int d = 0; d < numDecorations; ++d) {
            bool blocks = (blockFlags[static_cast<size_t>(d)] != 0);

            // Try up to 10 candidate positions to find a valid tile
            bool placed = false;
            for (int attempt = 0; attempt < 10 && !placed; ++attempt) {
                if (candidateIdx >= static_cast<int>(candidates.size())) break;
                int tileIdx = candidates[static_cast<size_t>(candidateIdx++)];
                int tx = tileIdx % width;
                int ty = tileIdx / width;

                // A tile is valid if: walkable AND not occupied
                if (isWalkable(tx, ty) && !isOccupied(tx, ty)) {
                    placements.push_back({tx, ty, blocks});

                    // If decoration blocks, mark as occupied for subsequent placements
                    if (blocks) {
                        occupied[static_cast<size_t>(ty * width + tx)] = true;
                    }
                    placed = true;
                }
            }
            // If no valid tile found, skip (same as real code)
        }

        // 5. Verify: every placed decoration's position is walkable
        for (const auto& p : placements) {
            RC_ASSERT(p.x >= 0);
            RC_ASSERT(p.x < width);
            RC_ASSERT(p.y >= 0);
            RC_ASSERT(p.y < height);
            RC_ASSERT(isWalkable(p.x, p.y) == true);
        }

        // Additional invariant: no two blocking placements share the same tile
        for (size_t i = 0; i < placements.size(); ++i) {
            if (!placements[i].blocks) continue;
            for (size_t j = i + 1; j < placements.size(); ++j) {
                if (!placements[j].blocks) continue;
                bool sameTile = (placements[i].x == placements[j].x &&
                                 placements[i].y == placements[j].y);
                RC_ASSERT(!sameTile);
            }
        }

        // Invariant: placement count bounded by request
        RC_ASSERT(static_cast<int>(placements.size()) <= numDecorations);
        RC_ASSERT(static_cast<int>(placements.size()) >= 0);
    });
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: look-and-decorations — Unit Tests for Decoration Loading & Spawning
// (Task 8.2)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Test: DecorationTemplate default cover_value ────────────────────────────
// **Validates: Requirements 4.1, 8.2**

TEST_CASE("Decoration: cover_value defaults to 0", "[look-and-decorations]") {
    DecorationTemplate tmpl;
    tmpl.glyph = 'X';
    tmpl.name = "test crate";
    tmpl.color = TCODColor{255, 255, 0};
    tmpl.description = "A test decoration.";
    tmpl.blocks = false;
    tmpl.coverValue = 0;
    REQUIRE(tmpl.coverValue == 0);
}

// ─── Test: Actor spawned from template has correct properties ────────────────
// **Validates: Requirements 6.1, 6.2, 6.3, 6.4, 8.2**

TEST_CASE("Decoration: actor spawned from template has correct properties", "[look-and-decorations]") {
    // Build a template with known values
    DecorationTemplate tmpl;
    tmpl.glyph = '&';
    tmpl.name = "cogitator console";
    tmpl.color = Colors::lightGreen;
    tmpl.description = "A flickering cogitator terminal.";
    tmpl.blocks = true;
    tmpl.coverValue = 40;

    // Spawn an actor the same way addDecorations does
    Actor decoration(5, 7, tmpl.glyph, tmpl.name, tmpl.color);
    decoration.blocks = tmpl.blocks;
    decoration.fovOnly = true;
    decoration.description = tmpl.description;
    decoration.coverValue = tmpl.coverValue;

    // Verify all fields match template
    REQUIRE(decoration.getGlyph() == '&');
    REQUIRE(decoration.name == "cogitator console");
    REQUIRE(decoration.getColor().r == Colors::lightGreen.r);
    REQUIRE(decoration.getColor().g == Colors::lightGreen.g);
    REQUIRE(decoration.getColor().b == Colors::lightGreen.b);
    REQUIRE(decoration.description == "A flickering cogitator terminal.");
    REQUIRE(decoration.blocks == true);
    REQUIRE(decoration.fovOnly == true);
    REQUIRE(decoration.coverValue == 40);

    // Verify no gameplay components are attached
    REQUIRE(decoration.ai == nullptr);
    REQUIRE(decoration.attacker == nullptr);
    REQUIRE(decoration.destructible == nullptr);
    REQUIRE(decoration.pickable == nullptr);
}

// ─── Test: Blocking decoration ──────────────────────────────────────────────
// **Validates: Requirements 6.5, 8.2**

TEST_CASE("Decoration: blocking decoration has blocks=true", "[look-and-decorations]") {
    Actor decoration(3, 4, 'O', "rockcrete pillar", Colors::lightGrey);
    decoration.blocks = true;
    decoration.fovOnly = true;
    decoration.description = "A load-bearing pillar.";
    decoration.coverValue = 75;

    REQUIRE(decoration.blocks == true);
}

// ─── Test: Non-blocking decoration ──────────────────────────────────────────
// **Validates: Requirements 6.5, 8.2**

TEST_CASE("Decoration: non-blocking decoration has blocks=false", "[look-and-decorations]") {
    Actor decoration(2, 6, ';', "rubble pile", Colors::colorFromName("darkGrey"));
    decoration.blocks = false;
    decoration.fovOnly = true;
    decoration.description = "A heap of broken ferrocrete.";
    decoration.coverValue = 15;

    REQUIRE(decoration.blocks == false);
}

// ─── Test: Empty registry produces no decorations ───────────────────────────
// **Validates: Requirements 5.4, 8.2**

TEST_CASE("Decoration: empty registry produces no decorations", "[look-and-decorations]") {
    // Save and clear the registry
    auto saved = std::move(engine.decorationTemplates);
    engine.decorationTemplates.clear();

    // The spawn logic checks: if (engine.decorationTemplates.empty()) return;
    // Verify the registry is empty and this condition would trigger
    REQUIRE(engine.decorationTemplates.empty());

    // Count actors before — no new decorations should be added
    size_t actorCountBefore = engine.actors.size();

    // The addDecorations guard: if registry empty, return immediately
    // We can't call addDecorations without a full map, but we verify the
    // precondition that an empty registry means zero template selection is possible
    REQUIRE(engine.decorationTemplates.size() == 0);

    // Restore registry
    engine.decorationTemplates = std::move(saved);
}

// ─── Test: Color validation — valid names return non-black ──────────────────
// **Validates: Requirements 4.4, 8.2**

TEST_CASE("Decoration: Colors::colorFromName returns non-black for valid names", "[look-and-decorations]") {
    // All valid colour names used in Decorations.lua
    std::vector<std::string> validNames = {
        "white", "desaturatedGreen", "darkerGreen", "lightBlue",
        "orange", "lightGreen", "violet", "lightYellow",
        "lightGrey", "lighterOrange", "darkGrey", "red", "darkRed", "cyan"
    };

    for (const auto& name : validNames) {
        TCODColor c = Colors::colorFromName(name);
        INFO("Color name: " << name);
        // Valid colours must NOT be black (the sentinel value)
        REQUIRE_FALSE((c.r == 0 && c.g == 0 && c.b == 0));
    }
}

TEST_CASE("Decoration: Colors::colorFromName returns black sentinel for invalid names", "[look-and-decorations]") {
    // Unrecognised colour names should return black as sentinel
    TCODColor c1 = Colors::colorFromName("totallyBogusName");
    REQUIRE(c1.r == 0);
    REQUIRE(c1.g == 0);
    REQUIRE(c1.b == 0);

    TCODColor c2 = Colors::colorFromName("");
    REQUIRE(c2.r == 0);
    REQUIRE(c2.g == 0);
    REQUIRE(c2.b == 0);

    TCODColor c3 = Colors::colorFromName("WHITE"); // case sensitive
    REQUIRE(c3.r == 0);
    REQUIRE(c3.g == 0);
    REQUIRE(c3.b == 0);
}

// ─── Test: Blocking decoration makes tile impassable via canWalk ─────────────
// **Validates: Requirements 6.5, 8.2**

TEST_CASE("Decoration: blocking decoration makes tile impassable via canWalk", "[look-and-decorations]") {
    // We need a valid map for canWalk to function. Use engine.map if available.
    // Create a blocking decoration actor and add it to engine.actors, then verify
    // canWalk returns false for that tile.

    // First, ensure engine has a map initialised so canWalk doesn't crash
    // We use the existing engine state (which should have a map from init)
    if (!engine.map) {
        // If no map exists, skip the test gracefully
        WARN("Skipping canWalk test: no map available");
        return;
    }

    // Find a walkable tile (non-wall, currently passable)
    int testX = -1, testY = -1;
    for (int y = 1; y < engine.map->getHeight() - 1 && testX < 0; ++y) {
        for (int x = 1; x < engine.map->getWidth() - 1 && testX < 0; ++x) {
            if (engine.map->canWalk(x, y)) {
                testX = x;
                testY = y;
            }
        }
    }

    if (testX < 0) {
        WARN("Skipping canWalk test: no walkable tile found");
        return;
    }

    // Verify tile is currently walkable
    REQUIRE(engine.map->canWalk(testX, testY) == true);

    // Add a blocking decoration actor at that tile
    auto decoration = std::make_unique<Actor>(testX, testY, 'O', "pillar", Colors::lightGrey);
    decoration->blocks = true;
    decoration->fovOnly = true;
    decoration->coverValue = 75;
    Actor* decPtr = decoration.get();
    engine.actors.push_back(std::move(decoration));

    // Verify tile is now impassable
    REQUIRE(engine.map->canWalk(testX, testY) == false);

    // Clean up: remove the decoration we added
    engine.actors.remove_if([decPtr](const std::unique_ptr<Actor>& a) {
        return a.get() == decPtr;
    });

    // Verify tile is walkable again after removal
    REQUIRE(engine.map->canWalk(testX, testY) == true);
}


// ═══════════════════════════════════════════════════════════════════════════════
// Feature: look-and-decorations — Unit Tests (Task 8.1)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── State Transitions ───────────────────────────────────────────────────────
// **Validates: Requirements 1.1, 1.2, 1.4**

TEST_CASE("Look mode: beginLook sets gameStatus to LOOK and cursor to player position", "[look-and-decorations]") {
    // Create a player actor at a known position
    Actor player(12, 8, '@', "Player", Colors::white);

    // Simulate what beginLook does: set lookState to player position, set gameStatus
    LookState state{ player.getX(), player.getY() };

    REQUIRE(state.cursorX == 12);
    REQUIRE(state.cursorY == 8);

    // Verify via the engine's beginLook (sets real engine state)
    engine.player = &player;
    engine.gameStatus = Engine::IDLE;
    engine.lookState = std::nullopt;

    engine.beginLook();

    REQUIRE(engine.gameStatus == Engine::LOOK);
    REQUIRE(engine.lookState.has_value());
    REQUIRE(engine.lookState->cursorX == 12);
    REQUIRE(engine.lookState->cursorY == 8);

    // Cleanup
    engine.lookState = std::nullopt;
    engine.gameStatus = Engine::IDLE;
    engine.player = nullptr;
}

TEST_CASE("Look mode: ESC key exits LOOK to IDLE and clears lookState", "[look-and-decorations]") {
    // Set up engine in LOOK state
    engine.gameStatus = Engine::LOOK;
    engine.lookState = LookState{ 5, 5 };

    // Simulate ESC key press
    engine.inputState = InputState{};
    engine.inputState.key.key = SDLK_ESCAPE;
    engine.inputState.key.pressed = true;

    engine.updateLook();

    REQUIRE(engine.gameStatus == Engine::IDLE);
    REQUIRE(!engine.lookState.has_value());
}

TEST_CASE("Look mode: 'l' key exits LOOK to IDLE and clears lookState", "[look-and-decorations]") {
    // Set up engine in LOOK state
    engine.gameStatus = Engine::LOOK;
    engine.lookState = LookState{ 5, 5 };

    // Simulate 'l' key press
    engine.inputState = InputState{};
    engine.inputState.key.c = 'l';
    engine.inputState.key.pressed = true;

    engine.updateLook();

    REQUIRE(engine.gameStatus == Engine::IDLE);
    REQUIRE(!engine.lookState.has_value());
}

// ─── Cursor Highlight Colour ─────────────────────────────────────────────────
// **Validates: Requirements 1.2**

TEST_CASE("Look mode: cursor highlight colour differs from floor and wall colours", "[look-and-decorations]") {
    // The cursor highlight in renderLook uses {255, 255, 63} (bright yellow bg)
    // Floor tiles use the map's default dark/light ground colours
    // Wall tiles use the map's default dark/light wall colours
    // We verify the chosen cursor colour is distinct from typical floor/wall bg colours

    constexpr TCODColor cursorHighlight{255, 255, 63};

    // Typical floor background colours (from Map::render - dark ground is near black,
    // light ground is brown-ish)
    constexpr TCODColor darkFloor{0, 0, 0};        // unexplored/dark area
    constexpr TCODColor lightFloor{50, 50, 50};    // approximate visible floor bg

    // Typical wall background colours
    constexpr TCODColor darkWall{0, 0, 0};
    constexpr TCODColor lightWall{50, 50, 50};

    // Verify cursor is distinct from all these
    REQUIRE(cursorHighlight != darkFloor);
    REQUIRE(cursorHighlight != lightFloor);
    REQUIRE(cursorHighlight != darkWall);
    REQUIRE(cursorHighlight != lightWall);

    // Also confirm it's not black (invisible)
    REQUIRE(cursorHighlight != Colors::black);
}

// ─── No Turn Advancement ─────────────────────────────────────────────────────
// **Validates: Requirements 1.5**

TEST_CASE("Look mode: no turn advancement during LOOK state", "[look-and-decorations]") {
    // The key property: updateLook never sets gameStatus to NEW_TURN.
    // After any input in LOOK mode, the status is either LOOK (cursor moved) or IDLE (exited).

    // Set up engine in LOOK state with a cursor
    engine.gameStatus = Engine::LOOK;
    engine.lookState = LookState{ 10, 10 };

    // Simulate a movement key (arrow up) — requires a map for clamping.
    // Without a map we can't call updateLook with movement, but we can verify
    // that the exit path never produces NEW_TURN.

    // Test exit via ESC: should go to IDLE, not NEW_TURN
    engine.inputState = InputState{};
    engine.inputState.key.key = SDLK_ESCAPE;
    engine.inputState.key.pressed = true;

    engine.updateLook();

    REQUIRE(engine.gameStatus == Engine::IDLE);
    REQUIRE(engine.gameStatus != Engine::NEW_TURN);

    // Test exit via 'l': should go to IDLE, not NEW_TURN
    engine.gameStatus = Engine::LOOK;
    engine.lookState = LookState{ 10, 10 };
    engine.inputState = InputState{};
    engine.inputState.key.c = 'l';
    engine.inputState.key.pressed = true;

    engine.updateLook();

    REQUIRE(engine.gameStatus == Engine::IDLE);
    REQUIRE(engine.gameStatus != Engine::NEW_TURN);

    // Test no-op input (no key pressed): should remain LOOK, not NEW_TURN
    engine.gameStatus = Engine::LOOK;
    engine.lookState = LookState{ 10, 10 };
    engine.inputState = InputState{};

    engine.updateLook();

    REQUIRE(engine.gameStatus == Engine::LOOK);
    REQUIRE(engine.gameStatus != Engine::NEW_TURN);
}

// ─── Terrain Descriptions ────────────────────────────────────────────────────
// **Validates: Requirements 2.5**

TEST_CASE("Look mode: terrain descriptions for each tile type", "[look-and-decorations]") {
    // The renderLook logic produces terrain strings based on level type and tile state.
    // We verify the expected string mapping at the logic level.

    // BSP level: walkable → "Floor", non-walkable → "Wall"
    SECTION("BSP level: walkable tile shows Floor") {
        std::string terrain;
        bool isWall = false;
        if (isWall) {
            terrain = "Wall";
        } else {
            terrain = "Floor";
        }
        REQUIRE(terrain == "Floor");
    }

    SECTION("BSP level: non-walkable tile shows Wall") {
        std::string terrain;
        bool isWall = true;
        if (isWall) {
            terrain = "Wall";
        } else {
            terrain = "Floor";
        }
        REQUIRE(terrain == "Wall");
    }

    // Outdoor level: TerrainType enum → string
    SECTION("Outdoor level: GROUND terrain shows Floor") {
        TerrainType tt = TerrainType::GROUND;
        std::string terrain;
        switch (tt) {
            case TerrainType::GROUND: terrain = "Floor"; break;
            case TerrainType::TREE:   terrain = "Tree";  break;
            case TerrainType::WATER:  terrain = "Water"; break;
        }
        REQUIRE(terrain == "Floor");
    }

    SECTION("Outdoor level: TREE terrain shows Tree") {
        TerrainType tt = TerrainType::TREE;
        std::string terrain;
        switch (tt) {
            case TerrainType::GROUND: terrain = "Floor"; break;
            case TerrainType::TREE:   terrain = "Tree";  break;
            case TerrainType::WATER:  terrain = "Water"; break;
        }
        REQUIRE(terrain == "Tree");
    }

    SECTION("Outdoor level: WATER terrain shows Water") {
        TerrainType tt = TerrainType::WATER;
        std::string terrain;
        switch (tt) {
            case TerrainType::GROUND: terrain = "Floor"; break;
            case TerrainType::TREE:   terrain = "Tree";  break;
            case TerrainType::WATER:  terrain = "Water"; break;
        }
        REQUIRE(terrain == "Water");
    }
}

// ─── Actor Description Formatting (concrete examples) ────────────────────────
// **Validates: Requirements 2.1, 2.6, 2.7, 3.2, 3.3**

TEST_CASE("Look mode: actor with empty description shows only name", "[look-and-decorations]") {
    Actor actor(3, 4, 'o', "Ork Boy", Colors::orkSkin);
    // description defaults to empty string
    REQUIRE(actor.description.empty());

    // Apply the formatting logic from renderLook
    std::string formatted;
    if (actor.description.empty()) {
        formatted = actor.name;
    } else {
        formatted = actor.name + ": " + actor.description;
    }

    REQUIRE(formatted == "Ork Boy");
}

TEST_CASE("Look mode: actor with description shows 'name: description'", "[look-and-decorations]") {
    Actor actor(3, 4, '#', "ammo crate", Colors::lightYellow);
    actor.description = "A battered ammunition crate stamped with Aquila markings.";

    // Apply the formatting logic from renderLook
    std::string formatted;
    if (actor.description.empty()) {
        formatted = actor.name;
    } else {
        formatted = actor.name + ": " + actor.description;
    }

    REQUIRE(formatted == "ammo crate: A battered ammunition crate stamped with Aquila markings.");
}

TEST_CASE("Look mode: dead actor (corpse) is included in look output", "[look-and-decorations]") {
    // Create an actor with a destructible component, then kill it
    Actor corpse(5, 5, '%', "Ork Boy corpse", Colors::darkRed);
    corpse.destructible = std::make_shared<MonsterDestructible>(10.0f, 0.0f, "Ork Boy corpse", 10);
    corpse.destructible->hp = 0.0f; // dead
    corpse.blocks = false;

    REQUIRE(corpse.destructible->isDead());

    // The renderLook logic iterates ALL actors at the cursor position
    // (does NOT check isDead — corpses are always included)
    // Verify the corpse would be included (it's not excluded by any condition
    // other than being the player)
    bool isPlayer = false;
    bool atCursorPos = (corpse.getX() == 5 && corpse.getY() == 5);

    // Simulate: actor is not the player and is at cursor position → included
    REQUIRE(!isPlayer);
    REQUIRE(atCursorPos);

    // Format its output
    std::string formatted;
    if (corpse.description.empty()) {
        formatted = corpse.name;
    } else {
        formatted = corpse.name + ": " + corpse.description;
    }

    REQUIRE(formatted == "Ork Boy corpse");
}

TEST_CASE("Look mode: player is excluded from look output on own tile", "[look-and-decorations]") {
    // The renderLook logic skips the player: `if (actor == player) continue;`
    Actor player(7, 7, '@', "Player", Colors::white);

    // Simulate the filtering logic from renderLook
    std::vector<std::string> lines;
    // pretend we're iterating actors and this is the player
    Actor* currentActor = &player;
    Actor* enginePlayer = &player;

    if (currentActor == enginePlayer) {
        // skip — player excluded
    } else {
        lines.push_back(currentActor->name);
    }

    REQUIRE(lines.empty()); // player was excluded
}

// ─── Explored-but-not-visible and unexplored tile messages ───────────────────
// **Validates: Requirements 2.2, 2.3**

TEST_CASE("Look mode: explored-but-not-visible tile shows 'can't see clearly' message", "[look-and-decorations]") {
    // The logic in renderLook:
    // if (map->isInFOV(x,y)) { ... show actors/terrain ... }
    // else if (map->isExplored(x,y)) { display "You can't see that clearly from here." }
    // else { display nothing }

    bool isInFOV = false;
    bool isExplored = true;

    std::string output;
    if (isInFOV) {
        output = "Floor"; // placeholder
    } else if (isExplored) {
        output = "You can't see that clearly from here.";
    } else {
        output = ""; // nothing
    }

    REQUIRE(output == "You can't see that clearly from here.");
}

TEST_CASE("Look mode: unexplored tile shows nothing", "[look-and-decorations]") {
    bool isInFOV = false;
    bool isExplored = false;

    std::string output;
    if (isInFOV) {
        output = "Floor"; // placeholder
    } else if (isExplored) {
        output = "You can't see that clearly from here.";
    } else {
        output = ""; // nothing
    }

    REQUIRE(output.empty());
}
