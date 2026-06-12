#pragma once

namespace marrow {

// overload, so we can use std::visit more effectively
template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

} // namespace marrow
