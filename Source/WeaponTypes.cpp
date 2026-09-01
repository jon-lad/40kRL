#include "main.hpp"
#include "WeaponTypes.hpp"
#include "Actor.hpp"
#include "CareerProgression.hpp"
#include "StatBlock.hpp"

#include <array>
#include <string>
#include <utility>

// ─── Parse functions ─────────────────────────────────────────────────────────

std::optional<SizeClassification> parseSizeClassification(std::string_view str) {
    static constexpr std::array<std::pair<std::string_view, SizeClassification>, 5> map = {{
        {"Melee",  SizeClassification::MELEE},
        {"Pistol", SizeClassification::PISTOL},
        {"Basic",  SizeClassification::BASIC},
        {"Heavy",  SizeClassification::HEAVY},
        {"Thrown", SizeClassification::THROWN}
    }};
    for (const auto& [name, value] : map) {
        if (str == name) return value;
    }
    return std::nullopt;
}

std::optional<WeaponGroup> parseWeaponGroup(std::string_view str) {
    static constexpr std::array<std::pair<std::string_view, WeaponGroup>, 9> map = {{
        {"Las",       WeaponGroup::LAS},
        {"SP",        WeaponGroup::SP},
        {"Bolt",      WeaponGroup::BOLT},
        {"Melta",     WeaponGroup::MELTA},
        {"Plasma",    WeaponGroup::PLASMA},
        {"Flame",     WeaponGroup::FLAME},
        {"Primitive", WeaponGroup::PRIMITIVE},
        {"Launcher",  WeaponGroup::LAUNCHER},
        {"Exotic",    WeaponGroup::EXOTIC}
    }};
    for (const auto& [name, value] : map) {
        if (str == name) return value;
    }
    return std::nullopt;
}

std::optional<DamageType> parseDamageType(std::string_view str) {
    static constexpr std::array<std::pair<std::string_view, DamageType>, 4> map = {{
        {"E", DamageType::E},
        {"X", DamageType::X},
        {"I", DamageType::I},
        {"R", DamageType::R}
    }};
    for (const auto& [name, value] : map) {
        if (str == name) return value;
    }
    return std::nullopt;
}

// ─── Display name functions ──────────────────────────────────────────────────

std::string_view sizeClassificationName(SizeClassification sc) {
    switch (sc) {
        case SizeClassification::MELEE:  return "Melee";
        case SizeClassification::PISTOL: return "Pistol";
        case SizeClassification::BASIC:  return "Basic";
        case SizeClassification::HEAVY:  return "Heavy";
        case SizeClassification::THROWN: return "Thrown";
    }
    return "";
}

std::string_view weaponGroupName(WeaponGroup wg) {
    switch (wg) {
        case WeaponGroup::LAS:       return "Las";
        case WeaponGroup::SP:        return "SP";
        case WeaponGroup::BOLT:      return "Bolt";
        case WeaponGroup::MELTA:     return "Melta";
        case WeaponGroup::PLASMA:    return "Plasma";
        case WeaponGroup::FLAME:     return "Flame";
        case WeaponGroup::PRIMITIVE: return "Primitive";
        case WeaponGroup::LAUNCHER:  return "Launcher";
        case WeaponGroup::EXOTIC:    return "Exotic";
    }
    return "";
}

std::string_view damageTypeName(DamageType dt) {
    switch (dt) {
        case DamageType::E: return "E";
        case DamageType::X: return "X";
        case DamageType::I: return "I";
        case DamageType::R: return "R";
    }
    return "";
}


// ─── Validation predicates ───────────────────────────────────────────────────

bool isValidRange(int range) {
    return range > 0;
}

WeaponValidationResult validateWeaponClassification(
    const std::string& entryName,
    bool isWeapon,
    const std::string& sizeClassStr,
    const std::string& weaponGroupStr,
    const std::string& damageTypeStr)
{
    // Non-weapon entries (armour, shields) are always accepted
    if (!isWeapon) {
        return { true, "" };
    }

    // Weapon entries require all three classification fields present and valid
    if (sizeClassStr.empty()) {
        return { false, "Equipment.lua: skipping '" + entryName + "' — missing required sizeClass." };
    }
    if (weaponGroupStr.empty()) {
        return { false, "Equipment.lua: skipping '" + entryName + "' — missing required weaponGroup." };
    }
    if (damageTypeStr.empty()) {
        return { false, "Equipment.lua: skipping '" + entryName + "' — missing required damageType." };
    }

    // Validate each field
    if (!parseSizeClassification(sizeClassStr).has_value()) {
        return { false, "Equipment.lua: skipping '" + entryName + "' — invalid sizeClass '" + sizeClassStr + "'." };
    }
    if (!parseWeaponGroup(weaponGroupStr).has_value()) {
        return { false, "Equipment.lua: skipping '" + entryName + "' — invalid weaponGroup '" + weaponGroupStr + "'." };
    }
    if (!parseDamageType(damageTypeStr).has_value()) {
        return { false, "Equipment.lua: skipping '" + entryName + "' — invalid damageType '" + damageTypeStr + "'." };
    }

    return { true, "" };
}


// ─── Proficiency Check Utility ───────────────────────────────────────────────

bool hasProficiency(const Actor* actor, WeaponGroup group) {
    std::string talentStr = "Weapon Training (" + std::string(weaponGroupName(group)) + ")";
    return hasTalent(actor, talentStr);   // null-safe; reads actor->career->talents
}

int proficiencyModifier(const Actor* actor, WeaponGroup group) {
    return hasProficiency(actor, group) ? 0 : -20;
}


// ─── Combat-Mode Gate ────────────────────────────────────────────────────────

bool isRangedAttackAllowed(SizeClassification sizeClass, int distance, int range) {
    switch (sizeClass) {
        case SizeClassification::PISTOL:
            return distance >= 1 && distance <= range;
        case SizeClassification::BASIC:
        case SizeClassification::HEAVY:
            return distance >= 2 && distance <= range;
        case SizeClassification::MELEE:
            return false; // melee weapons cannot make ranged attacks
        case SizeClassification::THROWN:
            return distance >= 1 && distance <= range;
    }
    return false;
}

CombatModeResult checkCombatMode(SizeClassification sizeClass, int distance, int range) {
    switch (sizeClass) {
        case SizeClassification::PISTOL:
            if (distance >= 1 && distance <= range) return { true, "" };
            return { false, "Target is out of range." };
        case SizeClassification::BASIC:
        case SizeClassification::HEAVY:
            if (distance == 1) return { false, "This weapon cannot be fired at adjacent targets." };
            if (distance > range) return { false, "Target is out of range." };
            return { true, "" };
        case SizeClassification::MELEE:
            if (distance == 1) return { true, "" }; // melee allowed at adjacent
            return { false, "Target is too far for a melee weapon." };
        case SizeClassification::THROWN:
            if (distance >= 1 && distance <= range) return { true, "" };
            return { false, "Target is out of range." };
    }
    return { false, "Unknown weapon type." };
}
