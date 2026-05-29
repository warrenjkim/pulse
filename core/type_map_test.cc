#include "core/type_map.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse {

namespace {

using ::testing::Eq;

TEST(TypeMapTest, SetAndGet) {
  TypeMap<int, float, std::string> map;
  map.set(42);
  map.set(3.14f);
  map.set<std::string>("hello");

  EXPECT_THAT(map.get<int>(), Eq(42));
  EXPECT_THAT(map.get<float>(), Eq(3.14f));
  EXPECT_THAT(map.get<std::string>(), Eq("hello"));
}

TEST(TypeMapTest, GetDefaultConstructs) {
  TypeMap<int, std::string> map;

  EXPECT_THAT(map.get<int>(), Eq(0));
  EXPECT_THAT(map.get<std::string>(), Eq(""));
}

TEST(TypeMapTest, SetOverwrites) {
  TypeMap<int> map;
  map.set(42);
  map.set(99);

  EXPECT_THAT(map.get<int>(), Eq(99));
}

TEST(TypeMapTest, MutableGet) {
  TypeMap<std::string> map;
  map.set<std::string>("hello");

  map.get<std::string>() += " world";

  EXPECT_THAT(map.get<std::string>(), Eq("hello world"));
}

TEST(TypeMapTest, ConstGet) {
  TypeMap<int> map;
  map.set(42);

  const TypeMap<int>& const_map = map;
  EXPECT_THAT(const_map.get<int>(), Eq(42));
}

TEST(TypeMapTest, PointerType) {
  int value = 42;
  TypeMap<int*> map;
  map.set(&value);

  EXPECT_THAT(map.get<int*>(), Eq(&value));
  EXPECT_THAT(*map.get<int*>(), Eq(42));
}

}  // namespace

}  // namespace pulse
