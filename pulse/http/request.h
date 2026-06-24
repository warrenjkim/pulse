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
  static std::string to_string(const pulse::http::Request& req) {
    return strings::cat(
        "Request{.method=",
        pulse::Stringify<pulse::http::Method>::to_string(req.method),
        ",.url=", pulse::Stringify<std::string>::to_string(req.url), ",.path=",
        pulse::Stringify<pulse::http::Parameters>::to_string(req.path),
        ",.query=",
        pulse::Stringify<pulse::http::Parameters>::to_string(req.query),
        ",.headers=",
        pulse::Stringify<std::unordered_map<std::string, std::string>>::
            ToString(req.headers),
        ",.body=", pulse::Stringify<std::string>::to_string(req.body), "}");
  }
};
