#pragma once

#include <string>
#include <unordered_map>

#include "core/stringify.h"
#include "http/method.h"
#include "strings/cat.h"

namespace pulse::http {

// TODO(store version number)
struct Request {
  Method method;
  std::string path;
  std::unordered_map<std::string, std::string> path_params;
  std::unordered_map<std::string, std::string> query_params;
  std::unordered_map<std::string, std::string> headers;
  std::string body;

  friend bool operator==(const Request&, const Request&) = default;
};

}  // namespace pulse::http

template <>
struct pulse::Stringify<pulse::http::Request> {
  static std::string to_string(const pulse::http::Request& req) {
    return strings::cat("Request{.method=", req.method, ",.path=", req.path,
                        ",.path_params=", req.path_params,
                        ",.query_params=", req.query_params,
                        ",.headers=", req.headers, ",.body=", req.body, "}");
  }
};
