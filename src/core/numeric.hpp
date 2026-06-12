#pragma once

#include <cstdint>
#include <limits>


// Check for overflows. better safe than sorry
namespace marrow {

inline bool add_overflows(std::uint64_t a, std::uint64_t b) noexcept {
    return b > std::numeric_limits<std::uint64_t>::max() - a;
}

inline bool mul_overflows(std::uint64_t a, std::uint64_t b) noexcept {
    return a != 0 && b > std::numeric_limits<std::uint64_t>::max() / a;
}

} // namespace marrow
