#include "lib/catch_amalgamated.hpp"
#include "lib/rapidcheck_catch.h"
#include "main.hpp"

#include <filesystem>
#include <string>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: region-based-npc-spawns — Property-Based Tests
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Property 4: Region persistence round-trip ───────────────────────────────
// Feature: region-based-npc-spawns, Property 4: Region persistence round-trip
// **Validates: Requirements 8.1, 8.2, 8.3**
//
// For any Region_Name string assigned to a level, persisting that level and then
// restoring it yields a string-equal Region_Name. The generated names include
// empty and non-ASCII strings to exercise the malformed/empty-string branch.
//
// NOTE (TDD): this test is expected to FAIL to compile/link until tasks 5.3 and
// 5.4 add `Map::getRegionName()` / `Map::setRegionName()` state and region
// persistence to `Map::save` / `Map::load`.

TEST_CASE("PBT: Property 4 — region persistence round-trip", "[pbt][property][region-based-npc-spawns]")
{
    rc::prop("assigned region name survives a Map save/load round-trip byte-for-byte", []() {
        // A small fixed map keeps generation fast; the region field is independent
        // of map dimensions and content.
        const int mapWidth  = 80;
        const int mapHeight = 43;

        // Generate an arbitrary Region_Name, including empty (length 0) and
        // non-ASCII bytes. rc::gen::string produces char-based strings that can
        // contain the full byte range, covering the malformed/non-ASCII branch.
        const std::string regionName = *rc::gen::string(0, 40);

        // Build and initialize the source map (without actors for speed), then
        // assign the region we want to persist.
        Map originalMap(mapWidth, mapHeight);
        originalMap.init(false, LevelType::BSP);
        originalMap.setRegionName(regionName);

        // Save to a temp file via TCODZip. Dimensions are written first, matching
        // the serializeCurrentLevel convention used elsewhere in the codebase.
        const char* tempFile = "__test_region_persistence_roundtrip.sav";
        {
            TCODZip zip;
            zip.putInt(mapWidth);
            zip.putInt(mapHeight);
            originalMap.save(zip);
            zip.saveToFile(tempFile);
        }

        // Load into a fresh Map.
        Map loadedMap(mapWidth, mapHeight);
        {
            TCODZip zip;
            zip.loadFromFile(tempFile);
            int w = zip.getInt();
            int h = zip.getInt();
            RC_ASSERT(w == mapWidth);
            RC_ASSERT(h == mapHeight);
            loadedMap.load(zip);
        }

        // Clean up the temp file.
        std::filesystem::remove(tempFile);

        // Round-trip property: a non-empty region name must survive byte-for-byte.
        // An empty region name resolves to the Default_Region on load (Req 8.5),
        // so for the empty case we only assert the restored name is non-empty and
        // deterministic — the byte-equality guarantee applies to non-empty names.
        if (!regionName.empty()) {
            RC_ASSERT(loadedMap.getRegionName() == regionName);
        } else {
            // Empty stored region -> Default_Region; never empty after load.
            RC_ASSERT(!loadedMap.getRegionName().empty());
        }
    });
}

// ─── Backward-compatible restore (EDGE_CASE) ─────────────────────────────────
// Feature: region-based-npc-spawns — Task 5.2
// **Validates: Requirements 8.4, 8.5**
//
// Two backward-compatibility edge cases for Map::load:
//   1. A pre-region snapshot that predates region support and therefore carries
//      NO trailing REGION_SENTINEL. On load, the level must resolve to the
//      Default_Region and the load must complete without aborting (Req 8.4).
//   2. A snapshot that DOES carry a REGION_SENTINEL but whose region string is
//      empty or garbled. On load, the level must resolve to the Default_Region
//      and the load must complete without aborting (Req 8.5).
//
// The Default_Region is whatever resolveDefaultRegion() yields (config value,
// else the compiled "Ork" fallback). Asserting against resolveDefaultRegion()
// keeps the test self-consistent with the load path the design specifies.
//
// Engine isolation (per test-isolation steering): these tests build Maps and
// TCODZip archives directly and read Scripts/Config.lua via resolveDefaultRegion();
// no global Engine (gui/map/player) is initialized or touched.
//
// NOTE (TDD): expected to FAIL to build/link until Task 5.4 makes Map::load read
// the trailing REGION_SENTINEL (and Task 2.2 provides resolveDefaultRegion()).

TEST_CASE("Map::load: pre-region snapshot without a region sentinel resolves to Default_Region",
          "[region-based-npc-spawns][region-persistence][edge-case]")
{
    const int mapWidth  = 80;
    const int mapHeight = 43;

    // Build a pre-region snapshot by saving a Map through the CURRENT Map::save
    // stream. A pre-region save is exactly one that ends after the existing
    // sections with no trailing REGION_SENTINEL — which is what the stream below
    // reproduces byte-for-byte (dimensions header + Map::save payload). This is
    // the most robust way to emulate an old save without hand-encoding the layout.
    const char* tempFile = "__test_region_backcompat_no_sentinel.sav";
    {
        Map sourceMap(mapWidth, mapHeight);
        sourceMap.init(false, LevelType::BSP);
        // Deliberately assign a non-default region on the source so that if the
        // region somehow leaked through, it would differ from the Default_Region
        // and fail the assertion. The current (pre-region) save writes no region,
        // so this value must NOT survive.
        sourceMap.setRegionName("SentinelLeakCanary");

        TCODZip zip;
        zip.putInt(mapWidth);
        zip.putInt(mapHeight);
        sourceMap.save(zip); // pre-region save: no REGION_SENTINEL appended
        zip.saveToFile(tempFile);
    }

    // Load into a fresh Map. The load must not abort and must assign the default.
    Map loadedMap(mapWidth, mapHeight);
    {
        TCODZip zip;
        zip.loadFromFile(tempFile);
        int w = zip.getInt();
        int h = zip.getInt();
        REQUIRE(w == mapWidth);
        REQUIRE(h == mapHeight);
        loadedMap.load(zip); // must complete without aborting (Req 8.4)
    }

    std::filesystem::remove(tempFile);

    // A pre-region snapshot resolves to the Default_Region.
    REQUIRE(loadedMap.getRegionName() == resolveDefaultRegion());
    REQUIRE_FALSE(loadedMap.getRegionName().empty());
}

TEST_CASE("Map::load: snapshot with an empty or garbled region resolves to Default_Region",
          "[region-based-npc-spawns][region-persistence][edge-case]")
{
    const int mapWidth  = 80;
    const int mapHeight = 43;

    // Helper: write a valid Map::save payload, then append a caller-controlled
    // trailing region field so we can exercise the empty/garbled branches.
    // Returns the file path written.
    auto writeSnapshot = [&](const char* path, bool appendSentinel,
                             int sentinelValue, const char* regionString) {
        Map sourceMap(mapWidth, mapHeight);
        sourceMap.init(false, LevelType::BSP);

        TCODZip zip;
        zip.putInt(mapWidth);
        zip.putInt(mapHeight);
        sourceMap.save(zip); // existing sections (no region yet)

        if (appendSentinel) {
            // Mimic the region field framing the design appends at end-of-stream.
            zip.putInt(sentinelValue);
            zip.putString(regionString);
        }
        zip.saveToFile(path);
    };

    auto loadRegion = [&](const char* path) {
        Map loadedMap(mapWidth, mapHeight);
        TCODZip zip;
        zip.loadFromFile(path);
        int w = zip.getInt();
        int h = zip.getInt();
        REQUIRE(w == mapWidth);
        REQUIRE(h == mapHeight);
        loadedMap.load(zip); // must complete without aborting (Req 8.5)
        return loadedMap.getRegionName();
    };

    SECTION("valid region sentinel followed by an EMPTY region string -> Default_Region")
    {
        const char* tempFile = "__test_region_backcompat_empty.sav";
        // Correct sentinel, but an empty region string. Per Req 8.5 the empty
        // stored region is treated as malformed and resolves to Default_Region.
        writeSnapshot(tempFile, /*appendSentinel*/ true, REGION_SENTINEL, "");

        const std::string region = loadRegion(tempFile);
        std::filesystem::remove(tempFile);

        REQUIRE(region == resolveDefaultRegion());
        REQUIRE_FALSE(region.empty());
    }

    SECTION("garbled trailing int (not the sentinel) -> treated as pre-region, Default_Region")
    {
        const char* tempFile = "__test_region_backcompat_garbled.sav";
        // A garbled trailing int that is NOT the REGION_SENTINEL must be handled
        // as a pre-region stream: the load resolves to Default_Region and does
        // not abort. 0xDEADBEEF is deliberately not equal to REGION_SENTINEL.
        writeSnapshot(tempFile, /*appendSentinel*/ true,
                      /*sentinelValue*/ static_cast<int>(0xDEADBEEF),
                      /*regionString*/ "GarbledRegionPayload");

        const std::string region = loadRegion(tempFile);
        std::filesystem::remove(tempFile);

        REQUIRE(region == resolveDefaultRegion());
        REQUIRE_FALSE(region.empty());
    }
}
