#include "core/uuid.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

using marrow::Uuid;

TEST_CASE("default uuid is nil", "[uuid]") {
    REQUIRE(Uuid{}.is_nil());
    REQUIRE(Uuid{}.to_string() == "00000000-0000-0000-0000-000000000000");
}

TEST_CASE("generated uuids are version 4 and clear", "[uuid]") {
    const Uuid a = Uuid::generate();
    const Uuid b = Uuid::generate();
    REQUIRE_FALSE(a.is_nil());
    REQUIRE(a != b);

    const std::string text = a.to_string();
    REQUIRE(text.size() == 36);
    REQUIRE(text[14] == '4'); // version
    const char variant = text[19];
    REQUIRE((variant == '8' || variant == '9' || variant == 'a' || variant == 'b'));
}

TEST_CASE("to_string round-trips through parse", "[uuid]") {
    const Uuid original = Uuid::generate();
    const auto parsed = Uuid::parse(original.to_string());
    REQUIRE(parsed.has_value());
    REQUIRE(*parsed == original);
}

TEST_CASE("parse accepts uppercase but to_string stays lowercase", "[uuid]") {
    const auto parsed = Uuid::parse("0123ABCD-EF01-4567-89AB-CDEF01234567");
    REQUIRE(parsed.has_value());
    REQUIRE(parsed->to_string() == "0123abcd-ef01-4567-89ab-cdef01234567");
}

TEST_CASE("parse rejects malformed input", "[uuid]") {
    REQUIRE_FALSE(Uuid::parse("").has_value());
    REQUIRE_FALSE(Uuid::parse("0123abcd-ef01-4567-89ab-cdef0123456").has_value());   // short
    REQUIRE_FALSE(Uuid::parse("0123abcd-ef01-4567-89ab-cdef012345678").has_value()); // long
    REQUIRE_FALSE(Uuid::parse("0123abcdxef01-4567-89ab-cdef01234567").has_value());  // bad dash
    REQUIRE_FALSE(Uuid::parse("0123abcd-ef01-4567-89ab-cdef0123456g").has_value());  // non-hex
}
