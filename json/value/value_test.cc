#include "pulse/json/value.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse {
namespace json {

namespace {

using ::testing::DoubleEq;
using ::testing::Eq;
using ::testing::Throws;

TEST(ValueTest, DefaultConstructor) {
  { EXPECT_THAT(Value(), Eq(nullptr)); }
  {
    Value v;
    EXPECT_THAT(v, Eq(nullptr));
  }
}

TEST(ValueTest, NullConstructor) {
  { EXPECT_THAT(Value(nullptr), Eq(nullptr)); }
  {
    Value v = nullptr;
    EXPECT_THAT(v, Eq(nullptr));
  }
}

TEST(ValueTest, BoolConstructor) {
  { EXPECT_THAT(Value(true), Eq(true)); }
  {
    Value v = true;
    EXPECT_THAT(v, Eq(true));
  }
}

TEST(ValueTest, IntConstructor) {
  { EXPECT_THAT(Value(42), Eq(42)); }
  {
    Value v = 42;
    EXPECT_THAT(v, Eq(42));
  }
}

TEST(ValueTest, DoubleConstructor) {
  { EXPECT_THAT((double)Value(1.23), DoubleEq(1.23)); }
  {
    Value v = 1.23;
    EXPECT_THAT((double)v, DoubleEq(1.23));
  }
}

TEST(ValueTest, ArrayConstructor) {
  array_t arr{nullptr, true, 42, 1.23, array_t{}, object_t{}};
  {
    Value v(arr);
    EXPECT_THAT(v, Eq(arr));
  }
  {
    Value v = arr;
    EXPECT_THAT(v, Eq(arr));
  }
}

TEST(ValueTest, ObjectConstructor) {
  object_t map{{"null", nullptr}, {"bool", true},       {"int32_t", 42},
               {"double", 1.23},  {"array", array_t{}}, {"map", object_t{}}};
  {
    Value v(map);
    EXPECT_THAT(v, Eq(map));
  }
  {
    Value v = map;
    EXPECT_THAT(v, Eq(map));
  }
}

TEST(ValueTest, CopyConstructor) {
  Value original = "test";
  Value copy(original);

  EXPECT_THAT(copy, Eq("test"));
  EXPECT_THAT(original, Eq("test"));
}

TEST(ValueTest, CopyAssignment) {
  Value original = "test";
  Value copy;
  copy = original;

  EXPECT_THAT(copy, Eq("test"));
  EXPECT_THAT(original, Eq("test"));
}

TEST(ValueTest, MoveConstructor) {
  Value original = "test";
  Value moved(std::move(original));

  EXPECT_THAT(moved, Eq("test"));
}

TEST(ValueTest, MoveAssignment) {
  Value original = "test";
  Value moved;
  moved = std::move(original);

  EXPECT_THAT(moved, Eq("test"));
}

TEST(ValueTest, ArrayPushBack) {
  Value v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  EXPECT_THAT(v.size(), Eq(3));
  EXPECT_THAT(v[0], Eq(1));
  EXPECT_THAT(v[1], Eq(2));
  EXPECT_THAT(v[2], Eq(3));
}

TEST(ValueTest, ObjectInsert) {
  Value v;
  v["name"] = "name";
  v.insert("age", 42);

  EXPECT_THAT(v.size(), Eq(2));
  EXPECT_THAT(v["name"], Eq("name"));
  EXPECT_THAT(v["age"], Eq(42));
}

TEST(ValueTest, TypeReassignment) {
  Value v = "string";
  EXPECT_THAT(v, Eq("string"));

  v = 42;
  EXPECT_THAT(v, Eq(42));

  v = true;
  EXPECT_THAT(v, Eq(true));

  v = 3.14;
  EXPECT_THAT((double)v, DoubleEq(3.14));

  v = nullptr;
  EXPECT_THAT(v, Eq(nullptr));

  v.push_back(1);
  EXPECT_THAT(v, Eq(array_t{1}));

  v = nullptr;
  EXPECT_THAT(v, Eq(nullptr));

  v["key"] = array_t{1};
  object_t map = object_t{{"key", array_t{1}}};
  EXPECT_THAT(v, Eq(map));
}

TEST(ValueTest, ArrayErase) {
  Value v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  v.erase(((array_t&)v).begin() + 1);

  EXPECT_THAT(v.size(), Eq(2));
  EXPECT_THAT(v[0], Eq(1));
  EXPECT_THAT(v[1], Eq(3));
}

TEST(ValueTest, ObjectErase) {
  Value v;
  v.insert("key1", "value1");
  v.insert("key2", "value2");

  EXPECT_THAT(v.size(), Eq(2));
  EXPECT_THAT(v["key1"], Eq("value1"));

  v.erase("key1");
  EXPECT_THAT(v.size(), Eq(1));
  EXPECT_THAT(v["key2"], Eq("value2"));
}

TEST(ValueTest, ArrayIteration) {
  Value v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  int32_t sum = 0;
  for (const Value& item : (const array_t&)v) {
    sum += (int32_t)item;
  }

  EXPECT_THAT(sum, Eq(6));
}

TEST(ValueTest, ObjectIteration) {
  Value v;
  v["a"] = 1;
  v["b"] = 2;
  v["c"] = 3;

  int32_t sum = 0;
  for (const auto& [_, value] : (const object_t&)v) {
    sum += (int32_t)value;
  }

  EXPECT_THAT(sum, Eq(6));
}

TEST(ValueTest, TypeErrorsThrow) {
  Value v = "string";
  EXPECT_THAT([&v]() { (void)v[0]; }, Throws<BadAccessException>());
  EXPECT_THAT([&v]() { (void)v["key"]; }, Throws<BadAccessException>());
  EXPECT_THAT([&v]() { (void)v.size(); }, Throws<BadAccessException>());
  EXPECT_THAT([&v]() { (void)v.empty(); }, Throws<BadAccessException>());
  EXPECT_THAT([&v]() { (void)(bool)v; }, Throws<BadAccessException>());
  EXPECT_THAT([&v]() { v.push_back(1); }, Throws<BadAccessException>());
  EXPECT_THAT([&v]() { (void)(double)v; }, Throws<BadAccessException>());
  EXPECT_THAT([&v]() { (void)(int32_t)v; }, Throws<BadAccessException>());
  EXPECT_THAT([&v]() { v.insert("key", 1); }, Throws<BadAccessException>());
}

}  // namespace

}  // namespace json
}  // namespace pulse
