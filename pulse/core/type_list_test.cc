#include "pulse/core/type_list.h"

#include <type_traits>

#include "gtest/gtest.h"

namespace pulse {

namespace {

struct A {};
struct B {};
struct C {};
struct D {};

TEST(TypeListTest, FlattenFlatList) {
  static_assert(
      std::is_same_v<Flatten<TypeList<>, A, B, C>::Type, TypeList<A, B, C>>);
}

TEST(TypeListTest, FlattenEmpty) {
  static_assert(std::is_same_v<Flatten<TypeList<>>::Type, TypeList<>>);
}

TEST(TypeListTest, FlattenSingleNestedList) {
  static_assert(std::is_same_v<Flatten<TypeList<>, TypeList<A, B>>::Type,
                               TypeList<A, B>>);
}

TEST(TypeListTest, FlattenMixedLeavesAndNesting) {
  static_assert(std::is_same_v<Flatten<TypeList<>, A, TypeList<B, C>, D>::Type,
                               TypeList<A, B, C, D>>);
}

TEST(TypeListTest, FlattenDeeplyNested) {
  static_assert(
      std::is_same_v<
          Flatten<TypeList<>, TypeList<A, TypeList<B, TypeList<C, D>>>>::Type,
          TypeList<A, B, C, D>>);
}

TEST(TypeListTest, FlattenPreservesOrder) {
  static_assert(std::is_same_v<Flatten<TypeList<>, D, TypeList<C, B>, A>::Type,
                               TypeList<D, C, B, A>>);
}

}  // namespace

}  // namespace pulse
