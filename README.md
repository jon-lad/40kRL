# 40kRL

A Warhammer 40,000 Rogue Trader roguelike built in C++17 with [libtcod](https://github.com/libtcod/libtcod) and [sol2](https://github.com/ThePhD/sol2) for Lua scripting.

## Overview

40kRL is a traditional turn-based roguelike set in the Warhammer 40K universe. The player creates a character by selecting a homeworld and career path (based on the Rogue Trader RPG), then explores procedurally generated levels, fights enemies, purchases advances with XP, and descends deeper into the underhive. The game uses a component-based entity system where actors gain capabilities through attached components.

## Features

- **Character generation** — select a homeworld (stat modifiers, starting skills/traits) and career path (rank-gated advances), then spend starting XP on characteristic, skill, and talent purchases
- **Rogue Trader progression** — XP spending on rank-gated advances replaces traditional leveling; rank up by spending XP thresholds
- **CharacterSheet component** — unified player identity (characteristics, career, skills, talents, traits) persisted through save/load
- **In-game advance purchases** — press `x` to open the advance overlay and spend earned XP mid-game
- Procedural dungeon generation via BSP (Binary Space Partitioning)
- Perlin noise outdoor terrain (ground, trees, water) with connectivity guarantee
- Equipment system with body slots (weapon, offhand, head, body) and stat modifiers
- Turn-based movement and combat with melee attacks and ranged shooting
- Ranged combat system with d100 roll-under vs Ballistic Skill, dodge tests, and ammunition management
- FOV (field of view) with libtcod's shadow-casting algorithm
- Scent-tracking monster AI that follows the player even outside line of sight
- Ranged AI enemies that shoot at range, reload when empty, and melee when cornered
- Look mode for inspecting tiles and reading actor descriptions
- Data-driven room decorations (crates, barrels, consoles, pillars, etc.)
- Lua-driven enemy, item, equipment, homeworld, career, skill, and talent definitions
- Item system with health potions, damage scrolls, and confusion scrolls
- Save/load via binary archives (TCODZip)
- Scrolling camera with a 160×86 map viewed through an 80×43 viewport
- Ascending/descending stairs between dungeon levels and the planet surface
- World map overlay with Perlin-noise biomes and fast-travel to hive cities
- WFC-generated hive city interiors with 10 distinct tile types and adjacency-based layouts

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

The solution includes a `40kRL_Tests` project using [Catch2](https://github.com/catchorg/Catch2) and [RapidCheck](https://github.com/emil-e/rapidcheck) for property-based testing.

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
│   ├── Homeworlds.lua  Homeworld templates (stat mods, starting skills/traits)
│   ├── Careers.lua   Career paths with ranks, advances, and combat stats
│   ├── Skills.lua    Skill definitions (name, combat/non-combat flag)
│   ├── Talents.lua   Talent definitions (name, description)
│   ├── Enemies.lua   Enemy definitions and spawn logic
│   ├── Equipment.lua Equipment item definitions (weapons, armour)
│   ├── Items.lua     Consumable item definitions and spawn logic
│   ├── Effects.lua   Item effect scripts
│   ├── Decorations.lua  Room decoration definitions
│   └── Map.Lua       Map generation parameters
├── Tests/            Unit and property-based tests (Catch2 + RapidCheck)
├── Docs/             Design notes and roadmap
└── .kiro/specs/      Feature specifications
```

## Controls

| Key | Action |
|-----|--------|
| Arrow keys / Numpad | Move / Attack |
| `g` | Pick up item (selection menu when multiple items on tile) |
| `i` | Open inventory (unequipped items) |
| `e` | Open equipment menu |
| `d` | Drop item |
| `x` | Open advance purchase overlay (spend XP on rank-gated advances) |
| `s` | Shoot ranged weapon (targeting mode — Enter to confirm, ESC to cancel) |
| `r` | Reload ranged weapon |
| `l` | Look mode (inspect tiles — arrows to move cursor, ESC to exit) |
| `c` | Character sheet (view characteristics and bonuses) |
| `m` | World map (view biomes, fast-travel to hive cities) |
| `<` | Ascend stairs |
| `>` | Descend stairs |
| `Esc` | Save and return to menu |
| `F12` | Toggle debug mode |

## Scripting

All game content is data-driven via Lua tables in `Scripts/`. You can add new enemies (`Enemies.lua`), items (`Items.lua`), equipment (`Equipment.lua`), decorations (`Decorations.lua`), homeworlds (`Homeworlds.lua`), careers (`Careers.lua`), skills (`Skills.lua`), and talents (`Talents.lua`) without recompiling.

Player combat stats (hp, defense, power, inventory size) are defined per career in `Careers.lua`. Each enemy template can specify the nine Rogue Trader characteristics (ws, bs, s, t, ag, int, per, wp, fel) — omitted fields default to 20.

## Roadmap

See `Docs/ToDo.txt` for the full roadmap. Key upcoming features:

- Space travel system (solar, subsector, sector scale with combat and encounters)
- Non-hostile and hostile NPCs with natural-looking AI
- Lua-driven AI for modifiable mob behaviour
- UI overhaul with larger grid/tiles
- Expanded map types (spacecraft, mines, agriworlds, xenos planets)
- Basic storyline and interactable skill-check objects
- Galactic strategy layer (faction control, crusades, world conquest)
- Equipment progression scaling with player rank
- Character description updated with injuries and modifications

## Releases

The Release Build workflow (Actions → Release Build → Run workflow) builds, tests, and publishes a GitHub Release with the game zip attached. Specify the version number (semver, no `v` prefix) when triggering.

## License

This is a personal project. No license has been specified.
