#include "pulse/core/stringify.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse {

namespace {

using ::testing::Eq;
using ::testing::MatchesRegex;

inline constexpr std::string_view kAddress = R"re(0x[0-9a-f]+)re";

enum UnscopedEnum { kFirst, kSecond };

enum class ScopedEnum { kFirst, kSecond };

struct Opaque {};

struct WithDebugString {};

struct WithoutDebugString {};

}  // namespace

template <>
struct Stringify<pulse::WithDebugString> {
  static std::string ToString(const WithDebugString&) { return "plain"; }
  static std::string DebugString(const WithDebugString&) { return "debug"; }
};

template <>
struct Stringify<pulse::WithoutDebugString> {
  static std::string ToString(const WithoutDebugString&) { return "plain"; }
};

namespace {

TEST(StringifiableTest, HoldsForSpecializedTypes) {
  static_assert(Stringifiable<int>);
  static_assert(Stringifiable<double>);
  static_assert(Stringifiable<bool>);
  static_assert(Stringifiable<char>);
  static_assert(Stringifiable<std::string>);
  static_assert(Stringifiable<char*>);
  static_assert(Stringifiable<const char*>);
  static_assert(Stringifiable<void*>);
  static_assert(Stringifiable<const void*>);
}

TEST(StringifiableTest, FailsForUnspecializedTypes) {
  static_assert(!Stringifiable<Opaque>);
}

TEST(StringifiableTest, HoldsForStringLiterals) {
  static_assert(Stringifiable<char[3]>);
  static_assert(Stringifiable<const char[3]>);
}

TEST(StdToStringableTest, ExcludesEnums) {
  static_assert(!StdToStringable<UnscopedEnum>);
  static_assert(!StdToStringable<ScopedEnum>);
  static_assert(!Stringifiable<UnscopedEnum>);
  static_assert(!Stringifiable<ScopedEnum>);
}

TEST(StdToStringableTest, ExcludesBoolAndCharacterTypes) {
  static_assert(!StdToStringable<bool>);
  static_assert(!StdToStringable<char>);
  static_assert(!StdToStringable<wchar_t>);
  static_assert(!StdToStringable<char8_t>);
  static_assert(!StdToStringable<char16_t>);
  static_assert(!StdToStringable<char32_t>);
}

TEST(StdToStringableTest, IncludesNarrowSignedAndUnsignedChar) {
  static_assert(StdToStringable<signed char>);
  static_assert(StdToStringable<unsigned char>);
  static_assert(std::is_same_v<std::uint8_t, unsigned char>);
}

TEST(StdToStringableTest, IncludesTheUsualArithmeticTypes) {
  static_assert(StdToStringable<short>);
  static_assert(StdToStringable<int>);
  static_assert(StdToStringable<long long>);
  static_assert(StdToStringable<unsigned>);
  static_assert(StdToStringable<float>);
  static_assert(StdToStringable<double>);
}

TEST(StringifyTest, PrintsBool) {
  EXPECT_THAT(ToString(true), Eq("true"));
  EXPECT_THAT(ToString(false), Eq("false"));
}

TEST(StringifyTest, StdToStringable_IntegerUsesStdToString) {
  EXPECT_THAT(ToString(42), Eq("42"));
  EXPECT_THAT(ToString(-42), Eq("-42"));
}

TEST(StringifyTest, StdToStringable_FloatingPointUsesStdToString) {
  EXPECT_THAT(ToString(1.5), Eq(std::to_string(1.5)));
}

TEST(StringifyTest, NarrowCharTypesPrintAsNumbers) {
  EXPECT_THAT(ToString(static_cast<std::uint8_t>(42)), Eq("42"));
  EXPECT_THAT(ToString(static_cast<signed char>(-42)), Eq("-42"));
}

TEST(StringifyTest, CharIsQuotedNotNumeric) {
  EXPECT_THAT(ToString('a'), Eq("\"a\""));
}

TEST(StringifyTest, CharEscapesQuotesAndBackslashes) {
  EXPECT_THAT(ToString('"'), Eq(R"("\"")"));
  EXPECT_THAT(ToString('\\'), Eq(R"("\\")"));
}

TEST(StringifyTest, StringIsQuoted) {
  EXPECT_THAT(ToString(std::string("hi")), Eq("\"hi\""));
}

TEST(StringifyTest, EmptyStringIsQuoted) {
  EXPECT_THAT(ToString(std::string()), Eq("\"\""));
}

TEST(StringifyTest, StringEscapesQuotesAndBackslashes) {
  EXPECT_THAT(ToString(std::string(R"(a"b\c)")), Eq(R"("a\"b\\c")"));
}

TEST(StringifyTest, StringWithEmbeddedNullKeepsBothHalves) {
  EXPECT_THAT(ToString(std::string("a\0b", 3)), Eq(std::string("\"a\0b\"", 5)));
}

TEST(StringifyTest, StringLiteralIsQuotedLikeAString) {
  EXPECT_THAT(ToString("hi"), Eq("\"hi\""));
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

TEST(StringifyTest, CharPointerEscapesLikeAString) {
  const char* value = R"(a"b\c)";
  EXPECT_THAT(ToString(value), Eq(R"("a\"b\\c")"));
}

TEST(StringifyTest, NullCharPointerPrintsNullptr) {
  EXPECT_THAT(ToString(static_cast<const char*>(nullptr)), Eq("nullptr"));
  EXPECT_THAT(ToString(static_cast<char*>(nullptr)), Eq("nullptr"));
}

TEST(StringifyTest, ConstVoidPointerPrintsHexAddress) {
  int value = 0;
  EXPECT_THAT(ToString(static_cast<const void*>(&value)),
              MatchesRegex(kAddress));
}

TEST(StringifyTest, MutableVoidPointerPrintsHexAddress) {
  int value = 0;
  EXPECT_THAT(ToString(static_cast<void*>(&value)), MatchesRegex(kAddress));
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

TEST(StringifyTest, DistinctAddressesRenderDistinctly) {
  int first = 0;
  int second = 0;
  EXPECT_THAT(ToString(static_cast<const void*>(&first)),
              testing::Ne(ToString(static_cast<const void*>(&second))));
}

TEST(StringifyTest, TypedPointersAreNotStringifiable) {
  static_assert(!Stringifiable<int*>);
  static_assert(!Stringifiable<Opaque*>);
}

TEST(DebugStringTest, UsesDebugStringWhenPresent) {
  static_assert(DebugStringifiable<WithDebugString>);
  EXPECT_THAT(DebugString(WithDebugString{}), Eq("debug"));
}

TEST(DebugStringTest, FallsBackToToStringWhenAbsent) {
  static_assert(!DebugStringifiable<WithoutDebugString>);
  EXPECT_THAT(DebugString(WithoutDebugString{}), Eq("plain"));
}

TEST(DebugStringTest, FallsBackForBuiltInSpecializations) {
  static_assert(!DebugStringifiable<int>);
  EXPECT_THAT(DebugString(42), Eq("42"));
  EXPECT_THAT(DebugString(std::string("hi")), Eq("\"hi\""));
  EXPECT_THAT(DebugString(true), Eq("true"));
}

}  // namespace

}  // namespace pulse
