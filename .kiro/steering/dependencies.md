# Project Dependencies

This project uses the following pinned library versions. All code, CI workflows, and build configurations must target these versions.

- **libtcod 2.2.0** — terminal rendering, FOV, noise, BSP, pathfinding
- **SDL3** — window management, input handling, audio (via libtcod)
- **sol2** — C++ binding for Lua scripting
- **Lua 5.4** — scripting runtime for effects, enemies, items, and config
- **Catch2 v3** — unit test framework (amalgamated, in Tests/lib/)
- **C++17** — language standard (MSVC v142+ toolset)

Do not introduce SDL2 code or headers. The InputHandler uses `<SDL3/SDL.h>` directly for event polling. libtcod handles its own SDL3 initialization internally.
