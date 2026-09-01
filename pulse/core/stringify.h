#pragma once

#include <concepts>
#include <cstdint>
#include <string>
#include <type_traits>

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

template <typename T>
  requires(std::same_as<std::remove_cv_t<T>, char*>)
std::string ToString(T value) {
  return Stringify<std::string>::ToString(value);
}

template <typename T>
  requires(std::is_pointer_v<T> && std::is_object_v<std::remove_pointer_t<T>> &&
           !std::same_as<std::remove_cv_t<std::remove_pointer_t<T>>, char>)
std::string ToString(T value) {
  constexpr char kHex[] = "0123456789abcdef";
  auto addr = reinterpret_cast<uintptr_t>(value);
  std::string out(2 + sizeof(addr) * 2, '0');
  out[1] = 'x';

  for (auto it = out.end() - 1; it != out.begin() + 1; --it) {
    *it = kHex[addr & 0xf];
    addr >>= 4;
  }

  return out;
}

}  // namespace pulse
