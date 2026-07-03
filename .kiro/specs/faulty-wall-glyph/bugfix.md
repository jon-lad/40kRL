# Bugfix Requirements Document

## Introduction

Wall corner glyphs ╗ and ╔ appear swapped/mirrored. The `DSE` and `DSW` constants in `CharConstants.h` have their Unicode codepoints transposed: `DSE` is assigned `0x2557` (╗, which draws lines South+West) when it should be `0x2554` (╔, lines South+East), and `DSW` is assigned `0x2554` (╔, lines South+East) when it should be `0x2557` (╗, lines South+West). The `chooseWallGlyph` conditions are correct but render the wrong character because the named constants point to the wrong codepoints.

## Bug Analysis

### Current Behavior (Defect)

1.1 WHEN `chooseWallGlyph` selects `CharConst::DSE` for a wall with neighbours below and to the right (`!top && bottom && !left && right`) THEN the system renders ╗ (0x2557, lines going down+left) instead of the expected ╔ (lines going down+right)

1.2 WHEN `chooseWallGlyph` selects `CharConst::DSW` for a wall with neighbours below and to the left (`!top && bottom && left && !right`) THEN the system renders ╔ (0x2554, lines going down+right) instead of the expected ╗ (lines going down+left)

### Expected Behavior (Correct)

2.1 WHEN `chooseWallGlyph` selects `CharConst::DSW` for a wall with neighbours below and to the left (`!top && bottom && left && !right`) THEN the system SHALL render ╗ (0x2557) which draws lines going South and West, matching the neighbour directions

2.2 WHEN `chooseWallGlyph` selects `CharConst::DSE` for a wall with neighbours below and to the right (`!top && bottom && !left && right`) THEN the system SHALL render ╔ (0x2554) which draws lines going South and East, matching the neighbour directions

### Unchanged Behavior (Regression Prevention)

3.1 WHEN `chooseWallGlyph` selects `CharConst::DNE` for a wall with neighbours above and to the right (`top && !bottom && !left && right`) THEN the system SHALL CONTINUE TO render ╚ (0x255A) correctly

3.2 WHEN `chooseWallGlyph` selects `CharConst::DNW` for a wall with neighbours above and to the left (`top && !bottom && left && !right`) THEN the system SHALL CONTINUE TO render ╝ (0x255D) correctly

3.3 WHEN `chooseWallGlyph` selects any tee, cross, or straight-line glyph (DCROSS, DTEES, DTEEN, DTEEE, DTEEW, DVLINE, DHLINE) THEN the system SHALL CONTINUE TO render the correct character for that pattern

3.4 WHEN wall tiles are rendered in explored-but-not-in-FOV state THEN the system SHALL CONTINUE TO use dark wall colour with the correct glyph
