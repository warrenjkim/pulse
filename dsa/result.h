#pragma once

#include <ostream>
#include <sstream>
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

template <typename T>
  requires requires(std::ostream& os, const T& t) { os << t; }
inline std::string to_string(const Result<T>& result) {
  if (result.ok()) {
    std::ostringstream oss;
    oss << "Ok(" << *result << ")";
    return oss.str();
  }

  return "Err(" + to_string(result.error()) + ")";
}

template <typename T>
  requires requires(std::ostream& os, const T& t) { os << t; }
inline std::ostream& operator<<(std::ostream& os, const Result<T>& result) {
  return os << to_string(result);
}

inline std::string to_string(const Result<void>& result) {
  return result.ok() ? "Ok" : "Err(" + to_string(result.error()) + ")";
}

inline std::ostream& operator<<(std::ostream& os, const Result<void>& result) {
  return os << to_string(result);
}

}  // namespace pulse
