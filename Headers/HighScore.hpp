#pragma once

#include <optional>
#include <string>
#include <vector>

// Forward declaration: serialization uses TCODZip but the pure core never
// touches engine.gui / engine.map / engine.player.
class TCODZip;

// A single recorded run outcome. All fields are plain data (Requirement 1, 5.1).
struct ScoreEntry {
	std::string characterName;    // player->name
	std::string careerName;       // CareerProgression::careerName
	std::string homeworldName;    // CareerProgression::homeworldName
	std::string rankTitle;        // resolved from CareerTemplate ranks (final rank)
	int         totalXp = 0;      // CareerProgression::xpPool  (>= 0)   (Req 1.2)
	int         deepestLevel = 1; // max Engine::dungeonLevel reached (>= 1) (Req 1.3)
	std::string outcome;          // "Slain by <name>" or "Slain" (Req 1.5-1.7, 5.1)
	std::string date;             // fixed human-readable format (Req 1.4)
};

// Outcome formatting (Requirements 1.6, 1.7). Pure function — no engine access.
// Formats the death outcome description from the cause-of-death actor name:
//   - non-empty cause  -> "Slain by " + cause   (Req 1.6)
//   - empty cause      -> "Slain"                (Req 1.7)
std::string formatOutcome(const std::string& cause);

// Ranking (Requirement 2). Pure functions — no engine access (Req 2.5).
// Returns negative if a ranks below b, 0 if equal rank, positive if a ranks above b.
int scoreCompare(const ScoreEntry& a, const ScoreEntry& b);

// Strict-weak "a ranks strictly above b" ordering used for descending sort.
bool scoreRanksHigher(const ScoreEntry& a, const ScoreEntry& b);

// The leaderboard: ordered, capacity-bounded collection (Requirement 3).
class Leaderboard {
public:
	static constexpr int DEFAULT_CAPACITY = 100; // Req 3.6

	explicit Leaderboard(int capacity = DEFAULT_CAPACITY);

	// Inserts entry at the correct sorted (descending) position, then truncates
	// to capacity. Returns true iff the entry earned and retained a place. (Req 3.1-3.5)
	bool insert(const ScoreEntry& entry);

	const std::vector<ScoreEntry>& entries() const { return entries_; }
	int  size()     const { return static_cast<int>(entries_.size()); }
	int  capacity() const { return capacity_; }
	bool empty()    const { return entries_.empty(); }

private:
	std::vector<ScoreEntry> entries_; // invariant: sorted descending, size <= capacity_
	int capacity_;
};

// Serialization (Requirement 6.7, 8). Pure — operates on a TCODZip archive only.
void serializeLeaderboard(const Leaderboard& board, TCODZip& zip);
Leaderboard deserializeLeaderboard(TCODZip& zip);

// ─── Death-screen view-decision seam (death-screen-highscore-jump) ────────────
//
// Pure, engine-free helpers describing how the high-score view should open on
// the death screen. No engine.* access (per test-isolation.md), so they are
// unit- and property-testable without initializing the Engine.

// Pure result describing how the high-score view should open on the death screen.
struct DeathScoreView {
	int  initialPage = 0;     // page the paginator should show first (>= 0)
	bool highlight   = false; // whether the earned entry should be highlighted
};

// Pure decision function — decides the paginator's initial page and whether to
// highlight the earned entry.
//   lastEntryIndex : the run's earned entry index, or nullopt if unranked
//   totalItems     : number of leaderboard entries
//   pageSize       : entries per page (clamped to >= 1 internally)
// Postconditions:
//   - if lastEntryIndex has value v in [0, totalItems): initialPage = v / pageSize, highlight = true
//   - otherwise: initialPage = 0, highlight = false
DeathScoreView computeDeathScoreView(std::optional<int> lastEntryIndex,
                                     int totalItems, int pageSize);

// Pure placement number: 1-based rank of the earned entry (entry index + 1).
int placementNumber(int entryIndex);
