#include "http/internal/transform.h"

#include <string>
#include <string_view>
#include <vector>

#include "dsa/error.h"
#include "dsa/result.h"
#include "http/request.h"
#include "http/response.h"

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

// TODO(store version number)
Result<Request> parse_header(std::string_view raw) {
  Request request;
  std::vector<std::string_view> header = split(raw, "\r\n");
  std::vector<std::string_view> request_line = split(header[0], " ");
  if (request_line.size() < 3) {
    std::cerr << "[" << __FILE__ << ":" << __LINE__
              << "] malformed request line: [" << header[0] << "]\n";
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
    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] invalid method: ["
              << method << "]\n";
    return Error{
        .code = Error::Code::kInternal,
        .message = "parse_header: invalid method: " + std::string(method)};
  }

  std::vector<std::string_view> path_with_params = split(request_line[1], "?");
  if (path_with_params.empty() || path_with_params[0].empty() ||
      path_with_params[0][0] != '/') {
    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] invalid path: ["
              << path_with_params[0] << "]\n";
    return Error{.code = Error::Code::kInternal,
                 .message = "parse_header: invalid path" +
                            std::string(path_with_params[0])};
  }

  request.path = std::string(path_with_params[0]);
  if (path_with_params.size() > 1) {
    for (std::string_view param : split(path_with_params[1], "&")) {
      if (size_t i = param.find('=');
          i != std::string::npos && i + 1 < param.size()) {
        request.params[std::string(param.substr(0, i))] =
            std::string(param.substr(i + 1));
        continue;
      }
      std::cerr << "[" << __FILE__ << ":" << __LINE__
                << "] malformed query parameter: [" << param << "]\n";
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

    std::cerr << "[" << __FILE__ << ":" << __LINE__
              << "] malformed header field: [" << header[i] << "]\n";
    return Error{.code = Error::Code::kInternal,
                 .message = "parse_header: malformed header field: " +
                            std::string(header[i])};
  }

  std::cerr << "[" << __FILE__ << ":" << __LINE__
            << "] parsed request: method=" << static_cast<int>(request.method)
            << " path=" << request.path << "\n";
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
