#pragma once

#include "http/request.h"
#include "http/response.h"

namespace pulse::http {

// Abstract base for HTTP request handlers.
//
// Subclass and override `operator()` to implement an endpoint:
//
//   struct YourEndpointHandler : Handler {
//     Response operator()(const Request& request) override;
//   };
//
//   server.route(Method::kGet, "/", std::make_unique<YourEndpointHandler>());
struct Handler {
  virtual Response operator()(const Request& request) = 0;
  virtual ~Handler() = default;
};

}  // namespace pulse::http
