#pragma once

#include <concepts>
#include <string>
#include <string_view>

namespace pulse {

template <typename T>
struct Stringify {
  static std::string to_string(const T& value);
};

template <typename T>
concept Stringifiable = requires(const T& t) {
  { Stringify<T>::to_string(t) } -> std::convertible_to<std::string>;
};

template <Stringifiable T>
std::string to_string(const T& value) {
  return Stringify<T>::to_string(value);
}

template <typename T>
concept StdToStringable = requires(const T& t) {
  { std::to_string(t) } -> std::same_as<std::string>;
};

template <StdToStringable T>
struct Stringify<T> {
  static std::string to_string(const T& value) { return std::to_string(value); }
};

template <>
struct Stringify<std::string_view> {
  static std::string to_string(const std::string_view& value) {
    return std::string(value);
  }
};

template <>
struct Stringify<std::string> {
  static std::string to_string(const std::string& value) { return value; }
};

}  // namespace pulse
