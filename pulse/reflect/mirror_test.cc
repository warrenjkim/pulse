#include "pulse/reflect/mirror.h"

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

}  // namespace

}  // namespace pulse::reflect
