#include "pulse/http/parse_form.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/http/parameters.h"
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

Result<char> DecodeOctet(std::string_view string, size_t* i) {
  if (*i + 2 >= string.size()) {
    return Error{.code = Error::Code::kInvalidArgument,
                 .message = "truncated percent sequence"};
  }

  Result<int> hi = Hex(string[++(*i)]);
  if (!hi.ok()) {
    return hi.error();
  }

  Result<int> lo = Hex(string[++(*i)]);
  if (!lo.ok()) {
    return lo.error();
  }

  return static_cast<char>((*hi << 4) | *lo);
}

Result<std::string> DecodeString(std::string_view string) {
  std::string out;
  out.reserve(string.size());
  for (size_t i = 0; i < string.size(); i++) {
    if (string[i] == '%') {
      Result<char> c = DecodeOctet(string, &i);
      if (!c.ok()) {
        return c.error();
      }

      out += *c;
    } else if (string[i] == '+') {
      out += ' ';
    } else {
      out += string[i];
    }
  }

  return out;
}

}  // namespace

Result<Parameters> ParseForm(std::string_view body) {
  Parameters params;

  size_t start = 0;
  while (start <= body.size()) {
    const size_t delimiter = body.find('&', start);
    const size_t end =
        delimiter == std::string_view::npos ? body.size() : delimiter;
    const std::string_view pair = body.substr(start, end - start);
    start = end + 1;

    if (pair.empty()) {
      continue;
    }

    const size_t eq = pair.find('=');
    if (eq == std::string_view::npos) {
      return Error{.code = Error::Code::kInvalidArgument,
                   .message = strings::Cat("missing '=' in pair: ", pair)};
    }

    Result<std::string> key = DecodeString(pair.substr(0, eq));
    if (!key.ok()) {
      return key.error();
    }

    Result<std::string> value = DecodeString(pair.substr(eq + 1));
    if (!value.ok()) {
      return value.error();
    }

    params[*std::move(key)] = *std::move(value);
  }

  return params;
}

}  // namespace pulse::http
