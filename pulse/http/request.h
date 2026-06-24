#pragma once

#include <string>
#include <unordered_map>

#include "pulse/core/stringify.h"
#include "pulse/http/method.h"
#include "pulse/http/parameters.h"
#include "pulse/strings/cat.h"

namespace pulse::http {

// TODO(store version number)
struct Request {
  Method method;
  std::string url;
  Parameters path;
  Parameters query;
  std::unordered_map<std::string, std::string> headers;
  std::string body;

  friend bool operator==(const Request&, const Request&) = default;
};

}  // namespace pulse::http

template <>
struct pulse::Stringify<pulse::http::Request> {
  static std::string ToString(const pulse::http::Request& req) {
    return strings::Cat("Request{.method=", req.method, ",.url=", req.url,
                        ",.path=", req.path, ",.query=", req.query,
                        ",.headers=", req.headers, ",.body=", req.body, "}");
  }
};
