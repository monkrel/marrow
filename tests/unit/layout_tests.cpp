#include "core/model/layout.hpp"

#include "model_builders.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace marrow;
using namespace marrow::test;

namespace {

// small helper for tests where size calculation is expected to pass
std::uint64_t size_of(const FieldType& type, const ProjectModel& project = {}) {
    const auto result = field_type_size(type, project);
    REQUIRE(result.ok());
    return result.value();
}

} // namespace

TEST_CASE("primitive type sizes", "[layout]") {
    CHECK(size_of(int_t(8)) == 1);
    CHECK(size_of(int_t(16)) == 2);
    CHECK(size_of(int_t(32)) == 4);
    CHECK(size_of(int_t(64, false)) == 8);
    CHECK(size_of(FieldType{BoolType{32}}) == 4);
    CHECK(size_of(floating_t(32)) == 4);
    CHECK(size_of(floating_t(64)) == 8);
    CHECK(size_of(FieldType{BytesType{17}}) == 17);
    CHECK(size_of(FieldType{PaddingType{3}}) == 3);
    CHECK(size_of(FieldType{BitfieldType{32, 4, 9}}) == 4);
    CHECK(size_of(FieldType{VectorType{3}}) == 12);
    CHECK(size_of(FieldType{MatrixType{3, 4}}) == 48);
    CHECK(size_of(FieldType{FunctionAddressType{}}) == 8);
    CHECK(size_of(FieldType{VTableType{12}}) == 8); // only the vtable pointer is stored here
    CHECK(size_of(FieldType{PointerType{}}) == 8);
    CHECK(size_of(FieldType{Utf8StringType{32}}) == 32);
    CHECK(size_of(FieldType{Utf16StringType{16}}) == 32);
}

TEST_CASE("enum reference size comes from the enum definition", "[layout]") {
    ProjectModel project;
    project.enums.push_back(make_enum("Flags", 16));
    const EnumId id = project.enums.front().id;

    CHECK(size_of(FieldType{EnumRefType{id}}, project) == 2);

    const auto missing = field_type_size(FieldType{EnumRefType{EnumId::generate()}}, project);
    REQUIRE_FALSE(missing.ok());
}

TEST_CASE("array sizes multiply and nest", "[layout]") {
    const FieldType array_of_int{ArrayType{box_type(int_t(32)), 10}};
    CHECK(size_of(array_of_int) == 40);

    const FieldType nested{ArrayType{box_type(array_of_int), 3}};
    CHECK(size_of(nested) == 120);

    const auto no_element = field_type_size(FieldType{ArrayType{nullptr, 4}}, {});
    REQUIRE_FALSE(no_element.ok());
}

TEST_CASE("class size is max of declared size and field extent", "[layout]") {
    ProjectModel project;
    ClassDef cls = make_class("Player", {
                                            field("health", 0x10, int_t(32)),
                                            field("position", 0x20, FieldType{VectorType{3}}),
                                        });
    project.classes.push_back(cls);

    const auto derived = class_size(project.classes.front(), project);
    REQUIRE(derived.ok());
    CHECK(derived.value() == 0x2C); // position ends at 0x20 + sizeof(Vector3) 

    project.classes.front().declared_size = 0x100;
    const auto declared = class_size(project.classes.front(), project);
    REQUIRE(declared.ok());
    CHECK(declared.value() == 0x100);
}

TEST_CASE("embedded class contributes its full size", "[layout]") {
    ProjectModel project;
    project.classes.push_back(make_class("Inner", {field("value", 0, int_t(64))}));
    const ClassId inner_id = project.classes.front().id;

    project.classes.push_back(
        make_class("Outer", {field("inner", 8, FieldType{ClassInstanceType{inner_id}})}));

    const auto size = class_size(project.classes.back(), project);
    REQUIRE(size.ok());
    CHECK(size.value() == 16);
}

TEST_CASE("union size is the largest member", "[layout]") {
    UnionType u;
    u.members.push_back(field("as_int", 0, int_t(32)));
    u.members.push_back(field("as_double", 0, floating_t(64)));
    u.members.push_back(field("as_bytes", 0, FieldType{BytesType{3}}));

    CHECK(size_of(FieldType{UnionType{u}}) == 8);
}

TEST_CASE("by value class cycles are an error, pointer cycles are not", "[layout]") {
    ProjectModel project;
    project.classes.push_back(make_class("A"));
    project.classes.push_back(make_class("B"));
    const ClassId a_id = project.classes[0].id;
    const ClassId b_id = project.classes[1].id;

    project.classes[0].fields.push_back(field("b", 0, FieldType{ClassInstanceType{b_id}}));
    project.classes[1].fields.push_back(field("a", 0, FieldType{ClassInstanceType{a_id}}));

    REQUIRE_FALSE(class_size(project.classes[0], project).ok());

    // replacing one by value class with a pointer, what makes the cycle legal
    project.classes[1].fields[0].type = ptr_to_class(a_id);
    const auto size = class_size(project.classes[0], project);
    REQUIRE(size.ok());
    CHECK(size.value() == 8);
}

TEST_CASE("missing class reference is an error", "[layout]") {
    const auto result =
        field_type_size(FieldType{ClassInstanceType{ClassId::generate()}}, {});
    REQUIRE_FALSE(result.ok());
}
