#include "core/uuid.hpp"

#include <cstring>
#include <random>

namespace marrow {

namespace {

int hex_value(char c) noexcept {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

} // namespace


// https://cryptosys.net/pki/uuid-rfc4122.html
Uuid Uuid::generate() {
    thread_local std::mt19937_64 rng = [] {
        std::random_device device;
        std::seed_seq seq{device(), device(), device(), device(),
                          device(), device(), device(), device()};
        return std::mt19937_64(seq);
    }();

    Uuid uuid;
    const std::uint64_t hi = rng();
    const std::uint64_t lo = rng();
    std::memcpy(uuid.bytes_.data(), &hi, 8);
    std::memcpy(uuid.bytes_.data() + 8, &lo, 8);
    // RFC 4122: version 4, variant 1
    uuid.bytes_[6] = static_cast<std::uint8_t>((uuid.bytes_[6] & 0x0F) | 0x40);
    uuid.bytes_[8] = static_cast<std::uint8_t>((uuid.bytes_[8] & 0x3F) | 0x80);
    return uuid;
}

std::optional<Uuid> Uuid::parse(std::string_view text) {
    if (text.size() != 36 || text[8] != '-' || text[13] != '-' || text[18] != '-' ||
        text[23] != '-') {
        return std::nullopt;
    }
    Uuid uuid;
    std::size_t byte_index = 0;
    for (std::size_t i = 0; i < text.size();) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            ++i;
            continue;
        }
        const int hi = hex_value(text[i]);
        const int lo = hex_value(text[i + 1]);
        if (hi < 0 || lo < 0) {
            return std::nullopt;
        }
        uuid.bytes_[byte_index++] = static_cast<std::uint8_t>(hi << 4 | lo);
        i += 2;
    }
    return uuid;
}

std::string Uuid::to_string() const {
    static constexpr char digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(36);
    for (std::size_t i = 0; i < bytes_.size(); ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            out.push_back('-');
        }
        out.push_back(digits[bytes_[i] >> 4]);
        out.push_back(digits[bytes_[i] & 0x0F]);
    }
    return out;
}

} // namespace marrow
