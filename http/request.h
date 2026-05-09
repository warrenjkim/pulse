#pragma once

#include <string>
#include <unordered_map>

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
