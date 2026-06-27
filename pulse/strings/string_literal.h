#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>

namespace pulse::strings {

template <size_t N>
struct StringLiteral {
  constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }

  consteval operator std::string_view() const {
    return std::string_view(value, N - 1);
  }

  operator std::string() const { return std::string(value, N - 1); }

  char value[N];
};

}  // namespace pulse::strings
