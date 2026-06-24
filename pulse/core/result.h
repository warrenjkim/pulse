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
  constexpr Result(T value) : data_(std::move(value)) {}

  constexpr Result(Error error) : data_(std::move(error)) {}

  constexpr bool ok() const { return std::holds_alternative<T>(data_); }

  constexpr const Error& error() const& { return std::get<Error>(data_); }

  constexpr Error&& error() && { return std::get<Error>(std::move(data_)); }

  constexpr const T& operator*() const& { return std::get<T>(data_); }

  constexpr T& operator*() & { return std::get<T>(data_); }

  constexpr T&& operator*() && { return std::get<T>(std::move(data_)); }

  constexpr const T* operator->() const { return &std::get<T>(data_); }

  constexpr T* operator->() { return &std::get<T>(data_); }

 private:
  std::variant<T, Error> data_;
};

template <typename T>
class [[nodiscard]] Result<T&> {
 public:
  constexpr Result(T& value) : data_(std::addressof(value)) {}

  constexpr Result(Error error) : data_(std::move(error)) {}

  constexpr bool ok() const { return std::holds_alternative<T*>(data_); }

  constexpr const Error& error() const& { return std::get<Error>(data_); }

  constexpr Error&& error() && { return std::get<Error>(std::move(data_)); }

  constexpr T& operator*() const { return *std::get<T*>(data_); }

  constexpr T* operator->() const { return std::get<T*>(data_); }

 private:
  std::variant<T*, Error> data_;
};

template <>
class [[nodiscard]] Result<void> {
 public:
  struct Ok {};

  constexpr Result() : data_(Ok{}) {}

  constexpr Result(Error error) : data_(std::move(error)) {}

  constexpr bool ok() const { return std::holds_alternative<Ok>(data_); }

  constexpr const Error& error() const& { return std::get<Error>(data_); }

  constexpr Error&& error() && { return std::get<Error>(std::move(data_)); }

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
