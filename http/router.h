#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "dsa/result.h"
#include "http/handler.h"
#include "http/method.h"
#include "http/pattern.h"

namespace pulse::http {

class Router {
 public:
  struct Match {
    const Handler* handler;
    Pattern::Captures path_params;
  };

  // Registers a route. Returns an error if the pattern is malformed or if
  // (method, pattern) was already registered.
  Result<void> add(Method method, std::string_view raw_pattern,
                   std::unique_ptr<Handler> handler);

  // Matches on a given path. Returns a Match on a matched route, std::nullopt
  // otherwise.
  std::optional<Match> match(Method method, std::string_view path) const;

 private:
  struct Route {
    Pattern pattern;
    std::string raw_pattern;
    std::unique_ptr<Handler> handler;
  };

  std::unordered_map<Method, std::vector<Route>> routes_;
};

}  // namespace pulse::http
