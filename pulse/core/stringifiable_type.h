#pragma once

#include <string>

#include "pulse/core/stringify.h"

namespace pulse {

namespace internal {

// A stand-in type for any type that is `Stringifiable`.
//
// A `requires` expression can only name concrete types, so placeholder types
// are not permitted. Used when a concept requires that some API accepts
// arbitrary `Stringifiable` values.
struct StringifiableType {};

}  // namespace internal

template <>
struct Stringify<internal::StringifiableType> {
  static std::string ToString(const internal::StringifiableType&) { return ""; }
};

}  // namespace pulse
