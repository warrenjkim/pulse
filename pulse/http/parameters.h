#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/http/parameter_parser.h"

namespace pulse::http {

class Parameters {
 public:
  template <typename T>
  Result<T> get(std::string_view key) const {
    if (auto it = map_.find(key); it != map_.end()) {
      return ParameterParser<T>::Convert(it->second);
    }

    return Error{.code = Error::Code::kNotFound, .message = ""};
  }

  friend bool operator==(const Parameters&, const Parameters&) = default;

 private:
  struct StringHash {
    using is_transparent = void;

    size_t operator()(std::string_view sv) const {
      return std::hash<std::string_view>{}(sv);
    }
  };

  std::unordered_map<std::string, std::string, StringHash, std::equal_to<>>
      map_;
};

}  // namespace pulse::http
