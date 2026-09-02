# Bugfix Requirements Document

## Introduction

During character generation the player is never given an opportunity to name their
character. The player `Actor` is created with the hardcoded literal name `"Player"`
in `Engine::init()` and no step of the character-generation state machine
(`beginCharGen` / `updateCharGen` / `renderCharGen` / `charGenGoBack`) ever assigns
`player->name`. As a result every character is called "Player", and because the
high-score system copies `player->name` into `ScoreEntry.characterName` on death,
every leaderboard entry is also recorded as "Player".

This bugfix adds a name-entry step to character generation so the player can type a
custom name, applies that name to `player->name` during the DONE finalization step,
and sanitizes the input (fallback for empty, maximum length, printable characters
only) so the resulting name is always valid.

## Bug Analysis

### Current Behavior (Defect)

1.1 WHEN the player completes character generation THEN the system leaves `player->name`
set to the hardcoded default `"Player"` because no character-generation step assigns it.

1.2 WHEN the player wishes to name their character during character generation THEN the
system provides no name-entry step (the `Step` enum is `{ HOMEWORLD, CAREER, ADVANCES, DONE }`
with no NAME step and `CharGenState` has no name buffer field), so no keyboard input is
ever accumulated into a character name.

1.3 WHEN the character dies and a high-score entry is recorded THEN the system stores
`ScoreEntry.characterName` as `"Player"` because it copies the unmodified `player->name`.

### Expected Behavior (Correct)

2.1 WHEN the player reaches the name-entry step of character generation THEN the system SHALL
prompt the player to type a character name, accumulating printable characters into a name
buffer, supporting Backspace to delete the last character, and committing the buffer on Enter.

2.2 WHEN the player commits a non-empty name during character generation THEN the system SHALL
sanitize it (trim to the maximum length, keep printable characters only) and assign the
sanitized result to `player->name` during the DONE finalization step.

2.3 WHEN the player commits an empty or whitespace-only name (or a name that sanitizes to
empty) THEN the system SHALL substitute the default fallback name `"Rogue Trader"` and assign
it to `player->name`.

2.4 WHEN the player enters a name longer than the maximum length (24 characters) THEN the
system SHALL truncate the name to the maximum length before assigning it to `player->name`.

2.5 WHEN the player enters a name containing non-printable characters THEN the system SHALL
strip those characters, keeping only printable characters in the final name.

2.6 WHEN the character dies after entering a custom name THEN the system SHALL record that
sanitized custom name (not `"Player"`) in `ScoreEntry.characterName`.

### Unchanged Behavior (Regression Prevention)

3.1 WHEN the player navigates the HOMEWORLD, CAREER, and ADVANCES steps THEN the system SHALL
CONTINUE TO apply homeworld modifiers, career Rank 1 grants, XP advance purchases, and
back-navigation (Escape) exactly as before.

3.2 WHEN character generation reaches the DONE step THEN the system SHALL CONTINUE TO build
the `CharacterSheet`, apply career combat stats, populate characteristics/skills/talents/traits,
and grant starting equipment exactly as before.

3.3 WHEN a name is already valid (non-empty, within the maximum length, printable characters
only) THEN the system SHALL CONTINUE TO use that name unchanged (sanitization is the identity
for already-valid names).

3.4 WHEN the high-score system records an entry for a character with a valid custom name THEN
the system SHALL CONTINUE TO copy `player->name` into `ScoreEntry.characterName` unchanged.
