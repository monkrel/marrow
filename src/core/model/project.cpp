#include "core/model/project.hpp"

#include <algorithm>

namespace marrow {

namespace {

template <typename Container, typename Id>
auto* find_by_id(Container& items, Id id) noexcept {
    const auto it =
        std::find_if(items.begin(), items.end(), [&](const auto& item) { return item.id == id; });
    return it != items.end() ? &*it : nullptr;
}

} // namespace

const ClassDef* ProjectModel::find_class(ClassId id) const noexcept {
    return find_by_id(classes, id);
}

ClassDef* ProjectModel::find_class(ClassId id) noexcept {
    return find_by_id(classes, id);
}

const EnumDef* ProjectModel::find_enum(EnumId id) const noexcept {
    return find_by_id(enums, id);
}

EnumDef* ProjectModel::find_enum(EnumId id) noexcept {
    return find_by_id(enums, id);
}

} // namespace marrow
