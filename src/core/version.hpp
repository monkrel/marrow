#pragma once

#include <string_view>

namespace marrow {

inline constexpr std::string_view kAppName = "Marrow";

std::string_view app_version() noexcept;

} // namespace marrow
