#pragma once

#include <string>
#include <utility>
#include <variant>

#include "core/error.h"
#include "core/stringify.h"

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

template <typename T>
struct pulse::Stringify<pulse::Result<T>> {
  static std::string to_string(const pulse::Result<T>& result) {
    if (result.ok()) {
      return "Result{.value=" + pulse::Stringify<T>::to_string(*result) + "}";
    }

    return "Result{.error=" +
           pulse::Stringify<pulse::Error>::to_string(result.error()) + "}";
  }
};

template <>
struct pulse::Stringify<pulse::Result<void>> {
  static std::string to_string(const pulse::Result<void>& result) {
    if (result.ok()) {
      return "Result{.value=Ok{}}";
    }

    return "Result{.error=" +
           pulse::Stringify<pulse::Error>::to_string(result.error()) + "}";
  }
};
