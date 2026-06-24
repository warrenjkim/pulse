#include "pulse/http/router.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/http/handler.h"
#include "pulse/http/method.h"
#include "pulse/http/pattern.h"
#include "pulse/strings/cat.h"

namespace pulse::http {

std::optional<Router::RouteMatch> Router::Match(Method method,
                                                std::string_view path) const {
  const auto it = routes_.find(method);
  if (it == routes_.end()) {
    return std::nullopt;
  }

  for (const Route& route : it->second) {
    if (std::optional<Pattern::Captures> captures = route.pattern.Match(path)) {
      return Router::RouteMatch{.handler = route.handler.get(),
                                .path_params = *std::move(captures)};
    }
  }

  return std::nullopt;
}

Result<void> Router::Add(Method method, std::string_view raw_pattern,
                         std::unique_ptr<Handler> handler) {
  Result<Pattern> pattern = Pattern::Make(raw_pattern);
  if (!pattern.ok()) {
    return pattern.error();
  }

  std::vector<Route>& routes = routes_[method];
  for (const Route& route : routes) {
    if (route.raw_pattern == raw_pattern) {
      return pulse::Error{
          .code = pulse::Error::Code::kAlreadyExists,
          .message = strings::cat("raw_pattern '", std::string(raw_pattern),
                                  "' already exists")};
    }
  }

  routes.push_back({.pattern = *std::move(pattern),
                    .raw_pattern = std::string(raw_pattern),
                    .handler = std::move(handler)});
  std::sort(routes.begin(), routes.end(),
            [](const Route& lhs, const Route& rhs) -> bool {
              return lhs.pattern.captures() < rhs.pattern.captures();
            });

  return Result<void>{};
}

}  // namespace pulse::http
