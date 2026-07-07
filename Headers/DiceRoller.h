#pragma once

#include <string>
#include <functional>
#include <optional>

struct DiceSpec {
    int count;   // N (number of dice, 1-9)
    int sides;   // X (sides per die, 1-100)
};

class DiceRoller {
public:
    // Parses "NdX" format. Returns nullopt on invalid input.
    static std::optional<DiceSpec> parse(const std::string& spec);

    // Formats a DiceSpec back to "NdX" string.
    static std::string format(const DiceSpec& spec);

    // Rolls N dice of X sides and returns the sum.
    // Uses the provided RNG function (defaults to uniform random).
    static int roll(const DiceSpec& spec,
                    std::function<int(int)> rollDie = nullptr);

    // Convenience: parse + roll in one call. Returns 0 on invalid spec.
    static int rollFromString(const std::string& spec,
                              std::function<int(int)> rollDie = nullptr);

private:
    static int defaultRollDie(int sides);
};
