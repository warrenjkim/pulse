#pragma once

#include <concepts>
#include <cstddef>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

#include "pulse/core/demangle.h"
#include "pulse/core/stringify.h"

namespace pulse::internal {

template <typename T>
concept PairLike = requires(const T& value) {
  value.first;
  value.second;
};

template <typename T>
concept StringLike = std::convertible_to<const T&, std::string_view>;

template <typename T>
using RangeValue = std::ranges::range_value_t<T>;

template <typename T>
using Key = std::remove_cvref_t<decltype(RangeValue<T>::first)>;

template <typename T>
using Value = std::remove_cvref_t<decltype(RangeValue<T>::second)>;

template <typename T>
concept RenderableRange =
    std::ranges::input_range<T> && !StringLike<T> && !PairLike<RangeValue<T>> &&
    Stringifiable<RangeValue<T>>;

template <typename T>
concept RenderableMap =
    std::ranges::input_range<T> && !StringLike<T> && PairLike<RangeValue<T>> &&
    Stringifiable<Key<T>> && Stringifiable<Value<T>>;

// Returns the name of a range without its template arguments.
//
//   IntBag -> IntBag
//   std::vector<int,...> -> std::vector
inline std::string RangeName(std::string_view type_name) {
  return std::string(type_name.substr(0, type_name.find('<')));
}

template <typename T>
bool HasTemplateArguments() {
  return TypeName<T>().find('<') != std::string_view::npos;
}

// Returns the canonical type label used when rendering a value inside a
// container.
//
// NOTE: Container types are reconstructed from their element types so
// implementation-specific template arguments such as allocators are omitted.
template <typename T>
const std::string& TypeLabel();

// Resolves the canonical type label for `T`.
//
// Container labels are reconstructed recursively from their key/value or
// element types. Non-container types use `TypeName<T>()`, while explicitly
// aliased types use their canonical alias.
template <typename T>
std::string CanonicalTypeLabel() {
  if constexpr (!TypeAlias<T>::kName.empty()) {
    return std::string(TypeAlias<T>::kName);
  } else if constexpr (RenderableMap<T>) {
    const std::string name = RangeName(TypeName<T>());
    if (!HasTemplateArguments<T>()) {
      return name;
    }

    return name + "<" + TypeLabel<Key<T>>() + "," + TypeLabel<Value<T>>() + ">";
  } else if constexpr (RenderableRange<T>) {
    const std::string name = RangeName(TypeName<T>());
    if (!HasTemplateArguments<T>()) {
      return name;
    }

    return name + "<" + TypeLabel<RangeValue<T>>() + ">";
  } else {
    return TypeName<T>();
  }
}

template <typename T>
const std::string& TypeLabel() {
  static const std::string label = CanonicalTypeLabel<T>();
  return label;
}

}  // namespace pulse::internal

namespace pulse {

template <internal::RenderableRange R>
struct Stringify<R> {
  static std::string ToString(const R& range) {
    std::string out = internal::TypeLabel<R>() + "{";

    bool first = true;
    for (const auto& element : range) {
      if (!first) {
        out += ",";
      }

      first = false;
      out += Stringify<internal::RangeValue<R>>::ToString(element);
    }

    return out + "}";
  }

  static std::string DebugString(const R& range) { return ToString(range); }
};

template <internal::RenderableMap R>
struct Stringify<R> {
  static std::string ToString(const R& range) {
    std::string out = internal::TypeLabel<R>() + "{";

    bool first = true;
    for (const auto& [key, value] : range) {
      if (!first) {
        out += ",";
      }

      first = false;
      out += "{" + Stringify<internal::Key<R>>::ToString(key) + "," +
             Stringify<internal::Value<R>>::ToString(value) + "}";
    }

    return out + "}";
  }

  static std::string DebugString(const R& range) { return ToString(range); }
};

}  // namespace pulse
