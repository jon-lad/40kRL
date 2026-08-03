#pragma once
#include <optional>
#include <string>
#include <string_view>

// Physical form-factor classification for weapons.
// Drives combat-mode restrictions (melee-only, pistol-in-melee, basic/heavy blocked in melee).
enum class SizeClassification {
    MELEE,    // melee-only, adjacent targets
    PISTOL,   // can fire at adjacent targets
    BASIC,    // cannot fire at adjacent targets
    HEAVY,    // cannot fire at adjacent targets
    THROWN    // ranged, any tile within range stat
};

// Technology/mechanism proficiency category.
// Drives the -20 penalty when character lacks "Weapon Training (<Group>)".
enum class WeaponGroup {
    LAS,
    SP,
    BOLT,
    MELTA,
    PLASMA,
    FLAME,
    PRIMITIVE,
    LAUNCHER,
    EXOTIC
};

// Type of harm inflicted. Selects the critical-hit injury table.
enum class DamageType {
    E,  // Energy
    X,  // Explosive
    I,  // Impact
    R   // Rending
};

// Parsing utilities: return std::nullopt on unrecognised input.
std::optional<SizeClassification> parseSizeClassification(std::string_view str);
std::optional<WeaponGroup> parseWeaponGroup(std::string_view str);
std::optional<DamageType> parseDamageType(std::string_view str);

// Display name for each enum value (for UI and error messages).
std::string_view sizeClassificationName(SizeClassification sc);
std::string_view weaponGroupName(WeaponGroup wg);
std::string_view damageTypeName(DamageType dt);

// ─── Validation predicates (used by Equipment Loader) ────────────────────────

// Returns true iff range > 0 (valid range stat for a ranged weapon).
bool isValidRange(int range);

// Result of validating a weapon entry's classification fields.
struct WeaponValidationResult {
    bool accepted = false;          // true if entry should be loaded
    std::string warningMessage;     // non-empty if entry was rejected (describes why)
};

// Validates classification fields for a weapon entry.
// - If isWeapon is true (has melee or ranged stats): requires all three fields present and valid.
// - If isWeapon is false (armour, shield): always accepted regardless of fields.
// sizeClassStr, weaponGroupStr, damageTypeStr may be empty to indicate "field missing".
WeaponValidationResult validateWeaponClassification(
    const std::string& entryName,
    bool isWeapon,
    const std::string& sizeClassStr,
    const std::string& weaponGroupStr,
    const std::string& damageTypeStr);

// ─── Proficiency Check Utility ───────────────────────────────────────────────

class Actor;  // forward declaration

// Returns true if the actor has the required weapon training talent for the given group.
// Checks CareerProgression::talents for "Weapon Training (<GroupName>)".
bool hasProficiency(const Actor* actor, WeaponGroup group);

// Returns the proficiency penalty (0 or -20) for the given actor and weapon group.
// Returns -20 if the actor lacks the matching "Weapon Training (<GroupName>)" talent, else 0.
int proficiencyModifier(const Actor* actor, WeaponGroup group);
