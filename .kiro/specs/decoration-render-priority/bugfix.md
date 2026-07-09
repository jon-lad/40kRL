# Bugfix Requirements Document

## Introduction

Decorations (crates, barrels, consoles, pillars, etc.) are rendered on top of actors (player, enemies) when they share the same tile. This hides the player or enemy glyph, making it impossible to see what actor occupies the tile. The root cause is that the render loop in `Engine::render()` draws actors in list-insertion order without considering visual priority — decorations appended later in the list are drawn over actors that appear earlier.

## Bug Analysis

### Current Behavior (Defect)

1.1 WHEN a decoration and an actor (player or enemy) occupy the same tile THEN the system renders the decoration's glyph on top of the actor's glyph, hiding the actor

1.2 WHEN decorations are spawned via `addDecorations()` or `addOutdoorDecorations()` THEN the system appends them to the end of the actors list without managing draw order, causing them to render over actors that were inserted earlier

### Expected Behavior (Correct)

2.1 WHEN a decoration and an actor (player or enemy) occupy the same tile THEN the system SHALL render the actor's glyph on top of the decoration's glyph, ensuring actors are always visible

2.2 WHEN decorations are spawned THEN the system SHALL position them in the draw order such that they are always rendered beneath actors that have gameplay components (ai, destructible, or attacker)

### Unchanged Behavior (Regression Prevention)

3.1 WHEN no actor shares a tile with a decoration THEN the system SHALL CONTINUE TO render the decoration normally at its tile position

3.2 WHEN an actor dies and becomes a corpse THEN the system SHALL CONTINUE TO render the corpse beneath living actors (existing sendToBack behavior preserved)

3.3 WHEN multiple decorations share the same tile THEN the system SHALL CONTINUE TO render them without any specific ordering guarantee between decorations

3.4 WHEN the player moves off a decoration's tile THEN the system SHALL CONTINUE TO render the decoration normally at its tile position

3.5 WHEN decorations are loaded from a saved game or level cache THEN the system SHALL CONTINUE TO position them correctly in draw order beneath actors
