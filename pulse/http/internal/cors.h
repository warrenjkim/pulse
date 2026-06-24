#pragma once

#include "pulse/http/request.h"
#include "pulse/http/response.h"

namespace pulse::http {

// Returns a CORS preflight response for an `OPTIONS` request. Returns a 400
// response if `Access-Control-Request-Headers` is absent.
Response CorsPreflight(const Request& request);

// Appends Access-Control-Allow-Origin to an existing response.
void AddCorsHeaders(Response* response);

}  // namespace pulse::http
