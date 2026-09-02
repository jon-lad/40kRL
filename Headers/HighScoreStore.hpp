#pragma once

#include "HighScore.hpp"

// Engine-layer file wrapper for the high-score leaderboard. Touches the
// filesystem only — it never accesses engine.gui / engine.map / engine.player,
// so it stays out of the pure core while confining all "graceful degradation"
// behavior (Requirement 7) to the I/O boundary.
class HighScoreStore {
public:
	// Persistence file, kept separate from the main save file game.sav (Req 6.1).
	static constexpr const char* FILE_NAME = "highscores.dat";

	// Loads the leaderboard from FILE_NAME. On a missing, empty, unreadable, or
	// corrupt file, returns an empty Leaderboard (Requirements 7.1-7.3). Never
	// throws.
	static Leaderboard load();

	// Serializes the leaderboard and writes it to FILE_NAME (Requirements 6.4).
	static void save(const Leaderboard& board);
};
