#include "http/internal/parse.h"

#include <string_view>
#include <vector>

#include "dsa/error.h"
#include "dsa/result.h"
#include "http/request.h"

namespace pulse::http {

namespace {

// TODO(store version number)
// Parses the request line and header fields into `request`. `header` must be
// the result of splitting the header section on "\r\n". The first element is
// the request line. The next element(s) are the header field(s).
Result<bool> parse_header(std::vector<std::string_view> header,
                          Request* request) {
  std::vector<std::string_view> request_line = split(header[0], " ");
  if (request_line.size() < 3) {
    return Error{.code = Error::Code::kInternal,
                 .message = "parse_header: malformed request line"};
  }

  if (std::string_view method = request_line[0]; method == "GET") {
    request->method = Method::kGet;
  } else if (method == "POST") {
    request->method = Method::kPost;
  } else if (method == "PUT") {
    request->method = Method::kPut;
  } else if (method == "DELETE") {
    request->method = Method::kDelete;
  } else {
    return Error{
        .code = Error::Code::kInternal,
        .message = "parse_header: invalid method: " + std::string(method)};
  }

  std::vector<std::string_view> path_with_params = split(request_line[1], "?");
  if (path_with_params.empty() || path_with_params[0].empty() ||
      path_with_params[0][0] != '/') {
    return Error{.code = Error::Code::kInternal,
                 .message = "parse_header: invalid path" +
                            std::string(path_with_params[0])};
  }

  request->path = std::string(path_with_params[0]);
  if (path_with_params.size() > 1) {
    for (std::string_view param : split(path_with_params[1], "&")) {
      if (size_t i = param.find('=');
          i != std::string::npos && i + 1 < param.size()) {
        request->params[std::string(param.substr(0, i))] =
            std::string(param.substr(i + 1));
        continue;
      }

      return Error{.code = Error::Code::kInternal,
                   .message = "parse_header: malformed query parameter: " +
                              std::string(param)};
    }
  }

  for (size_t i = 1; i < header.size(); i++) {
    if (size_t match = header[i].find(": ");
        match != std::string::npos && match + 2 < header[i].size()) {
      request->headers[std::string(header[i].substr(0, match))] =
          std::string(header[i].substr(match + 2));
      continue;
    }

    return Error{.code = Error::Code::kInternal,
                 .message = "parse_header: malformed header field: " +
                            std::string(header[i])};
  }

  return true;
}

}  // namespace

std::vector<std::string_view> split(std::string_view string,
                                    std::string_view delimiter) {
  size_t cursor = 0;
  size_t match = string.find(delimiter, cursor);
  std::vector<std::string_view> parts;
  while (match != std::string::npos && cursor < string.size()) {
    parts.push_back(string.substr(cursor, match - cursor));
    cursor = match + delimiter.size();
    match = string.find(delimiter, cursor);
  }

  if (cursor <= string.size()) {
    parts.push_back(string.substr(cursor));
  }

  return parts;
}

Result<Request> parse(std::string_view raw) {
  Request request;
  size_t split_index = raw.find("\r\n\r\n");
  if (split_index == std::string::npos) {
    return Error{.code = Error::Code::kInternal,
                 .message = "parse: missing header/body separator"};
  }

  if (Result<bool> result =
          parse_header(split(raw.substr(0, split_index), "\r\n"), &request);
      !result.ok()) {
    return Error{.code = result.error().code,
                 .message = "parse: " + result.error().message};
  }

  request.body = raw.substr(split_index + 4);

  return request;
}

}  // namespace pulse::http
