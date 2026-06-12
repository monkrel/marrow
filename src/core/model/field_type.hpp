#pragma once

#include "core/ids.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace marrow {

// Field types are data, not behavior
// here we only describing shapes

struct FieldType;
struct FieldDef;

// --- Simple types ---

struct BytesType {
    std::uint32_t length = 1;
};

struct PaddingType {
    std::uint32_t length = 1;
};

struct IntType {
    bool is_signed = true;
    std::uint8_t bits = 32; // 8, 16, 32, 64
};

struct BoolType {
    std::uint8_t storage_bits = 8; // 8, 16, 32, 64
};

struct FloatType {
    std::uint8_t bits = 32; // 32, 64
};

struct EnumRefType {
    EnumId id; // storage size comes from the enum definition
};

struct BitfieldType {
    std::uint8_t storage_bits = 32; // 8, 16, 32, 64
    std::uint8_t bit_offset = 0;
    std::uint8_t bit_count = 1; // bit_offset + bit_count <= storage_bits
};

struct Utf8StringType {
    std::uint32_t length = 16; // code units (1 byte each)
};

struct Utf16StringType {
    std::uint32_t length = 16; // code units (2 bytes each)
};

struct ClassInstanceType {
    ClassId id; // embedded by value; size comes from the class
};

struct VectorType {
    std::uint8_t components = 4; // 2, 3, or 4 floats
};

struct MatrixType {
    std::uint8_t rows = 4; // valid shapes: 3x3, 3x4, 4x4
    std::uint8_t cols = 4;
};

struct FunctionAddressType {};

// The field stores only the vtable pointer
// The function addresses are stored behind that pointer
struct VTableType {
    std::uint32_t entry_count = 1;
};


// --- Recursive types ---
// Nested types are shared so FieldType stays cheap to copy

struct PointerType {
    // Only one target should be set
    // If both are empty, this is an untyped pointer
    std::optional<ClassId> pointee_class;
    std::shared_ptr<const FieldType> pointee_type;
};

struct ArrayType {
    std::shared_ptr<const FieldType> element;
    std::uint32_t count = 1;
};

// Union members share the same memory
// Their offsets are relative to the union and should be 0
struct UnionType {
    std::vector<FieldDef> members;
};

struct FieldType {
    std::variant<BytesType, PaddingType, IntType, BoolType, FloatType, EnumRefType, BitfieldType,
                 Utf8StringType, Utf16StringType, ClassInstanceType, VectorType, MatrixType,
                 FunctionAddressType, VTableType, PointerType, ArrayType, UnionType>
        value;
};

struct FieldDef {
    FieldId id;
    std::string name; // may be empty for padding only
    std::string comment;
    ByteOffset offset = 0;
    FieldType type;
};

inline std::shared_ptr<const FieldType> box_type(FieldType type) {
    return std::make_shared<const FieldType>(std::move(type));
}

} // namespace marrow
