#pragma once

#include <ostream>
#include <string>

namespace pulse {

struct Error {
  enum class Code { kInternal };

  Code code;
  std::string message;
};

inline std::string to_string(const Error::Code& code) {
  switch (code) {
    case Error::Code::kInternal:
    default:
      return "kInternal";
  }
}

inline std::ostream& operator<<(std::ostream& os, const Error::Code& code) {
  return os << to_string(code);
}

inline std::string to_string(const Error& error) {
  return "[" + to_string(error.code) + "] " + error.message;
}

inline std::ostream& operator<<(std::ostream& os, const Error& error) {
  return os << to_string(error);
}

}  // namespace pulse
