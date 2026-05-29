#pragma once

#include <optional>
#include <string>

#include "pulse/json/parse/reader.h"
#include "pulse/json/parse/token.h"

namespace pulse {
namespace json {

class Lexer {
 public:
  struct Error {
    TokenType expected;
    size_t pos;
    std::string message;

    explicit Error(TokenType expected, size_t pos, std::string message)
        : expected(expected), pos(pos), message(std::move(message)) {}

    bool operator==(const Error& other) const noexcept {
      return expected == other.expected && pos == other.pos &&
             message == other.message;
    }
  };

  explicit Lexer(std::string json);

  Lexer(Lexer&&) noexcept = default;
  Lexer& operator=(Lexer&&) noexcept = default;

  Lexer(const Lexer&) = delete;
  Lexer& operator=(const Lexer&) = delete;

  Lexer& operator++();
  const Token& operator*() const noexcept;
  const Token* operator->() const noexcept;
  operator bool() const noexcept;

  bool ok() const noexcept;
  Error error() const;

  bool eof() const noexcept;

 private:
  Token next_token();

  Token lex_literal(const std::string& literal, TokenType type);

  Token lex_string();
  std::optional<std::string> lex_ctrl();

  Token lex_number();
  Token lex_integer();
  Token lex_fraction();
  Token lex_exponent();

  void strip_whitespace();

  Reader reader_;
  Token curr_;
  std::optional<Error> error_;
};

}  // namespace json
}  // namespace pulse
