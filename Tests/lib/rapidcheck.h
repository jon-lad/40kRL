#pragma once
// Minimal RapidCheck-compatible API for property-based testing with Catch2.
// This provides rc::check() and basic generators sufficient for the 40kRL test suite.
// For a full RapidCheck experience, replace this with the actual RapidCheck library.

#include <functional>
#include <random>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace rc {

// Forward declarations for result types used by rapidcheck_catch.h bridge
struct SuccessResult {
    std::vector<std::pair<std::string, int>> distribution;
};

struct FailureResult {
    std::string description;
    int iteration;
};

struct GaveUpResult {
    std::string description;
};

// Tagged union for test results
class TestResult {
public:
    enum class Type { Success, Failure, GaveUp };

    TestResult() : type_(Type::Success) {}

    template<typename T>
    bool is() const {
        if constexpr (std::is_same_v<T, SuccessResult>) return type_ == Type::Success;
        else if constexpr (std::is_same_v<T, FailureResult>) return type_ == Type::Failure;
        else return type_ == Type::GaveUp;
    }

    template<typename T>
    T get() const {
        if constexpr (std::is_same_v<T, SuccessResult>) return success_;
        else if constexpr (std::is_same_v<T, FailureResult>) return failure_;
        else return gaveUp_;
    }

    static TestResult makeSuccess() {
        TestResult r;
        r.type_ = Type::Success;
        return r;
    }

    static TestResult makeFailure(const std::string& desc, int iteration) {
        TestResult r;
        r.type_ = Type::Failure;
        r.failure_.description = desc;
        r.failure_.iteration = iteration;
        return r;
    }

private:
    Type type_;
    SuccessResult success_;
    FailureResult failure_;
    GaveUpResult gaveUp_;
};

namespace detail {

// Run a testable and return a result
template<typename Testable>
TestResult checkTestable(Testable&& testable, int iterations = 100) {
    std::mt19937 rng(std::random_device{}());
    for (int i = 0; i < iterations; ++i) {
        try {
            testable();
        } catch (const std::exception& e) {
            return TestResult::makeFailure(
                std::string("Exception at iteration ") + std::to_string(i) + ": " + e.what(), i);
        } catch (...) {
            return TestResult::makeFailure(
                std::string("Unknown exception at iteration ") + std::to_string(i), i);
        }
    }
    return TestResult::makeSuccess();
}

inline void printResultMessage(const TestResult& result, std::ostream& os) {
    if (result.is<SuccessResult>()) {
        os << "OK, passed 100 tests";
    } else if (result.is<FailureResult>()) {
        auto f = result.get<FailureResult>();
        os << "Falsifiable after " << f.iteration << " tests: " << f.description;
    }
}

} // namespace detail

// Forward declaration — defined below, used by Gen::operator*
inline std::mt19937& currentRng();

// Minimal generator type
template<typename T>
struct Gen {
    std::function<T(std::mt19937&)> generate;

    T operator()(std::mt19937& rng) const { return generate(rng); }

    // Dereference operator — matches real RapidCheck's *gen syntax for generating values
    T operator*() const { return generate(currentRng()); }
};

namespace gen {

// Generate a random bool
inline Gen<bool> arbitrary_bool() {
    return {[](std::mt19937& rng) {
        return std::uniform_int_distribution<int>(0, 1)(rng) == 1;
    }};
}

// Generate a random int in range [min, max]
inline Gen<int> inRange(int min, int max) {
    return {[min, max](std::mt19937& rng) {
        return std::uniform_int_distribution<int>(min, max)(rng);
    }};
}

// Generate a random element from a container/initializer_list
template<typename T>
Gen<T> elementOf(std::vector<T> elements) {
    return {[elements](std::mt19937& rng) {
        std::uniform_int_distribution<size_t> dist(0, elements.size() - 1);
        return elements[dist(rng)];
    }};
}

// Generate a random character in a range
inline Gen<char> charInRange(char min, char max) {
    return {[min, max](std::mt19937& rng) {
        return static_cast<char>(std::uniform_int_distribution<int>(
            static_cast<int>(min), static_cast<int>(max))(rng));
    }};
}

// Generate a random string of given length range
inline Gen<std::string> string(int minLen, int maxLen) {
    return {[minLen, maxLen](std::mt19937& rng) {
        int len = std::uniform_int_distribution<int>(minLen, maxLen)(rng);
        std::string s;
        s.reserve(len);
        for (int i = 0; i < len; ++i) {
            s += static_cast<char>(std::uniform_int_distribution<int>('a', 'z')(rng));
        }
        return s;
    }};
}

// Generate a string using a custom character generator
inline Gen<std::string> stringOf(Gen<char> charGen, int minLen, int maxLen) {
    return {[charGen, minLen, maxLen](std::mt19937& rng) mutable {
        int len = std::uniform_int_distribution<int>(minLen, maxLen)(rng);
        std::string s;
        s.reserve(len);
        for (int i = 0; i < len; ++i) {
            s += charGen(rng);
        }
        return s;
    }};
}

// Generate a container (vector) of N items
template<typename T>
Gen<std::vector<T>> container(int minSize, int maxSize, Gen<T> elemGen) {
    return {[minSize, maxSize, elemGen](std::mt19937& rng) mutable {
        int size = std::uniform_int_distribution<int>(minSize, maxSize)(rng);
        std::vector<T> result;
        result.reserve(size);
        for (int i = 0; i < size; ++i) {
            result.push_back(elemGen(rng));
        }
        return result;
    }};
}

} // namespace gen

// Thread-local RNG for use in check() lambdas
inline std::mt19937& currentRng() {
    thread_local std::mt19937 rng(std::random_device{}());
    return rng;
}

// Generate a value using a Gen within a check() lambda
template<typename T>
T generate(Gen<T> gen) {
    return gen(currentRng());
}

// Run a property check with the given number of iterations.
// The callable receives no arguments — use rc::generate() inside to get random values.
template<typename Func>
void check(const std::string& description, Func&& func, int iterations = 100) {
    std::mt19937& rng = currentRng();
    rng.seed(std::random_device{}());

    for (int i = 0; i < iterations; ++i) {
        try {
            func();
        } catch (const std::exception& e) {
            throw std::runtime_error(
                "Property failed: " + description +
                " at iteration " + std::to_string(i) + ": " + e.what());
        }
    }
}

// Assertion helper for use inside property checks
inline void RC_ASSERT(bool condition, const std::string& msg = "Assertion failed") {
    if (!condition) {
        throw std::runtime_error(msg);
    }
}

} // namespace rc

// Convenience macro matching RapidCheck's RC_ASSERT
#ifndef RC_ASSERT
#define RC_ASSERT(expr) \
    do { if (!(expr)) throw std::runtime_error("RC_ASSERT failed: " #expr); } while(false)
#endif

// RC_PRE — precondition: discard this iteration if not met (no-op in our stub,
// just skip the rest of the lambda via early return if you want strict behaviour).
#ifndef RC_PRE
#define RC_PRE(expr) \
    do { if (!(expr)) return; } while(false)
#endif
