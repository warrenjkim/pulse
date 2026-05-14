#pragma once

#include <string>

#include "core/enum_macros.h"
#include "core/stringify.h"

namespace pulse {

#define ERROR_CODE_TABLE(X) X(kInternal, "INTERNAL")

struct Error {
  PULSE_ENUM(Code, ERROR_CODE_TABLE)

  Code code;
  std::string message;

  friend bool operator==(const Error&, const Error&) = default;
};

}  // namespace pulse

PULSE_ENUM_TO_STRING(pulse::Error::Code, ERROR_CODE_TABLE)

template <>
struct pulse::Stringify<pulse::Error> {
  static std::string to_string(const pulse::Error& error) {
    return "[" + pulse::Stringify<pulse::Error::Code>::to_string(error.code) +
           "] " + error.message;
  }
};
