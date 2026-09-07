#include "pulse/core/stringify.h"

#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse {

namespace {

using ::testing::Eq;
using ::testing::MatchesRegex;

inline constexpr std::string_view kAddress = R"re(0x[0-9a-f]+)re";

TEST(StringifyTest, PrintsBool) {
  EXPECT_THAT(ToString(true), Eq("true"));
  EXPECT_THAT(ToString(false), Eq("false"));
}

TEST(StringifyTest, StdToStringable_IntegerUsesStdToString) {
  EXPECT_THAT(ToString(42), Eq("42"));
}

TEST(StringifyTest, StdToStringable_FloatingPointUsesStdToString) {
  EXPECT_THAT(ToString(1.5), Eq(std::to_string(1.5)));
}

TEST(StringifyTest, StringIsQuoted) {
  EXPECT_THAT(ToString(std::string("hi")), Eq("\"hi\""));
}

TEST(StringifyTest, StringEscapesQuotesAndBackslashes) {
  EXPECT_THAT(ToString(std::string(R"(a"b\c)")), Eq(R"("a\"b\\c")"));
}

TEST(StringifyTest, CharPointerIsQuotedLikeAString) {
  char literal[] = "hi";
  char* value = literal;
  EXPECT_THAT(ToString(value), Eq("\"hi\""));
}

TEST(StringifyTest, ConstCharPointerIsQuotedLikeAString) {
  const char* value = "hi";
  EXPECT_THAT(ToString(value), Eq("\"hi\""));
}

TEST(StringifyTest, ConstVoidPointerPrintsHexAddress) {
  int value = 0;
  EXPECT_THAT(ToString(static_cast<const void*>(&value)),
              MatchesRegex(kAddress));
}

TEST(StringifyTest, ConstVoidPointerNullPrintsZeroAddress) {
  EXPECT_THAT(ToString(static_cast<const void*>(nullptr)),
              MatchesRegex(kAddress));
}

TEST(StringifyTest, ConstVoidPointerReflectsTheActualAddress) {
  int value = 0;
  const void* ptr = &value;

  EXPECT_THAT(ToString(ptr), Eq(Stringify<const void*>::ToString(ptr)));
}

}  // namespace

}  // namespace pulse
