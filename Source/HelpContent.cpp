// Source/HelpContent.cpp
#include "HelpContent.hpp"

namespace HelpContent {

// ─── Movement Section ────────────────────────────────────────────────────────
static const HelpEntry movementEntries[] = {
    { "Arrow keys", "Move in 4 directions (N/S/E/W)" },
    { "KP_1",       "Move south-west" },
    { "KP_2",       "Move south" },
    { "KP_3",       "Move south-east" },
    { "KP_4",       "Move west" },
    { "KP_5",       "End turn (wait in place)" },
    { "KP_6",       "Move east" },
    { "KP_7",       "Move north-west" },
    { "KP_8",       "Move north" },
    { "KP_9",       "Move north-east" },
    { "h",          "Move west (vi-key)" },
    { "j",          "Move south (vi-key)" },
    { "k",          "Move north (vi-key)" },
    { "l",          "Move east (vi-key)" },
    { "y",          "Move north-west (vi-key)" },
    { "u",          "Move north-east (vi-key)" },
    { "b",          "Move south-west (vi-key)" },
    { "n",          "Move south-east (vi-key)" },
};

// ─── Actions Section ─────────────────────────────────────────────────────────
static const HelpEntry actionsEntries[] = {
    { "g",    "Pick up item" },
    { "s",    "Shoot (ranged attack)" },
    { "r",    "Reload weapon" },
    { "o",    "Open door" },
    { "a",    "Aim (called shot)" },
    { "A",    "All-out attack" },
    { "R",    "Run (double move)" },
    { "C",    "Charge (move + melee)" },
    { "KP_5", "End turn" },
};

// ─── Overlays Section ────────────────────────────────────────────────────────
static const HelpEntry overlaysEntries[] = {
    { "i", "Inventory" },
    { "c", "Character / equipment sheet" },
    { "l", "Look mode" },
    { "x", "Advances (XP spending)" },
    { "m", "World map" },
    { "e", "Equipment" },
    { "?", "Help (this screen)" },
};

// ─── Navigation Section ──────────────────────────────────────────────────────
static const HelpEntry navigationEntries[] = {
    { "<",   "Ascend stairs" },
    { ">",   "Descend stairs" },
    { "ESC", "Save and return to menu" },
};

// ─── Sections Array ──────────────────────────────────────────────────────────
static const HelpSection sections[] = {
    { "Movement",   Span<const HelpEntry>(movementEntries) },
    { "Actions",    Span<const HelpEntry>(actionsEntries) },
    { "Overlays",   Span<const HelpEntry>(overlaysEntries) },
    { "Navigation", Span<const HelpEntry>(navigationEntries) },
};

Span<const HelpSection> allSections() {
    return Span<const HelpSection>(sections);
}

int totalLineCount() {
    int lines = 0;
    for (const auto& section : sections) {
        lines += 1; // section title
        lines += static_cast<int>(section.entries.size()); // entries
        lines += 1; // spacing after section
    }
    return lines;
}

} // namespace HelpContent
