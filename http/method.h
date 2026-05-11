#pragma once

#include <ostream>
#include <string>

namespace pulse::http {

enum class Method { kGet, kPost, kPut, kDelete };

inline std::string to_string(Method method) {
  switch (method) {
    case Method::kGet:
      return "GET";
    case Method::kPost:
      return "POST";
    case Method::kPut:
      return "PUT";
    case Method::kDelete:
      return "DELETE";
    default:
      return "UNKNOWN";
  }
}

inline std::ostream& operator<<(std::ostream& os, Method method) {
  return os << to_string(method);
}

}  // namespace pulse::http
