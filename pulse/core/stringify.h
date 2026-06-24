#pragma once

#include <concepts>
#include <string>

namespace pulse {

template <typename T>
struct Stringify {
  static std::string ToString(const T& value);
};

template <typename T>
concept Stringifiable = requires(const T& t) {
  { Stringify<T>::ToString(t) } -> std::convertible_to<std::string>;
};

template <Stringifiable T>
std::string ToString(const T& value) {
  return Stringify<T>::ToString(value);
}

template <typename T>
concept StdToStringable = requires(const T& t) {
  { std::to_string(t) } -> std::same_as<std::string>;
};

template <StdToStringable T>
struct Stringify<T> {
  static std::string ToString(const T& value) { return std::to_string(value); }
};

template <>
struct Stringify<std::string> {
  static std::string ToString(const std::string& value) {
    std::string out = "\"";
    for (char c : value) {
      if (c == '"' || c == '\\') {
        out += '\\';
      }

      out += c;
    }

    return out + "\"";
  }
};

}  // namespace pulse
