#pragma once

#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

#include "pulse/core/pretty_function.h"
#include "pulse/core/stringify.h"
#include "pulse/reflect/mirror.h"

namespace pulse::internal {

template <typename M>
inline constexpr bool kFieldsStringifiable = false;

template <typename T, typename... D>
inline constexpr bool kFieldsStringifiable<reflect::Mirror<T, D...>> =
    (Stringifiable<std::remove_cvref_t<typename D::Type>> && ...);

// A reflected class whose fields are all stringifiable.
//
// NOTE: Ranges are excluded so a reflected class that is also a range keeps
// rendering as a container instead of matching two specializations.
template <typename T>
concept ReflectedStringifiable =
    reflect::Reflected<T> && !std::ranges::range<T> &&
    kFieldsStringifiable<reflect::Describe<T>>;

}  // namespace pulse::internal

namespace pulse {

// Renders a class marked `[[pulse::reflect]]` with its field names:
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
