#pragma once

#include <array>
#include <string_view>

// Index order matches the fixed serialization order.
enum class CharId : int {
	WS = 0, BS, S, T, Ag, Int, Per, WP, Fel, COUNT
};

// Stores the nine Rogue Trader RPG characteristics for an Actor.
// All values are clamped to [1, 99]. Bonus = value / 10 (integer division).
class Characteristics : public Persistent {
public:
	static constexpr int MIN_VALUE = 1;
	static constexpr int MAX_VALUE = 99;
	static constexpr int CHAR_COUNT = static_cast<int>(CharId::COUNT);

	// Constructs with all characteristics set to the given default (clamped).
	explicit Characteristics(int defaultValue = 30);

	// Get/set individual characteristics (clamped on set).
	int  get(CharId id) const;
	void set(CharId id, int value);

	// Bonus = value / 10 (integer division, always in [0, 9]).
	int  bonus(CharId id) const;

	// Abbreviated display name for a characteristic.
	static std::string_view abbreviation(CharId id);

	// Serialization — writes/reads all 9 values in fixed CharId order.
	void save(TCODZip& zip) override;
	void load(TCODZip& zip) override;

private:
	std::array<int, CHAR_COUNT> values_;

	static int clamp(int value);
};
