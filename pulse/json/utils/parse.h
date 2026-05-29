#pragma once

#include <string>

#include "pulse/json/parse/lexer.h"
#include "pulse/json/parse/parser.h"
#include "pulse/json/value.h"

namespace pulse {
namespace json {

inline Value operator""_json(const char* json, size_t len) {
  return Parser(Lexer(std::string(json, len))).parse();
}

inline Value parse(std::string json) {
  return Parser(Lexer(std::move(json))).parse();
}

}  // namespace json
}  // namespace pulse
