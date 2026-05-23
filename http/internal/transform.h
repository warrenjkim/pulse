#pragma once

#include <string_view>

#include "dsa/result.h"
#include "http/request.h"
#include "http/response.h"

namespace pulse::http {

// Parses a raw HTTP/1.1 request header into a `Request`. Expects the following
// format:
//
//   METHOD path(\?key=value(&key=value)*)? HTTP/1.1\r\n
//   (Header-Key: Header-Value\r\n)*
//   \r\n
//   (body)?
//
// Query parameters and body are optional.
//
// Returns an `Error` if:
//   * The request line is malformed.
//   * The method is not one of `GET`, `POST`, `PUT`, `DELETE`.
//   * The path does not begin with '/'.
//   * The query parameter(s) are not well-formed.
//   * The headers are not well-formed.
// Returns a `Request` with an empty body otherwise.
Result<Request> parse_header(std::string_view raw);

// Serializes a `Response` into a raw HTTP/1.1 response.
std::string serialize(const Response& response);

}  // namespace pulse::http
