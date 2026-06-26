#include "pulse/http/url.h"

#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/error.h"
#include "pulse/core/result.h"

namespace pulse::http {

namespace {

using ::testing::Eq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

struct DecodePercentOkCase {
  std::string_view name;
  std::string_view input;
  std::string_view expected;
};

class DecodePercentOkTest : public TestWithParam<DecodePercentOkCase> {};

TEST_P(DecodePercentOkTest, Decodes) {
  const auto& [name, input, expected] = GetParam();
  Result<std::string> result = DecodePercent(input);
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, Eq(expected));
}

INSTANTIATE_TEST_SUITE_P(
    DecodePercentTest, DecodePercentOkTest,
    ValuesIn<DecodePercentOkCase>({
        {.name = "NoEncoding", .input = "hello", .expected = "hello"},
        {.name = "UppercaseHex",
         .input = "hello%20world",
         .expected = "hello world"},
        {.name = "LowercaseHex",
         .input = "hello%2fworld",
         .expected = "hello/world"},
        {.name = "PlusIsLiteral",
         .input = "hello+world",
         .expected = "hello+world"},
        {.name = "MultipleSequences",
         .input = "foo%20bar%2Fbaz",
         .expected = "foo bar/baz"},
        {.name = "EncodedPercent", .input = "100%25", .expected = "100%"},
        {.name = "Empty", .input = "", .expected = ""},
    }),
    [](const TestParamInfo<DecodePercentOkCase>& info) {
      return std::string(info.param.name);
    });

struct DecodePercentErrorCase {
  std::string_view name;
  std::string_view input;
};

class DecodePercentErrorTest : public TestWithParam<DecodePercentErrorCase> {};

TEST_P(DecodePercentErrorTest, ReturnsError) {
  Result<std::string> result = DecodePercent(GetParam().input);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kInvalidArgument));
}

INSTANTIATE_TEST_SUITE_P(DecodePercentTest, DecodePercentErrorTest,
                         ValuesIn<DecodePercentErrorCase>({
                             {.name = "TruncatedSequence", .input = "foo%2"},
                             {.name = "InvalidHexDigit", .input = "foo%ZZ"},
                             {.name = "TrailingPercent", .input = "foo%"},
                         }),
                         [](const TestParamInfo<DecodePercentErrorCase>& info) {
                           return std::string(info.param.name);
                         });

}  // namespace

}  // namespace pulse::http
