#pragma once

#include <memory>
#include <string>
#include <utility>
#include <variant>

#include "pulse/core/error.h"
#include "pulse/core/stringify.h"
#include "pulse/strings/cat.h"

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

template <typename T>
class [[nodiscard]] Result<T&> {
 public:
  Result(T& value) : data_(std::addressof(value)) {}

  Result(Error error) : data_(std::move(error)) {}

  bool ok() const { return std::holds_alternative<T*>(data_); }

  const Error& error() const& { return std::get<Error>(data_); }

  Error&& error() && { return std::get<Error>(std::move(data_)); }

  T& operator*() const { return *std::get<T*>(data_); }

  T* operator->() const { return std::get<T*>(data_); }

 private:
  std::variant<T*, Error> data_;
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
  static std::string ToString(const pulse::Result<T>& result) {
    if (result.ok()) {
      return strings::Cat("Result{.value=", *result, "}");
    }

    return strings::Cat("Result{.error=", result.error(), "}");
  }
};

template <>
struct pulse::Stringify<pulse::Result<void>> {
  static std::string ToString(const pulse::Result<void>& result) {
    if (result.ok()) {
      return "Result{.value=Ok{}}";
    }

    return strings::Cat("Result{.error=", result.error(), "}");
  }
};
