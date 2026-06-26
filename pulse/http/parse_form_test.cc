#include "pulse/http/parse_form.h"

#include <string>
#include <string_view>
#include <unordered_map>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/http/parameters.h"

namespace pulse::http {

namespace {

using ::testing::Eq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

struct ParseFormErrorCase {
  std::string_view name;
  std::string_view body;
};

class ParseFormErrorTest : public TestWithParam<ParseFormErrorCase> {};

TEST_P(ParseFormErrorTest, ReturnsError) {
  Result<Parameters> result = ParseForm(GetParam().body);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kInvalidArgument));
}

INSTANTIATE_TEST_SUITE_P(
    ParseFormTest, ParseFormErrorTest,
    ValuesIn<ParseFormErrorCase>(
        {{.name = "MissingEquals", .body = "nameonly"},
         {.name = "TruncatedPercentSequence", .body = "name=foo%2"},
         {.name = "InvalidHexDigit", .body = "name=foo%ZZ"}}),
    [](const TestParamInfo<ParseFormErrorCase>& info) {
      return std::string(info.param.name);
    });

struct ParseFormCase {
  std::string_view name;
  std::string_view body;
  std::unordered_map<std::string_view, std::string_view> expected;
};

class ParseFormOkTest : public TestWithParam<ParseFormCase> {};

TEST_P(ParseFormOkTest, Parses) {
  const auto& [name, body, expected] = GetParam();
  Result<Parameters> result = ParseForm(body);
  ASSERT_TRUE(result.ok());
  for (const auto& [key, value] : expected) {
    EXPECT_THAT(*(result->Get<std::string>(key)), Eq(value));
  }
}

INSTANTIATE_TEST_SUITE_P(
    ParseFormTest, ParseFormOkTest,
    ValuesIn<ParseFormCase>(
        {{.name = "EmptyBody", .body = "", .expected = {}},
         {.name = "SinglePair",
          .body = "name=foo",
          .expected = {{"name", "foo"}}},
         {.name = "MultiplePairs",
          .body = "name=foo&type=bar",
          .expected = {{"name", "foo"}, {"type", "bar"}}},
         {.name = "EmptyValue", .body = "name=", .expected = {{"name", ""}}},
         {.name = "EmptyPairsSkipped",
          .body = "name=foo&&type=bar&",
          .expected = {{"name", "foo"}, {"type", "bar"}}},
         {.name = "PlusAsSpace",
          .body = "name=hello+world",
          .expected = {{"name", "hello world"}}},
         {.name = "UppercaseHex",
          .body = "name=hello%20world",
          .expected = {{"name", "hello world"}}},
         {.name = "LowercaseHex",
          .body = "name=hello%2fworld",
          .expected = {{"name", "hello/world"}}},
         {.name = "EncodedKey",
          .body = "na%6De=foo",
          .expected = {{"name", "foo"}}},
         {.name = "LiteralPlus",
          .body = "name=foo%2Bbar",
          .expected = {{"name", "foo+bar"}}},
         {.name = "EncodedAmpersandInValue",
          .body = "name=foo%26bar&type=baz",
          .expected = {{"name", "foo&bar"}, {"type", "baz"}}}}),
    [](const TestParamInfo<ParseFormCase>& info) {
      return std::string(info.param.name);
    });

}  // namespace

}  // namespace pulse::http
