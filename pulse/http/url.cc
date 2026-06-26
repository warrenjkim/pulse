#include "pulse/http/url.h"

#include <cstddef>
#include <string>
#include <string_view>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/strings/cat.h"

namespace pulse::http {

namespace {

Result<int> Hex(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }

  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }

  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }

  return Error{.code = Error::Code::kInvalidArgument,
               .message = strings::Cat("invalid hex digit: ", c)};
}

Result<char> DecodeOctet(std::string_view input, size_t* i) {
  if (*i + 2 >= input.size()) {
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "truncated percent sequence"};
  }

  Result<int> hi = Hex(input[++(*i)]);
  if (!hi.ok()) {
    return hi.error();
  }

  Result<int> lo = Hex(input[++(*i)]);
  if (!lo.ok()) {
    return lo.error();
  }

  return static_cast<char>((*hi << 4) | *lo);
}

}  // namespace

Result<std::string> DecodePercent(std::string_view input) {
  std::string out;
  out.reserve(input.size());
  for (size_t i = 0; i < input.size(); i++) {
    if (input[i] == '%') {
      Result<char> c = DecodeOctet(input, &i);
      if (!c.ok()) return c.error();
      out += *c;
    } else {
      out += input[i];
    }
  }
  return out;
}

}  // namespace pulse::http
