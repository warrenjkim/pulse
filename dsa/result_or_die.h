#pragma once

#include <cstdlib>
#include <iostream>
#include <source_location>
#include <utility>

#include "dsa/result.h"

namespace pulse {

template <typename T>
[[nodiscard]] T unwrap_or_die(
    Result<T> value,
    std::source_location loc_ = std::source_location::current()) {
  if (!value.ok()) {
    std::cerr << "[" << loc_.file_name() << ":" << loc_.line()
              << "] unwrap_or_die: " << value.error().message << "\n";
    std::abort();
  }

  return *std::move(value);
}

inline void die_if_error(
    Result<void> value,
    std::source_location loc_ = std::source_location::current()) {
  if (!value.ok()) {
    std::cerr << "[" << loc_.file_name() << ":" << loc_.line()
              << "] die_if_error: " << value.error().message << "\n";
    std::abort();
  }
}

}  // namespace pulse
