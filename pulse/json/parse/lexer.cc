#include "pulse/json/parse/lexer.h"

#include <cctype>    // isdigit, isspace, isxdigit, tolower
#include <optional>  // nullopt, optional

#include "pulse/json/parse/token.h"

namespace pulse {
namespace json {

Lexer::Lexer(std::string json)
    : reader_(std::move(json)), curr_(TokenType::UNKNOWN, "") {}

Lexer& Lexer::operator++() {
  curr_ = next_token();

  return *this;
}

const Token& Lexer::operator*() const noexcept { return curr_; }

const Token* Lexer::operator->() const noexcept { return &curr_; }

Lexer::operator bool() const noexcept { return !eof() && ok(); }

bool Lexer::ok() const noexcept { return !error_; }

Lexer::Error Lexer::error() const { return *error_; }

bool Lexer::eof() const noexcept {
  return curr_.type == TokenType::END_OF_JSON;
}

Token Lexer::next_token() {
  strip_whitespace();
  if (reader_.eof()) {
    return Token(TokenType::END_OF_JSON, "");
  }

  switch (reader_.peek()) {
    case 'n':
      return lex_literal("null", TokenType::JSON_NULL);
    case 't':
      return lex_literal("true", TokenType::BOOLEAN);
    case 'f':
      return lex_literal("false", TokenType::BOOLEAN);
    case '"':
      return lex_string();
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return lex_number();
    case '[':
      return Token(TokenType::ARRAY_START, reader_.get());
    case ']':
      return Token(TokenType::ARRAY_END, reader_.get());
    case '{':
      return Token(TokenType::OBJECT_START, reader_.get());
    case ':':
      return Token(TokenType::COLON, reader_.get());
    case '}':
      return Token(TokenType::OBJECT_END, reader_.get());
    case ',':
      return Token(TokenType::COMMA, reader_.get());
    default:
      error_ = Error(TokenType::UNKNOWN, reader_.tell(),
                     "unknown token: " + reader_.substr(reader_.tell()));
      return Token(TokenType::UNKNOWN, reader_.get());
  }
}

Token Lexer::lex_literal(const std::string& literal, TokenType type) {
  size_t start = reader_.tell();
  std::string res;
  for (char c : literal) {
    if (reader_.eof()) {
      error_ = Error(
          type, start,
          "incomplete literal: got '" + res + "', expected '" + literal + "'");
      return Token(TokenType::UNKNOWN, res);
    }

    if (reader_.peek() != c) {
      error_ = Error(
          type, start,
          "unexpected literal: got '" + res + "', expected '" + literal + "'");
      return Token(TokenType::UNKNOWN, res);
    }

    res += reader_.get();
  }

  return Token(type, res);
}

Token Lexer::lex_string() {
  size_t start = reader_.tell();
  if (!reader_.expect('"') || reader_.eof()) {
    error_ = Error(TokenType::QUOTE, start, "unterminated string");
    return Token(TokenType::UNKNOWN, "");
  }

  std::string res;
  while (!reader_.eof()) {
    if (reader_.expect('"')) {
      return Token(TokenType::STRING, std::move(res));
    }

    if (reader_.peek() == '\\') {
      size_t start = reader_.tell();
      std::optional<std::string> ctrl = lex_ctrl();
      if (!ctrl) {
        std::string token = res + reader_.substr(start, reader_.tell() - start);
        error_ = Error(TokenType::STRING, start,
                       "invalid control character: " + token);
        return Token(TokenType::UNKNOWN, token);
      }

      res += *ctrl;
      continue;
    }

    res += reader_.get();
  }

  error_ = Error(TokenType::QUOTE, start, "unterminated string");
  return Token(TokenType::UNKNOWN, std::move(res));
}

std::optional<std::string> Lexer::lex_ctrl() {
  if (!reader_.expect('\\') || reader_.eof()) {
    return std::nullopt;
  }

  switch (reader_.get()) {
    case 'u': {
      std::string hex_digits;
      hex_digits.reserve(4);
      for (size_t i = 0; i < 4; i++) {
        if (reader_.eof() || !isxdigit(reader_.peek())) {
          return std::nullopt;
        }

        hex_digits += reader_.get();
      }

      return "\\u" + hex_digits;
    }
    case '"':
      return "\"";
    case '\\':
      return "\\";
    case '/':
      return "/";
    case 'b':
      return "\b";
    case 'f':
      return "\f";
    case 'n':
      return "\n";
    case 'r':
      return "\r";
    case 't':
      return "\t";
    default:
      return std::nullopt;
  }
}

Token Lexer::lex_number() {
  Token integer = lex_integer();
  if (integer.type == TokenType::UNKNOWN) {
    return integer;
  }

  Token fraction = lex_fraction();
  if (fraction.type == TokenType::UNKNOWN) {
    fraction.value = integer.value + fraction.value;
    return fraction;
  }

  Token exponent = lex_exponent();
  if (exponent.type == TokenType::UNKNOWN) {
    exponent.value = integer.value + fraction.value + exponent.value;
    return exponent;
  }

  return Token(fraction.type, integer.value + fraction.value + exponent.value);
}

Token Lexer::lex_integer() {
  size_t start = reader_.tell();
  std::string integer;
  if (reader_.peek() == '-') {
    integer += reader_.get();
  }

  if (reader_.eof()) {
    error_ = Error(TokenType::INTEGRAL, start, "invalid integer: " + integer);
    return Token(TokenType::UNKNOWN, integer);
  }

  if (reader_.peek() < '0' || reader_.peek() > '9') {
    error_ = Error(TokenType::INTEGRAL, start, "invalid integer: " + integer);
    return Token(TokenType::UNKNOWN, integer);
  }

  integer += reader_.get();
  if (integer.back() == '0') {
    bool invalid = false;
    while (!reader_.eof() && isdigit(reader_.peek())) {
      invalid = true;
      integer += reader_.get();
    }

    if (invalid) {
      error_ = Error(TokenType::INTEGRAL, start, "invalid integer: " + integer);
      return Token(TokenType::UNKNOWN, integer);
    }

    return Token(TokenType::INTEGRAL, integer);
  }

  while (!reader_.eof() && isdigit(reader_.peek())) {
    integer += reader_.get();
  }

  return Token(TokenType::INTEGRAL, integer);
}

Token Lexer::lex_fraction() {
  size_t start = reader_.tell();
  std::string fraction;
  if (reader_.peek() != '.') {
    return Token(TokenType::INTEGRAL, fraction);
  }

  fraction += reader_.get();
  if (reader_.eof() || !isdigit(reader_.peek())) {
    error_ = Error(TokenType::DOUBLE, start, "invalid fraction: " + fraction);
    return Token(TokenType::UNKNOWN, fraction);
  }

  while (!reader_.eof() && isdigit(reader_.peek())) {
    fraction += reader_.get();
  }

  return Token(TokenType::DOUBLE, fraction);
}

Token Lexer::lex_exponent() {
  size_t start = reader_.tell();
  std::string exponent;
  if (reader_.eof() || tolower(reader_.peek()) != 'e') {
    return Token(TokenType::INTEGRAL, exponent);
  }

  exponent += reader_.get();
  if (!reader_.eof() && (reader_.peek() == '+' || reader_.peek() == '-')) {
    exponent += reader_.get();
  }

  if (reader_.eof() || !isdigit(reader_.peek())) {
    error_ = Error(TokenType::INTEGRAL, start, "invalid exponent: " + exponent);
    return Token(TokenType::UNKNOWN, exponent);
  }

  while (!reader_.eof() && isdigit(reader_.peek())) {
    exponent += reader_.get();
  }

  return Token(TokenType::INTEGRAL, exponent);
}

void Lexer::strip_whitespace() {
  while (!reader_.eof() && isspace(reader_.peek())) {
    (void)reader_.get();
  }
}

}  // namespace json
}  // namespace pulse
