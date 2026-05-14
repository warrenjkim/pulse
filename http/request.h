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
  std::unordered_map<std::string, std::string> params;
  std::unordered_map<std::string, std::string> headers;
  std::string body;

  friend bool operator==(const Request&, const Request&) = default;
};

}  // namespace pulse::http

template <>
struct pulse::Stringify<pulse::http::Request> {
  static std::string to_string(const pulse::http::Request& req) {
    std::string out =
        "Request{\n  method: " +
        pulse::Stringify<pulse::http::Method>::to_string(req.method) +
        "\n  path: " + req.path + "\n";

    if (!req.params.empty()) {
      out += "  params: {\n";
      for (const auto& [k, v] : req.params) {
        out += "    " + k + ": " + v + "\n";
      }

      out += "  }\n";
    }

    if (!req.headers.empty()) {
      out += "  headers: {\n";
      for (const auto& [k, v] : req.headers) {
        out += "    " + k + ": " + v + "\n";
      }

      out += "  }\n";
    }

    return out + (req.body.empty() ? "" : "  body: " + req.body) + "\n}";
  }
};
