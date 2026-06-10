# Core domain library
add_library(marrow_core STATIC
    version.cpp
    version.hpp)

target_include_directories(marrow_core PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(marrow_core PRIVATE marrow_warnings)