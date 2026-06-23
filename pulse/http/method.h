#pragma once

#include "pulse/core/enum_macros.h"

namespace pulse::http {

#define METHOD_TABLE(X) \
  X(kGet, "GET")        \
  X(kPost, "POST")      \
  X(kPut, "PUT")        \
  X(kDelete, "DELETE")  \
  X(kOptions, "OPTIONS")

PULSE_ENUM(Method, METHOD_TABLE);

}  // namespace pulse::http

PULSE_ENUM_TO_STRING(pulse::http::Method, METHOD_TABLE);
