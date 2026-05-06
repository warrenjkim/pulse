#pragma once

#include <string>

namespace pulse {

struct Error {
  enum class Code { kInternal };

  Code code;
  std::string message;
};

}  // namespace pulse
