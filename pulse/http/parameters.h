#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/core/stringify.h"
#include "pulse/http/parameter_parser.h"
#include "pulse/strings/cat.h"

namespace pulse::http {

class Parameters {
 public:
  Parameters() = default;

  Parameters(
      std::initializer_list<std::pair<const std::string, std::string>> init)
      : map_(init.begin(), init.end()) {}

  template <Parseable T>
  Result<T> Get(std::string_view key) const {
    if (auto it = map_.find(key); it != map_.end()) {
      return ParameterParser<T>::Parse(it->second);
    }

    return Error{.code = Error::Code::kNotFound,
                 .message = strings::Cat("parameter not found: ", key)};
  }

  std::string& operator[](std::string_view key) {
    return map_[std::string(key)];
  }

  friend bool operator==(const Parameters&, const Parameters&) = default;

 private:
  struct StringHash {
    using is_transparent = void;

    size_t operator()(std::string_view sv) const {
      return std::hash<std::string_view>{}(sv);
    }
  };

  friend struct pulse::Stringify<Parameters>;

  std::unordered_map<std::string, std::string, StringHash, std::equal_to<>>
      map_;
};

}  // namespace pulse::http

template <>
struct pulse::Stringify<pulse::http::Parameters> {
  static std::string ToString(const pulse::http::Parameters& p) {
    std::string out = "Parameters{";
    for (const auto& [key, value] : p.map_) {
      out += strings::Cat("{", key, ",", value, "},");
    }

    if (!p.map_.empty()) {
      out.pop_back();
    }

    return strings::Cat(out, "}");
  }
};
