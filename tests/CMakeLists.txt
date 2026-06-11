find_package(Catch2 CONFIG REQUIRED)

add_executable(marrow_unit_tests
    unit/version_tests.cpp)

target_link_libraries(marrow_unit_tests PRIVATE
    marrow_core
    marrow_warnings
    Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(marrow_unit_tests)