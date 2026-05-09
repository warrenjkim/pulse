#include "http/internal/serialize.h"

#include <string>
#include <string_view>

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

std::string serialize(const Response& response) {
  return "HTTP/1.1 " + std::to_string(response.status) + " " +
         std::string(reason(response.status)) +
         "\r\nContent-Type: " + response.content_type +
         "\r\nContent-Length: " + std::to_string(response.body.size()) +
         "\r\n\r\n" + response.body;
}

}  // namespace pulse::http
