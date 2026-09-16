#include "pulse/core/pretty_function.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse {

namespace pretty_function_test {

struct Struct {};

template <typename A, typename B>
struct Two {};

enum class Enum { kOne, kTwo };

template <Enum E>
struct Tagged {};

namespace {

struct Anonymous {};

}  // namespace

}  // namespace pretty_function_test

namespace {

using ::testing::Eq;

TEST(TypeNameTest, PlainType) {
  static_assert(TypeName<int>() == "int");
  static_assert(TypeName<pretty_function_test::Struct>() ==
                "pulse::pretty_function_test::Struct");
}

TEST(TypeNameTest, Alias) {
  static_assert(TypeName<std::string>() == "std::string");
  static_assert(TypeName<std::string_view>() == "std::string_view");
  static_assert(TypeName<uint8_t>() == "uint8_t");
  static_assert(TypeName<uint16_t>() == "uint16_t");
  static_assert(TypeName<uint32_t>() == "uint32_t");
  static_assert(TypeName<uint64_t>() == "uint64_t");
  static_assert(TypeName<int8_t>() == "int8_t");
  static_assert(TypeName<int16_t>() == "int16_t");
  static_assert(TypeName<int64_t>() == "int64_t");
  static_assert(TypeName<size_t>() == "size_t");
}

TEST(TypeNameTest, AnonymousNamespace) {
  static_assert(TypeName<pretty_function_test::Anonymous>() ==
                "pulse::pretty_function_test::Anonymous");
}

TEST(TypeNameTest, CommaSeparatedTemplateArguments) {
  static_assert(TypeName<pretty_function_test::Two<int, bool>>() ==
                "pulse::pretty_function_test::Two<int,bool>");
  static_assert(
      TypeName<pretty_function_test::Two<pretty_function_test::Two<int, bool>,
                                         char>>() ==
      "pulse::pretty_function_test::Two<pulse::pretty_function_test::"
      "Two<int,bool>,char>");
}

TEST(TypeNameTest, EnumTemplateArgument) {
  static_assert(
      TypeName<
          pretty_function_test::Tagged<pretty_function_test::Enum::kTwo>>() ==
      R"(pulse::pretty_function_test::Tagged<pulse::pretty_function_test::Enum::kTwo>)");
}

TEST(TypeNameTest, Reference) {
  static_assert(TypeName<pretty_function_test::Struct&>() ==
                "pulse::pretty_function_test::Struct&");
  static_assert(TypeName<pretty_function_test::Struct&&>() ==
                "pulse::pretty_function_test::Struct&&");
}

TEST(TypeNameTest, Pointer) {
  static_assert(TypeName<pretty_function_test::Struct*>() ==
                "pulse::pretty_function_test::Struct*");
  static_assert(TypeName<pretty_function_test::Struct**>() ==
                "pulse::pretty_function_test::Struct**");
}

TEST(TypeNameTest, LeadingQualifier) {
  static_assert(TypeName<const pretty_function_test::Struct>() ==
                "const pulse::pretty_function_test::Struct");
  static_assert(TypeName<volatile pretty_function_test::Struct>() ==
                "volatile pulse::pretty_function_test::Struct");
  static_assert(TypeName<const volatile pretty_function_test::Struct>() ==
                "const volatile pulse::pretty_function_test::Struct");
}

TEST(TypeNameTest, QualifierAfterDeclarator) {
  static_assert(TypeName<pretty_function_test::Struct* const>() ==
                "pulse::pretty_function_test::Struct* const");
  static_assert(TypeName<pretty_function_test::Struct* volatile>() ==
                "pulse::pretty_function_test::Struct* volatile");
  static_assert(TypeName<pretty_function_test::Struct* const volatile>() ==
                "pulse::pretty_function_test::Struct* const volatile");
}

TEST(TypeNameTest, QualifierBeforeDeclarator) {
  static_assert(TypeName<const pretty_function_test::Struct&>() ==
                "const pulse::pretty_function_test::Struct&");
  static_assert(TypeName<const pretty_function_test::Struct*>() ==
                "const pulse::pretty_function_test::Struct*");
  static_assert(TypeName<const pretty_function_test::Struct* const>() ==
                "const pulse::pretty_function_test::Struct* const");
}

TEST(TypeNameTest, SameStorageEachCall) {
  EXPECT_EQ(TypeName<pretty_function_test::Struct>().data(),
            TypeName<pretty_function_test::Struct>().data());
}

TEST(TypeNameTest, UsableAtRuntime) {
  EXPECT_THAT(std::string(TypeName<pretty_function_test::Struct>()),
              Eq("pulse::pretty_function_test::Struct"));
}

TEST(ValueNameTest, Enumerator) {
  static_assert(ValueName<pretty_function_test::Enum::kOne>() ==
                "pulse::pretty_function_test::Enum::kOne");
  static_assert(ValueName<pretty_function_test::Enum::kTwo>() ==
                "pulse::pretty_function_test::Enum::kTwo");
}

TEST(ValueNameTest, Integer) {
  static_assert(ValueName<42>() == "42");
  static_assert(ValueName<-1>() == "-1");
}

TEST(ValueNameTest, Bool) {
  static_assert(ValueName<true>() == "true");
  static_assert(ValueName<false>() == "false");
}

TEST(ValueNameTest, NoMatchingEnumerator) {
  static_assert(ValueName<static_cast<pretty_function_test::Enum>(7)>().find(
                    '7') != std::string_view::npos);
}

}  // namespace

}  // namespace pulse
