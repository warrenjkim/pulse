#include "pulse/core/stringify.h"

#include "gtest/gtest.h"
#include "pulse/reflect/testdata/types.h"

namespace pulse {

namespace {

TEST(StringifyTest, ToString) {
  EXPECT_EQ(ToString(ns::A{.i = 2, .j = true}), "ns::A{.i=2,.j=true}");
  EXPECT_EQ(ToString(ns::B{.a = {.i = 1, .j = false}}),
            "ns::B{.a=ns::A{.i=1,.j=false}}");
}

TEST(StringifyTest, Stringifiable) {
  static_assert(Stringifiable<ns::A>);
  static_assert(!Stringifiable<ns::D>);
}

}  // namespace

}  // namespace pulse
