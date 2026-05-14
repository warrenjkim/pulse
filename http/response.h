#pragma once

#include <string>

#include "core/stringify.h"

namespace pulse::http {

struct Response {
  std::string content_type;
  int status;
  std::string body;
  friend bool operator==(const Response&, const Response&) = default;
};

}  // namespace pulse::http

template <>
struct pulse::Stringify<pulse::http::Response> {
  static std::string to_string(const pulse::http::Response& res) {
    return "Response{\n  status: " + std::to_string(res.status) +
           "\n  content_type: " + res.content_type + "\n" +
           (res.body.empty() ? "" : "  body: " + res.body) + "\n}";
  }
};
