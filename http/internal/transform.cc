#include "http/internal/transform.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "core/error.h"
#include "core/log.h"
#include "core/result.h"
#include "core/stringify.h"
#include "http/method.h"
#include "http/request.h"
#include "http/response.h"
#include "strings/split.h"

namespace pulse::http {

namespace {

constexpr std::string_view reason(int status) {
  switch (status) {
    case 200:
      return "OK";
    case 201:
      return "Created";
    case 400:
      return "Bad Request";
    case 404:
      return "Not Found";
    case 500:
    default:
      return "Internal Server Error";
  }
}

}  // namespace

// TODO(store version number)
Result<Request> parse_header(std::string_view raw) {
  Request request;
  std::vector<std::string_view> header = strings::split(raw, "\r\n");
  std::vector<std::string_view> request_line = strings::split(header[0], " ");
  if (request_line.size() < 3) {
    Log() << "malformed request line: [" << header[0] << "]";
    return Error{.code = Error::Code::kInternal,
                 .message = "parse_header: malformed request line"};
  }

  if (std::string_view method = request_line[0]; method == "GET") {
    request.method = Method::kGet;
  } else if (method == "POST") {
    request.method = Method::kPost;
  } else if (method == "PUT") {
    request.method = Method::kPut;
  } else if (method == "DELETE") {
    request.method = Method::kDelete;
  } else {
    Log() << "invalid method: [" << method << "]";
    return Error{
        .code = Error::Code::kInternal,
        .message = "parse_header: invalid method: " + std::string(method)};
  }

  std::vector<std::string_view> path_with_query_params =
      strings::split(request_line[1], "?");
  if (path_with_query_params.empty() || path_with_query_params[0].empty() ||
      path_with_query_params[0][0] != '/') {
    Log() << "invalid path: [" << path_with_query_params[0] << "]";
    return Error{.code = Error::Code::kInternal,
                 .message = "parse_header: invalid path" +
                            std::string(path_with_query_params[0])};
  }

  request.path = std::string(path_with_query_params[0]);
  if (path_with_query_params.size() > 1) {
    for (std::string_view param :
         strings::split(path_with_query_params[1], "&")) {
      if (size_t i = param.find('=');
          i != std::string::npos && i + 1 < param.size()) {
        request.query_params[std::string(param.substr(0, i))] =
            std::string(param.substr(i + 1));
        continue;
      }

      Log() << "malformed query parameter: [" << param << "]";
      return Error{.code = Error::Code::kInternal,
                   .message = "parse_header: malformed query parameter: " +
                              std::string(param)};
    }
  }

  for (size_t i = 1; i < header.size(); i++) {
    if (header[i].empty()) {
      continue;
    }

    if (size_t match = header[i].find(": ");
        match != std::string::npos && match + 2 < header[i].size()) {
      request.headers[std::string(header[i].substr(0, match))] =
          std::string(header[i].substr(match + 2));
      continue;
    }

    Log() << "malformed header field: [" << header[i] << "]";
    return Error{.code = Error::Code::kInternal,
                 .message = "parse_header: malformed header field: " +
                            std::string(header[i])};
  }

  Log() << "parsed request: " << pulse::to_string(request);
  return request;
}

std::string serialize(const Response& response) {
  return "HTTP/1.1 " + std::to_string(response.status) + " " +
         std::string(reason(response.status)) +
         "\r\nContent-Type: " + response.content_type +
         "\r\nContent-Length: " + std::to_string(response.body.size()) +
         "\r\n\r\n" + response.body;
}

}  // namespace pulse::http
