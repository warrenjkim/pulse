#pragma once

#include <ostream>
#include <string>

namespace pulse::http {

struct Response {
  std::string content_type;
  int status;
  std::string body;
};

inline std::string to_string(const Response& res) {
  return "Response{\n  status: " + std::to_string(res.status) +
         "\n  content_type: " + res.content_type + "\n" +
         (res.body.empty() ? "" : "  body: " + res.body) + "\n}";
}

inline std::ostream& operator<<(std::ostream& os, const Response& res) {
  return os << to_string(res);
}

}  // namespace pulse::http
