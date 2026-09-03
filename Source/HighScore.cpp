#include "HighScore.hpp"

#include "libtcod.hpp"

#include <algorithm>
#include <cstdint>

// ─────────────────────────────────────────────────────────────────────────────
// High-score pure core.
//
// This translation unit is engine-independent: it never touches
// engine.gui / engine.map / engine.player, so it is unit- and property-testable
// without initializing the Engine (per test-isolation.md, Requirements 2.5,
// 3.7, 6.7).
//
// Ranking  ...... task 2.3 (Requirements 2.1-2.5, 5.2)
// Leaderboard ... task 3.3 (Requirements 3.1-3.7)
// Serialization . task 4.4 (Requirements 6.7, 7.3, 8.1-8.3)
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Serialization format marker ("HSDR"). deserializeLeaderboard returns an empty
// board when the archive does not begin with this sentinel (Req 7.3, design).
constexpr int HIGHSCORE_SENTINEL = 0x48534452;

// Current on-disk format version.
constexpr int HIGHSCORE_FORMAT_VERSION = 1;

// A generous upper bound on the entry count read from a corrupt archive. Any
// count outside [0, cap] is treated as corruption and yields an empty board so
// deserialization stays total and never over-allocates (Req 7.3).
constexpr int MAX_REASONABLE_ENTRIES = 1000000;

// Three-way string comparison returning <0 / 0 / >0. std::string::compare
// already provides this; wrapped for clarity at the call sites below.
int compareString(const std::string& a, const std::string& b)
{
	const int c = a.compare(b);
	return (c < 0) ? -1 : (c > 0) ? 1 : 0;
}

} // namespace

// ─── Outcome formatting (Req 1.6, 1.7) ───────────────────────────────────────
//
// Pure formatting of the death outcome description from the cause-of-death
// actor name. When the cause is known (non-empty) the outcome reads
// "Slain by <cause>" (Req 1.6); when it is unavailable (empty) it is simply
// "Slain" (Req 1.7). Engine::recordRunOutcome calls this so the exact same
// rule is exercised by the engine-free test binary.
std::string formatOutcome(const std::string& cause)
{
	return cause.empty() ? std::string("Slain") : ("Slain by " + cause);
}

// ─── Score-name resolution (highscore-name-not-cadaver) ───────────────────────
//
// Chooses which name a run's ScoreEntry records. On death the player actor's
// name is replaced with the corpse name before the outcome is recorded, so the
// chosen name captured beforehand takes precedence. Pure — no engine access.
std::string resolveScoreName(const std::string& chosenName,
                             const std::string& liveActorName)
{
	return chosenName.empty() ? liveActorName : chosenName;
}

// ─── Ranking (task 2.3) ──────────────────────────────────────────────────────
//
// Returns positive if a ranks strictly above b, negative if a ranks below b,
// and 0 only when the two entries are equal across every field.
//
// Order of comparison (Req 2.1-2.4):
//   1. totalXp        (primary, greater ranks higher)
//   2. deepestLevel   (tiebreak, greater ranks higher)
//   3. characterName  ┐
//   4. date           │ fixed lexicographic fallback so that entries with
//   5. outcome        │ equal ranking keys still have a deterministic, total,
//   6. careerName     │ antisymmetric order (Req 2.4). Smaller string ranks
//   7. homeworldName  │ higher (arbitrary but consistent) so the order is
//   8. rankTitle      ┘ stable and repeatable.
int scoreCompare(const ScoreEntry& a, const ScoreEntry& b)
{
	// 1. Primary key: total experience earned. Greater XP ranks higher (Req 2.2).
	if (a.totalXp != b.totalXp) {
		return (a.totalXp > b.totalXp) ? 1 : -1;
	}

	// 2. Tiebreak: deepest dungeon level reached. Greater depth ranks higher
	//    (Req 2.3).
	if (a.deepestLevel != b.deepestLevel) {
		return (a.deepestLevel > b.deepestLevel) ? 1 : -1;
	}

	// 3-8. Fixed lexicographic fallback for a deterministic total order (Req 2.4).
	//      A smaller string value ranks higher, consistently. Because every
	//      field is eventually compared, two entries compare equal (return 0)
	//      only when they are identical across all fields.
	if (int c = compareString(a.characterName, b.characterName)) return -c;
	if (int c = compareString(a.date, b.date))                   return -c;
	if (int c = compareString(a.outcome, b.outcome))             return -c;
	if (int c = compareString(a.careerName, b.careerName))       return -c;
	if (int c = compareString(a.homeworldName, b.homeworldName)) return -c;
	if (int c = compareString(a.rankTitle, b.rankTitle))         return -c;

	// All fields equal — the entries rank equally.
	return 0;
}

bool scoreRanksHigher(const ScoreEntry& a, const ScoreEntry& b)
{
	return scoreCompare(a, b) > 0;
}

// ─── Leaderboard (task 3.3) ──────────────────────────────────────────────────

Leaderboard::Leaderboard(int capacity)
	// Clamp capacity to >= 1 so the board can always hold at least one entry
	// (design: "clamp to >=1 if needed"). A non-positive capacity would make
	// insertion meaningless.
	: capacity_(capacity < 1 ? 1 : capacity)
{
}

bool Leaderboard::insert(const ScoreEntry& entry)
{
	// Find the first position whose entry ranks strictly below `entry`. Because
	// entries_ is maintained sorted descending, inserting there keeps the order
	// (Req 3.1). Using scoreRanksHigher(entry, existing) preserves stable
	// insertion (equal-ranked existing entries stay ahead of the newcomer).
	auto pos = std::find_if(entries_.begin(), entries_.end(),
		[&entry](const ScoreEntry& existing) {
			return scoreRanksHigher(entry, existing);
		});

	const auto inserted = entries_.insert(pos, entry);

	// Remember whether the inserted element is the one we would truncate away.
	// Its index is stable until we potentially erase the (now) last element.
	const auto insertedIndex = inserted - entries_.begin();

	// Truncate to capacity by erasing the lowest-ranked (last) entry (Req 3.3).
	bool survived = true;
	if (static_cast<int>(entries_.size()) > capacity_) {
		// The overflow element is always the last one after a sorted insert.
		const auto lastIndex = static_cast<decltype(insertedIndex)>(entries_.size()) - 1;
		if (insertedIndex == lastIndex) {
			// The entry we just inserted is the one being evicted: it ranked
			// below every retained entry on a full board (Req 3.4).
			survived = false;
		}
		entries_.pop_back();
	}

	// Returns true iff the inserted entry earned and kept a place (Req 3.5).
	return survived;
}

// ─── Serialization (task 4.4) ────────────────────────────────────────────────
//
// IMPORTANT — robustness against corrupt input (Req 7.3):
//
// libtcod's TCODZip::getString() is NOT bounds-checked. It reads a length int
// from the stream and then returns a raw pointer into its internal buffer at
// the current offset, advancing by that length. Feeding it a corrupt archive
// (e.g. a valid sentinel followed by a bogus entry count and a short/garbled
// body) makes it read a garbage length and hand back a pointer that, when
// copied into a std::string, walks off the end of the buffer → SIGSEGV.
// Checking getRemainingBytes() beforehand does NOT help, because the *length
// prefix itself* is the untrusted value.
//
// Therefore this module does its own string framing that never calls
// getString(): each string is written as [int byteLength][byteLength chars].
// On read we take the length via the (bounds-checkable) getInt(), validate it
// against getRemainingBytes(), and only then read exactly that many chars with
// getChar(). A garbage length can never cause an out-of-bounds read.

namespace {

// Writes a string as an explicit byte-length prefix followed by its raw bytes.
void putFramedString(TCODZip& zip, const std::string& s)
{
	zip.putInt(static_cast<int>(s.size()));
	for (char c : s) {
		zip.putChar(c);
	}
}

bool canReadInt(const TCODZip& zip)
{
	return zip.getRemainingBytes() >= sizeof(int);
}

// Reads a framed string safely. Returns false (leaving `out` untouched) if the
// archive is exhausted or claims a length that cannot be satisfied by the
// bytes remaining — the guard that prevents the SIGSEGV described above.
bool tryReadFramedString(TCODZip& zip, std::string& out)
{
	if (!canReadInt(zip)) {
		return false;
	}
	const int len = zip.getInt();
	if (len < 0) {
		return false;
	}
	// The claimed length must fit within the bytes still available. This is the
	// critical bound: it rejects corrupt/oversized lengths before any char read.
	if (static_cast<uint32_t>(len) > zip.getRemainingBytes()) {
		return false;
	}
	std::string result;
	result.reserve(static_cast<size_t>(len));
	for (int i = 0; i < len; ++i) {
		result.push_back(zip.getChar());
	}
	out = std::move(result);
	return true;
}

} // namespace

void serializeLeaderboard(const Leaderboard& board, TCODZip& zip)
{
	// Header: sentinel, format version, entry count (Req 8, design format).
	zip.putInt(HIGHSCORE_SENTINEL);
	zip.putInt(HIGHSCORE_FORMAT_VERSION);
	zip.putInt(board.size());

	// Write all eight fields per entry so nothing is lost across a round trip
	// (Req 8.2). Descending order is preserved because entries() is already
	// sorted descending (Req 8.3).
	for (const ScoreEntry& e : board.entries()) {
		putFramedString(zip, e.characterName);
		putFramedString(zip, e.careerName);
		putFramedString(zip, e.homeworldName);
		putFramedString(zip, e.rankTitle);
		zip.putInt(e.totalXp);
		zip.putInt(e.deepestLevel);
		putFramedString(zip, e.outcome);
		putFramedString(zip, e.date);
	}
}

Leaderboard deserializeLeaderboard(TCODZip& zip)
{
	// Read the sentinel first. On any mismatch (empty/foreign/corrupt archive),
	// return an empty board (Req 7.3). Guard the read itself: an empty archive
	// has no bytes to read.
	if (!canReadInt(zip)) {
		return Leaderboard{};
	}
	const int sentinel = zip.getInt();
	if (sentinel != HIGHSCORE_SENTINEL) {
		return Leaderboard{};
	}

	// Format version (currently unused beyond presence; read to stay in sync).
	if (!canReadInt(zip)) {
		return Leaderboard{};
	}
	(void)zip.getInt();

	// Entry count. Guard against corrupt/absurd counts so we never over-allocate
	// or loop on garbage (Req 7.3).
	if (!canReadInt(zip)) {
		return Leaderboard{};
	}
	const int count = zip.getInt();
	if (count < 0 || count > MAX_REASONABLE_ENTRIES) {
		return Leaderboard{};
	}

	// Rebuild entries. The archive stores them in descending order, but we
	// reconstruct via insert() so the board's invariants are re-established even
	// if the on-disk order were perturbed (Req 8.1, 8.3).
	Leaderboard board(Leaderboard::DEFAULT_CAPACITY);
	std::vector<ScoreEntry> loaded;
	loaded.reserve(static_cast<size_t>(count));

	for (int i = 0; i < count; ++i) {
		ScoreEntry e;

		// Every field is guarded. On any exhaustion mid-entry (truncated or
		// corrupt body claiming more entries than it contains), bail out to an
		// empty board so deserialization stays total and never reads OOB.
		if (!tryReadFramedString(zip, e.characterName)) return Leaderboard{};
		if (!tryReadFramedString(zip, e.careerName))    return Leaderboard{};
		if (!tryReadFramedString(zip, e.homeworldName)) return Leaderboard{};
		if (!tryReadFramedString(zip, e.rankTitle))     return Leaderboard{};

		if (!canReadInt(zip)) return Leaderboard{};
		e.totalXp = zip.getInt();
		if (!canReadInt(zip)) return Leaderboard{};
		e.deepestLevel = zip.getInt();

		if (!tryReadFramedString(zip, e.outcome)) return Leaderboard{};
		if (!tryReadFramedString(zip, e.date))    return Leaderboard{};

		loaded.push_back(std::move(e));
	}

	for (const ScoreEntry& e : loaded) {
		board.insert(e);
	}

	return board;
}

// ─── Death-screen view-decision seam (death-screen-highscore-jump) ────────────
//
// Pure, engine-free (no engine.gui / engine.map / engine.player). These are the
// single testable seam for the two-phase death screen: they decide which page
// the leaderboard opens on and whether the earned entry is highlighted.

DeathScoreView computeDeathScoreView(std::optional<int> lastEntryIndex,
                                     int totalItems, int pageSize)
{
	// Clamp pageSize to >= 1 so v / effectivePageSize can never divide by zero
	// (mirrors the std::max(1, ...) used when the paginator is built).
	const int effectivePageSize = std::max(1, pageSize);

	// A ranked run has an earned index within [0, totalItems). An unset or
	// out-of-range index is treated as unranked: first page, no highlight.
	if (lastEntryIndex.has_value()) {
		const int v = *lastEntryIndex;
		if (v >= 0 && v < totalItems) {
			return DeathScoreView{ v / effectivePageSize, true };
		}
	}

	return DeathScoreView{ 0, false };
}

int placementNumber(int entryIndex)
{
	// 1-based rank; matches the i + 1 rank renderLeaderboard prints.
	return entryIndex + 1;
}
