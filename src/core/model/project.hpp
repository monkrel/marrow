#pragma once

#include "core/model/field_type.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace marrow {

struct EnumEntry {
    std::string name;
    std::uint64_t value = 0; // raw value, signed enums interpret it as signed
};

struct EnumDef {
    EnumId id;
    std::string name;
    bool is_signed = false;
    std::uint8_t bits = 32; // 8, 16, 32, 64
    bool flags = false;
    std::vector<EnumEntry> entries; // values may repeat for aliases, but names must be unique
};

struct ClassDef {
    ClassId id;
    std::string name;
    std::string comment;
    std::string address_expression;
    std::optional<std::uint64_t> declared_size; // optional user defined class size
    std::vector<FieldDef> fields;
};

struct TargetInfo {
    std::string executable;
    std::string architecture = "x64";
};

// configurable per project
struct ValidationLimits {
    std::uint64_t max_class_size = 16ull * 1024 * 1024;
    std::uint32_t max_array_elements = 1'048'576;
    std::uint32_t max_string_units = 4'096; // Code units, not bytes	
};

// saved project data only
struct ProjectModel {
    Uuid project_id;
    std::string name = "Untitled";
    std::string description;
    TargetInfo target;
    ValidationLimits limits;
    std::vector<EnumDef> enums;
    std::vector<ClassDef> classes;

    const ClassDef* find_class(ClassId id) const noexcept;
    ClassDef* find_class(ClassId id) noexcept;

    const EnumDef* find_enum(EnumId id) const noexcept;
    EnumDef* find_enum(EnumId id) noexcept;
};

} // namespace marrow
