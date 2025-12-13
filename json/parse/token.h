#pragma once

#include <string>

namespace pulse {
namespace json {

enum class TokenType {
  OBJECT_START,
  OBJECT_END,
  ARRAY_START,
  ARRAY_END,
  QUOTE,
  COMMA,
  COLON,
  STRING,
  DOUBLE,
  INTEGRAL,
  BOOLEAN,
  JSON_NULL,
  UNKNOWN,
  END_OF_JSON
};

struct Token {
  TokenType type;
  std::string value;

  explicit Token(TokenType type, std::string value)
      : type(type), value(std::move(value)) {}

  explicit Token(TokenType type, char value)
      : type(type), value(std::string(1, value)) {}

  bool operator==(const Token& other) const noexcept {
    return value == other.value && type == other.type;
  }
};

}  // namespace json
}  // namespace pulse
