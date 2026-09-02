#pragma once

#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Pure, engine-free character-name sanitization seam (character-name-entry).
//
// This header declares the testable seam used by character generation to
// normalize a raw, player-typed name into a valid one. It has NO dependency on
// the Engine, SDL, GUI, or libtcod, so it can be exercised directly by the
// engine-free test binary (see .kiro/steering/test-isolation.md).
// ─────────────────────────────────────────────────────────────────────────────

// Maximum number of characters a sanitized character name may contain.
constexpr int MAX_NAME_LENGTH = 24;

// Default name returned when a raw input reduces to empty after sanitization.
inline constexpr const char* DEFAULT_CHARACTER_NAME = "Rogue Trader";

// Normalize a raw, player-typed name into a valid character name by applying,
// in order:
//   1. Strip non-printable characters (keep bytes where std::isprint is true).
//   2. Truncate to MAX_NAME_LENGTH (24) characters.
//   3. Trim leading/trailing whitespace.
//   4. If the result is empty, return DEFAULT_CHARACTER_NAME ("Rogue Trader").
//
// The returned string is always non-empty, no longer than MAX_NAME_LENGTH, and
// composed of printable characters only. Already-valid names are returned
// unchanged (identity).
std::string sanitizeCharacterName(const std::string& input);
