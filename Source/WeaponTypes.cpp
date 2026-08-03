#include "WeaponTypes.h"

#include <array>
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
