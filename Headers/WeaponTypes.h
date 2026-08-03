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
