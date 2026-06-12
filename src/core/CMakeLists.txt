# Core domain library
add_library(marrow_core STATIC
    ids.hpp
    numeric.hpp
    result.hpp
    uuid.cpp
    uuid.hpp
    version.cpp
    version.hpp
    visit.hpp
    model/field_type.hpp
    model/layout.cpp
    model/layout.hpp
    model/project.cpp
    model/project.hpp)

target_include_directories(marrow_core PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(marrow_core PRIVATE marrow_warnings)