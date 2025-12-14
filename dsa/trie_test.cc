#include "pulse/dsa/trie.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse {

namespace {

using ::testing::Eq;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::Pointee;

TEST(TrieTest, DefaultConstructor) { Trie<std::string, int> trie; }

TEST(TrieTest, InsertAndPrefix) {
  Trie<std::string, int> trie;
  trie.insert("hello", 1);

  EXPECT_TRUE(trie.prefix(""));
  EXPECT_TRUE(trie.prefix("h"));
  EXPECT_TRUE(trie.prefix("hel"));
  EXPECT_TRUE(trie.prefix("hell"));
  EXPECT_TRUE(trie.prefix("hello"));

  EXPECT_FALSE(trie.prefix("helloo"));
  EXPECT_FALSE(trie.prefix("world"));
}

TEST(TrieTest, InsertAndMatch) {
  Trie<std::string, int> trie;
  trie.insert("hello", 1);

  EXPECT_TRUE(trie.match("hello"));
  EXPECT_FALSE(trie.match("hell"));
  EXPECT_FALSE(trie.match("helloo"));
  EXPECT_FALSE(trie.match("world"));
}

TEST(TrieTest, InsertMultipleKeys) {
  Trie<std::string, int> trie;
  trie.insert("hello", 1);
  trie.insert("help", 2);
  trie.insert("world", 3);

  EXPECT_TRUE(trie.match("hello"));
  EXPECT_TRUE(trie.match("help"));
  EXPECT_TRUE(trie.match("world"));
  EXPECT_FALSE(trie.match("hel"));

  EXPECT_TRUE(trie.prefix("hel"));
  EXPECT_TRUE(trie.prefix("wor"));
}

TEST(TrieTest, InsertOverwrite) {
  Trie<std::string, int> trie;
  trie.insert("hello", 1);
  trie.insert("hello", 2);

  EXPECT_THAT(trie.get("hello"), Pointee(Eq(2)));
}

TEST(TrieTest, Get) {
  Trie<std::string, int> trie;
  trie.insert("hello", 42);

  EXPECT_THAT(trie.get("hello"), Pointee(Eq(42)));
  EXPECT_THAT(trie.get("hell"), IsNull());
  EXPECT_THAT(trie.get("world"), IsNull());
}

TEST(TrieTest, OperatorBracketAccess) {
  Trie<std::string, int> trie;
  trie.insert("hello", 42);

  EXPECT_THAT(trie["hello"], Eq(42));
}

TEST(TrieTest, OperatorBracketModify) {
  Trie<std::string, int> trie;
  trie.insert("hello", 1);

  trie["hello"] = 100;

  EXPECT_THAT(trie.get("hello"), Pointee(Eq(100)));
}

TEST(TrieTest, RemoveEmpty) {
  Trie<std::string, int> trie;
  EXPECT_FALSE(trie.erase("hello"));
}

TEST(TrieTest, RemoveExisting) {
  Trie<std::string, int> trie;
  trie.insert("hello", 1);

  EXPECT_TRUE(trie.erase("hello"));
  EXPECT_FALSE(trie.match("hello"));
  EXPECT_THAT(trie.get("hello"), IsNull());
}

TEST(TrieTest, RemoveNonExisting) {
  Trie<std::string, int> trie;
  trie.insert("hello", 1);

  EXPECT_FALSE(trie.erase("world"));
  EXPECT_FALSE(trie.erase("hell"));
}

TEST(TrieTest, RemovePreservesOtherKeys) {
  Trie<std::string, int> trie;
  trie.insert("hello", 1);
  trie.insert("help", 2);

  EXPECT_TRUE(trie.erase("hello"));

  EXPECT_FALSE(trie.match("hello"));
  EXPECT_TRUE(trie.match("help"));
  EXPECT_THAT(trie.get("help"), Pointee(Eq(2)));
}

TEST(TrieTest, RemovePrefixKey) {
  Trie<std::string, int> trie;
  trie.insert("hel", 1);
  trie.insert("hello", 2);

  EXPECT_TRUE(trie.erase("hel"));

  EXPECT_FALSE(trie.match("hel"));
  EXPECT_TRUE(trie.match("hello"));
  EXPECT_TRUE(trie.prefix("hel"));
}

TEST(TrieTest, RemoveLongerKey) {
  Trie<std::string, int> trie;
  trie.insert("hel", 1);
  trie.insert("hello", 2);

  EXPECT_TRUE(trie.erase("hello"));

  EXPECT_TRUE(trie.match("hel"));
  EXPECT_FALSE(trie.match("hello"));
  EXPECT_FALSE(trie.prefix("hello"));
}

TEST(TrieTest, EmptyKey) {
  Trie<std::string, int> trie;
  trie.insert("", 42);

  EXPECT_TRUE(trie.match(""));
  EXPECT_TRUE(trie.prefix(""));
  EXPECT_THAT(trie.get(""), Pointee(Eq(42)));
}

TEST(TrieTest, VectorKey) {
  Trie<std::vector<int>, std::string> trie;
  trie.insert(std::vector<int>{1, 2, 3}, "foo");

  EXPECT_TRUE(trie.match(std::vector<int>{1, 2, 3}));
  EXPECT_TRUE(trie.prefix(std::vector<int>{1, 2}));
  EXPECT_FALSE(trie.match(std::vector<int>{1, 2}));
}

TEST(TrieTest, StringViewLookup) {
  Trie<std::string, int> trie;
  trie.insert("hello", 42);

  std::string_view sv = "hello";
  EXPECT_TRUE(trie.match(sv));
  EXPECT_TRUE(trie.prefix(sv));
  EXPECT_THAT(trie.get(sv), Pointee(Eq(42)));
}

TEST(TrieTest, MoveOnlyValue) {
  Trie<std::string, std::unique_ptr<int>> trie;
  trie.insert("hello", std::make_unique<int>(42));

  EXPECT_TRUE(trie.match("hello"));

  const std::unique_ptr<int>* val = trie.get("hello");
  ASSERT_THAT(val, NotNull());
  ASSERT_THAT(*val, NotNull());
  EXPECT_THAT(**val, Eq(42));
}

TEST(TrieTest, ToStringComplex) {
  Trie<std::string, int> trie;

  trie.insert("test", 1);
  trie.insert("tester", 2);
  trie.insert("testing", 3);
  trie.insert("tess", 4);

  trie.insert("alpha", 5);
  trie.insert("alphabet", 6);
  trie.insert("aloe", 7);
  trie.insert("altar", 8);

  trie.insert("alt", 9);
  trie.insert("alt", 10);

  trie.insert("world", 11);
  trie.insert("worm", 12);

  EXPECT_THAT(to_string(trie), Eq("a-l-o-(e)\n"
                                  "   `-p-h-(a)-b-e-(t)\n"
                                  "   `-(t)-a-(r)\n"
                                  "t-e-s-(s)\n"
                                  "     `-(t)-e-(r)\n"
                                  "          `-i-n-(g)\n"
                                  "w-o-r-l-(d)\n"
                                  "     `-(m)"));
}

}  // namespace

}  // namespace pulse
