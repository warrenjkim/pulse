#include "pulse/json/parse.h"

#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/json/value.h"

namespace pulse::json {

namespace {

using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

struct ParseTestCase {
  std::string_view name;
  std::string_view input;
  Result<value> expected;
};

class ParseTest : public TestWithParam<ParseTestCase> {};

TEST_P(ParseTest, Parse) {
  Result<value> result = parse(GetParam().input);
  ASSERT_THAT(result.ok(), Eq(GetParam().expected.ok()))
      << (result.ok() ? "expected error" : result.error().message);
  if (GetParam().expected.ok()) {
    EXPECT_THAT(*result, Eq(*GetParam().expected));
  } else {
    EXPECT_THAT(result.error().code, Eq(GetParam().expected.error().code));
    EXPECT_THAT(result.error().message,
                HasSubstr(GetParam().expected.error().message));
  }
}

INSTANTIATE_TEST_SUITE_P(
    ParserTest, ParseTest,
    ValuesIn<ParseTestCase>({
        {.name = "Null", .input = "null", .expected = value(nullptr)},
        {.name = "BoolTrue", .input = "true", .expected = value(true)},
        {.name = "BoolFalse", .input = "false", .expected = value(false)},
        {.name = "Int", .input = "42", .expected = value(42)},
        {.name = "NegativeInt", .input = "-42", .expected = value(-42)},
        {.name = "Zero", .input = "0", .expected = value(0)},
        {.name = "Double", .input = "3.14", .expected = value(3.14)},
        {.name = "NegativeDouble", .input = "-3.14", .expected = value(-3.14)},
        {.name = "Exponent", .input = "1e2", .expected = value(1e2)},
        {.name = "ExponentPlus", .input = "1e+2", .expected = value(1e2)},
        {.name = "ExponentMinus", .input = "1e-2", .expected = value(1e-2)},
        {.name = "ExponentUpper", .input = "1E2", .expected = value(1e2)},
        {.name = "DoubleWithExponent",
         .input = "1.5e2",
         .expected = value(1.5e2)},
        {.name = "String", .input = R"("hello")", .expected = value("hello")},
        {.name = "EmptyString", .input = R"("")", .expected = value("")},
        {.name = "StringEscapeSequences",
         .input = R"("\"\\\/\n\t\r\b\f\u0041\uD83D\uDE00")",
         .expected = value("\"\\/\n\t\r\b\fA😀")},
        {.name = "EmptyArray", .input = "[]", .expected = value(array_t{})},
        {.name = "SingleElementArray",
         .input = "[1]",
         .expected = value(array_t{1})},
        {.name = "SimpleArray",
         .input = R"([1,"two",3.14,null,true])",
         .expected = value(array_t{1, "two", 3.14, nullptr, true})},
        {.name = "NestedArray",
         .input = "[[1,2],[3,4]]",
         .expected = value(array_t{array_t{1, 2}, array_t{3, 4}})},
        {.name = "EmptyObject", .input = "{}", .expected = value(object_t{})},
        {.name = "SimpleObject",
         .input = R"({"a":1,"b":"two"})",
         .expected = value(object_t{{"a", 1}, {"b", "two"}})},
        {.name = "NestedObject",
         .input = R"({"outer":{"inner":42}})",
         .expected = value(object_t{{"outer", object_t{{"inner", 42}}}})},
        {.name = "ObjectWithArray",
         .input = R"({"arr":[1,2,3]})",
         .expected = value(object_t{{"arr", array_t{1, 2, 3}}})},
        {.name = "ArrayWithObject",
         .input = R"([{"key":"val"}])",
         .expected = value(array_t{object_t{{"key", "val"}}})},
        {.name = "WhitespacePadded", .input = "  42  ", .expected = value(42)},
        {.name = "WhitespaceInArray",
         .input = "[ 1 , 2 , 3 ]",
         .expected = value(array_t{1, 2, 3})},
        {.name = "WhitespaceInObject",
         .input = R"({ "a" : 1 , "b" : 2 })",
         .expected = value(object_t{{"a", 1}, {"b", 2}})},
        {.name = "AllTypes",
         .input =
             R"({"null":null,"bool":true,"int":42,"double":3.14,"string":"hello","array":[1,2],"object":{"key":"val"}})",
         .expected = value(object_t{{"null", nullptr},
                                    {"bool", true},
                                    {"int", 42},
                                    {"double", 3.14},
                                    {"string", "hello"},
                                    {"array", array_t{1, 2}},
                                    {"object", object_t{{"key", "val"}}}})},
        {.name = "InvalidNull",
         .input = "nul",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "null"}},
        {.name = "InvalidTrue",
         .input = "tru",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "true"}},
        {.name = "InvalidFalse",
         .input = "fals",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "false"}},
        {.name = "LeadingZero",
         .input = "01",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "leading zero"}},
        {.name = "TrailingDecimal",
         .input = "1.",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "fraction"}},
        {.name = "InvalidExponent",
         .input = "1e",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "exponent"}},
        {.name = "InvalidExponentSign",
         .input = "1e+",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "exponent"}},
        {.name = "BareNegative",
         .input = "-",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "integer"}},
        {.name = "UnterminatedString",
         .input = R"("hello)",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "unterminated"}},
        {.name = "InvalidEscape",
         .input = R"("\z")",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "control character"}},
        {.name = "InvalidUnicode",
         .input = R"("\u12")",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "control character"}},
        {.name = "LoneHighSurrogate",
         .input = R"("\uD83D")",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "low surrogate"}},
        {.name = "InvalidLowSurrogate",
         .input = R"("\uD83D\u1234")",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "low surrogate"}},
        {.name = "UnterminatedArray",
         .input = "[1,2",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "unterminated"}},
        {.name = "ArrayMissingComma",
         .input = "[1 2]",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "']'"}},
        {.name = "ArrayTrailingComma",
         .input = "[1,]",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "unexpected"}},
        {.name = "ArrayUnexpectedToken",
         .input = "[1:2]",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "']'"}},
        {.name = "UnterminatedObject",
         .input = R"({"a":1)",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "unterminated"}},
        {.name = "ObjectMissingColon",
         .input = R"({"a" 1})",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "':'"}},
        {.name = "ObjectMissingComma",
         .input = R"({"a":1 "b":2})",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "'}'"}},
        {.name = "ObjectTrailingComma",
         .input = R"({"a":1,})",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "string key"}},
        {.name = "ObjectNonStringKey",
         .input = R"({1:"val"})",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "string key"}},
        {.name = "TrailingGarbage",
         .input = "42 x",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "unknown"}},
        {.name = "MultipleValues",
         .input = "1 2",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = "unexpected"}},
        {.name = "EmptyInput",
         .input = "",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = ""}},
        {.name = "WhitespaceOnly",
         .input = "   ",
         .expected = Error{.code = Error::Code::kInvalidArgument,
                           .message = ""}},
    }),
    [](const TestParamInfo<ParseTestCase>& info) {
      return std::string(info.param.name);
    });

}  // namespace

}  // namespace pulse::json
