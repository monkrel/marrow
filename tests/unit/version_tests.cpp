#include "core/version.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("app version is the MVP version", "[version]") {
    REQUIRE(marrow::app_version() == "0.1.0-mvp");
    REQUIRE(marrow::kAppName == "Marrow");
}
