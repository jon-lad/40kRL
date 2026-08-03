#include "WeaponTypes.h"

// Stub implementations — will be completed in Task 1.2.
// These return nullopt / empty strings so the test project links.

std::optional<SizeClassification> parseSizeClassification(std::string_view /*str*/) {
    return std::nullopt;
}

std::optional<WeaponGroup> parseWeaponGroup(std::string_view /*str*/) {
    return std::nullopt;
}

std::optional<DamageType> parseDamageType(std::string_view /*str*/) {
    return std::nullopt;
}

std::string_view sizeClassificationName(SizeClassification /*sc*/) {
    return "";
}

std::string_view weaponGroupName(WeaponGroup /*wg*/) {
    return "";
}

std::string_view damageTypeName(DamageType /*dt*/) {
    return "";
}
