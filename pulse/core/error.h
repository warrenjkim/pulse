#pragma once

#include <string>

#include "pulse/core/enum_macros.h"
#include "pulse/core/stringify.h"
#include "pulse/strings/cat.h"

namespace pulse {

#define ERROR_CODE_TABLE(X)                     \
  X(kOk, "OK")                                  \
  X(kInvalidArgument, "INVALID_ARGUMENT")       \
  X(kNotFound, "NOT_FOUND")                     \
  X(kAlreadyExists, "ALREADY_EXISTS")           \
  X(kFailedPrecondition, "FAILED_PRECONDITION") \
  X(kUnavailable, "UNAVAILABLE")                \
  X(kInternal, "INTERNAL")

struct Error {
  PULSE_ENUM(Code, ERROR_CODE_TABLE);

  Code code;
  std::string message;

  friend bool operator==(const Error&, const Error&) = default;
};

}  // namespace pulse

PULSE_ENUM_TO_STRING(pulse::Error::Code, ERROR_CODE_TABLE);

template <>
struct pulse::Stringify<pulse::Error> {
  static std::string ToString(const pulse::Error& error) {
    return strings::Cat("Error{.code=", error.code, ",.message=", error.message,
                        "}");
  }
};
