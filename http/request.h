#pragma once

#include <string>
#include <unordered_map>

#include "core/stringify.h"
#include "http/method.h"

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
    return "Request{.method=" +
           pulse::Stringify<pulse::http::Method>::to_string(req.method) +
           ",.path=" + req.path + ",.path_params=" +
           pulse::Stringify<std::unordered_map<std::string, std::string>>::
               to_string(req.path_params) +
           ",.query_params=" +
           pulse::Stringify<std::unordered_map<std::string, std::string>>::
               to_string(req.query_params) +
           ",.headers=" +
           pulse::Stringify<std::unordered_map<std::string, std::string>>::
               to_string(req.headers) +
           ",.body=" + req.body + "}";
  }
};
