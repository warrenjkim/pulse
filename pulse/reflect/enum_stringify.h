#pragma once

#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "pulse/core/pretty_function.h"
#include "pulse/core/stringify.h"

namespace pulse::reflect::internal {

// An enum that no other `Stringify` specialization claims.
template <typename E>
concept AutoStringifiableEnum =
    std::is_enum_v<E> && !pulse::TypeName<E>().starts_with("std::") &&
    requires { E{std::underlying_type_t<E>{}}; };

// The underlying values scanned for enumerators, clamped to [-128, 255].
template <typename E>
struct EnumRange {
  using U = std::underlying_type_t<E>;

  static constexpr long long kMin = std::is_signed_v<U> ? -128 : 0;

  static constexpr long long kMax =
      std::numeric_limits<U>::max() < 255
          ? static_cast<long long>(std::numeric_limits<U>::max())
          : 255;
};

// True when `name` spells an enumerator rather than Clang's `(E)7` spelling of
// a value with no enumerator.
constexpr bool IsEnumerator(std::string_view name) {
  const size_t rparen = name.rfind(')');
  if (!name.starts_with('(') || rparen == std::string_view::npos ||
      rparen + 1 == name.size()) {
    return true;
  }

  for (size_t i = rparen + 1; i < name.size(); i++) {
    if (name[i] != '-' && (name[i] < '0' || name[i] > '9')) {
      return true;
    }
  }

  return false;
}

template <typename E, long long... I>
consteval std::array<std::string_view, sizeof...(I)> EnumNames(
    std::integer_sequence<long long, I...>) {
  return {pulse::ValueName<static_cast<E>(EnumRange<E>::kMin + I)>()...};
}

// The spelling of every value in `EnumRange<E>`, indexed from `kMin`.
template <typename E>
inline constexpr auto kEnumNames = EnumNames<E>(
    std::make_integer_sequence<long long,
                               EnumRange<E>::kMax - EnumRange<E>::kMin + 1>());

// Renders `value` as its qualified enumerator, or `E(n)` when it has none.
template <typename E>
std::string EnumToString(E value) {
  using R = EnumRange<E>;

  const auto underlying = std::to_underlying(value);
  const auto i = static_cast<long long>(underlying);
  if (i >= R::kMin && i <= R::kMax) {
    const std::string_view name = kEnumNames<E>[i - R::kMin];
    if (IsEnumerator(name)) {
      return std::string(name);
    }
  }

  return std::string(pulse::TypeName<E>()) + "(" + std::to_string(underlying) +
         ")";
}

}  // namespace pulse::reflect::internal

namespace pulse {

// Renders any scoped enum as its qualified enumerator.
//
// NOTE: An explicit `Stringify<E>` always wins over this partial
// specialization.
template <reflect::internal::AutoStringifiableEnum E>
struct Stringify<E> {
  static std::string ToString(const E& value) {
    return reflect::internal::EnumToString(value);
  }

  static std::string DebugString(const E& value) { return ToString(value); }
};

}  // namespace pulse
