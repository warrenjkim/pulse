#include "pulse/reflect/struct_stringify.h"

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/stringify.h"

namespace {

struct Flat {
  int i;
  bool b;
  std::string s;
};

struct Empty {};

struct C {
  int i;
};

struct B {
  C c;
  int j;
};

struct A {
  B b;
  Empty e;
  int k;
};

struct Base {
  int i;
};

struct Derived : Base {
  int j;
};

struct Leaf : Derived {
  int k;
};

struct Only : Base {};

struct Ranges {
  std::vector<int> v;
  std::map<std::string, int> m;
  std::array<int, 2> a;
  int c[2];
};

struct Bits {
  unsigned int i : 1;
  unsigned int j : 4;
};

struct Pointers {
  int* p;
  const char* s;
};

class Private {
 public:
  explicit Private(int i) : i_(i) {}

 private:
  [[maybe_unused]] int i_;
};

struct Public {
  explicit Public(int i) : i(i) {}

  int i;
};

struct Custom {
  int64_t units;
  int32_t nanos;
};

class Opaque {
 public:
  explicit Opaque(int i) : i_(i) {}

 private:
  [[maybe_unused]] int i_;
};

struct Delegates {
  Custom c;
  Opaque o;
};

union U {
  int i;
  float f;
};

}  // namespace

namespace pulse {

template <>
struct Stringify<Custom> {
  static std::string ToString(const Custom&) { return "$1.50"; }
};

template <>
struct Stringify<Opaque> {
  static std::string ToString(const Opaque&) { return "Opaque{...}"; }
};

namespace {

using ::testing::AllOf;
using ::testing::EndsWith;
using ::testing::Eq;
using ::testing::StartsWith;

TEST(StructStringifyTest, AutoStringifiable) {
  static_assert(reflect::internal::AutoStringifiable<Flat>);
  static_assert(reflect::internal::AutoStringifiable<Derived>);
  static_assert(reflect::internal::AutoStringifiable<Private>);
  static_assert(!reflect::internal::AutoStringifiable<int>);
  static_assert(!reflect::internal::AutoStringifiable<U>);
  static_assert(!reflect::internal::AutoStringifiable<std::string>);
  static_assert(!reflect::internal::AutoStringifiable<std::vector<int>>);
  static_assert(!reflect::internal::AutoStringifiable<std::optional<int>>);
}

TEST(StructStringifyTest, Decomposable) {
  static_assert(reflect::internal::Decomposable<Flat>);
  static_assert(reflect::internal::Decomposable<Only>);
  static_assert(reflect::internal::Decomposable<Public>);
  static_assert(!reflect::internal::Decomposable<Derived>);
  static_assert(!reflect::internal::Decomposable<Private>);
}

TEST(StructStringifyTest, Flat) {
  EXPECT_THAT(ToString(Flat{.i = 1, .b = true, .s = "x"}),
              Eq(R"(Flat{.i=1,.b=true,.s="x"})"));
}

TEST(StructStringifyTest, Empty) {
  EXPECT_THAT(ToString(Empty{}), Eq("Empty{}"));
}

TEST(StructStringifyTest, Nested) {
  EXPECT_THAT(ToString(A{.b = {.c = {.i = 1}, .j = 2}, .e = {}, .k = 3}),
              Eq("A{.b=B{.c=C{.i=1},.j=2},.e=Empty{},.k=3}"));
}

TEST(StructStringifyTest, Inheritance) {
  EXPECT_THAT(ToString(Derived{{1}, 2}), Eq("Derived{.i=1,.j=2}"));
  EXPECT_THAT(ToString(Leaf{{{1}, 2}, 3}), Eq("Leaf{.i=1,.j=2,.k=3}"));
  EXPECT_THAT(ToString(Only{{1}}), Eq("Only{.i=1}"));
}

TEST(StructStringifyTest, NonAggregate) {
  EXPECT_THAT(ToString(Private(1)), Eq("Private{.i_=1}"));
  EXPECT_THAT(ToString(Public(1)), Eq("Public{.i=1}"));
}

TEST(StructStringifyTest, Specializations) {
  EXPECT_THAT(ToString(Custom{.units = 1, .nanos = 500000000}), Eq("$1.50"));
  EXPECT_THAT(ToString(Delegates{.c = {}, .o = Opaque(1)}),
              Eq("Delegates{.c=$1.50,.o=Opaque{...}}"));
}

TEST(StructStringifyTest, Ranges) {
  EXPECT_THAT(
      ToString(Ranges{.v = {1, 2}, .m = {{"a", 1}}, .a = {3, 4}, .c = {5, 6}}),
      Eq(R"(Ranges{.v=std::vector<int>{1,2},.m=std::map<std::string,int>{{"a",1}},.a=std::array<int,2>{3,4},.c=int[2]{5,6}})"));
  EXPECT_THAT(ToString(Ranges{}),
              Eq("Ranges{.v=std::vector<int>{},.m=std::map<std::string,int>{},"
                 ".a=std::array<int,2>{0,0},.c=int[2]{0,0}}"));
}

TEST(StructStringifyTest, Bitfields) {
  EXPECT_THAT(ToString(Bits{.i = 1, .j = 4}), Eq("Bits{.i=1,.j=4}"));
}

TEST(StructStringifyTest, Pointers) {
  int i = 1;

  EXPECT_THAT(ToString(Pointers{.p = &i, .s = "x"}),
              AllOf(StartsWith("Pointers{.p=0x"), EndsWith(R"(,.s="x"})")));
}

TEST(StructStringifyTest, InsideContainers) {
  EXPECT_THAT(ToString(std::vector<Flat>{{.i = 1, .b = false, .s = "y"}}),
              Eq(R"(std::vector<Flat>{Flat{.i=1,.b=false,.s="y"}})"));
  EXPECT_THAT(ToString(std::map<std::string, C>{{"a", {.i = 1}}}),
              Eq(R"(std::map<std::string,C>{{"a",C{.i=1}}})"));
}

}  // namespace

}  // namespace pulse
