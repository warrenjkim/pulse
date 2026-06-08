#pragma once

#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "pulse/core/stringify.h"
#include "pulse/strings/cat.h"

namespace pulse::json {

class value;

using array_t = std::vector<value>;
using object_t = std::map<std::string, value, std::less<>>;

template <typename T>
concept JsonType = std::same_as<T, std::nullptr_t> || std::same_as<T, bool> ||
                   std::same_as<T, int64_t> || std::same_as<T, double> ||
                   std::same_as<T, std::string> || std::same_as<T, array_t> ||
                   std::same_as<T, object_t>;

template <typename T>
concept StringKey = std::convertible_to<std::decay_t<T>, std::string_view>;

template <typename T>
concept IndexKey =
    std::integral<std::decay_t<T>> && !std::same_as<std::decay_t<T>, bool> &&
    !std::same_as<std::decay_t<T>, char>;

template <typename T>
concept KeyType = StringKey<T> || IndexKey<T>;

class value {
 public:
  value() : value_(nullptr) {}

  value(nullptr_t) : value_(nullptr) {}

  value(bool b) : value_(b) {}

  value(std::string s) : value_(std::move(s)) {}

  value(std::string_view s) : value_(std::string(s)) {}

  value(const char* s) : value_(std::string(s)) {}

  value(array_t a) : value_(std::move(a)) {}

  value(object_t o) : value_(std::move(o)) {}

  template <typename T>
    requires(std::same_as<T, char>)
  value(T c) : value_(std::string(1, c)) {}

  template <std::integral T>
    requires(!std::same_as<T, bool>)
  value(T n) : value_(static_cast<int64_t>(n)) {}

  template <std::floating_point T>
  value(T d) : value_(static_cast<double>(d)) {}

  ~value() = default;

  value(const value&) = default;

  value(value&&) = default;

  value& operator=(const value&) = default;

  value& operator=(value&&) = default;

  template <typename T>
    requires(!std::same_as<std::decay_t<T>, value>)
  value& operator=(T&& v);

  template <KeyType T>
  value& operator[](const T& key);

  template <JsonType T>
  bool is() const noexcept;

  template <JsonType T>
  T& as();

  template <JsonType T>
  const T& as() const;

  friend bool operator==(const value&, const value&) = default;

  template <JsonType T>
  friend bool operator==(const value& v, const T& other) noexcept;

 private:
  friend struct Stringify<value>;

  std::string_view type_name() const noexcept;

  template <JsonType T>
  static constexpr std::string_view type_name() noexcept;

  std::variant<std::nullptr_t, bool, int64_t, double, std::string, array_t,
               object_t>
      value_;
};

// Implementation details below;

template <typename T>
  requires(!std::same_as<std::decay_t<T>, value>)
value& value::operator=(T&& v) {
  value_ = std::forward<T>(v);

  return *this;
}

template <KeyType T>
value& value::operator[](const T& key) {
  if constexpr (StringKey<T>) {
    if (std::holds_alternative<std::nullptr_t>(value_)) {
      value_.emplace<object_t>();
    }

    return std::get<object_t>(value_)[key];
  }

  if constexpr (IndexKey<T>) {
    if (std::holds_alternative<std::nullptr_t>(value_)) {
      value_.emplace<array_t>();
    }

    return std::get<array_t>(value_)[static_cast<size_t>(key)];
  }
}

template <JsonType T>
bool value::is() const noexcept {
  return std::holds_alternative<T>(value_);
}

template <JsonType T>
T& value::as() {
  return std::get<T>(value_);
}

template <JsonType T>
const T& value::as() const {
  return std::get<T>(value_);
}

template <JsonType T>
bool operator==(const value& v, const T& other) noexcept {
  return std::holds_alternative<T>(v.value_) && std::get<T>(v.value_) == other;
}

inline std::string_view value::type_name() const noexcept {
  switch (value_.index()) {
    case 0:
      return "null";
    case 1:
      return "bool";
    case 2:
      return "int64";
    case 3:
      return "double";
    case 4:
      return "string";
    case 5:
      return "array";
    case 6:
      return "object";
  }

  std::unreachable();
}

template <JsonType T>
constexpr std::string_view value::type_name() noexcept {
  if constexpr (std::same_as<T, std::nullptr_t>) {
    return "null";
  } else if constexpr (std::same_as<T, bool>) {
    return "bool";
  } else if constexpr (std::same_as<T, int64_t>) {
    return "int64";
  } else if constexpr (std::same_as<T, double>) {
    return "double";
  } else if constexpr (std::same_as<T, std::string>) {
    return "string";
  } else if constexpr (std::same_as<T, array_t>) {
    return "array";
  } else if constexpr (std::same_as<T, object_t>) {
    return "object";
  }
}

}  // namespace pulse::json

template <>
struct pulse::Stringify<pulse::json::array_t> {
  static std::string to_string(const pulse::json::array_t& array) {
    std::string out;
    pulse::Stringify<pulse::json::array_t>::to_string(array, &out);
    return out;
  }

  static void to_string(const pulse::json::array_t& v, std::string* out) {
    pulse::strings::append(out, "[");
    for (const auto& v : v) {
      pulse::strings::append(out, v, ",");
    }

    if (out->back() == ',') {
      out->pop_back();
    }

    pulse::strings::append(out, "]");
  }
};

template <>
struct pulse::Stringify<pulse::json::object_t> {
  static std::string to_string(const pulse::json::object_t& object) {
    std::string out;
    pulse::Stringify<pulse::json::object_t>::to_string(object, &out);
    return out;
  }

  static void to_string(const pulse::json::object_t& v, std::string* out) {
    pulse::strings::append(out, "{");
    for (const auto& [k, v] : v) {
      pulse::strings::append(out, "\"", k, "\"", ":", v, ",");
    }

    if (out->back() == ',') {
      out->pop_back();
    }

    pulse::strings::append(out, "}");
  }
};

template <>
struct pulse::Stringify<pulse::json::value> {
  static std::string to_string(const pulse::json::value& v) {
    std::string out;
    pulse::Stringify<pulse::json::value>::to_string(v, &out);
    return out;
  }

 private:
  static void to_string(const pulse::json::value& v, std::string* out) {
    std::visit(
        [out](const auto& v) {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::same_as<T, std::nullptr_t>) {
            pulse::strings::append(out, "null");
          } else if constexpr (std::same_as<T, bool>) {
            pulse::strings::append(out, v ? "true" : "false");
          } else if constexpr (std::same_as<T, int64_t>) {
            pulse::strings::append(out, v);
          } else if constexpr (std::same_as<T, double>) {
            char buffer[32];
            auto [ptr, unused_ec] = std::to_chars(
                buffer, buffer + sizeof(buffer), v, std::chars_format::general);
            *ptr = '\0';
            pulse::strings::append(out, buffer);
          } else if constexpr (std::same_as<T, std::string>) {
            pulse::strings::append(out, "\"", v, "\"");
          } else if constexpr (std::same_as<T, pulse::json::array_t>) {
            pulse::Stringify<pulse::json::array_t>::to_string(v, out);
          } else if constexpr (std::same_as<T, pulse::json::object_t>) {
            pulse::Stringify<pulse::json::object_t>::to_string(v, out);
          }
        },
        v.value_);
  }
};
