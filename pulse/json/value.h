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

class Value;

using Array = std::vector<Value>;
using Object = std::map<std::string, Value, std::less<>>;

template <typename T>
concept JsonType = std::same_as<T, std::nullptr_t> || std::same_as<T, bool> ||
                   std::same_as<T, int64_t> || std::same_as<T, double> ||
                   std::same_as<T, std::string> || std::same_as<T, Array> ||
                   std::same_as<T, Object>;

template <typename T>
concept StringKey = std::convertible_to<std::decay_t<T>, std::string_view>;

template <typename T>
concept IndexKey =
    std::integral<std::decay_t<T>> && !std::same_as<std::decay_t<T>, bool> &&
    !std::same_as<std::decay_t<T>, char>;

template <typename T>
concept KeyType = StringKey<T> || IndexKey<T>;

class Value {
 public:
  constexpr Value() : value_(nullptr) {}

  constexpr Value(nullptr_t) : value_(nullptr) {}

  constexpr Value(bool b) : value_(b) {}

  constexpr Value(std::string s) : value_(std::move(s)) {}

  constexpr Value(std::string_view s) : value_(std::string(s)) {}

  constexpr Value(const char* s) : value_(std::string(s)) {}

  constexpr Value(Array a) : value_(std::move(a)) {}

  constexpr Value(Object o) : value_(std::move(o)) {}

  template <typename T>
    requires(std::same_as<T, char>)
  constexpr Value(T c) : value_(std::string(1, c)) {}

  template <std::integral T>
    requires(!std::same_as<T, bool>)
  constexpr Value(T n) : value_(static_cast<int64_t>(n)) {}

  template <std::floating_point T>
  constexpr Value(T d) : value_(static_cast<double>(d)) {}

  constexpr ~Value() = default;

  constexpr Value(const Value&) = default;

  constexpr Value(Value&&) = default;

  constexpr Value& operator=(const Value&) = default;

  constexpr Value& operator=(Value&&) = default;

  template <typename T>
    requires(!std::same_as<std::decay_t<T>, Value>)
  constexpr Value& operator=(T&& v);

  template <KeyType T>
  constexpr Value& operator[](const T& key);

  template <JsonType T>
  constexpr bool is() const noexcept;

  template <JsonType T>
  constexpr T& as();

  template <JsonType T>
  constexpr const T& as() const;

  friend constexpr bool operator==(const Value&, const Value&) = default;

  template <JsonType T>
  friend constexpr bool operator==(const Value& v, const T& other) noexcept;

 private:
  friend struct Stringify<Value>;

  constexpr std::string_view TypeName() const noexcept;

  template <JsonType T>
  static constexpr std::string_view TypeName() noexcept;

  std::variant<std::nullptr_t, bool, int64_t, double, std::string, Array,
               Object>
      value_;
};

// Implementation details below;

template <typename T>
  requires(!std::same_as<std::decay_t<T>, Value>)
constexpr Value& Value::operator=(T&& v) {
  value_ = std::forward<T>(v);

  return *this;
}

template <KeyType T>
constexpr Value& Value::operator[](const T& key) {
  if constexpr (StringKey<T>) {
    if (std::holds_alternative<std::nullptr_t>(value_)) {
      value_.emplace<Object>();
    }

    return std::get<Object>(value_)[key];
  }

  if constexpr (IndexKey<T>) {
    if (std::holds_alternative<std::nullptr_t>(value_)) {
      value_.emplace<Array>();
    }

    return std::get<Array>(value_)[static_cast<size_t>(key)];
  }
}

template <JsonType T>
constexpr bool Value::is() const noexcept {
  return std::holds_alternative<T>(value_);
}

template <JsonType T>
constexpr T& Value::as() {
  return std::get<T>(value_);
}

template <JsonType T>
constexpr const T& Value::as() const {
  return std::get<T>(value_);
}

template <JsonType T>
constexpr bool operator==(const Value& v, const T& other) noexcept {
  return std::holds_alternative<T>(v.value_) && std::get<T>(v.value_) == other;
}

constexpr std::string_view Value::TypeName() const noexcept {
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
constexpr std::string_view Value::TypeName() noexcept {
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
  } else if constexpr (std::same_as<T, Array>) {
    return "array";
  } else if constexpr (std::same_as<T, Object>) {
    return "object";
  }
}

}  // namespace pulse::json

template <>
struct pulse::Stringify<pulse::json::Array> {
  static std::string ToString(const pulse::json::Array& array) {
    std::string out;
    pulse::Stringify<pulse::json::Array>::ToString(array, &out);
    return out;
  }

  static void ToString(const pulse::json::Array& v, std::string* out) {
    pulse::strings::Append(out, "[");
    for (const auto& v : v) {
      pulse::strings::Append(out, v, ",");
    }

    if (out->back() == ',') {
      out->pop_back();
    }

    pulse::strings::Append(out, "]");
  }
};

template <>
struct pulse::Stringify<pulse::json::Object> {
  static std::string ToString(const pulse::json::Object& object) {
    std::string out;
    pulse::Stringify<pulse::json::Object>::ToString(object, &out);
    return out;
  }

  static void ToString(const pulse::json::Object& v, std::string* out) {
    pulse::strings::Append(out, "{");
    for (const auto& [k, v] : v) {
      pulse::strings::Append(out, "\"", k, "\"", ":", v, ",");
    }

    if (out->back() == ',') {
      out->pop_back();
    }

    pulse::strings::Append(out, "}");
  }
};

template <>
struct pulse::Stringify<pulse::json::Value> {
  static std::string ToString(const pulse::json::Value& v) {
    std::string out;
    pulse::Stringify<pulse::json::Value>::ToString(v, &out);
    return out;
  }

 private:
  static void ToString(const pulse::json::Value& v, std::string* out) {
    std::visit(
        [out](const auto& v) {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::same_as<T, std::nullptr_t>) {
            pulse::strings::Append(out, "null");
          } else if constexpr (std::same_as<T, bool>) {
            pulse::strings::Append(out, v ? "true" : "false");
          } else if constexpr (std::same_as<T, int64_t>) {
            pulse::strings::Append(out, v);
          } else if constexpr (std::same_as<T, double>) {
            char buffer[32];
            auto [ptr, unused_ec] = std::to_chars(
                buffer, buffer + sizeof(buffer), v, std::chars_format::general);
            *ptr = '\0';
            pulse::strings::Append(out, buffer);
          } else if constexpr (std::same_as<T, std::string>) {
            pulse::strings::Append(out, "\"", v, "\"");
          } else if constexpr (std::same_as<T, pulse::json::Array>) {
            pulse::Stringify<pulse::json::Array>::ToString(v, out);
          } else if constexpr (std::same_as<T, pulse::json::Object>) {
            pulse::Stringify<pulse::json::Object>::ToString(v, out);
          }
        },
        v.value_);
  }
};
