#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.h"

// ─── Camera unit tests ───────────────────────────────────────────────────────

TEST_CASE("Camera::apply adds offset to world coords", "[camera]")
{
    Camera cam(5, 3, 80, 43, 160, 86);
    auto [sx, sy] = cam.apply(10, 20);
    REQUIRE(sx == 15);
    REQUIRE(sy == 23);
}

TEST_CASE("Camera::getWorldLocation subtracts offset from screen coords", "[camera]")
{
    Camera cam(5, 3, 80, 43, 160, 86);
    auto [wx, wy] = cam.getWorldLocation(15, 23);
    REQUIRE(wx == 10);
    REQUIRE(wy == 20);
}

// ─── Property-based tests ─────────────────────────────────────────────────────

TEST_CASE("PBT: Camera apply/getWorldLocation is a bijection", "[camera][pbt]")
{
    rc::prop("apply then getWorldLocation returns original world coords", []() {
        const int offsetX = *rc::gen::inRange(-500, 500);
        const int offsetY = *rc::gen::inRange(-500, 500);
        const int worldX  = *rc::gen::inRange(-1000, 1000);
        const int worldY  = *rc::gen::inRange(-1000, 1000);

        Camera cam(offsetX, offsetY, 80, 43, 160, 86);
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
