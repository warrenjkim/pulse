#pragma once

#include <concepts>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

#include "pulse/core/error.h"
#include "pulse/core/result.h"

namespace pulse::http {

template <typename T>
struct ParameterParser {};

template <typename T>
concept Parseable = requires(std::string_view sv) {
  { ParameterParser<T>::Parse(sv) } -> std::same_as<Result<T>>;
};

template <>
struct ParameterParser<bool> {
  static Result<bool> Parse(std::string_view value) {
    if (value == "true") {
      return true;
    }

    if (value == "false") {
      return false;
    }

    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "invalid bool: " + std::string(value)};
  }
};

template <>
struct ParameterParser<int> {
  static Result<int> Parse(std::string_view value) {
    int val;
    if (std::sscanf(value.data(), "%d", &val) == 1) {
      return val;
    }
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "invalid int: " + std::string(value)};
  }
};

template <>
struct ParameterParser<int64_t> {
  static Result<int64_t> Parse(std::string_view value) {
    int64_t val;
    if (std::sscanf(value.data(), "%lld", &val) == 1) {
      return val;
    }
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "invalid int64_t: " + std::string(value)};
  }
};

template <>
struct ParameterParser<double> {
  static Result<double> Parse(std::string_view value) {
    double val;
    if (std::sscanf(value.data(), "%lf", &val) == 1) {
      return val;
    }
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "invalid double: " + std::string(value)};
  }
};

template <>
struct ParameterParser<std::string> {
  static Result<std::string> Parse(std::string_view value) {
    return std::string(value);
  }
};

}  // namespace pulse::http
