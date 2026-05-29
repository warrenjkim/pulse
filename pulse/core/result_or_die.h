#pragma once

#include <cstdlib>
#include <source_location>
#include <utility>

#include "core/log.h"
#include "core/result.h"

namespace pulse {

template <typename T>
[[nodiscard]] T unwrap_or_die(
    Result<T> value,
    std::source_location loc = std::source_location::current()) {
  if (!value.ok()) {
    Log(loc) << "unwrap_or_die: " << value.error().message;
    std::abort();
  }

  return *std::move(value);
}

inline void die_if_error(
    Result<void> value,
    std::source_location loc = std::source_location::current()) {
  if (!value.ok()) {
    Log(loc) << "die_if_error: " << value.error().message;
    std::abort();
  }
}

}  // namespace pulse
