#include "pulse/http/parse_form.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/http/parameters.h"
#include "pulse/http/url.h"
#include "pulse/strings/cat.h"

namespace pulse::http {

namespace {

Result<std::string> DecodeFormString(std::string_view input) {
  std::string out;
  out.reserve(input.size());
  for (size_t i = 0; i < input.size(); i++) {
    if (input[i] == '%') {
      if (i + 2 >= input.size()) {
        return Error{.code = Error::Code::kInvalidArgument,
                     .message = "truncated percent sequence"};
      }

      Result<std::string> decoded = DecodePercent(input.substr(i, 3));
      if (!decoded.ok()) {
        return decoded.error();
      }

      out += *decoded;
      i += 2;
    } else if (input[i] == '+') {
      out += ' ';
    } else {
      out += input[i];
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

    Result<std::string> key = DecodeFormString(pair.substr(0, eq));
    if (!key.ok()) {
      return key.error();
    }

    Result<std::string> value = DecodeFormString(pair.substr(eq + 1));
    if (!value.ok()) {
      return value.error();
    }

    params[*std::move(key)] = *std::move(value);
  }
  return params;
}

}  // namespace pulse::http
