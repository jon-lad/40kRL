#include "HighScore.hpp"

#include "libtcod.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Stub definitions for the high-score pure core.
//
// These stubs exist so that both 40kRL.vcxproj and Tests/40kRL_Tests.vcxproj
// link successfully once tests reference these symbols. The real behavior is
// implemented in later tasks (ranking: task 2.3, insertion: task 3.3,
// serialization: task 4.4).
// ─────────────────────────────────────────────────────────────────────────────

// ─── Ranking (task 2.3) ──────────────────────────────────────────────────────

int scoreCompare(const ScoreEntry& /*a*/, const ScoreEntry& /*b*/)
{
	return 0;
}

bool scoreRanksHigher(const ScoreEntry& a, const ScoreEntry& b)
{
	return scoreCompare(a, b) > 0;
}

// ─── Leaderboard (task 3.3) ──────────────────────────────────────────────────

Leaderboard::Leaderboard(int capacity)
	: capacity_(capacity)
{
}

bool Leaderboard::insert(const ScoreEntry& /*entry*/)
{
	return false;
}

// ─── Serialization (task 4.4) ────────────────────────────────────────────────

void serializeLeaderboard(const Leaderboard& /*board*/, TCODZip& /*zip*/)
{
}

Leaderboard deserializeLeaderboard(TCODZip& /*zip*/)
{
	return Leaderboard{};
}
