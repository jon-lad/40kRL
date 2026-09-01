#include "lib/catch_amalgamated.hpp"

#include "HighScore.hpp"
#include "HighScoreStore.hpp"

#include "libtcod.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

// ═══════════════════════════════════════════════════════════════════════════════
// Feature: high-score-system — Integration tests for HighScoreStore (task 6.2)
//
// HighScoreStore reads/writes the fixed file HighScoreStore::FILE_NAME
// ("highscores.dat"). It only touches the filesystem — never
// engine.gui/map/player — so these tests need no Engine initialization
// (per test-isolation.md).
//
// FILE_NAME is a compile-time constant and is NOT parameterizable, so to test
// load()/save() directly without clobbering a real leaderboard on disk, each
// test:
//   1. backs up any pre-existing highscores.dat to a temp path,
//   2. removes it so the store starts from a known clean state,
//   3. runs its assertions,
//   4. cleans up the file the test created and restores the original.
//
// This mirrors the temp-file back-up/clean-up discipline used elsewhere in the
// suite (e.g. the _level_cache_temp.sav / __test_*.sav patterns).
//
// **Validates: Requirements 6.1, 6.3, 7.1, 7.3, 7.4, 8.1**
// ═══════════════════════════════════════════════════════════════════════════════

namespace {

namespace fs = std::filesystem;

// RAII guard: preserves any real highscores.dat that exists before a test runs
// and restores it afterwards, so tests never destroy a player's real board.
// On construction it moves an existing highscores.dat aside; on destruction it
// removes whatever the test left behind and restores the backup.
class HighScoreFileGuard {
public:
    HighScoreFileGuard()
        : liveFile_(HighScoreStore::FILE_NAME)
        , backupFile_(std::string(HighScoreStore::FILE_NAME) + ".test_backup")
    {
        std::error_code ec;
        if (fs::exists(liveFile_, ec) && !ec) {
            fs::rename(liveFile_, backupFile_, ec);
            hadBackup_ = !ec;
        }
        // Ensure the store starts each test from a clean, absent-file state.
        fs::remove(liveFile_, ec);
    }

    ~HighScoreFileGuard()
    {
        std::error_code ec;
        // Remove any file the test produced.
        fs::remove(liveFile_, ec);
        // Restore the player's real file if we moved one aside.
        if (hadBackup_) {
            fs::rename(backupFile_, liveFile_, ec);
        }
    }

    HighScoreFileGuard(const HighScoreFileGuard&) = delete;
    HighScoreFileGuard& operator=(const HighScoreFileGuard&) = delete;

private:
    std::string liveFile_;
    std::string backupFile_;
    bool        hadBackup_ = false;
};

// Build a ScoreEntry with fully-populated, distinguishable fields so that a
// round trip through disk can be verified field-by-field.
ScoreEntry makeEntry(const std::string& name, int xp, int depth)
{
    ScoreEntry e;
    e.characterName = name;
    e.careerName    = "Rogue Trader";
    e.homeworldName = "Void Born";
    e.rankTitle     = "Seneschal";
    e.totalXp       = xp;
    e.deepestLevel  = depth;
    e.outcome       = "Slain by Ork Nob";
    e.date          = "2024-07-01 13:37";
    return e;
}

// Structural equality across every serialized field (Req 8.2).
bool entriesFieldEqual(const ScoreEntry& a, const ScoreEntry& b)
{
    return a.characterName == b.characterName
        && a.careerName    == b.careerName
        && a.homeworldName == b.homeworldName
        && a.rankTitle     == b.rankTitle
        && a.totalXp       == b.totalXp
        && a.deepestLevel  == b.deepestLevel
        && a.outcome       == b.outcome
        && a.date          == b.date;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Save-then-load equivalence on disk (Req 6.1, 8.1)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("HighScoreStore: save then load reproduces the board on disk", "[high-score-system]")
{
    HighScoreFileGuard guard;

    Leaderboard original(10);
    original.insert(makeEntry("Aaron", 100, 3));
    original.insert(makeEntry("Bella", 500, 1));
    original.insert(makeEntry("Cyrus", 300, 7));

    HighScoreStore::save(original);

    // The store writes to its fixed FILE_NAME.
    REQUIRE(fs::exists(HighScoreStore::FILE_NAME));

    const Leaderboard loaded = HighScoreStore::load();

    // Same number of entries survived the disk round trip (Req 8.1).
    REQUIRE(loaded.size() == original.size());

    // Every field of every entry survived intact, in the same descending order
    // the board maintained (Req 8.1, 8.3).
    const auto& orig = original.entries();
    const auto& got = loaded.entries();
    REQUIRE(orig.size() == got.size());
    for (size_t i = 0; i < orig.size(); ++i) {
        REQUIRE(entriesFieldEqual(got[i], orig[i]));
    }

    // Highest XP first (Bella 500), then depth-driven order for the rest.
    REQUIRE(got[0].characterName == "Bella");
}

// ─────────────────────────────────────────────────────────────────────────────
// Missing file → empty board (Req 7.1)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("HighScoreStore: missing file loads an empty board", "[high-score-system]")
{
    HighScoreFileGuard guard;

    // Guard already removed any existing file; confirm it is absent.
    REQUIRE_FALSE(fs::exists(HighScoreStore::FILE_NAME));

    const Leaderboard loaded = HighScoreStore::load();

    REQUIRE(loaded.empty());
    REQUIRE(loaded.size() == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Empty (zero-length) file → empty board (Req 7.2)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("HighScoreStore: empty file loads an empty board", "[high-score-system]")
{
    HighScoreFileGuard guard;

    // Create a zero-length file at the store's fixed path.
    {
        std::ofstream ofs(HighScoreStore::FILE_NAME, std::ios::binary | std::ios::trunc);
    }
    REQUIRE(fs::exists(HighScoreStore::FILE_NAME));
    REQUIRE(fs::file_size(HighScoreStore::FILE_NAME) == 0);

    const Leaderboard loaded = HighScoreStore::load();

    REQUIRE(loaded.empty());
    REQUIRE(loaded.size() == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Truncated / garbage file → empty board without throwing (Req 7.3)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("HighScoreStore: truncated or garbage file loads an empty board", "[high-score-system]")
{
    HighScoreFileGuard guard;

    // Write arbitrary non-archive bytes to the store's fixed path.
    {
        std::ofstream ofs(HighScoreStore::FILE_NAME, std::ios::binary | std::ios::trunc);
        const char junk[] = "this is not a valid TCODZip highscore archive \x00\x01\x02\xFF";
        ofs.write(junk, sizeof(junk));
    }
    REQUIRE(fs::exists(HighScoreStore::FILE_NAME));
    REQUIRE(fs::file_size(HighScoreStore::FILE_NAME) > 0);

    // load() must never throw and must fall back to an empty board (Req 7.3).
    Leaderboard loaded(1);
    REQUIRE_NOTHROW(loaded = HighScoreStore::load());

    REQUIRE(loaded.empty());
    REQUIRE(loaded.size() == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// First save after empty init produces a loadable file (Req 7.4)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("HighScoreStore: first save after empty init writes a loadable file", "[high-score-system]")
{
    HighScoreFileGuard guard;

    // Start from a missing file, so the in-memory board is the empty default
    // (as after graceful degradation on a first run).
    REQUIRE_FALSE(fs::exists(HighScoreStore::FILE_NAME));

    Leaderboard board = HighScoreStore::load();
    REQUIRE(board.empty());

    // Insert the first entry and persist it (Req 7.4).
    const bool earned = board.insert(makeEntry("FirstBlood", 250, 5));
    REQUIRE(earned);
    HighScoreStore::save(board);

    REQUIRE(fs::exists(HighScoreStore::FILE_NAME));

    // The freshly written file must be loadable and reproduce the entry.
    const Leaderboard reloaded = HighScoreStore::load();
    REQUIRE(reloaded.size() == 1);
    REQUIRE(entriesFieldEqual(reloaded.entries()[0], makeEntry("FirstBlood", 250, 5)));
}

// ─────────────────────────────────────────────────────────────────────────────
// Independence from game.sav: removing game.sav leaves highscores.dat intact
// and loadable (Req 6.3)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("HighScoreStore: removing game.sav leaves highscores.dat intact and loadable", "[high-score-system]")
{
    HighScoreFileGuard guard;

    // Persist a leaderboard to the high-score file.
    Leaderboard board(10);
    board.insert(makeEntry("Survivor", 777, 9));
    HighScoreStore::save(board);
    REQUIRE(fs::exists(HighScoreStore::FILE_NAME));

    // Simulate the "delete save" flow that removes game.sav. We stand up a
    // stand-in game.sav (using a temp-scoped path) to prove that deleting it
    // does not disturb the high-score file. Back up any real game.sav first.
    const char* saveFile = "game.sav";
    const std::string saveBackup = std::string(saveFile) + ".test_backup";
    std::error_code ec;
    bool hadRealSave = false;
    if (fs::exists(saveFile, ec) && !ec) {
        fs::rename(saveFile, saveBackup, ec);
        hadRealSave = !ec;
    }

    // Create a placeholder game.sav, then remove it (mirrors Engine::save's
    // std::remove("game.sav") on player death).
    {
        std::ofstream ofs(saveFile, std::ios::binary | std::ios::trunc);
        ofs << "placeholder save data";
    }
    REQUIRE(fs::exists(saveFile));
    std::remove(saveFile);
    REQUIRE_FALSE(fs::exists(saveFile));

    // The high-score file is untouched and still loadable (Req 6.3).
    REQUIRE(fs::exists(HighScoreStore::FILE_NAME));
    const Leaderboard reloaded = HighScoreStore::load();
    REQUIRE(reloaded.size() == 1);
    REQUIRE(entriesFieldEqual(reloaded.entries()[0], makeEntry("Survivor", 777, 9)));

    // Restore any real game.sav we moved aside.
    if (hadRealSave) {
        fs::rename(saveBackup, saveFile, ec);
    } else {
        // Ensure we do not leave a stray placeholder / backup behind.
        fs::remove(saveBackup, ec);
    }
}
