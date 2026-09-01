
#include "main.hpp"

#include <algorithm>

Characteristics::Characteristics(int defaultValue)
{
	values_.fill(clamp(defaultValue));
	modifiers_.fill(0);
}

int Characteristics::clamp(int value)
{
	return std::max(MIN_VALUE, std::min(MAX_VALUE, value));
}

int Characteristics::get(CharId id) const
{
	return clamp(values_[static_cast<int>(id)] + modifiers_[static_cast<int>(id)]);
}

int Characteristics::getBase(CharId id) const
{
	return values_[static_cast<int>(id)];
}

void Characteristics::addModifier(CharId id, int amount)
{
	modifiers_[static_cast<int>(id)] += amount;
}

void Characteristics::removeModifier(CharId id, int amount)
{
	modifiers_[static_cast<int>(id)] -= amount;
}

int Characteristics::getModifier(CharId id) const
{
	return modifiers_[static_cast<int>(id)];
}

void Characteristics::set(CharId id, int value)
{
	values_[static_cast<int>(id)] = clamp(value);
}

int Characteristics::bonus(CharId id) const
{
	return get(id) / 10;
}

std::string_view Characteristics::abbreviation(CharId id)
{
	static constexpr std::string_view names[] = {
		"WS", "BS", "S", "T", "Ag", "Int", "Per", "WP", "Fel"
	};
	return names[static_cast<int>(id)];
}

void Characteristics::save(TCODZip& zip)
{
	for (int i = 0; i < CHAR_COUNT; ++i) {
		zip.putInt(values_[i]);
	}
}

void Characteristics::load(TCODZip& zip)
{
	for (int i = 0; i < CHAR_COUNT; ++i) {
		set(static_cast<CharId>(i), zip.getInt());
	}
}
