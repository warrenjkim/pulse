#include "pulse/json/utils/to_string.h"

#include <iomanip>
#include <sstream>
#include <string>

#include "pulse/json/parse/lexer.h"
#include "pulse/json/parse/token.h"
#include "pulse/json/value.h"

namespace {

struct Printer {
  size_t level = 0;
  const pulse::json::PrintOptions& opts;

  std::string indent() const {
    return opts.compact ? "" : std::string(level * opts.tab_width, ' ');
  }

  std::string format(double d) const {
    std::ostringstream oss;
    oss << std::setprecision(15) << std::defaultfloat << d;

    return oss.str();
  }

  std::string print(const pulse::json::Value& value) {
    return value.visit(
        []() -> std::string { return "null"; },
        [](bool b) -> std::string { return (b ? "true" : "false"); },
        [](int32_t i) -> std::string { return std::to_string(i); },
        [this](double d) -> std::string { return format(d); },
        [](const std::string& s) -> std::string { return "\"" + s + "\""; },
        [this](const pulse::json::array_t& a) -> std::string {
          if (a.empty()) {
            return "[]";
          }

          std::string array;
          array += "[";
          array += (opts.compact ? "" : "\n");
          level++;
          for (size_t i = 0; i < a.size(); i++) {
            array += indent() + print(a[i]) +
                     (i < a.size() - 1 || opts.trailing_commas ? "," : "") +
                     (opts.compact ? "" : "\n");
          }

          level--;
          array += indent() + "]";

          return array;
        },
        [this](const pulse::json::object_t& o) -> std::string {
          if (o.empty()) {
            return "{}";
          }

          std::string object;
          object += "{";
          object += (opts.compact ? "" : "\n");
          level++;
          size_t i = 0;
          for (const auto& [k, v] : o) {
            object += indent() + "\"" + k + "\":" + (opts.compact ? "" : " ") +
                      print(v) +
                      (i++ < o.size() - 1 || opts.trailing_commas ? "," : "") +
                      (opts.compact ? "" : "\n");
          }

          level--;
          object += indent() + "}";

          return object;
        });
  }
};

}  // namespace

namespace pulse {
namespace json {

std::string to_string(TokenType type) {
  switch (type) {
    case TokenType::OBJECT_START:
      return "TokenType::ObjectStart";
    case TokenType::OBJECT_END:
      return "TokenType::ObjectEnd";
    case TokenType::ARRAY_START:
      return "TokenType::ArrayStart";
    case TokenType::ARRAY_END:
      return "TokenType::ArrayEnd";
    case TokenType::QUOTE:
      return "TokenType::Quote";
    case TokenType::COMMA:
      return "TokenType::Comma";
    case TokenType::COLON:
      return "TokenType::Colon";
    case TokenType::STRING:
      return "TokenType::String";
    case TokenType::DOUBLE:
      return "TokenType::Double";
    case TokenType::INTEGRAL:
      return "TokenType::Integral";
    case TokenType::BOOLEAN:
      return "TokenType::Boolean";
    case TokenType::JSON_NULL:
      return "TokenType::Null";
    case TokenType::UNKNOWN:
      return "TokenType::Unknown";
    case TokenType::END_OF_JSON:
      return "TokenType::EndOfJson";
  }

  __builtin_unreachable();
}

std::string to_string(const Token& token) {
  return "(" + to_string(token.type) + ", " + token.value + ")";
}

std::string to_string(const Lexer::Error& error) {
  std::string msg = "Error at position " + std::to_string(error.pos) + ". ";
  if (error.expected != TokenType::UNKNOWN) {
    msg += "Expected " + to_string(error.expected) + ": ";
  }

  msg += error.message;

  return msg;
}

std::string to_string(const Value& value, const PrintOptions& opts) {
  return Printer{.opts = opts}.print(value);
}

}  // namespace json
}  // namespace pulse
