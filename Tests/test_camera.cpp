#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"
#include "Constants.h"

// ─── Camera unit tests ───────────────────────────────────────────────────────

TEST_CASE("Camera::apply adds offset to world coords", "[camera]")
{
    Camera cam(5, 3, layout::VIEWPORT_WIDTH, layout::VIEWPORT_HEIGHT, 160, 86);
    auto [sx, sy] = cam.apply(10, 20);
    REQUIRE(sx == 15);
    REQUIRE(sy == 23);
}

TEST_CASE("Camera::getWorldLocation subtracts offset from screen coords", "[camera]")
{
    Camera cam(5, 3, layout::VIEWPORT_WIDTH, layout::VIEWPORT_HEIGHT, 160, 86);
    auto [wx, wy] = cam.getWorldLocation(15, 23);
    REQUIRE(wx == 10);
    REQUIRE(wy == 20);
}

TEST_CASE("Camera uses new viewport dimensions (96x42)", "[camera]")
{
    // Verify the layout constants are what we expect
    REQUIRE(layout::VIEWPORT_WIDTH == 96);
    REQUIRE(layout::VIEWPORT_HEIGHT == 42);

    Camera cam(layout::VIEWPORT_X, 0, layout::VIEWPORT_WIDTH, layout::VIEWPORT_HEIGHT, 160, 86);
    REQUIRE(cam.width == 96);
    REQUIRE(cam.height == 42);
}

TEST_CASE("Camera centres player within 96x42 viewport", "[camera]")
{
    const int mapW = 160, mapH = 86;
    Camera cam(0, 0, layout::VIEWPORT_WIDTH, layout::VIEWPORT_HEIGHT, mapW, mapH);
    Actor player(80, 43, '@', "Player", Colors::white);
    cam.update(&player, false);

    // Player at (80, 43) with viewport 96x42:
    // Expected x = -(80) + 96/2 = -80 + 48 = -32
    // Expected y = -(43) + 42/2 = -43 + 21 = -22
    // Both within valid clamping range [-(mapDim - vpDim), 0]
    // x clamp: [-(160-96), 0] = [-64, 0] → -32 is valid
    // y clamp: [-(86-42), 0]  = [-44, 0] → -22 is valid
    REQUIRE(cam.x == -32);
    REQUIRE(cam.y == -22);

    // Verify the player appears at screen centre
    auto [screenX, screenY] = cam.apply(80, 43);
    REQUIRE(screenX == 48); // 96/2
    REQUIRE(screenY == 21); // 42/2
}

TEST_CASE("Camera clamps at map edges with new viewport", "[camera]")
{
    const int mapW = 160, mapH = 86;
    Camera cam(0, 0, layout::VIEWPORT_WIDTH, layout::VIEWPORT_HEIGHT, mapW, mapH);

    SECTION("Player near top-left corner") {
        Actor player(5, 3, '@', "Player", Colors::white);
        cam.update(&player, false);
        // Unclamped: x = -5 + 48 = 43, y = -3 + 21 = 18
        // Clamp x > 0 → x = 0; y > 0 → y = 0
        REQUIRE(cam.x == 0);
        REQUIRE(cam.y == 0);
    }

    SECTION("Player near bottom-right corner") {
        Actor player(155, 83, '@', "Player", Colors::white);
        cam.update(&player, false);
        // Unclamped: x = -155 + 48 = -107, y = -83 + 21 = -62
        // Clamp x < -(160-96) = -64 → x = -64
        // Clamp y < -(86-42) = -44 → y = -44
        REQUIRE(cam.x == -64);
        REQUIRE(cam.y == -44);
    }

    SECTION("Player at map centre") {
        Actor player(80, 43, '@', "Player", Colors::white);
        cam.update(&player, false);
        // Unclamped: x = -80 + 48 = -32, y = -43 + 21 = -22
        // Both within valid range
        REQUIRE(cam.x == -32);
        REQUIRE(cam.y == -22);
    }
}

// ─── Property-based tests ─────────────────────────────────────────────────────

TEST_CASE("PBT: Camera apply/getWorldLocation is a bijection", "[camera][pbt]")
{
    rc::prop("apply then getWorldLocation returns original world coords", []() {
        const int offsetX = *rc::gen::inRange(-500, 500);
        const int offsetY = *rc::gen::inRange(-500, 500);
        const int worldX  = *rc::gen::inRange(-1000, 1000);
        const int worldY  = *rc::gen::inRange(-1000, 1000);

        Camera cam(offsetX, offsetY, layout::VIEWPORT_WIDTH, layout::VIEWPORT_HEIGHT, 160, 86);
        auto [sx, sy] = cam.apply(worldX, worldY);
        auto [rx, ry] = cam.getWorldLocation(sx, sy);
        RC_ASSERT(rx == worldX);
        RC_ASSERT(ry == worldY);
    });
}

TEST_CASE("PBT: Camera centres on player after update (BSP mode)", "[camera][pbt]")
{
    rc::prop("camera offset centres the player in the viewport, then clamps to map bounds", []() {
        const int mapW    = 160;
        const int mapH    = 86;
        const int playerX = *rc::gen::inRange(0, mapW);
        const int playerY = *rc::gen::inRange(0, mapH);
        const int vpW     = *rc::gen::inRange(20, 120);
        const int vpH     = *rc::gen::inRange(10, 60);

        Camera cam(0, 0, vpW, vpH, mapW, mapH);
        Actor player(playerX, playerY, '@', "Player", Colors::white);
        cam.update(&player, false);

        // Expected: centred then clamped
        int expectedX = -(playerX) + vpW / 2;
        int expectedY = -(playerY) + vpH / 2;
        if (expectedX > 0) expectedX = 0;
        if (expectedY > 0) expectedY = 0;
        if (expectedX < -(mapW - vpW)) expectedX = -(mapW - vpW);
        if (expectedY < -(mapH - vpH)) expectedY = -(mapH - vpH);

        RC_ASSERT(cam.x == expectedX);
        RC_ASSERT(cam.y == expectedY);
    });
}
