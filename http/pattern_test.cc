#include "http/pattern.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "dsa/result.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse::http {

namespace {

using ::testing::Eq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

struct MakeOkTestCase {
  std::string_view name;
  std::string_view pattern;
  size_t segments;
  int captures;
};

class MakeOkTest : public TestWithParam<MakeOkTestCase> {};

TEST_P(MakeOkTest, MakeOk) {
  const MakeOkTestCase& params = GetParam();
  Result<Pattern> result = Pattern::Make(params.pattern);
  ASSERT_TRUE(result.ok()) << result.error().message;
  EXPECT_THAT(result->segments(), Eq(params.segments));
  EXPECT_THAT(result->captures(), Eq(params.captures));
}

INSTANTIATE_TEST_SUITE_P(
    PatternTest, MakeOkTest,
    ValuesIn<MakeOkTestCase>({
        {.name = "LiteralOnly",
         .pattern = "/health",
         .segments = 2,
         .captures = 0},
        {.name = "SingleCapture",
         .pattern = "/accounts/{id}",
         .segments = 3,
         .captures = 1},
        {.name = "MultipleCaptures",
         .pattern = "/users/{user_id}/posts/{post_id}",
         .segments = 5,
         .captures = 2},
        {.name = "RootPattern", .pattern = "/", .segments = 2, .captures = 0},
    }),
    [](const TestParamInfo<MakeOkTestCase>& info) {
      return std::string(info.param.name);
    });

struct MakeErrorTestCase {
  std::string_view name;
  std::string_view pattern;
};

class MakeErrorTest : public TestWithParam<MakeErrorTestCase> {};

TEST_P(MakeErrorTest, MakeError) {
  Result<Pattern> result = Pattern::Make(GetParam().pattern);
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kInvalidArgument));
}

INSTANTIATE_TEST_SUITE_P(
    PatternTest, MakeErrorTest,
    ValuesIn<MakeErrorTestCase>({
        {.name = "EmptyPattern", .pattern = ""},
        {.name = "MissingLeadingSlash", .pattern = "accounts/{id}"},
        {.name = "UnclosedCapture", .pattern = "/accounts/{id"},
        {.name = "StrayClosingBrace", .pattern = "/accounts/id}"},
        {.name = "NestedBraces", .pattern = "/accounts/{i{d}}"},
        {.name = "DuplicateCaptureNames", .pattern = "/users/{id}/posts/{id}"},
        {.name = "EmptyBraces", .pattern = "/accounts/{}"},
    }),
    [](const TestParamInfo<MakeErrorTestCase>& info) {
      return std::string(info.param.name);
    });

struct MatchOkTestCase {
  std::string_view name;
  std::string_view pattern;
  std::string_view path;
  Pattern::Captures expected;
};

class MatchOkTest : public TestWithParam<MatchOkTestCase> {};

TEST_P(MatchOkTest, Matches) {
  const MatchOkTestCase& params = GetParam();
  Result<Pattern> result = Pattern::Make(params.pattern);
  ASSERT_TRUE(result.ok()) << result.error().message;

  std::optional<Pattern::Captures> captures = result->match(params.path);
  ASSERT_TRUE(captures.has_value());
  EXPECT_THAT(*captures, Eq(params.expected));
}

INSTANTIATE_TEST_SUITE_P(
    PatternTest, MatchOkTest,
    ValuesIn<MatchOkTestCase>({
        {.name = "LiteralExact",
         .pattern = "/health",
         .path = "/health",
         .expected = {}},
        {.name = "SingleCapture",
         .pattern = "/accounts/{id}",
         .path = "/accounts/42",
         .expected = {{"id", "42"}}},
        {.name = "NonNumericSegment",
         .pattern = "/accounts/{id}",
         .path = "/accounts/checking",
         .expected = {{"id", "checking"}}},
        {.name = "MultipleCaptures",
         .pattern = "/users/{user_id}/posts/{post_id}",
         .path = "/users/42/posts/7",
         .expected = {{"user_id", "42"}, {"post_id", "7"}}},
        {.name = "MixedLiteralAndCapture",
         .pattern = "/accounts/{id}/transactions",
         .path = "/accounts/42/transactions",
         .expected = {{"id", "42"}}},
        {.name = "RootPattern", .pattern = "/", .path = "/", .expected = {}},
    }),
    [](const TestParamInfo<MatchOkTestCase>& info) {
      return std::string(info.param.name);
    });

struct MatchFailTestCase {
  std::string_view name;
  std::string_view pattern;
  std::string_view path;
};

class MatchFailTest : public TestWithParam<MatchFailTestCase> {};

TEST_P(MatchFailTest, DoesNotMatch) {
  const MatchFailTestCase& params = GetParam();
  Result<Pattern> result = Pattern::Make(params.pattern);
  ASSERT_TRUE(result.ok()) << result.error().message;
  EXPECT_FALSE(result->match(params.path).has_value());
}

INSTANTIATE_TEST_SUITE_P(
    PatternTest, MatchFailTest,
    ValuesIn<MatchFailTestCase>({
        {.name = "LiteralMismatch", .pattern = "/health", .path = "/status"},
        {.name = "LiteralMismatchPrefix",
         .pattern = "/health",
         .path = "/healthy"},
        {.name = "TooFewSegments",
         .pattern = "/accounts/{id}",
         .path = "/accounts"},
        {.name = "TooManySegments",
         .pattern = "/accounts/{id}",
         .path = "/accounts/42/transactions"},
        {.name = "TrailingSlashStrict",
         .pattern = "/accounts/{id}",
         .path = "/accounts/42/"},
        {.name = "MissingLeadingSlash",
         .pattern = "/accounts/{id}",
         .path = "accounts/42"},
        {.name = "WrongLiteralAfterCapture",
         .pattern = "/accounts/{id}/transactions",
         .path = "/accounts/42/balance"},
        {.name = "EmptyPath", .pattern = "/health", .path = ""},
    }),
    [](const TestParamInfo<MatchFailTestCase>& info) {
      return std::string(info.param.name);
    });

}  // namespace

}  // namespace pulse::http
