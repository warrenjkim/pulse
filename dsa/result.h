#pragma once

#include <variant>

#include "dsa/error.h"

namespace pulse {

template <typename T>
class [[nodiscard]] Result {
 public:
  Result(T value) : data_(std::move(value)) {}

  Result(Error error) : data_(std::move(error)) {}

  bool ok() const { return std::holds_alternative<T>(data_); }

  const Error& error() const& { return std::get<Error>(data_); }

  Error&& error() && { return std::get<Error>(std::move(data_)); }

  const T& operator*() const& { return std::get<T>(data_); }

  T& operator*() & { return std::get<T>(data_); }

  T&& operator*() && { return std::get<T>(std::move(data_)); }

  const T* operator->() const { return &std::get<T>(data_); }

  T* operator->() { return &std::get<T>(data_); }

 private:
  std::variant<T, Error> data_;
};

#define RETURN_IF_ERROR(expr)            \
  do {                                   \
    auto&& _result = (expr);             \
    if (!_result.ok()) {                 \
      return std::move(_result).error(); \
    }                                    \
  } while (false)

#define ASSIGN_OR_RETURN(var, expr)          \
  auto&& _result_##var = (expr);             \
  if (!_result_##var.ok()) {                 \
    return std::move(_result_##var).error(); \
  }                                          \
  auto var = *std::move(_result_##var);

}  // namespace pulse
