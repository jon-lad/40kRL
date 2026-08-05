// Headers/HelpContent.h
#pragma once
#include <cstddef>
#include <string_view>

namespace HelpContent {

struct HelpEntry {
    std::string_view key;         // e.g. "g", "KP_5", "Arrow keys"
    std::string_view description; // e.g. "Pick up item"
};

// Lightweight non-owning view over a contiguous array (C++17 substitute for std::span)
template <typename T>
struct Span {
    const T* data_ = nullptr;
    std::size_t size_ = 0;

    constexpr Span() = default;
    constexpr Span(const T* d, std::size_t s) : data_(d), size_(s) {}

    template <std::size_t N>
    constexpr Span(const T (&arr)[N]) : data_(arr), size_(N) {}

    constexpr const T* begin() const { return data_; }
    constexpr const T* end()   const { return data_ + size_; }
    constexpr std::size_t size() const { return size_; }
    constexpr bool empty() const { return size_ == 0; }
    constexpr const T& operator[](std::size_t i) const { return data_[i]; }
};

struct HelpSection {
    std::string_view title;          // e.g. "Movement"
    Span<const HelpEntry> entries;
};

// Returns all help sections in defined order
Span<const HelpSection> allSections();

// Total line count (sections + entries + spacing) for scroll calculations
int totalLineCount();

} // namespace HelpContent
