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

template <>
class [[nodiscard]] Result<void> {
 public:
  struct Ok {};

  Result() : data_(Ok{}) {}

  Result(Error error) : data_(std::move(error)) {}

  bool ok() const { return std::holds_alternative<Ok>(data_); }

  const Error& error() const& { return std::get<Error>(data_); }

  Error&& error() && { return std::get<Error>(std::move(data_)); }

 private:
  std::variant<Ok, Error> data_;
};

}  // namespace pulse
