# 40kRL

A Warhammer 40,000 themed roguelike built in C++17 with [libtcod](https://github.com/libtcod/libtcod) and [sol2](https://github.com/ThePhD/sol2) for Lua scripting.

## Overview

40kRL is a traditional turn-based roguelike set in the Warhammer 40K universe. The player explores procedurally generated dungeon levels, fights Orks and other enemies, collects items, and descends deeper into the underhive. The game uses a component-based entity system where actors gain capabilities (combat, AI, inventory, pickable items) through attached components rather than deep inheritance.

## Features

- Procedural dungeon generation via BSP (Binary Space Partitioning)
- Turn-based movement and combat with melee attacks
- FOV (field of view) with libtcod's shadow-casting algorithm
- Scent-tracking monster AI that follows the player even outside line of sight
- Lua-driven enemy and item definitions for easy modding
- Item system with health potions, damage scrolls, and confusion scrolls
- Save/load via binary archives (TCODZip)
- Scrolling camera with a 160×86 map viewed through an 80×43 viewport
- Level-up system with XP rewards

## Building

### Requirements

- Visual Studio 2019+ (MSVC v142 toolset)
- Windows 10 SDK
- [libtcod 1.15.0](https://github.com/libtcod/libtcod/releases) (x86 MSVC build)
- [sol2](https://github.com/ThePhD/sol2) (header-only Lua binding)
- Lua 5.x

### Steps

1. Clone the repository
2. Download libtcod 1.15.0 and place it so the include path is accessible (the project expects it at a sibling directory — adjust `IncludePath` in the `.vcxproj` if needed)
3. Open `40kRL.sln` in Visual Studio
4. Build the `40kRL` project (Debug or Release, x86 or x64)
5. Ensure `libtcod.dll`, `SDL2.dll`, and `terminal.png` are in the output directory alongside the executable

### Running Tests

The solution includes a `40kRL_Tests` project using [Catch2](https://github.com/catchorg/Catch2) and [RapidCheck](https://github.com/emil-e/rapidcheck) for property-based testing. Build and run the test project from Visual Studio's Test Explorer.

## Project Structure

```
40kRL/
├── Headers/          C++ header files
├── Source/           C++ implementation files
├── Scripts/          Lua data scripts
│   ├── Config.lua    Game configuration (map size, FOV, stats)
│   ├── Enemies.lua   Enemy definitions and spawn logic
│   ├── Items.lua     Item definitions and spawn logic
│   ├── Effects.lua   Item effect scripts
│   └── Map.Lua       Map generation parameters
├── Tests/            Unit and property-based tests (Catch2 + RapidCheck)
├── Docs/             Design notes and todo list
└── .kiro/specs/      Feature specifications
```

## Controls

| Key | Action |
|-----|--------|
| Arrow keys / Numpad | Move / Attack |
| `g` | Pick up item |
| `i` | Open inventory |
| `d` | Drop item |
| `>` | Descend stairs |
| `Esc` | Save and return to menu |

## Scripting

Enemy and item definitions live in `Scripts/` as Lua tables. You can add new enemies by editing `Scripts/Enemies.lua` or new items in `Scripts/Items.lua` without recompiling. Game configuration (map dimensions, FOV radius, player stats) is in `Scripts/Config.lua`.

## Roadmap

- Perlin noise outdoor map generation (level 20+)
- Ranged combat (guns)
- Equipment and wearable items
- Character generation
- NPC dialogue
- World/planet travel system

## License

This is a personal project. No license has been specified.
