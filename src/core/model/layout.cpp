#include "core/model/layout.hpp"

#include "core/numeric.hpp"
#include "core/visit.hpp"

#include <algorithm>
#include <vector>

namespace marrow {

namespace {

constexpr std::uint64_t kPointerSize = 8; //x64 only at the moment, x32 later


// Internal versions keep track of classes currently being sized
// This is used to detect by-value class cycles
Result<std::uint64_t> type_size_impl(const FieldType& type, const ProjectModel& project,
                                     std::vector<ClassId>& visiting);

Result<std::uint64_t> class_size_impl(const ClassDef& cls, const ProjectModel& project,
                                      std::vector<ClassId>& visiting) {
    // A class can't contain itself by value, directly or indirectly
    if (std::find(visiting.begin(), visiting.end(), cls.id) != visiting.end()) {
        return Error{"by-value class cycle involving '" + cls.name + "'"};
    }
    visiting.push_back(cls.id);

    // Start with the declared size, if the user set one
    std::uint64_t end = cls.declared_size.value_or(0);

    for (const FieldDef& field : cls.fields) {
        const auto size = type_size_impl(field.type, project, visiting);
        if (!size.ok()) {
            visiting.pop_back();
            return size;
        }
        if (add_overflows(field.offset, size.value())) {
            visiting.pop_back();
            return Error{"field '" + field.name + "' in class '" + cls.name +
                         "' overflows 64-bit offset arithmetic"};
        }
        // Class size must reach the end of every field
        end = std::max(end, field.offset + size.value());
    }

    visiting.pop_back();
    return end;
}

Result<std::uint64_t> type_size_impl(const FieldType& type, const ProjectModel& project,
                                     std::vector<ClassId>& visiting) {
    return std::visit(
        overloaded{
            [](const BytesType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.length};
            },
            [](const PaddingType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.length};
            },
            [](const IntType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.bits} / 8u;                // 8 bits in 1 byte
            },
            [](const BoolType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.storage_bits} / 8u;        // 8 bits in 1 byte
            },
            [](const FloatType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.bits} / 8u;                // 8 bits in 1 byte (32 - 4 bytes for float, 64 - 8 bytes for double)
            },
            [&](const EnumRefType& t) -> Result<std::uint64_t> {
                const EnumDef* def = project.find_enum(t.id);
                if (!def) {
                    return Error{"enum reference does not resolve: " + to_string(t.id)};
                }
                return std::uint64_t{def->bits} / 8u;
            },
            [](const BitfieldType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.storage_bits} / 8u;
            },
            [](const Utf8StringType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.length};
            },
            [](const Utf16StringType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.length} * 2u;
            },
            [&](const ClassInstanceType& t) -> Result<std::uint64_t> {
                const ClassDef* def = project.find_class(t.id);
                if (!def) {
                    return Error{"class reference does not resolve: " + to_string(t.id)};
                }
                return class_size_impl(*def, project, visiting);
            },
            [](const VectorType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.components} * 4u;
            },
            [](const MatrixType& t) -> Result<std::uint64_t> {
                return std::uint64_t{t.rows} * t.cols * 4u;
            },
            [](const FunctionAddressType&) -> Result<std::uint64_t> { return kPointerSize; },
            [](const VTableType&) -> Result<std::uint64_t> { return kPointerSize; },
            [](const PointerType&) -> Result<std::uint64_t> { return kPointerSize; },
            [&](const ArrayType& t) -> Result<std::uint64_t> {
                if (!t.element) {
                    return Error{"array has no element type"};
                }
                const auto element_size = type_size_impl(*t.element, project, visiting);
                if (!element_size.ok()) {
                    return element_size;
                }
                if (mul_overflows(element_size.value(), t.count)) {
                    return Error{"array size overflows 64-bit arithmetic"};
                }
                return element_size.value() * t.count;
            },
            [&](const UnionType& t) -> Result<std::uint64_t> {
                // A union fits enough space for its largest member
                std::uint64_t largest = 0;
                for (const FieldDef& member : t.members) {
                    const auto member_size = type_size_impl(member.type, project, visiting);
                    if (!member_size.ok()) {
                        return member_size;
                    }
                    if (add_overflows(member.offset, member_size.value())) {
                        return Error{"union member '" + member.name +
                                     "' overflows 64-bit offset arithmetic"};
                    }
                    largest = std::max(largest, member.offset + member_size.value());
                }
                return largest;
            },
        },
        type.value);
}

} // namespace

Result<std::uint64_t> field_type_size(const FieldType& type, const ProjectModel& project) {
    std::vector<ClassId> visiting;
    return type_size_impl(type, project, visiting);
}

Result<std::uint64_t> class_size(const ClassDef& cls, const ProjectModel& project) {
    std::vector<ClassId> visiting;
    return class_size_impl(cls, project, visiting);
}

} // namespace marrow
