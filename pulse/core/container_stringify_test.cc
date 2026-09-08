#include "pulse/core/container_stringify.h"

#include <array>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/pretty_function.h"
#include "pulse/core/stringify.h"

namespace pulse {

namespace container_stringify_test {

struct Opaque {};

struct IntBag {
  std::vector<int> values;

  auto begin() const { return values.begin(); }
  auto end() const { return values.end(); }
};

namespace {

using ::testing::AllOf;
using ::testing::EndsWith;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::StartsWith;

TEST(ContainerStringifyTest, MatchesRanges) {
  static_assert(internal::RenderableRange<std::vector<int>>);
  static_assert(internal::RenderableRange<std::set<int>>);
  static_assert(internal::RenderableRange<std::array<int, 3>>);
  static_assert(internal::RenderableRange<std::span<const int>>);
  static_assert(internal::RenderableRange<IntBag>);
  static_assert(internal::RenderableRange<int[3]>);
  static_assert(internal::RenderableRange<int[2][3]>);
}

TEST(ContainerStringifyTest, MatchesMaps) {
  static_assert(internal::RenderableMap<std::map<std::string, int>>);
  static_assert(internal::RenderableMap<std::unordered_map<std::string, int>>);
}

TEST(ContainerStringifyTest, RangesAndMapsAreDisjoint) {
  static_assert(!internal::RenderableMap<std::vector<int>>);
  static_assert(!internal::RenderableRange<std::map<std::string, int>>);
}

TEST(ContainerStringifyTest, ExcludesStrings) {
  static_assert(!internal::RenderableRange<std::string>);
  static_assert(!internal::RenderableRange<std::string_view>);
  static_assert(!internal::RenderableRange<char[4]>);
}

TEST(ContainerStringifyTest, RequiresStringifiableElements) {
  static_assert(!Stringifiable<Opaque>);
  static_assert(!internal::RenderableRange<std::vector<Opaque>>);
  static_assert(!internal::RenderableRange<Opaque[3]>);
  static_assert(!internal::RenderableMap<std::map<Opaque, int>>);
  static_assert(!internal::RenderableMap<std::map<int, Opaque>>);
}

TEST(ContainerStringifyTest, NamesContainers) {
  EXPECT_THAT(internal::TypeLabel<std::vector<int>>(), Eq("std::vector<int>"));
  EXPECT_THAT((internal::TypeLabel<std::map<int, std::string>>()),
              Eq("std::map<int,std::string>"));
  EXPECT_THAT(internal::TypeLabel<std::span<const int>>(),
              Eq("std::span<int>"));
  EXPECT_THAT(internal::TypeLabel<IntBag>(),
              Eq("pulse::container_stringify_test::IntBag"));
}

TEST(ContainerStringifyTest, KeepsStaticExtents) {
  EXPECT_THAT((internal::TypeLabel<std::array<int, 3>>()),
              Eq("std::array<int,3>"));
  EXPECT_THAT((internal::TypeLabel<std::array<std::array<int, 2>, 3>>()),
              Eq("std::array<std::array<int,2>,3>"));
  EXPECT_THAT((internal::TypeLabel<std::span<const int, 4>>()),
              Eq("std::span<int,4>"));
}

TEST(ContainerStringifyTest, NamesCArrays) {
  EXPECT_THAT(internal::TypeLabel<int[3]>(), Eq("int[3]"));
  EXPECT_THAT(internal::TypeLabel<int[1][2][3]>(), Eq("int[1][2][3]"));
  EXPECT_THAT(internal::TypeLabel<int[]>(), Eq("int[]"));
  EXPECT_THAT(internal::TypeLabel<int[][1]>(), Eq("int[][1]"));
  EXPECT_THAT((internal::TypeLabel<std::map<int, std::string>[2]>()),
              Eq("std::map<int,std::string>[2]"));
  EXPECT_THAT(internal::TypeLabel<Opaque[3]>(),
              Eq(std::string(TypeName<Opaque>()) + "[3]"));
}

TEST(ContainerStringifyTest, RecursesThroughElementTypes) {
  using T = std::vector<std::unordered_map<std::string, std::vector<int>>>;

  EXPECT_THAT(
      internal::TypeLabel<T>(),
      Eq("std::vector<std::unordered_map<std::string,std::vector<int>>>"));
}

TEST(ContainerStringifyTest, LeavesOtherTypesToTypeName) {
  EXPECT_THAT(internal::TypeLabel<int>(), Eq("int"));
  EXPECT_THAT(internal::TypeLabel<std::string>(), Eq("std::string"));
  EXPECT_THAT(internal::TypeLabel<Opaque>(),
              Eq(std::string(TypeName<Opaque>())));
}

TEST(ContainerStringifyTest, RendersRanges) {
  EXPECT_THAT(ToString(std::vector<int>{1, 2, 3}),
              Eq("std::vector<int>{1,2,3}"));
  EXPECT_THAT(ToString(std::vector<std::string>{}),
              Eq("std::vector<std::string>{}"));
  EXPECT_THAT(ToString(std::deque<bool>{true, false}),
              Eq("std::deque<bool>{true,false}"));
  EXPECT_THAT(ToString(std::set<int>{3, 1, 2}), Eq("std::set<int>{1,2,3}"));
  EXPECT_THAT(ToString(std::list<int>{1}), Eq("std::list<int>{1}"));
  EXPECT_THAT(ToString(IntBag{{1, 2}}),
              Eq("pulse::container_stringify_test::IntBag{1,2}"));
}

TEST(ContainerStringifyTest, RendersStaticExtentRanges) {
  EXPECT_THAT((ToString(std::array<std::string, 2>{"a", "b"})),
              Eq(R"(std::array<std::string,2>{"a","b"})"));
  EXPECT_THAT((ToString(std::array<int, 0>{})), Eq("std::array<int,0>{}"));
}

TEST(ContainerStringifyTest, RendersCArrays) {
  const int values[3] = {1, 2, 3};
  EXPECT_THAT(ToString(values), Eq("int[3]{1,2,3}"));

  const int grid[2][3] = {{1, 2, 3}, {4, 5, 6}};
  EXPECT_THAT(ToString(grid), Eq("int[2][3]{int[3]{1,2,3},int[3]{4,5,6}}"));

  const std::vector<int> rows[2] = {{1, 2}, {3}};
  EXPECT_THAT(ToString(rows), Eq("std::vector<int>[2]{std::vector<int>{1,2},"
                                 "std::vector<int>{3}}"));
}

TEST(ContainerStringifyTest, RendersNestedContainers) {
  EXPECT_THAT(ToString(std::vector<std::vector<int>>{{1, 2}, {}, {3}}),
              Eq("std::vector<std::vector<int>>{"
                 "std::vector<int>{1,2},"
                 "std::vector<int>{},"
                 "std::vector<int>{3}}"));
  EXPECT_THAT((ToString(std::map<int, std::vector<int>>{{1, {2, 3}}})),
              Eq("std::map<int,std::vector<int>>{{1,std::vector<int>{2,3}}}"));
}

TEST(ContainerStringifyTest, RendersMaps) {
  EXPECT_THAT((ToString(std::map<std::string, int>{{"a", 1}, {"b", 2}})),
              Eq(R"(std::map<std::string,int>{{"a",1},{"b",2}})"));
  EXPECT_THAT((ToString(std::multimap<int, int>{{1, 2}, {1, 3}})),
              Eq("std::multimap<int,int>{{1,2},{1,3}}"));
  EXPECT_THAT((ToString(std::unordered_map<std::string, int>{})),
              Eq("std::unordered_map<std::string,int>{}"));
}

TEST(ContainerStringifyTest, RendersUnorderedMaps) {
  const std::string out =
      ToString(std::unordered_map<std::string, int>{{"a", 1}, {"b", 2}});

  EXPECT_THAT(out, AllOf(StartsWith("std::unordered_map<std::string,int>{"),
                         HasSubstr(R"({"a",1})"), HasSubstr(R"({"b",2})"),
                         HasSubstr("},{"), EndsWith("}")));
}

TEST(ContainerStringifyTest, UsesScalarSpecializations) {
  EXPECT_THAT(ToString(std::vector<std::string>{"a,", "b"}),
              Eq(R"(std::vector<std::string>{"a,","b"})"));
  EXPECT_THAT(ToString(std::vector<const char*>{"a", "b"}),
              Eq(R"(std::vector<const char*>{"a","b"})"));

  const char text[4] = "abc";
  EXPECT_THAT(ToString(text), Eq(R"("abc")"));
}

}  // namespace

}  // namespace container_stringify_test

}  // namespace pulse
