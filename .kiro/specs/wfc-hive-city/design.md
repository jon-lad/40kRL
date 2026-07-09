# Design Document: WFC Hive City Generator

## Overview

This feature adds a Wave Function Collapse (WFC) level generator to 40kRL that produces hive city interiors when the player fast-travels to a hive city from the world map. The WFC generator is implemented as a standalone module (`WfcGenerator`) that accepts a Lua-defined tileset and grid dimensions, runs the WFC constraint-solving algorithm, and outputs a fully-resolved 2D grid of tile assignments guaranteed to be connected.

The Map class integrates the WFC output by translating resolved tiles into TCODMap walkability/transparency data, rendering tiles with their defined glyphs and colours, and spawning actors. WFC is added as `LevelType::WFC = 2` alongside existing BSP and OUTDOOR generation, reusing the same 160×86 grid and seeded RNG infrastructure.

**Key design decisions:**
- **Separate module**: WfcGenerator is a standalone class with no dependency on Map, Engine, or libtcod. This makes the core algorithm testable in isolation with property-based tests.
- **Lua-driven tileset**: Tile definitions and adjacency rules live in `Scripts/WfcTiles.lua`, following the existing sol2 pattern used by Decorations.lua and Config.lua.
- **Deterministic**: Given the same seed and tileset, WFC produces identical output. The Map's existing seeded TCODRandom provides the seed; WfcGenerator uses its own `std::mt19937` seeded from that value.
- **Graceful fallback**: If WFC fails after exhausting retries, the Map silently falls back to BSP generation using the same seed, keeping the Engine unaware of the failure.

---

## Architecture

```mermaid
graph TD
    subgraph Engine Layer
        E[Engine] --> M[Map]
    end

    subgraph Map Layer
        M -->|"init(withActors, WFC)"| IW[initWfc]
        IW --> TL[TilesetLoader]
        IW --> WG[WfcGenerator]
        IW --> TR[Translation]
        IW --> AS[Actor Spawning]
    end

    subgraph WFC Module - standalone, testable
        TL -->|"loads Scripts/WfcTiles.lua"| TS[WfcTileset]
        WG -->|"uses"| TS
        WG -->|"produces"| RG[Resolved Grid]
        WG -->|"validates"| CC[Connectivity Check]
    end

    subgraph Rendering
        M -->|"renderWfc()"| RW[WFC Renderer]
        RW -->|"reads"| TID[wfcTileIds array]
        RW -->|"reads"| TS
    end

    subgraph Persistence
        M -->|"save()"| SER[Serialize tile indices + seed]
        M -->|"load()"| DES[Deserialize + reapply properties]
    end
```

**Module boundary**: The `WfcGenerator` class owns only the algorithm. It takes a `WfcTileset` (pre-validated collection of tiles with adjacency rules) and grid dimensions as input, and returns a `WfcResult` containing either a resolved grid or a failure indicator. It does not know about TCODMap, actors, rendering, or Lua loading.

**Tileset loading**: A free function `loadWfcTileset(sol::state&)` handles Lua parsing, validation, symmetry correction, and produces a `WfcTileset` struct. This function lives in WfcGenerator.cpp but is logically separate from the algorithm.

---

## Components and Interfaces

### WfcTile (Data Structure)

```cpp
// Headers/WfcGenerator.h
struct WfcTile {
    std::string id;           // unique identifier (e.g. "corridor")
    int glyph;                // display character
    std::string colorName;    // colour name for Colors::colorFromName()
    bool walkable;
    bool transparent;
    float weight;             // selection bias (default 1.0)
    // Adjacency: allowed neighbour IDs per direction
    std::vector<std::string> adjNorth;
    std::vector<std::string> adjSouth;
    std::vector<std::string> adjEast;
    std::vector<std::string> adjWest;
};
```

### WfcTileset (Data Structure)

```cpp
struct WfcTileset {
    std::vector<WfcTile> tiles;
    std::unordered_map<std::string, int> idToIndex; // fast ID→index lookup

    // Returns tile index for a given ID, or -1 if not found
    int indexOf(const std::string& id) const;
    bool isValid() const; // true if tiles.size() >= 5
};
```

### WfcResult (Data Structure)

```cpp
struct WfcResult {
    bool success;
    std::vector<int> grid;  // flat array [x + y * width], values are tile indices
    int attemptsUsed;       // how many restarts were needed
    long seedUsed;          // the actual seed that produced this grid
};
```

### WfcConfig (Data Structure)

```cpp
struct WfcConfig {
    int maxRestarts = 10;
    float minWalkablePercent = 0.20f;
    int minStairDistance = 20;
    int gridWidth = 160;
    int gridHeight = 86;
    std::optional<long> seedOverride; // from Config.lua wfcSeed
};
```

### WfcGenerator (Class)

```cpp
class WfcGenerator {
public:
    // Run WFC algorithm. Returns resolved grid or failure.
    // rngSeed: deterministic seed for generation
    // tileset: pre-validated tileset
    // config: generation parameters
    static WfcResult generate(long rngSeed, const WfcTileset& tileset,
                              const WfcConfig& config);

    // Connectivity check: returns true if all walkable cells form one connected component
    // and walkable count >= minWalkable. Exposed for testing.
    static bool checkConnectivity(const std::vector<int>& grid, int width, int height,
                                  const WfcTileset& tileset, int minWalkableCells);

private:
    // Internal state for a single generation attempt
    struct Cell {
        std::vector<int> domain; // indices into tileset.tiles
    };

    // Select lowest-entropy cell (fewest domain options). Ties broken by RNG.
    static int selectLowestEntropy(const std::vector<Cell>& cells, std::mt19937& rng);

    // Collapse a cell: weighted random selection from its domain.
    static int collapseCell(Cell& cell, const WfcTileset& tileset, std::mt19937& rng);

    // Propagate constraints from a collapsed cell. Returns false on contradiction.
    static bool propagate(std::vector<Cell>& cells, int startIdx,
                          int width, int height, const WfcTileset& tileset);
};
```

### Tileset Loading (Free Functions)

```cpp
// Load and validate tileset from Lua. Returns empty tileset on failure.
// Logs warnings/errors via the provided callback.
using LogCallback = std::function<void(const std::string&)>;

WfcTileset loadWfcTileset(const std::string& filepath, LogCallback logger);

// Load WFC config parameters from Config.lua
WfcConfig loadWfcConfig(const std::string& filepath, LogCallback logger);
```

### Map Integration (Added Members)

```cpp
// In Map.h, add to private section:
std::vector<int> wfcTileIds;           // resolved tile index per cell
std::shared_ptr<WfcTileset> wfcTileset; // cached tileset for rendering

// In Map.h, add to private section (methods):
void initWfc(bool withActors);
void renderWfc() const;
void placeWfcActors();
```

---

## Data Models

### Grid Representation

The WFC grid is a flat `std::vector<int>` indexed as `x + y * width`. Each value is an index into the `WfcTileset::tiles` vector. During generation, cells hold a domain (set of possible tile indices). After resolution, each cell holds exactly one index.

### WFC Algorithm State (Internal)

During a single generation attempt:
- `cells`: `std::vector<Cell>` — one per grid position, each holding its remaining domain
- `rng`: `std::mt19937` — seeded from `rngSeed + attemptNumber` for deterministic restarts
- Generation loop: select → collapse → propagate → repeat until all cells resolved or contradiction

### Serialization Format (TCODZip)

```
[SAVE_VERSION_SENTINEL]  // existing 0x4F444F52
[levelType = 2]          // LevelType::WFC
[seed]                   // long, the seed used for generation
[tiles explored/scent]   // existing per-tile state
[currentScentValue]
[DIMS_SENTINEL]          // existing 0x44494D53
[width]
[height]
[WFC_SENTINEL]           // new: 0x57464347 ("WFCG")
[tileCount]              // int: number of tile types in tileset at save time
[wfcTileIds...]          // width*height ints: tile index per cell
```

On load, if the tileset's tile count doesn't match the stored `tileCount`, the level is regenerated from the stored seed rather than loading potentially corrupted tile indices.

### Lua Tileset Format (Scripts/WfcTiles.lua)

```lua
wfc_tiles = {
    {
        id = "corridor",
        glyph = string.byte("."),
        color = "lightGrey",
        walkable = true,
        transparent = true,
        weight = 3.0,
        adjacency = {
            north = {"corridor", "hab-unit", "wide-corridor", "market", "manufactorum", "chapel", "ventilation-duct"},
            south = {"corridor", "hab-unit", "wide-corridor", "market", "manufactorum", "chapel", "ventilation-duct"},
            east  = {"corridor", "hab-unit", "wide-corridor", "market", "manufactorum", "chapel", "ventilation-duct", "sump-pool"},
            west  = {"corridor", "hab-unit", "wide-corridor", "market", "manufactorum", "chapel", "ventilation-duct", "sump-pool"}
        }
    },
    -- ... more tiles
}
```

---

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system — essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Deterministic Generation

*For any* valid tileset and any seed value, running `WfcGenerator::generate` twice with the same seed and tileset SHALL produce identical grid output.

**Validates: Requirements 3.7**

### Property 2: Lowest-Entropy Cell Selection

*For any* grid state with multiple unresolved cells having different domain sizes, `selectLowestEntropy` SHALL return a cell whose domain size is minimal among all unresolved cells.

**Validates: Requirements 3.2**

### Property 3: Collapse Selects Only From Domain

*For any* cell with a non-empty domain, `collapseCell` SHALL return a tile index that is a member of that cell's domain.

**Validates: Requirements 3.3**

### Property 4: Propagation Maintains Adjacency Consistency

*For any* grid state after propagation completes without contradiction, every pair of adjacent resolved cells (i, j) in direction D SHALL satisfy: tile(i) allows tile(j) in direction D, AND tile(j) allows tile(i) in the opposite direction.

**Validates: Requirements 3.4**

### Property 5: Connectivity Check Correctness

*For any* grid where all walkable cells are four-directionally connected, `checkConnectivity` SHALL return true. *For any* grid with multiple disconnected walkable regions, `checkConnectivity` SHALL return false.

**Validates: Requirements 4.1, 4.2, 4.3, 4.4**

### Property 6: Minimum Walkable Area Invariant

*For any* grid accepted by `WfcGenerator::generate` (result.success == true), the count of walkable cells SHALL be at least `config.minWalkablePercent * width * height`.

**Validates: Requirements 4.5**

### Property 7: Tile Definition Parsing Preserves Fields

*For any* valid tile definition with all required fields (and optional weight), `loadWfcTileset` SHALL produce a WfcTile whose fields exactly match the input values, with weight defaulting to 1.0 when absent.

**Validates: Requirements 2.2, 2.6**

### Property 8: Symmetry Auto-Correction

*For any* tileset where tile A allows tile B to its east but tile B does NOT allow tile A to its west, after `loadWfcTileset` validation, tile B's west adjacency list SHALL contain tile A's ID (and vice versa for all directions).

**Validates: Requirements 2.4**

### Property 9: TCODMap Translation Correctness

*For any* resolved WFC grid and its corresponding tileset, after Map translation, every cell (x, y) SHALL have TCODMap walkability equal to `tileset.tiles[grid[x + y*width]].walkable` and transparency equal to `tileset.tiles[grid[x + y*width]].transparent`.

**Validates: Requirements 5.1**

### Property 10: Serialization Round-Trip

*For any* valid WFC level state (levelType, seed, tile IDs, explored flags), serializing then deserializing SHALL produce an equivalent state where levelType == WFC, all tile IDs match, and all explored/scent values are preserved.

**Validates: Requirements 1.3, 9.1, 9.2**

### Property 11: Monster Safe-Zone Exclusion

*For any* WFC level generated with actors, every spawned monster SHALL be positioned at Euclidean distance > 5 cells from the player's starting position.

**Validates: Requirements 8.4**

### Property 12: Fallback Seed Preservation

*For any* WFC generation that fails and falls back to BSP, the BSP generation SHALL use the same seed that was intended for WFC generation.

**Validates: Requirements 10.4**

---

## Error Handling

| Error Condition | Recovery | User Feedback |
|----------------|----------|---------------|
| `WfcTiles.lua` missing or syntax error | Fall back to BSP generation | Gui error message identifying load failure |
| Fewer than 5 valid tiles after validation | Fall back to BSP generation | Gui error message |
| Tile entry missing required field | Skip that tile, continue loading | Gui warning identifying tile and field |
| Asymmetric adjacency rule | Auto-correct by adding reciprocal | Gui warning identifying the pair |
| WFC contradiction (empty domain) | Restart with `seed + attemptCounter` | Silent (logged in debug mode) |
| All 10 restarts exhausted | Fall back to BSP generation | Gui warning: "WFC generation failed" |
| Connectivity check fails | Restart (counts toward 10 max) | Silent (logged in debug mode) |
| Walkable area < 20% | Restart (counts toward 10 max) | Silent (logged in debug mode) |
| Config.lua missing/invalid | Use compiled defaults | Silent |
| Config value out of range | Clamp to valid bounds | Gui warning |
| Tileset changed since save | Regenerate from stored seed | Gui warning |

**Fallback guarantee**: The `initWfc` method always produces a playable level. If WFC fails for any reason, BSP generation runs with the same seed. The Engine never sees an error — `Map::init` returns normally regardless of which generator succeeded.

---

## Testing Strategy

### Property-Based Tests (RapidCheck stub)

Property-based tests exercise the WfcGenerator's core algorithm in isolation. Each property test runs 100+ iterations with random inputs using the project's existing RapidCheck stub (`Tests/lib/rapidcheck.h`) and Catch2 v3.

**Test file**: `Tests/test_wfc_generator.cpp`

**Configuration**: Minimum 100 iterations per property (the stub's default).

**Tag format**: Each test is tagged with `[pbt][wfc]` and a comment referencing the design property.

Properties to implement:
1. Deterministic generation — Feature: wfc-hive-city, Property 1
2. Lowest-entropy selection — Feature: wfc-hive-city, Property 2
3. Collapse from domain only — Feature: wfc-hive-city, Property 3
4. Propagation adjacency consistency — Feature: wfc-hive-city, Property 4
5. Connectivity check correctness — Feature: wfc-hive-city, Property 5
6. Minimum walkable area — Feature: wfc-hive-city, Property 6
7. Tile parsing preserves fields — Feature: wfc-hive-city, Property 7
8. Symmetry auto-correction — Feature: wfc-hive-city, Property 8
9. TCODMap translation — Feature: wfc-hive-city, Property 9
10. Serialization round-trip — Feature: wfc-hive-city, Property 10
11. Monster safe-zone — Feature: wfc-hive-city, Property 11
12. Fallback seed preservation — Feature: wfc-hive-city, Property 12

**Generator strategy**: Tests use small grids (e.g., 8×8 or 16×16) with minimal tilesets (5-8 tiles) for fast execution. Tile IDs are generated as random short strings; adjacency rules are generated to be self-consistent (symmetric). Seeds are random ints.

### Unit Tests (Example-Based)

**Test file**: `Tests/test_wfc_generator.cpp` (same file, `[unit][wfc]` tags)

- Dispatch: `Map::init(WFC)` calls `initWfc`
- Render dispatch: `render()` calls `renderWfc()` for WFC levels
- Stair distance: stairs placed >= 20 apart
- Fallback: impossible tileset triggers BSP fallback with `levelType == BSP`
- Config clamping: out-of-range values clamped correctly
- Actor counts within configured [min, max] range

### Integration Tests

**Test file**: `Tests/test_wfc_integration.cpp` (tagged `[integration][wfc]`)

- Full generation with default `WfcTiles.lua`: produces playable level
- FOV computation works on WFC levels
- LevelCache stores and retrieves WFC snapshots correctly
- Default tileset loads without warnings

### Smoke Tests

**Test file**: `Tests/test_wfc_integration.cpp` (tagged `[smoke][wfc]`)

- `LevelType::WFC == 2`
- Default tileset contains all 10 required tile types
- Bulkhead-wall is non-walkable and non-transparent
- Corridor permits adjacency with all walkable types
