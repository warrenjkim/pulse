#pragma once

#include <cstdlib>
#include <source_location>
#include <utility>

#include "pulse/core/log.h"
#include "pulse/core/result.h"

namespace pulse {

template <typename T>
[[nodiscard]] T UnwrapOrDie(
    Result<T> value,
    std::source_location loc = std::source_location::current()) {
  if (!value.ok()) {
    Log(loc) << "UnwrapOrDie: " << value.error().message;
    std::abort();
  }

  return *std::move(value);
}

inline void DieIfError(
    Result<void> value,
    std::source_location loc = std::source_location::current()) {
  if (!value.ok()) {
    Log(loc) << "DieIfError: " << value.error().message;
    std::abort();
  }
}

}  // namespace pulse
