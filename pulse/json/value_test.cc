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

TEST(ValueTest, DefaultIsNull) { EXPECT_TRUE(Value().is<nullptr_t>()); }

TEST(ValueTest, NullConstruction) {
  EXPECT_TRUE(Value(nullptr).is<nullptr_t>());
}

TEST(ValueTest, BoolConstruction) { EXPECT_TRUE(Value(true).is<bool>()); }

TEST(ValueTest, IntConstruction) { EXPECT_TRUE(Value(42).is<int64_t>()); }

TEST(ValueTest, DoubleConstruction) { EXPECT_TRUE(Value(3.14).is<double>()); }

TEST(ValueTest, StringConstruction) {
  EXPECT_TRUE(Value(std::string("hello")).is<std::string>());
}

TEST(ValueTest, StringViewConstruction) {
  EXPECT_TRUE(Value(std::string_view("hello")).is<std::string>());
}

TEST(ValueTest, CStringConstruction) {
  EXPECT_TRUE(Value("hello").is<std::string>());
}

TEST(ValueTest, ArrayConstruction) {
  EXPECT_TRUE(Value(Array{1, 2}).is<Array>());
}

TEST(ValueTest, ObjectConstruction) {
  EXPECT_TRUE(Value(Object{{"key", "val"}}).is<Object>());
}

TEST(ValueTest, CopyConstruction) {
  Value original = "hello";
  Value copy(original);
  EXPECT_THAT(copy, Eq(std::string("hello")));
  EXPECT_THAT(original, Eq(std::string("hello")));
}

TEST(ValueTest, MoveConstruction) {
  EXPECT_THAT(Value(Value("hello")), Eq(std::string("hello")));
}

TEST(ValueTest, CopyAssignment) {
  Value original = "hello";
  Value copy;
  copy = original;
  EXPECT_THAT(copy, Eq(std::string("hello")));
  EXPECT_THAT(original, Eq(std::string("hello")));
}

TEST(ValueTest, MoveAssignment) {
  Value moved;
  moved = Value("hello");
  EXPECT_THAT(moved, Eq(std::string("hello")));
}

TEST(ValueTest, Reassignment) {
  Value v = "hello";
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
  Value v;
  v["key"] = "value";
  EXPECT_TRUE(v.is<Object>());
}

TEST(ValueTest, ObjectAccess) {
  EXPECT_THAT(Value(Object{{"key", "value"}})["key"], Eq(std::string("value")));
}

TEST(ValueTest, NestedObject) {
  EXPECT_THAT(Value(Object{{"outer", Object{{"inner", 42}}}})["outer"]["inner"],
              Eq(42));
}

TEST(ValueTest, EqualityNull) { EXPECT_THAT(Value(), Eq(nullptr)); }

TEST(ValueTest, EqualityBool) { EXPECT_THAT(Value(true), Eq(true)); }

TEST(ValueTest, EqualityInt) { EXPECT_THAT(Value(42), Eq(42)); }

TEST(ValueTest, EqualityDouble) { EXPECT_THAT(Value(3.14), Eq(3.14)); }

TEST(ValueTest, EqualityString) {
  EXPECT_THAT(Value("hello"), Eq(std::string("hello")));
}

TEST(ValueTest, EqualityValue) {
  EXPECT_THAT(Value(42), Eq(Value(42)));
  EXPECT_THAT(Value(42), Ne(Value(24)));
}

struct StringifyTestCase {
  std::string_view name;
  Value input;
  std::string_view expected;
};

class StringifyTest : public TestWithParam<StringifyTestCase> {};

TEST_P(StringifyTest, Stringify) {
  EXPECT_THAT(pulse::ToString(GetParam().input), StrEq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    ValueTest, StringifyTest,
    ValuesIn<StringifyTestCase>({
        {.name = "Null", .input = Value(), .expected = "null"},
        {.name = "BoolTrue", .input = Value(true), .expected = "true"},
        {.name = "BoolFalse", .input = Value(false), .expected = "false"},
        {.name = "Int", .input = Value(42), .expected = "42"},
        {.name = "Double", .input = Value(3.14), .expected = "3.14"},
        {.name = "String", .input = Value("hello"), .expected = "\"hello\""},
        {.name = "EmptyArray", .input = Value(Array{}), .expected = "[]"},
        {.name = "EmptyObject", .input = Value(Object{}), .expected = "{}"},
        {.name = "Array",
         .input = Value(Array{1, "two", 3.0}),
         .expected = "[1,\"two\",3]"},
        {.name = "AllTypesInArray",
         .input = Value(Array{nullptr, true, 42, 3.14, "hello",
                              Array{1, "two", 3.0}, Object{{"key", "val"}}}),
         .expected =
             "[null,true,42,3.14,\"hello\",[1,\"two\",3],{\"key\":\"val\"}]"},
        {.name = "Object",
         .input = Value(Object{{"a", 1}, {"b", "two"}}),
         .expected = "{\"a\":1,\"b\":\"two\"}"},
        {.name = "AllTypesInObject",
         .input = Value(Object{{"null", nullptr},
                               {"bool", true},
                               {"int", 42},
                               {"double", 3.14},
                               {"string", "hello"},
                               {"array", Array{1, "two", 3.0}},
                               {"object", Object{{"key", "val"}}}}),
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
