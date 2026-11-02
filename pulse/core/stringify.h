#pragma once

#include <concepts>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include "pulse/core/pretty_function.h"
#include "pulse/reflect/mirror.h"

namespace pulse {

// Specialize to make a type stringifiable:
//
//   template <>
//   struct pulse::Stringify<YourType> {
//     static std::string ToString(const YourType& value);
//   };
//
// NOTE: `DebugString` is optional. Specializations that do not define it fall
// back to `ToString` via the free `DebugString` below.
template <typename T>
struct Stringify;

template <typename T>
concept Stringifiable = requires(const T& t) {
  { Stringify<T>::ToString(t) } -> std::convertible_to<std::string>;
};

template <typename T>
concept DebugStringifiable = requires(const T& t) {
  { Stringify<T>::DebugString(t) } -> std::convertible_to<std::string>;
};

template <Stringifiable T>
std::string ToString(const T& value) {
  return Stringify<T>::ToString(value);
}

template <Stringifiable T>
std::string DebugString(const T& value) {
  if constexpr (DebugStringifiable<T>) {
    return Stringify<T>::DebugString(value);
  } else {
    return Stringify<T>::ToString(value);
  }
}

namespace internal {

// `char*`, `const char*`, `char[N]`, `const char[N]`.
template <typename T>
concept CString = std::same_as<std::decay_t<T>, char*> ||
                  std::same_as<std::decay_t<T>, const char*>;

// `void*` and `const void*`.
template <typename T>
concept VoidPointer = std::same_as<std::decay_t<T>, void*> ||
                      std::same_as<std::decay_t<T>, const void*>;

// The character types rendered as text rather than as numbers. `signed char`
// and `unsigned char` are excluded on purpose: `uint8_t` is `unsigned char`,
// and a byte should log as `42`, not `"*"`.
template <typename T>
concept CharLike = std::same_as<T, char> || std::same_as<T, wchar_t> ||
                   std::same_as<T, char8_t> || std::same_as<T, char16_t> ||
                   std::same_as<T, char32_t>;

inline std::string HexAddress(const void* value) {
  static constexpr char kHex[] = "0123456789abcdef";
  auto addr = reinterpret_cast<uintptr_t>(value);
  std::string out(2 + sizeof(addr) * 2, '0');
  out[1] = 'x';
  for (auto it = out.end() - 1; it != out.begin() + 1; --it) {
    *it = kHex[addr & 0xf];
    addr >>= 4;
  }

  return out;
}

}  // namespace internal

// Arithmetic types rendered by `std::to_string`.
template <typename T>
concept StdToStringable =
    std::is_arithmetic_v<T> && !std::same_as<T, bool> && !internal::CharLike<T>;

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

template <>
struct Stringify<bool> {
  static std::string ToString(bool value) { return value ? "true" : "false"; }
};

template <>
struct Stringify<char> {
  static std::string ToString(char value) {
    return Stringify<std::string>::ToString(std::string(1, value));
  }
};

template <internal::CString T>
struct Stringify<T> {
  static std::string ToString(const char* value) {
    return value == nullptr ? "nullptr"
                            : Stringify<std::string>::ToString(value);
  }
};

template <internal::VoidPointer T>
struct Stringify<T> {
  static std::string ToString(const void* value) {
    return internal::HexAddress(value);
  }
};

template <Stringifiable T>
struct Stringify<std::optional<T>> {
  static std::string ToString(const std::optional<T>& value) {
    std::string out = "std::optional<";
    out += TypeName<T>();
    out += ">{";
    out += value.has_value() ? Stringify<T>::ToString(*value) : "nullopt";
    out += "}";

    return out;
  }
};

namespace internal {

template <typename M>
inline constexpr bool kFieldsStringifiable = false;

template <typename T, typename... D>
inline constexpr bool kFieldsStringifiable<reflect::Mirror<T, D...>> =
    (Stringifiable<std::remove_cvref_t<typename D::Type>> && ...);

// A class with member `begin()` and `end()`, like a container.
template <typename T>
concept MemberRange = requires(const T& value) {
  value.begin();
  value.end();
};

// A reflected class whose fields are all stringifiable.
//
// NOTE: Member ranges are excluded so a reflected class that is also a range
// keeps rendering as a container instead of matching two specializations.
template <typename T>
concept ReflectedStringifiable = reflect::Reflected<T> && !MemberRange<T> &&
                                 kFieldsStringifiable<reflect::Describe<T>>;

}  // namespace internal

// Renders a class marked `pulse_reflect` with its field names:
//
//   ns::A{.i=2,.j=true}
template <internal::ReflectedStringifiable T>
struct Stringify<T> {
  static std::string ToString(const T& value) {
    std::string out(TypeName<T>());
    out += "{";

    bool first = true;
    reflect::ForEachField(
        value, [&first, &out](std::string_view name, const auto& field) {
          if (!first) {
            out += ",";
          }

          first = false;
          out += ".";
          out += name;
          out += "=";
          out += pulse::ToString(field);
        });
    out += "}";

    return out;
  }
};

}  // namespace pulse
