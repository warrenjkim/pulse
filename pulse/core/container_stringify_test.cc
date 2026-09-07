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
#include "pulse/core/demangle.h"
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

TEST(RenderableRangeTest, MatchesSequenceRanges) {
  static_assert(internal::RenderableRange<std::vector<int>>);
  static_assert(internal::RenderableRange<std::deque<int>>);
  static_assert(internal::RenderableRange<std::list<int>>);
  static_assert(internal::RenderableRange<std::set<int>>);
  static_assert(internal::RenderableRange<std::array<int, 3>>);
  static_assert(internal::RenderableRange<std::span<const int>>);
  static_assert(internal::RenderableRange<IntBag>);
}

TEST(RenderableRangeTest, MatchesMapRanges) {
  static_assert(internal::RenderableMap<std::map<std::string, int>>);
  static_assert(internal::RenderableMap<std::unordered_map<std::string, int>>);
  static_assert(internal::RenderableMap<std::multimap<int, int>>);
}

TEST(RenderableRangeTest, SequenceAndMapRangesAreDisjoint) {
  static_assert(!internal::RenderableMap<std::vector<int>>);
  static_assert(!internal::RenderableRange<std::map<std::string, int>>);
}

TEST(RenderableRangeTest, ExcludesStrings) {
  static_assert(!internal::RenderableRange<std::string>);
  static_assert(!internal::RenderableRange<std::string_view>);
}

TEST(RenderableRangeTest, RequiresStringifiableElements) {
  static_assert(!Stringifiable<Opaque>);
  static_assert(!internal::RenderableRange<std::vector<Opaque>>);
  static_assert(!internal::RenderableMap<std::map<Opaque, int>>);
  static_assert(!internal::RenderableMap<std::map<int, Opaque>>);
}

TEST(TypeLabelTest, NamesContainers) {
  EXPECT_THAT(internal::TypeLabel<std::vector<int>>(), Eq("std::vector<int>"));
  EXPECT_THAT((internal::TypeLabel<std::unordered_map<int, bool>>()),
              Eq("std::unordered_map<int,bool>"));
  EXPECT_THAT((internal::TypeLabel<std::map<int, int>>()),
              Eq("std::map<int,int>"));
}

TEST(TypeLabelTest, RecursesThroughNestedContainers) {
  using T = std::vector<std::unordered_map<std::string, std::vector<int>>>;

  EXPECT_THAT(
      internal::TypeLabel<T>(),
      Eq("std::vector<std::unordered_map<std::string,std::vector<int>>>"));
}

TEST(TypeLabelTest, OmitsContainerImplementationArguments) {
  EXPECT_THAT(internal::TypeLabel<std::vector<int>>(), Eq("std::vector<int>"));
  EXPECT_THAT((internal::TypeLabel<std::map<int, std::string>>()),
              Eq("std::map<int,std::string>"));
}

TEST(TypeLabelTest, OmitsArrayExtent) {
  EXPECT_THAT((internal::TypeLabel<std::array<int, 3>>()),
              Eq("std::array<int>"));
}

TEST(TypeLabelTest, UsesAliases) {
  EXPECT_THAT(internal::TypeLabel<std::string>(), Eq("std::string"));
  EXPECT_THAT(internal::TypeLabel<std::string_view>(), Eq("std::string_view"));
}

TEST(TypeLabelTest, LeavesOtherTypesToTypeName) {
  EXPECT_THAT(internal::TypeLabel<int>(), Eq("int"));
  EXPECT_THAT(internal::TypeLabel<Opaque>(), Eq(TypeName<Opaque>()));
}

TEST(TypeLabelTest, DoesNotInventTemplateArgumentsForCustomRanges) {
  EXPECT_THAT(internal::TypeLabel<IntBag>(),
              Eq("pulse::container_stringify_test::IntBag"));
}

TEST(StringifySequenceTest, RendersElements) {
  EXPECT_THAT(ToString(std::vector<int>{1, 2, 3}),
              Eq("std::vector<int>{1,2,3}"));
  EXPECT_THAT(ToString(std::vector<int>{7}), Eq("std::vector<int>{7}"));
  EXPECT_THAT(ToString(std::vector<std::string>{}),
              Eq("std::vector<std::string>{}"));
}

TEST(StringifySequenceTest, PreservesElementStrings) {
  EXPECT_THAT(ToString(std::vector<std::string>{"a,", "b"}),
              Eq("std::vector<std::string>{\"a,\",\"b\"}"));
}

TEST(StringifySequenceTest, RendersNestedContainers) {
  using T = std::vector<std::vector<int>>;

  EXPECT_THAT(ToString(T{{1, 2}, {}, {3}}), Eq("std::vector<std::vector<int>>{"
                                               "std::vector<int>{1,2},"
                                               "std::vector<int>{},"
                                               "std::vector<int>{3}}"));
}

TEST(StringifySequenceTest, RendersOtherRanges) {
  EXPECT_THAT(ToString(std::deque<int>{1, 2}), Eq("std::deque<int>{1,2}"));
  EXPECT_THAT(ToString(std::list<bool>{true, false}),
              Eq("std::list<bool>{true,false}"));
  EXPECT_THAT((ToString(std::array<int, 2>{4, 5})), Eq("std::array<int>{4,5}"));
  EXPECT_THAT(ToString(std::set<int>{3, 1, 2}), Eq("std::set<int>{1,2,3}"));
}

TEST(StringifySequenceTest, RendersUserDefinedRanges) {
  EXPECT_THAT(ToString(IntBag{{1, 2}}),
              Eq("pulse::container_stringify_test::IntBag{1,2}"));
}

TEST(StringifyMapTest, RendersEntries) {
  EXPECT_THAT((ToString(std::map<std::string, int>{{"a", 1}, {"b", 2}})),
              Eq("std::map<std::string,int>{{\"a\",1},{\"b\",2}}"));

  EXPECT_THAT((ToString(std::multimap<int, int>{{1, 2}, {1, 3}})),
              Eq("std::multimap<int,int>{{1,2},{1,3}}"));
}

TEST(StringifyMapTest, RendersEmpty) {
  EXPECT_THAT((ToString(std::unordered_map<std::string, int>{})),
              Eq("std::unordered_map<std::string,int>{}"));
}

TEST(StringifyMapTest, RendersNestedValues) {
  EXPECT_THAT((ToString(std::map<int, std::vector<int>>{{1, {2, 3}}})),
              Eq("std::map<int,std::vector<int>>{{1,std::vector<int>{2,3}}}"));
}

TEST(StringifyMapTest, RendersUnorderedEntries) {
  const std::string out =
      ToString(std::unordered_map<std::string, int>{{"a", 1}, {"b", 2}});

  EXPECT_THAT(out, AllOf(StartsWith("std::unordered_map<std::string,int>{"),
                         HasSubstr("{\"a\",1}"), HasSubstr("{\"b\",2}"),
                         HasSubstr("},{"), EndsWith("}")));
}

TEST(StringifyContainerTest, UsesScalarSpecializations) {
  EXPECT_THAT(ToString(std::string("hi")), Eq("\"hi\""));
  EXPECT_THAT(ToString(std::vector<const char*>{"a", "b"}),
              Eq("std::vector<char const*>{\"a\",\"b\"}"));
}

}  // namespace

}  // namespace container_stringify_test

}  // namespace pulse
