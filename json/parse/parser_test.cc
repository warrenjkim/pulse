#include "pulse/json/parse/parser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/json/parse/lexer.h"

namespace pulse {
namespace json {

namespace {

using ::testing::Eq;
using ::testing::Throws;

TEST(ParserTest, UnexpectedTokenAfterParsing) {
  EXPECT_THAT([] { Parser(Lexer("{} x")).parse(); }, Throws<ParseException>());
}

TEST(ParserTest, UnexpectedFirstToken) {
  EXPECT_THAT([] { Parser(Lexer("}")).parse(); }, Throws<ParseException>());
}

TEST(ParserTest, ArrayMissingComma) {
  EXPECT_THAT([] { Parser(Lexer("[1 2]")).parse(); }, Throws<ParseException>());
}

TEST(ParserTest, ArrayUnterminated) {
  EXPECT_THAT([] { Parser(Lexer("[1, 2")).parse(); }, Throws<ParseException>());
}

TEST(ParserTest, ArrayUnexpectedTokenAfterValue) {
  EXPECT_THAT([] { Parser(Lexer("[1:2]")).parse(); }, Throws<ParseException>());
}

TEST(ParserTest, ArrayTrailingComma) {
  EXPECT_THAT([] { Parser(Lexer("[1,]")).parse(); }, Throws<ParseException>());
}

TEST(ParserTest, ObjectMissingColon) {
  EXPECT_THAT([] { Parser(Lexer(R"({"a" 1})")).parse(); },
              Throws<ParseException>());
}

TEST(ParserTest, ObjectMissingCommaBetweenPairs) {
  EXPECT_THAT([] { Parser(Lexer(R"({"a":1 "b":2})")).parse(); },
              Throws<ParseException>());
}

TEST(ParserTest, ObjectUnterminated) {
  EXPECT_THAT([] { Parser(Lexer(R"({"a":1)")).parse(); },
              Throws<ParseException>());
}

TEST(ParserTest, StringUnicodeHighSurrogateWithoutLowSurrogate) {
  EXPECT_THAT([] { Parser(Lexer(R"("\uD83D")")).parse(); },
              Throws<ParseException>());
}

TEST(ParserTest, StringUnicodeHighSurrogateFollowedByNonLowSurrogate) {
  EXPECT_THAT([] { Parser(Lexer(R"("\uD83D\u1234")")).parse(); },
              Throws<ParseException>());
}

TEST(ParserTest, UnicodeLoneLowSurrogate) {
  EXPECT_THAT([] { Parser(Lexer(R"("\uDC00")")).parse(); },
              Throws<ParseException>());
}

TEST(ParserTest, ObjectTrailingComma) {
  EXPECT_THAT([] { Parser(Lexer(R"({"a":1,})")).parse(); },
              Throws<ParseException>());
}

TEST(ParserTest, EmptyObject) {
  EXPECT_THAT(Parser(Lexer("{}")).parse(), Eq(object_t{}));
}

TEST(ParserTest, SimpleObject) {
  EXPECT_THAT(Parser(Lexer(R"({
    "int": 1,
    "str": "two",
    "float": 3.4,
    "null": null,
    "bool": true,
    "obj": {},
    "arr": []
  })"))
                  .parse(),
              Eq(object_t{
                  {"int", 1},
                  {"str", "two"},
                  {"float", 3.4},
                  {"null", nullptr},
                  {"bool", true},
                  {"obj", object_t{}},
                  {"arr", array_t{}},
              }));
}

TEST(ParserTest, EmptyArray) {
  EXPECT_THAT(Parser(Lexer("[]")).parse(), Eq(array_t{}));
}

TEST(ParserTest, SimpleArray) {
  EXPECT_THAT(Parser(Lexer("[1, \"two\", 3.4, null, true, {}, []]")).parse(),
              Eq(array_t{1, "two", 3.4, nullptr, true, object_t{}, array_t{}}));
}

}  // namespace

}  // namespace json
}  // namespace pulse
