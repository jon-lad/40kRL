#include "HighScoreStore.hpp"

#include "libtcod.hpp"

#include <cstdio>
#include <filesystem>

// ─────────────────────────────────────────────────────────────────────────────
// High-score file store (task 6.1).
//
// Engine-layer wrapper that reads/writes highscores.dat. It only touches the
// filesystem — never engine.gui / engine.map / engine.player — so it can be
// exercised by tests that never initialize the Engine (per test-isolation.md).
//
// All "graceful degradation" for missing/empty/unreadable/corrupt files
// (Requirement 7) is handled here at the I/O boundary, keeping
// deserializeLeaderboard itself pure and total.
// ─────────────────────────────────────────────────────────────────────────────

Leaderboard HighScoreStore::load()
{
	namespace fs = std::filesystem;

	std::error_code ec;

	// Missing file → empty board (Req 7.1). Any filesystem error while probing
	// is treated conservatively as "no usable file".
	if (!fs::exists(FILE_NAME, ec) || ec) {
		return Leaderboard{};
	}

	// Empty (zero-length) file → empty board (Req 7.2).
	const auto fileSize = fs::file_size(FILE_NAME, ec);
	if (ec || fileSize == 0) {
		return Leaderboard{};
	}

	// Unreadable / corrupt file → empty board without throwing (Req 7.3).
	// TCODZip::loadFromFile can fail, and deserializeLeaderboard is total but we
	// still wrap the whole read in a try/catch as an outer safety net.
	try {
		TCODZip zip;
		zip.loadFromFile(FILE_NAME);
		return deserializeLeaderboard(zip);
	}
	catch (...) {
		return Leaderboard{};
	}
}

void HighScoreStore::save(const Leaderboard& board)
{
	// Serialize every field and write the archive to FILE_NAME (Req 6.4). A
	// first save after an empty init produces a valid, loadable file (Req 7.4).
	TCODZip zip;
	serializeLeaderboard(board, zip);
	zip.saveToFile(FILE_NAME);
}
