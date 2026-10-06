#include "pulse/core/pretty_function.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "gtest/gtest.h"

namespace pulse {

namespace pretty_function_test {

struct A {};

template <typename T, typename U>
struct B {};

enum class E { kA, kB };

template <E V>
struct C {};

namespace {

struct D {};

}  // namespace

}  // namespace pretty_function_test

namespace {

using pretty_function_test::A;
using pretty_function_test::B;
using pretty_function_test::C;
using pretty_function_test::D;
using pretty_function_test::E;

TEST(TypeNameTest, Alias) {
  static_assert(TypeName<std::string>() == "std::string");
  static_assert(TypeName<std::string_view>() == "std::string_view");
}

TEST(TypeNameTest, Clean) {
  static_assert(TypeName<int>() == "int");
  static_assert(TypeName<A>() == "pulse::pretty_function_test::A");
  static_assert(TypeName<D>() == "pulse::pretty_function_test::D");
  static_assert(TypeName<B<int, bool>>() ==
                "pulse::pretty_function_test::B<int,bool>");
  static_assert(TypeName<B<B<int, bool>, char>>() ==
                "pulse::pretty_function_test::B<pulse::pretty_function_test::"
                "B<int,bool>,char>");
  static_assert(TypeName<C<E::kB>>() ==
                "pulse::pretty_function_test::C<pulse::pretty_function_test::"
                "E::kB>");
}

TEST(TypeNameTest, Declarators) {
  static_assert(TypeName<A&>() == "pulse::pretty_function_test::A&");
  static_assert(TypeName<A&&>() == "pulse::pretty_function_test::A&&");
  static_assert(TypeName<A*>() == "pulse::pretty_function_test::A*");
  static_assert(TypeName<A**>() == "pulse::pretty_function_test::A**");
}

TEST(TypeNameTest, Qualifiers) {
  static_assert(TypeName<const A>() == "const pulse::pretty_function_test::A");
  static_assert(TypeName<volatile A>() ==
                "volatile pulse::pretty_function_test::A");
  static_assert(TypeName<const volatile A>() ==
                "const volatile pulse::pretty_function_test::A");
  static_assert(TypeName<A* const>() ==
                "pulse::pretty_function_test::A* const");
  static_assert(TypeName<A* volatile>() ==
                "pulse::pretty_function_test::A* volatile");
  static_assert(TypeName<A* const volatile>() ==
                "pulse::pretty_function_test::A* const volatile");
  static_assert(TypeName<const A&>() ==
                "const pulse::pretty_function_test::A&");
  static_assert(TypeName<const A*>() ==
                "const pulse::pretty_function_test::A*");
  static_assert(TypeName<const A* const>() ==
                "const pulse::pretty_function_test::A* const");
}

TEST(TypeNameTest, Storage) {
  EXPECT_EQ(TypeName<A>().data(), TypeName<A>().data());
}

TEST(ValueNameTest, Enumerator) {
  static_assert(ValueName<E::kA>() == "pulse::pretty_function_test::E::kA");
  static_assert(ValueName<E::kB>() == "pulse::pretty_function_test::E::kB");
  static_assert(ValueName<static_cast<E>(7)>() ==
                "(pulse::pretty_function_test::E)7");
}

TEST(ValueNameTest, Integral) {
  static_assert(ValueName<42>() == "42");
  static_assert(ValueName<-1>() == "-1");
  static_assert(ValueName<true>() == "true");
  static_assert(ValueName<false>() == "false");
}

}  // namespace

}  // namespace pulse
