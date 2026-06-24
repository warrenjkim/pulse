#include "pulse/http/internal/cors.h"

#include <string>
#include <string_view>

#include "pulse/http/request.h"
#include "pulse/http/response.h"

namespace pulse::http {

namespace {

constexpr std::string_view kAllowOrigin = "Access-Control-Allow-Origin";
constexpr std::string_view kAllowMethods = "Access-Control-Allow-Methods";
constexpr std::string_view kAllowHeaders = "Access-Control-Allow-Headers";
constexpr std::string_view kMaxAge = "Access-Control-Max-Age";
constexpr std::string_view kRequestHeaders = "Access-Control-Request-Headers";

constexpr std::string_view kOrigin = "*";
constexpr std::string_view kMethods = "GET, POST, PUT, DELETE, OPTIONS";
constexpr std::string_view kMaxAgeValue = "86400";

}  // namespace

Response CorsPreflight(const Request& request) {
  auto it = request.headers.find(std::string(kRequestHeaders));
  if (it == request.headers.end()) {
    return Response{.status = 400};
  }

  return Response{.headers =
                      {
                          {std::string(kAllowOrigin), std::string(kOrigin)},
                          {std::string(kAllowMethods), std::string(kMethods)},
                          {std::string(kAllowHeaders), it->second},
                          {std::string(kMaxAge), std::string(kMaxAgeValue)},
                      },
                  .status = 200};
}

void AddCorsHeaders(Response* response) {
  response->headers[std::string(kAllowOrigin)] = std::string(kOrigin);
}

}  // namespace pulse::http
