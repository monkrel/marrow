#pragma once

#include <array>
#include <compare>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace marrow {

// 128-bit identifier (RFC 4122 version 4)
class Uuid {
public:
    constexpr Uuid() = default;

    static Uuid generate();

    // Accepts the canonical 8-4-4-4-12 format (case-insensitive)
    static std::optional<Uuid> parse(std::string_view text);

    // Canonical lowercase 8-4-4-4-12 format
    std::string to_string() const;

    bool is_nil() const noexcept { return *this == Uuid{}; }

    auto operator<=>(const Uuid&) const = default;

    const std::array<std::uint8_t, 16>& bytes() const noexcept { return bytes_; }

private:
    std::array<std::uint8_t, 16> bytes_{};
};

} // namespace marrow

template <>
struct std::hash<marrow::Uuid> {
    std::size_t operator()(const marrow::Uuid& uuid) const noexcept {
        const auto& b = uuid.bytes();
        std::uint64_t hi = 0;
        std::uint64_t lo = 0;
        for (std::size_t i = 0; i < 8; ++i) {
            hi = hi << 8 | b[i];
        }
        for (std::size_t i = 8; i < 16; ++i) {
            lo = lo << 8 | b[i];
        }
        return std::hash<std::uint64_t>{}(hi ^ (lo * 0x9E3779B97F4A7C15ull)); /*the golden ratio*/
    }
};
