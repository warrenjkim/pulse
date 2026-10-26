#include <string>
#include <string_view>

#include "gtest/gtest.h"
#include "pulse/reflect/mirror.h"

namespace ns {

struct [[pulse::reflect]] A {
  int i;
  bool j;
};

}  // namespace ns

namespace pulse::reflect {

namespace {

TEST(EndToEndTest, Reflected) { static_assert(Reflected<ns::A>); }

TEST(EndToEndTest, FieldCount) { static_assert(kFieldCount<ns::A> == 2); }

TEST(EndToEndTest, ForEachField) {
  static_assert([] {
    std::string names;
    int sum = 0;
    ForEachField(ns::A{.i = 2, .j = true},
                 [&](std::string_view name, const auto& v) {
                   names += name;
                   sum += v;
                 });
    return names == "ij" && sum == 3;
  }());
}

}  // namespace

}  // namespace pulse::reflect
