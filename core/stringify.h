#pragma once

#include <string>

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

}  // namespace pulse
