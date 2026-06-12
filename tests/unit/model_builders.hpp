#pragma once

#include "core/model/project.hpp"

#include <string>
#include <utility>
#include <vector>

// helpers used by layout/model tests
namespace marrow::test {

inline FieldType int_t(std::uint8_t bits, bool is_signed = true) {
    return FieldType{IntType{is_signed, bits}};
}

// not named float_t because <math.h> already defines that name
inline FieldType floating_t(std::uint8_t bits = 32) {
    return FieldType{FloatType{bits}};
}

inline FieldDef field(std::string name, ByteOffset offset, FieldType type) {
    return FieldDef{FieldId::generate(), std::move(name), "", offset, std::move(type)};
}

inline ClassDef make_class(std::string name, std::vector<FieldDef> fields = {}) {
    ClassDef cls;
    cls.id = ClassId::generate();
    cls.name = std::move(name);
    cls.fields = std::move(fields);
    return cls;
}

inline EnumDef make_enum(std::string name, std::uint8_t bits) {
    EnumDef def;
    def.id = EnumId::generate();
    def.name = std::move(name);
    def.bits = bits;
    return def;
}

inline FieldType ptr_to_class(ClassId id) {
    PointerType type;
    type.pointee_class = id;
    return FieldType{std::move(type)};
}

} // namespace marrow::test
