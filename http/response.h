#pragma once

#include <string>

#include "core/stringify.h"
#include "strings/cat.h"

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
    return strings::cat(
        "Response{.content_type=",
        pulse::Stringify<std::string>::to_string(res.content_type),
        ",.status=", pulse::Stringify<int>::to_string(res.status),
        ",.body=", pulse::Stringify<std::string>::to_string(res.body), "}");
  }
};
