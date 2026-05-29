#include "pulse/json/parse/lexer.h"

#include <ostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/json/parse/token.h"
#include "pulse/json/utils/to_string.h"

namespace pulse {
namespace json {

inline void PrintTo(const Token& token, std::ostream* os) {
  *os << to_string(token);
}

inline void PrintTo(const Lexer::Error& error, std::ostream* os) {
  *os << to_string(error);
}

namespace {

using ::testing::Eq;

TEST(LexerTest, LexInvalidNull) {
  Lexer lexer("nul");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(
      lexer.error(),
      Eq(Lexer::Error(TokenType::JSON_NULL,
                      /*pos=*/0,
                      "incomplete literal: got 'nul', expected 'null'")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "nul")));
}

TEST(LexerTest, LexInvalidTrue) {
  Lexer lexer("tru");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(
      lexer.error(),
      Eq(Lexer::Error(TokenType::BOOLEAN,
                      /*pos=*/0,
                      "incomplete literal: got 'tru', expected 'true'")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "tru")));
}

TEST(LexerTest, LexInvalidFalse) {
  Lexer lexer("fals");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(
      lexer.error(),
      Eq(Lexer::Error(TokenType::BOOLEAN,
                      /*pos=*/0,
                      "incomplete literal: got 'fals', expected 'false'")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "fals")));
}

TEST(LexerTest, LexNull) {
  Lexer lexer("null");
  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::JSON_NULL, "null")));
}

TEST(LexerTest, LexTrue) {
  Lexer lexer("true");
  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::BOOLEAN, "true")));
}

TEST(LexerTest, LexFalse) {
  Lexer lexer("false");
  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::BOOLEAN, "false")));
}

TEST(LexerTest, LexInvalidUnterminatedString) {
  Lexer lexer("\"hello");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(lexer.error(),
              Eq(Lexer::Error(TokenType::QUOTE,
                              /*pos=*/0, "unterminated string")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "hello")));
}

TEST(LexerTest, LexInvalidBadEscapeChar) {
  Lexer lexer("\"hello\\z\"");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(
      lexer.error(),
      Eq(Lexer::Error(TokenType::STRING,
                      /*pos=*/6, "invalid control character: hello\\z")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "hello\\z")));
}

TEST(LexerTest, LexInvalidBadUnicode) {
  Lexer lexer("\"\\u12\"");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(lexer.error(),
              Eq(Lexer::Error(TokenType::STRING,
                              /*pos=*/1, "invalid control character: \\u12")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "\\u12")));
}

TEST(LexerTest, LexStringSimple) {
  Lexer lexer("\"hello\"");
  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::STRING, "hello")));
}

TEST(LexerTest, LexStringEscapeChars) {
  Lexer lexer("\"hello\\nworld\"");
  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::STRING, "hello\nworld")));
}

TEST(LexerTest, LexStringUnicode) {
  Lexer lexer("\"\\u0041\"");
  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::STRING, "\\u0041")));
}

TEST(LexerTest, LexInvalidNumberDash) {
  Lexer lexer("-");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(lexer.error(), Eq(Lexer::Error(TokenType::INTEGRAL, /*pos=*/0,
                                             "invalid integer: -")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "-")));
}

TEST(LexerTest, LexInvalidNumberLeadingZero) {
  Lexer lexer("01");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(lexer.error(), Eq(Lexer::Error(TokenType::INTEGRAL, /*pos=*/0,
                                             "invalid integer: 01")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "01")));
}

TEST(LexerTest, LexInvalidNumberTrailingDecimal) {
  Lexer lexer("1.");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(lexer.error(), Eq(Lexer::Error(TokenType::DOUBLE, /*pos=*/1,
                                             "invalid fraction: .")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "1.")));
}

TEST(LexerTest, LexInvalidNumberExponentOnly) {
  Lexer lexer("1e");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(lexer.error(), Eq(Lexer::Error(TokenType::INTEGRAL, /*pos=*/1,
                                             "invalid exponent: e")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "1e")));
}

TEST(LexerTest, LexInvalidNumberExponentSign) {
  Lexer lexer("1e+");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(lexer.error(), Eq(Lexer::Error(TokenType::INTEGRAL, /*pos=*/1,
                                             "invalid exponent: e+")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "1e+")));
}

TEST(LexerTest, LexNumberIntegral) {
  Lexer lexer("123");
  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::INTEGRAL, "123")));
}

TEST(LexerTest, LexNumberDouble) {
  Lexer lexer("12.34");
  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::DOUBLE, "12.34")));
}

TEST(LexerTest, LexPunctuation) {
  Lexer lexer("{}[],:");

  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::OBJECT_START, "{")));

  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::OBJECT_END, "}")));

  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::ARRAY_START, "[")));

  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::ARRAY_END, "]")));

  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::COMMA, ",")));

  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::COLON, ":")));
}

TEST(LexerTest, LexWhitespace) {
  Lexer lexer("   \n\t 123");
  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::INTEGRAL, "123")));
}

TEST(LexerTest, LexUnknown) {
  Lexer lexer("@");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_FALSE(lexer.ok());
  EXPECT_THAT(lexer.error(), Eq(Lexer::Error(TokenType::UNKNOWN, /*pos=*/0,
                                             "unknown token: @")));
  EXPECT_THAT(*lexer, Eq(Token(TokenType::UNKNOWN, "@")));
}

TEST(LexerTest, IncrementOperator) {
  Lexer lexer("true false");

  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_FALSE(lexer.eof());
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::BOOLEAN, "true")));

  ++lexer;
  EXPECT_TRUE(lexer);
  EXPECT_FALSE(lexer.eof());
  EXPECT_TRUE(lexer.ok());
  EXPECT_THAT(*lexer, Eq(Token(TokenType::BOOLEAN, "false")));

  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_TRUE(lexer.eof());
}

TEST(LexerTest, EofOnEmptyInput) {
  Lexer lexer("");
  ++lexer;
  EXPECT_FALSE(lexer);
  EXPECT_TRUE(lexer.eof());
}

}  // namespace

}  // namespace json
}  // namespace pulse
