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
