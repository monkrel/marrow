find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(marrow WIN32
    logging.cpp
    logging.hpp
    main.cpp)

target_compile_definitions(marrow PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN)

target_link_libraries(marrow PRIVATE
    marrow_core
    marrow_warnings
    imgui::imgui
    spdlog::spdlog
    d3d11
    dxgi
    d3dcompiler)
