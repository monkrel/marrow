#pragma once

#include "core/uuid.hpp"

#include <cstdint>

namespace marrow {

// Makes different ID types so they cannot be accidentally mixed.
template <typename Tag>
struct TypedId {
    Uuid value;

    static TypedId generate() { return TypedId{Uuid::generate()}; }

    bool is_nil() const noexcept { return value.is_nil(); }

    auto operator<=>(const TypedId&) const = default;
};

using ClassId = TypedId<struct ClassIdTag>;
using EnumId = TypedId<struct EnumIdTag>;
using FieldId = TypedId<struct FieldIdTag>;

using Address = std::uint64_t;
using ByteOffset = std::uint64_t;

} // namespace marrow

template <typename Tag>
struct std::hash<marrow::TypedId<Tag>> {
    std::size_t operator()(const marrow::TypedId<Tag>& id) const noexcept {
        return std::hash<marrow::Uuid>{}(id.value);
    }
};
