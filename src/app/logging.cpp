#include "app/logging.hpp"

#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <windows.h>

#include <shlobj.h>

#include <filesystem>
#include <memory>
#include <vector>

namespace marrow::app {

namespace {

std::filesystem::path log_directory() {
    PWSTR local_app_data = nullptr;
    std::filesystem::path dir;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local_app_data))) {
        dir = std::filesystem::path(local_app_data) / L"Marrow" / L"Logs";
    }
    CoTaskMemFree(local_app_data);
    return dir;
}

} // namespace

void init_logging() {
    constexpr std::size_t max_file_bytes = 5 * 1024 * 1024;
    constexpr std::size_t max_files = 3;

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
    try {
        const auto dir = log_directory();
        if (!dir.empty()) {
            std::filesystem::create_directories(dir);
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                (dir / L"marrow.log").string(), max_file_bytes, max_files));
        }
    } catch (const std::exception&) {
    
    }

    auto logger = std::make_shared<spdlog::logger>("marrow", sinks.begin(), sinks.end());
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
#ifdef _DEBUG
    logger->set_level(spdlog::level::debug);
#else
    logger->set_level(spdlog::level::info);
#endif
    logger->flush_on(spdlog::level::info);
    spdlog::set_default_logger(std::move(logger));
}

} // namespace marrow::app
