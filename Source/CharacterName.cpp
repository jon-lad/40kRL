#include "CharacterName.hpp"

#include <algorithm>
#include <cctype>
#include <string>

// Pure, engine-free implementation. See Headers/CharacterName.hpp for the
// contract. No Engine/SDL/GUI/libtcod access (test-isolation compliant).

namespace {

// A byte is "printable" iff std::isprint treats it as such. Cast through
// unsigned char to avoid undefined behavior on negative char values.
bool isPrintableByte(char c)
{
    return std::isprint(static_cast<unsigned char>(c)) != 0;
}

bool isSpaceByte(char c)
{
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

} // namespace

std::string sanitizeCharacterName(const std::string& input)
{
    // 1. Strip non-printable characters.
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        if (isPrintableByte(c)) {
            result.push_back(c);
        }
    }

    // 2. Truncate to MAX_NAME_LENGTH.
    if (static_cast<int>(result.size()) > MAX_NAME_LENGTH) {
        result.resize(static_cast<size_t>(MAX_NAME_LENGTH));
    }

    // 3. Trim leading/trailing whitespace.
    const auto firstNonSpace = std::find_if_not(result.begin(), result.end(), isSpaceByte);
    const auto lastNonSpace = std::find_if_not(result.rbegin(), result.rend(), isSpaceByte).base();
    if (firstNonSpace >= lastNonSpace) {
        result.clear();
    } else {
        result = std::string(firstNonSpace, lastNonSpace);
    }

    // 4. If the result is empty, fall back to the default name.
    if (result.empty()) {
        return DEFAULT_CHARACTER_NAME;
    }

    return result;
}
