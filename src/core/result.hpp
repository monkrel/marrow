#pragma once

#include <string>
#include <utility>
#include <variant>

namespace marrow {

// Expected failures returns as values, not exceptions.
// So this is minimal replacement for std::expected, which is C++23.
struct Error {
    std::string message;
};

template <typename T>
class [[nodiscard]] Result {
public:
    Result(T value) : storage_(std::move(value)) {}
    Result(Error error) : storage_(std::move(error)) {}

    bool ok() const noexcept { return std::holds_alternative<T>(storage_); }
    explicit operator bool() const noexcept { return ok(); }

    const T& value() const { return std::get<T>(storage_); }
    const Error& error() const { return std::get<Error>(storage_); }

private:
    std::variant<T, Error> storage_;
};

} // namespace marrow
