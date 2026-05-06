#pragma once

#include <string>

namespace pulse::http {

struct Response {
  std::string content_type;
  int status;
  std::string body;
};

}  // namespace pulse::http
