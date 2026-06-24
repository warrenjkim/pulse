#pragma once

#include <string>
#include <unordered_map>

#include "pulse/core/stringify.h"
#include "pulse/strings/cat.h"

namespace pulse::http {

struct Response {
  std::unordered_map<std::string, std::string> headers;
  std::string content_type;
  int status;
  std::string body;
  friend bool operator==(const Response&, const Response&) = default;
};

}  // namespace pulse::http

template <>
struct pulse::Stringify<pulse::http::Response> {
  static std::string ToString(const pulse::http::Response& res) {
    return strings::Cat("Response{.content_type=", res.content_type,
                        ",.status=", res.status, ",.body=", res.body, "}");
  }
};
