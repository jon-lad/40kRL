# Reference Files Index
## Rogue Trader RPG mechanics for 40kRL implementation

These files are structured for AI readability — clean markdown with tables,
formulas, and stat blocks that can be directly referenced during game development.

### Clean Reference Files (use these)

| File | Contents |
|---|---|
| `RT-CoreMechanics.md` | d100 system, characteristics, tests, difficulty, combat structure, attack resolution, wounds, evasion, movement |
| `RT-Weapons.md` | All weapon profiles (las, SP, bolt, plasma, melta, flame, grenades, melee), weapon qualities, size classes, weapon groups |
| `RT-Bestiary.md` | Creature stat blocks (beasts, Orks, Rak'Gol, daemons), creature traits, size categories, Ork weapon profiles |
| `RT-CriticalEffects.md` | Critical damage tables by type (E/I/R/X) and location (head/body/arm/leg), conditions |
| `RT-CharacterCreation.md` | Characteristic generation, home worlds, careers, wounds, skills, XP costs, Profit Factor |

### Original OCR Files (preserved for reference, largely unreadable)

| File | Source | Status |
|---|---|---|
| `RT-rules.txt` | Rogue Trader Core Rulebook (2009 FFG) | Garbled OCR — unusable |
| `340049081-Rogue-Trader-The-Koronus-Bestiary.txt` | The Koronus Bestiary (2012 FFG) | Partially readable (narrative OK, headers garbled) |
| `850108181-The-Liber-Imperium-1-6.txt` | The Liber Imperium v1.6 (fan compilation) | Readable — full rules compilation for DH/OW/RT |

### How to Use

When implementing game features, reference the clean `.md` files:
- **Combat system**: `RT-CoreMechanics.md` §4-8
- **Weapon data for Lua scripts**: `RT-Weapons.md`
- **Enemy stat blocks**: `RT-Bestiary.md`
- **Injury/wound system**: `RT-CriticalEffects.md`
- **Character generation**: `RT-CharacterCreation.md`

The Liber Imperium `.txt` file remains as the authoritative full-text source
if deeper lookup is needed (e.g., specific talent descriptions, psychic powers,
vehicle rules, voidship combat).
