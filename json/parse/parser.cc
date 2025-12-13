#include "pulse/json/parse/parser.h"

#include <cstddef>  // size_t
#include <cstdint>  // int32_t, uint32_t
#include <map>
#include <string>

#include "pulse/json/parse/lexer.h"
#include "pulse/json/parse/token.h"
#include "pulse/json/utils/exception.h"
#include "pulse/json/utils/to_string.h"

namespace {

uint32_t to_code_point(const char c1, const char c2, const char c3,
                       const char c4) {
  auto to_uint32 = [](const char c) -> uint32_t {
    if (c >= '0' && c <= '9') {
      return uint32_t(c - '0');
    } else if (c >= 'A' && c <= 'F') {
      return uint32_t(c - 'A' + 10);
    } else if (c >= 'a' && c <= 'f') {
      return uint32_t(c - 'a' + 10);
    }

    return 0;
  };

  uint32_t code_point = 0;
  code_point = (code_point << 4) | to_uint32(c1);
  code_point = (code_point << 4) | to_uint32(c2);
  code_point = (code_point << 4) | to_uint32(c3);
  code_point = (code_point << 4) | to_uint32(c4);

  return code_point;
}

// https://www.ietf.org/rfc/rfc3629.txt
void emit_utf8(uint32_t code_point, std::string& res) {
  if (code_point > 0x10FFFF) {
    throw pulse::json::ParseException(
        "Code point out of Unicode range (> 0x10FFFF)");
  } else if (0xD800 <= code_point && code_point <= 0xDFFF) {
    throw pulse::json::ParseException(
        "Code point in surrogate range [0xD800, 0xDFFF]");
  }

  if (code_point < 0x80) {
    res += (char)(code_point);
  } else if (code_point < 0x800) {
    res += (char)(0xC0 | (code_point >> 6));
    res += (char)(0x80 | (code_point & 0x3F));
  } else if (code_point < 0x10000) {
    res += (char)(0xE0 | (code_point >> 12));
    res += (char)(0x80 | ((code_point >> 6) & 0x3F));
    res += (char)(0x80 | (code_point & 0x3F));
  } else {
    res += (char)(0xF0 | (code_point >> 18));
    res += (char)(0x80 | ((code_point >> 12) & 0x3F));
    res += (char)(0x80 | ((code_point >> 6) & 0x3F));
    res += (char)(0x80 | (code_point & 0x3F));
  }
}

std::string resolve_unicode_sequences(const std::string& s) {
  std::string res;
  res.reserve(s.length());
  size_t i = 0;
  size_t j = 0;
  while (j < s.length()) {
    if (s[j] == '\\' && s[j + 1] == 'u') {
      res.append(s, i, j - i);
      j += 6;

      uint32_t code_point =
          to_code_point(s[j - 4], s[j - 3], s[j - 2], s[j - 1]);

      // Section 3.8 Surrogates
      // https://www.unicode.org/versions/Unicode15.0.0/ch03.pdf
      // 0xD800 <= high-surrogate code point <= 0xDBFF
      uint32_t high_surrogate_cp = code_point;
      if (0xD800 <= high_surrogate_cp && high_surrogate_cp <= 0xDBFF) {
        j += 6;
        if (j + 1 >= s.length() || s[j] != '\\' || s[j + 1] != 'u') {
          throw pulse::json::ParseException(
              "Expected low surrogate after high surrogate: " +
              s.substr(j - 6, 6));
        }

        // 0xDC00 <= low-surrogate code point <= 0xDFFF
        uint32_t low_surrogate_cp =
            to_code_point(s[j - 4], s[j - 3], s[j - 2], s[j - 1]);
        if (!(0xDC00 <= low_surrogate_cp && low_surrogate_cp <= 0xDFFF)) {
          throw pulse::json::ParseException(
              "Invalid low surrogate (" + s.substr(j - 6, 6) +
              ") after high surrogate: " + s.substr(j - 12, 6));
        }

        code_point = 0x10000 + ((high_surrogate_cp - 0xD800) << 10) +
                     (low_surrogate_cp - 0xDC00);
      }

      emit_utf8(code_point, res);
      i = j;
    }

    j++;
  }

  if (i < s.length()) {
    res.append(s, i, s.length() - i);
  }

  return res;
}

}  // namespace

namespace pulse {
namespace json {

Parser::Parser(Lexer lexer) : lexer_(std::move(lexer)) {}

Value Parser::parse() {
  ++lexer_;
  if (!lexer_.ok()) {
    throw ParseException(to_string(lexer_.error()));
  }

  Value json = parse_value();
  if (!lexer_.eof()) {
    throw ParseException("Unexpected token: " + to_string(*lexer_));
  }

  return json;
}

Value Parser::parse_value() {
  switch (lexer_->type) {
    case TokenType::BOOLEAN:
      return parse_boolean();
    case TokenType::JSON_NULL:
      return parse_null();
    case TokenType::STRING:
      return parse_string();
    case TokenType::DOUBLE:
    case TokenType::INTEGRAL:
      return parse_number();
    case TokenType::ARRAY_START:
      return parse_array();
    case TokenType::OBJECT_START:
      return parse_object();
    default:
      throw ParseException("Unexpected token: " + to_string(*lexer_));
  }
}

nullptr_t Parser::parse_null() {
  ++lexer_;

  return nullptr;
}

bool Parser::parse_boolean() {
  bool value = lexer_->value == "true";
  ++lexer_;

  return value;
}

std::string Parser::parse_string() {
  std::string value = resolve_unicode_sequences(lexer_->value);
  ++lexer_;

  return value;
}

Value Parser::parse_number() {
  Value value;
  switch (lexer_->type) {
    case TokenType::DOUBLE:
      value = std::stod(lexer_->value);
      break;
    case TokenType::INTEGRAL:
      value = std::stoi(lexer_->value);
      break;
    default:
      __builtin_unreachable();
  }

  ++lexer_;

  return value;
}

array_t Parser::parse_array() {
  ++lexer_;
  if (!lexer_.ok()) {
    throw ParseException(to_string(lexer_.error()));
  }

  array_t value;
  if (lexer_->type == TokenType::ARRAY_END) {
    ++lexer_;
    return value;
  }

  while (lexer_) {
    value.push_back(parse_value());
    if (lexer_->type == TokenType::ARRAY_END) {
      ++lexer_;
      return value;
    }

    if (lexer_->type != TokenType::COMMA) {
      throw ParseException("Unexpected token: " + to_string(*lexer_));
    }

    ++lexer_;
  }

  throw ParseException(lexer_.ok() ? "Unterminated array"
                                   : to_string(lexer_.error()));
}

object_t Parser::parse_object() {
  ++lexer_;
  if (!lexer_.ok()) {
    throw ParseException(to_string(lexer_.error()));
  }

  object_t value;
  if (lexer_->type == TokenType::OBJECT_END) {
    ++lexer_;
    return value;
  }

  while (lexer_) {
    std::string key = parse_string();
    if (lexer_->type != TokenType::COLON) {
      throw ParseException("Unexpected token: " + to_string(*lexer_));
    }

    ++lexer_;
    if (!lexer_.ok()) {
      throw ParseException(to_string(lexer_.error()));
    }

    value[key] = parse_value();
    if (lexer_->type == TokenType::OBJECT_END) {
      ++lexer_;
      return value;
    }

    if (lexer_->type != TokenType::COMMA) {
      throw ParseException("Unexpected token: " + to_string(*lexer_));
    }

    ++lexer_;
  }

  throw ParseException(lexer_.ok() ? "Unterminated object"
                                   : to_string(lexer_.error()));
}

}  // namespace json
}  // namespace pulse
