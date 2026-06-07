#include "pulse/json/value.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/stringify.h"

namespace pulse::json {

namespace {

using ::testing::Eq;
using ::testing::IsTrue;
using ::testing::Ne;
using ::testing::StrEq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

TEST(ValueTest, DefaultIsNull) { EXPECT_TRUE(value().is<nullptr_t>()); }

TEST(ValueTest, NullConstruction) {
  EXPECT_TRUE(value(nullptr).is<nullptr_t>());
}

TEST(ValueTest, BoolConstruction) { EXPECT_TRUE(value(true).is<bool>()); }

TEST(ValueTest, IntConstruction) { EXPECT_TRUE(value(42).is<int64_t>()); }

TEST(ValueTest, DoubleConstruction) { EXPECT_TRUE(value(3.14).is<double>()); }

TEST(ValueTest, StringConstruction) {
  EXPECT_TRUE(value(std::string("hello")).is<std::string>());
}

TEST(ValueTest, StringViewConstruction) {
  EXPECT_TRUE(value(std::string_view("hello")).is<std::string>());
}

TEST(ValueTest, CStringConstruction) {
  EXPECT_TRUE(value("hello").is<std::string>());
}

TEST(ValueTest, ArrayConstruction) {
  EXPECT_TRUE(value(array_t{1, 2}).is<array_t>());
}

TEST(ValueTest, ObjectConstruction) {
  EXPECT_TRUE(value(object_t{{"key", "val"}}).is<object_t>());
}

TEST(ValueTest, CopyConstruction) {
  value original = "hello";
  value copy(original);
  EXPECT_THAT(copy, Eq(std::string("hello")));
  EXPECT_THAT(original, Eq(std::string("hello")));
}

TEST(ValueTest, MoveConstruction) {
  EXPECT_THAT(value(value("hello")), Eq(std::string("hello")));
}

TEST(ValueTest, CopyAssignment) {
  value original = "hello";
  value copy;
  copy = original;
  EXPECT_THAT(copy, Eq(std::string("hello")));
  EXPECT_THAT(original, Eq(std::string("hello")));
}

TEST(ValueTest, MoveAssignment) {
  value moved;
  moved = value("hello");
  EXPECT_THAT(moved, Eq(std::string("hello")));
}

TEST(ValueTest, Reassignment) {
  value v = "hello";
  EXPECT_THAT(v.is<std::string>(), IsTrue());
  v = 42;
  EXPECT_THAT(v.is<int64_t>(), IsTrue());
  EXPECT_THAT(v, Eq(42));
  v = true;
  EXPECT_THAT(v.is<bool>(), IsTrue());
  EXPECT_THAT(v, Eq(true));
  v = nullptr;
  EXPECT_THAT(v.is<nullptr_t>(), IsTrue());
}

TEST(ValueTest, ObjectImplicitPromotion) {
  value v;
  v["key"] = "value";
  EXPECT_TRUE(v.is<object_t>());
}

TEST(ValueTest, ObjectAccess) {
  EXPECT_THAT(value(object_t{{"key", "value"}})["key"],
              Eq(std::string("value")));
}

TEST(ValueTest, NestedObject) {
  EXPECT_THAT(
      value(object_t{{"outer", object_t{{"inner", 42}}}})["outer"]["inner"],
      Eq(42));
}

TEST(ValueTest, EqualityNull) { EXPECT_THAT(value(), Eq(nullptr)); }

TEST(ValueTest, EqualityBool) { EXPECT_THAT(value(true), Eq(true)); }

TEST(ValueTest, EqualityInt) { EXPECT_THAT(value(42), Eq(42)); }

TEST(ValueTest, EqualityDouble) { EXPECT_THAT(value(3.14), Eq(3.14)); }

TEST(ValueTest, EqualityString) {
  EXPECT_THAT(value("hello"), Eq(std::string("hello")));
}

TEST(ValueTest, EqualityValue) {
  EXPECT_THAT(value(42), Eq(value(42)));
  EXPECT_THAT(value(42), Ne(value(24)));
}

struct StringifyTestCase {
  std::string_view name;
  value input;
  std::string_view expected;
};

class StringifyTest : public TestWithParam<StringifyTestCase> {};

TEST_P(StringifyTest, Stringify) {
  EXPECT_THAT(pulse::to_string(GetParam().input), StrEq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    ValueTest, StringifyTest,
    ValuesIn<StringifyTestCase>({
        {.name = "Null", .input = value(), .expected = "null"},
        {.name = "BoolTrue", .input = value(true), .expected = "true"},
        {.name = "BoolFalse", .input = value(false), .expected = "false"},
        {.name = "Int", .input = value(42), .expected = "42"},
        {.name = "Double", .input = value(3.14), .expected = "3.14"},
        {.name = "String", .input = value("hello"), .expected = "\"hello\""},
        {.name = "EmptyArray", .input = value(array_t{}), .expected = "[]"},
        {.name = "EmptyObject", .input = value(object_t{}), .expected = "{}"},
        {.name = "Array",
         .input = value(array_t{1, "two", 3.0}),
         .expected = "[1,\"two\",3]"},
        {.name = "AllTypesInArray",
         .input = value(array_t{nullptr, true, 42, 3.14, "hello",
                                array_t{1, "two", 3.0},
                                object_t{{"key", "val"}}}),
         .expected =
             "[null,true,42,3.14,\"hello\",[1,\"two\",3],{\"key\":\"val\"}]"},
        {.name = "Object",
         .input = value(object_t{{"a", 1}, {"b", "two"}}),
         .expected = "{\"a\":1,\"b\":\"two\"}"},
        {.name = "AllTypesInObject",
         .input = value(object_t{{"null", nullptr},
                                 {"bool", true},
                                 {"int", 42},
                                 {"double", 3.14},
                                 {"string", "hello"},
                                 {"array", array_t{1, "two", 3.0}},
                                 {"object", object_t{{"key", "val"}}}}),
         .expected = "{\"array\":[1,\"two\",3],"
                     "\"bool\":true,"
                     "\"double\":3.14,"
                     "\"int\":42,"
                     "\"null\":null,"
                     "\"object\":{\"key\":\"val\"},"
                     "\"string\":\"hello\"}"},
    }),
    [](const TestParamInfo<StringifyTestCase>& info) {
      return std::string(info.param.name);
    });

}  // namespace

}  // namespace pulse::json
