#pragma once

#include <string_view>
#include <vector>

#include "dsa/result.h"
#include "http/request.h"
#include "http/response.h"

namespace pulse::http {

// TODO(might want to move this out into dsa. currently public just to make it
// easier to test)
// Splits `string` on each occurrence of `delimiter`, returning a vector of
// views into `string`. Empty tokens are preserved: i.e. a leading, trailing, or
// consecutive delimiter produces an empty `std::string_view` at that position.
//
// `string` must outlive the returned views.
std::vector<std::string_view> split(std::string_view string,
                                    std::string_view delimiter);

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
//   * The header/body separator is missing.
// Returns a `Request` with an empty body otherwise.
Result<Request> parse_header(std::string_view raw);

// Serializes a `Response` into a raw HTTP/1.1 response.
std::string to_string(const Response& response);

}  // namespace pulse::http
