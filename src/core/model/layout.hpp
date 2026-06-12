#pragma once

#include "core/model/project.hpp"
#include "core/result.hpp"

#include <cstdint>

namespace marrow {

// How many bytes this field type takes in a class
Result<std::uint64_t> field_type_size(const FieldType& type, const ProjectModel& project);

// Returns the class size in bytes
// max(declared_size, max(field.offset + field size))
Result<std::uint64_t> class_size(const ClassDef& cls, const ProjectModel& project);

} // namespace marrow
