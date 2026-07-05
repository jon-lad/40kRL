# 40kRL

A Warhammer 40,000 themed roguelike built in C++17 with [libtcod](https://github.com/libtcod/libtcod) and [sol2](https://github.com/ThePhD/sol2) for Lua scripting.

## Overview

40kRL is a traditional turn-based roguelike set in the Warhammer 40K universe. The player explores procedurally generated dungeon levels, fights Orks and other enemies, collects items, and descends deeper into the underhive. The game uses a component-based entity system where actors gain capabilities (combat, AI, inventory, pickable items) through attached components rather than deep inheritance.

## Features

- Procedural dungeon generation via BSP (Binary Space Partitioning)
- Perlin noise outdoor terrain (ground, trees, water) with connectivity guarantee
- Equipment system with body slots (weapon, offhand, head, body) and stat modifiers
- Turn-based movement and combat with melee attacks
- FOV (field of view) with libtcod's shadow-casting algorithm
- Scent-tracking monster AI that follows the player even outside line of sight
- Look mode for inspecting tiles and reading actor descriptions
- Data-driven room decorations (crates, barrels, consoles, pillars, etc.)
- Lua-driven enemy, item, and decoration definitions for easy modding
- Item system with health potions, damage scrolls, and confusion scrolls
- Save/load via binary archives (TCODZip)
- Scrolling camera with a 160×86 map viewed through an 80×43 viewport
- Level-up system with XP rewards
- Ascending/descending stairs between dungeon levels and the planet surface

## Building

### Requirements

- Visual Studio 2022+ (MSVC v145 toolset, C++17)
- Windows 10 SDK
- [vcpkg](https://github.com/microsoft/vcpkg) (C++ package manager)

### Steps

1. Clone the repository
2. Install and bootstrap vcpkg:
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   ```
3. Install dependencies with the overlay port:
   ```powershell
   C:\vcpkg\vcpkg.exe install libtcod:x64-windows sol2:x64-windows lua:x64-windows --overlay-ports=vcpkg-overlay/ports
   C:\vcpkg\vcpkg.exe integrate install
   ```
4. Open `40kRL.sln` in Visual Studio
5. Build the `40kRL` project (Debug or Release, x64)
6. Run from the project root directory (so `Scripts/` and `terminal.png` are found)

### Running Tests

The solution includes a `40kRL_Tests` project using [Catch2](https://github.com/catchorg/Catch2) and a minimal [RapidCheck](https://github.com/emil-e/rapidcheck)-compatible stub for property-based testing.

```powershell
msbuild 40kRL.sln /p:Configuration=Debug /p:Platform=x64 /t:40kRL_Tests
& .\x64\Debug\40kRL_Tests.exe --reporter compact
```

## Project Structure

```
40kRL/
├── Headers/          C++ header files
├── Source/           C++ implementation files
├── Scripts/          Lua data scripts
│   ├── Config.lua    Game configuration (map size, FOV, thresholds)
│   ├── Classes.lua   Player class stats (hp, defense, power, skill)
│   ├── Enemies.lua   Enemy definitions and spawn logic
│   ├── Items.lua     Item definitions and spawn logic
│   ├── Effects.lua   Item effect scripts
│   ├── Decorations.lua  Room decoration definitions (glyphs, colours, cover values)
│   └── Map.Lua       Map generation parameters
├── Tests/            Unit and property-based tests (Catch2 + RapidCheck)
├── Docs/             Design notes and todo list
└── .kiro/specs/      Feature specifications
```

## Controls

| Key | Action |
|-----|--------|
| Arrow keys / Numpad | Move / Attack |
| `g` | Pick up item (shows selection menu when multiple items on tile) |
| `i` | Open inventory (shows only unequipped items) |
| `e` | Open equipment menu |
| `d` | Drop item |
| `l` | Look mode (inspect tiles — move cursor with arrows, ESC to exit) |
| `<` | Ascend stairs |
| `>` | Descend stairs |
| `Esc` | Save and return to menu |
| `F12` | Toggle debug mode (level-skip and dev tools) |

## Scripting

Enemy and item definitions live in `Scripts/` as Lua tables. You can add new enemies by editing `Scripts/Enemies.lua`, new items in `Scripts/Items.lua`, or new room decorations in `Scripts/Decorations.lua` without recompiling. Game configuration (map dimensions, FOV radius, thresholds, decoration counts) is in `Scripts/Config.lua`. Player class stats (hp, defense, power, skill) are in `Scripts/Classes.lua`.

## Roadmap

- Perlin noise outdoor map generation (level 20+)
- Ranged combat (guns)
- Character generation
- NPC dialogue
- World/planet travel system

## Releases

The repository includes a manual **Release Build** workflow (Actions → Release Build → Run workflow) that builds and tests the project in either Debug or Release configuration. Build artifacts (exe, DLLs, scripts, assets) are uploaded as workflow artifacts for download.

## License

This is a personal project. No license has been specified.
