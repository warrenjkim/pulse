#pragma once

#include <string>
#include <string_view>

#include "pulse/core/container_stringify.h"  // IWYU pragma: export
#include "pulse/core/stringify.h"

namespace pulse::strings {

namespace internal {

inline std::string_view CatPiece(std::string_view sv) { return sv; }

inline std::string_view CatPiece(const std::string& s) { return s; }

inline std::string_view CatPiece(const char* s) { return s; }

template <Stringifiable T>
std::string CatPiece(const T& value) {
  return pulse::ToString(value);
}

}  // namespace internal

template <typename... Args>
void Append(std::string* dest, const Args&... args) {
  ((*dest += internal::CatPiece(args)), ...);
}

// TODO(should be Stringifiable)
template <typename... Args>
std::string Cat(const Args&... args) {
  std::string out;
  ((out += internal::CatPiece(args)), ...);
  return out;
}

}  // namespace pulse::strings
