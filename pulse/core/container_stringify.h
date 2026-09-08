#pragma once

#include <concepts>
#include <cstddef>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#include "pulse/core/pretty_function.h"
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
concept HasTupleSize = requires { std::tuple_size<T>::value; };

template <typename T>
concept HasStaticExtent = requires {
  { T::extent } -> std::convertible_to<size_t>;
  requires T::extent != std::dynamic_extent;
};

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

// Returns the canonical type label used when rendering a value inside a
// container.
//
// NOTE: Container types are reconstructed from their element types so
// implementation-specific template arguments such as allocators are omitted.
template <typename T>
const std::string& TypeLabel();

// Returns the name of `type_name` without its template arguments.
//
//   Struct -> Struct
//   std::vector<int,...> -> std::vector
inline std::string TemplateName(std::string_view type_name) {
  return std::string(type_name.substr(0, type_name.find('<')));
}

template <typename T>
consteval bool HasTemplateArguments() {
  return TypeName<T>().find('<') != std::string_view::npos;
}

// Returns the bracketed extents of an array type, outermost first.
//
//   int[] -> []
//   int[1] -> [1]
//   int[2][3] -> [2][3]
template <typename T>
std::string ArrayExtents() {
  if constexpr (std::is_array_v<T>) {
    constexpr size_t kExtent = std::extent_v<T>;
    return "[" + (kExtent != 0 ? std::to_string(kExtent) : "") + "]" +
           ArrayExtents<std::remove_extent_t<T>>();
  } else {
    return "";
  }
}

// Resolves the canonical type label for `T`.
//
// Container labels are reconstructed recursively from their key/value or
// element types. Non-container types use `TypeName<T>()`, while explicitly
// aliased types use their canonical alias.
template <typename T>
std::string CanonicalTypeLabel() {
  if constexpr (!TypeAlias<T>::kName.empty()) {
    return std::string(TypeAlias<T>::kName);
  } else if constexpr (std::is_array_v<T>) {
    return TypeLabel<std::remove_all_extents_t<T>>() + ArrayExtents<T>();
  } else if constexpr (RenderableMap<T>) {
    if constexpr (!HasTemplateArguments<T>()) {
      return TemplateName(TypeName<T>());
    } else {
      return TemplateName(TypeName<T>()) + "<" + TypeLabel<Key<T>>() + "," +
             TypeLabel<Value<T>>() + ">";
    }
  } else if constexpr (RenderableRange<T>) {
    if constexpr (HasTupleSize<T>) {
      return TemplateName(TypeName<T>()) + "<" + TypeLabel<RangeValue<T>>() +
             "," + std::to_string(std::tuple_size_v<T>) + ">";
    } else if constexpr (HasStaticExtent<T>) {
      return TemplateName(TypeName<T>()) + "<" + TypeLabel<RangeValue<T>>() +
             "," + std::to_string(T::extent) + ">";
    } else if constexpr (!HasTemplateArguments<T>()) {
      return TemplateName(TypeName<T>());
    } else {
      return TemplateName(TypeName<T>()) + "<" + TypeLabel<RangeValue<T>>() +
             ">";
    }
  } else {
    return std::string(TypeName<T>());
  }
}

template <typename T>
const std::string& TypeLabel() {
  static const std::string label = CanonicalTypeLabel<T>();
  return label;
}

// Renders `range` as `Label{a,b,c}`, with each element rendered by `render`.
template <typename R, std::invocable<const RangeValue<R>&> F>
std::string RenderRange(const R& range, const F& render) {
  std::string out = TypeLabel<R>() + "{";

  bool first = true;
  for (const auto& element : range) {
    if (!first) {
      out += ",";
    }

    first = false;
    out += render(element);
  }

  return out + "}";
}

}  // namespace pulse::internal

namespace pulse {

template <internal::RenderableRange R>
struct Stringify<R> {
  static std::string ToString(const R& range) {
    return internal::RenderRange(
        range, [](const internal::RangeValue<R>& element) {
          return Stringify<internal::RangeValue<R>>::ToString(element);
        });
  }

  static std::string DebugString(const R& range) { return ToString(range); }
};

template <internal::RenderableMap R>
struct Stringify<R> {
  static std::string ToString(const R& range) {
    return internal::RenderRange(
        range, [](const internal::RangeValue<R>& entry) {
          return "{" + Stringify<internal::Key<R>>::ToString(entry.first) +
                 "," + Stringify<internal::Value<R>>::ToString(entry.second) +
                 "}";
        });
  }

  static std::string DebugString(const R& range) { return ToString(range); }
};

}  // namespace pulse
