#include "DiceRoller.h"
#include <random>
#include <regex>

std::optional<DiceSpec> DiceRoller::parse(const std::string& spec) {
    // Match "NdX" where N is 1-9 and X is 1-100
    static const std::regex pattern(R"(^([1-9])d(\d{1,3})$)");
    std::smatch match;

    if (!std::regex_match(spec, match, pattern)) {
        return std::nullopt;
    }

    int count = std::stoi(match[1].str());
    int sides = std::stoi(match[2].str());

    // Validate ranges: N must be 1-9, X must be 1-100
    if (count < 1 || count > 9 || sides < 1 || sides > 100) {
        return std::nullopt;
    }

    return DiceSpec{ count, sides };
}

std::string DiceRoller::format(const DiceSpec& spec) {
    return std::to_string(spec.count) + "d" + std::to_string(spec.sides);
}

int DiceRoller::roll(const DiceSpec& spec, std::function<int(int)> rollDie) {
    if (!rollDie) {
        rollDie = defaultRollDie;
    }

    int sum = 0;
    for (int i = 0; i < spec.count; ++i) {
        sum += rollDie(spec.sides);
    }
    return sum;
}

int DiceRoller::rollFromString(const std::string& spec, std::function<int(int)> rollDie) {
    auto parsed = parse(spec);
    if (!parsed.has_value()) {
        return 0;
    }
    return roll(parsed.value(), rollDie);
}

int DiceRoller::defaultRollDie(int sides) {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, sides);
    return dist(rng);
}
