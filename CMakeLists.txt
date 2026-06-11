cmake_minimum_required(VERSION 3.28)

project(marrow
    VERSION 0.1.0
    DESCRIPTION "Memory inspection mvp tool"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MARROW_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

add_library(marrow_warnings INTERFACE)
target_compile_options(marrow_warnings INTERFACE
    /W4
    /permissive-
    /utf-8
    $<$<BOOL:${MARROW_WARNINGS_AS_ERRORS}>:/WX>)

enable_testing()

add_subdirectory(src/core)
add_subdirectory(src/app)
add_subdirectory(tests)