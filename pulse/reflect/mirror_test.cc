#include "pulse/reflect/mirror.h"

#include <string>
#include <string_view>

#include "gtest/gtest.h"

namespace pulse::reflect {

namespace {

struct A {
  int i;
  bool j;
};

struct B {};

struct I {
  using Type = int;
  static consteval const char* Name() { return "i"; }
  static constexpr const Type& Value(const A& v) { return v.i; }
};

struct J {
  using Type = bool;
  static consteval const char* Name() { return "j"; }
  static constexpr const Type& Value(const A& v) { return v.j; }
};

[[maybe_unused]] consteval auto PulseReflect(const A*) {
  return Mirror<A, I, J>{};
}

TEST(MirrorTest, FieldDescriptor) {
  static_assert(FieldDescriptor<I, A>);
  static_assert(!FieldDescriptor<I, B>);
}

TEST(MirrorTest, Reflected) {
  static_assert(Reflected<A>);
  static_assert(!Reflected<B>);
}

TEST(MirrorTest, FieldCount) { static_assert(kFieldCount<A> == 2); }

template <typename T, typename... D>
concept ValidMirror = requires { typename Mirror<T, D...>; };

TEST(MirrorTest, Mirror) {
  static_assert(ValidMirror<A, I, J>);
  static_assert(!ValidMirror<B, I>);
}

TEST(MirrorTest, FieldName) { static_assert(kFieldName<I> == "i"); }

TEST(MirrorTest, ForEachField) {
  static_assert([]() {
    std::string names;
    int sum = 0;
    ForEachField(A{.i = 2, .j = true},
                 [&](std::string_view name, const auto& v) {
                   names += name;
                   sum += v;
                 });
    return names == "ij" && sum == 3;
  }());
}

}  // namespace

}  // namespace pulse::reflect
