#include "pulse/core/demangle.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/result.h"

namespace pulse {

namespace demangle_test {

struct Widget {};

enum class Enum { kOne, kTwo, kThree };

template <Enum E>
struct Tagged {};

namespace {

using ::testing::Eq;
using ::testing::StrEq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

TEST(DemangleTest, GrammarFixturesAreWhatTheCompilerActuallyEmits) {
  EXPECT_THAT(typeid(Widget).name(), StrEq("N5pulse13demangle_test6WidgetE"));
  EXPECT_THAT(typeid(Result<int>).name(), StrEq("N5pulse6ResultIiEE"));
  EXPECT_THAT(typeid(Result<Widget>).name(),
              StrEq("N5pulse6ResultINS_13demangle_test6WidgetEEE"));
  EXPECT_THAT(typeid(Tagged<Enum::kThree>).name(),
              StrEq("N5pulse13demangle_test6TaggedILNS0_4EnumE2EEE"));
}

TEST(DemangleTest, NullInputYieldsEmptyStringRatherThanDereferencing) {
  EXPECT_THAT(Demangle(nullptr), Eq(""));
}

TEST(DemangleTest, StripsImplementationInlineNamespaces) {
  EXPECT_THAT(Demangle(typeid(std::vector<int>).name()),
              Eq("std::vector<int, std::allocator<int>>"));
}

struct TypeNameCase {
  std::string name;
  const std::string& (*type_name)();
  std::string expected;
};

class TypeNameTest : public TestWithParam<TypeNameCase> {};

TEST_P(TypeNameTest, Spelling) {
  EXPECT_THAT(GetParam().type_name(), Eq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    Qualifiers, TypeNameTest,
    ValuesIn<TypeNameCase>({
        {.name = "Plain",
         .type_name = &TypeName<Widget>,
         .expected = "pulse::demangle_test::Widget"},
        {.name = "Builtin", .type_name = &TypeName<int>, .expected = "int"},
        {.name = "Template",
         .type_name = &TypeName<Result<Widget>>,
         .expected = "pulse::Result<pulse::demangle_test::Widget>"},
        {.name = "Const",
         .type_name = &TypeName<const Widget>,
         .expected = "pulse::demangle_test::Widget const"},
        {.name = "Volatile",
         .type_name = &TypeName<volatile Widget>,
         .expected = "pulse::demangle_test::Widget volatile"},
        {.name = "ConstVolatile",
         .type_name = &TypeName<const volatile Widget>,
         .expected = "pulse::demangle_test::Widget const volatile"},
        {.name = "LvalueRef",
         .type_name = &TypeName<Widget&>,
         .expected = "pulse::demangle_test::Widget&"},
        {.name = "RvalueRef",
         .type_name = &TypeName<Widget&&>,
         .expected = "pulse::demangle_test::Widget&&"},
        {.name = "ConstRef",
         .type_name = &TypeName<const Widget&>,
         .expected = "pulse::demangle_test::Widget const&"},
        {.name = "PointerToConst",
         .type_name = &TypeName<const Widget*>,
         .expected = "pulse::demangle_test::Widget const*"},
        {.name = "ConstPointer",
         .type_name = &TypeName<Widget* const>,
         .expected = "pulse::demangle_test::Widget* const"},
    }),
    [](const TestParamInfo<TypeNameCase>& info) { return info.param.name; });

TEST(TypeNameTest, UsesCanonicalAliases) {
  EXPECT_THAT(TypeName<std::string>(), Eq("std::string"));
  EXPECT_THAT(TypeName<std::string_view>(), Eq("std::string_view"));
  EXPECT_THAT(TypeName<uint8_t>(), Eq("uint8_t"));
  EXPECT_THAT(TypeName<uint16_t>(), Eq("uint16_t"));
  EXPECT_THAT(TypeName<uint32_t>(), Eq("uint32_t"));
  EXPECT_THAT(TypeName<uint64_t>(), Eq("uint64_t"));
  EXPECT_THAT(TypeName<int8_t>(), Eq("int8_t"));
  EXPECT_THAT(TypeName<int16_t>(), Eq("int16_t"));
  EXPECT_THAT(TypeName<int64_t>(), Eq("int64_t"));
  EXPECT_THAT(TypeName<size_t>(), Eq("size_t"));
}

TEST(TypeNameTest, PreservesQualifiersOnAliasedTypes) {
  EXPECT_THAT(TypeName<const std::string>(), Eq("std::string const"));
  EXPECT_THAT(TypeName<std::string&>(), Eq("std::string&"));
  EXPECT_THAT(TypeName<std::string_view>(), Eq("std::string_view"));
}

TEST(TypeNameTest, DemanglesOncePerInstantiation) {
  EXPECT_EQ(&TypeName<Widget>(), &TypeName<Widget>());
}

}  // namespace

}  // namespace demangle_test

}  // namespace pulse
