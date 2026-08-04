# Rogue Trader Mechanics Fallback

When writing requirements or design documents for a new feature, if the user does not specify a particular game mechanic (e.g., how hit chance works, what damage formula to use, how armour interacts with penetration, critical effect thresholds, etc.), default to the Rogue Trader RPG rules as documented in the `Reference/` folder:

- `Reference/RT-CoreMechanics.md` — d100 system, attack resolution, damage formula, evasion, movement
- `Reference/RT-Weapons.md` — weapon profiles, qualities, damage types, range brackets
- `Reference/RT-Bestiary.md` — creature stat blocks, traits, Ork profiles
- `Reference/RT-CriticalEffects.md` — critical injury tables by damage type and location
- `Reference/RT-CharacterCreation.md` — characteristic generation, careers, wounds, XP

## Rules

1. If a feature requires a game mechanic and the user has not specified one, look it up in the Reference files and use the Rogue Trader rule as the default.
2. Cite which reference file and section the mechanic comes from in the requirements or design doc (e.g., "per RT-CoreMechanics §5, damage = weapon roll + SB - TB - AP").
3. If the mechanic is NOT covered by the Reference files (e.g., something unique to a roguelike adaptation with no tabletop equivalent), flag it to the user and ask how they want to handle it before proceeding.
4. The user can always override a Reference-based default by specifying their own mechanic — user-specified rules take priority over the Reference fallback.
