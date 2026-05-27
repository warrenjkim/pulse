#pragma once

#include <string>
#include <string_view>

#include "core/stringify.h"

namespace pulse::strings {

namespace internal {

inline std::string_view cat_piece(std::string_view sv) { return sv; }

inline std::string_view cat_piece(const std::string& s) { return s; }

inline std::string_view cat_piece(const char* s) { return s; }

template <Stringifiable T>
std::string cat_piece(const T& value) {
  return pulse::to_string(value);
}

}  // namespace internal

template <typename... Args>
std::string cat(const Args&... args) {
  std::string out;
  ((out += internal::cat_piece(args)), ...);
  return out;
}

}  // namespace pulse::strings
