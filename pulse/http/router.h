#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "pulse/core/result.h"
#include "pulse/http/handler.h"
#include "pulse/http/method.h"
#include "pulse/http/pattern.h"

namespace pulse::http {

template <HttpHandler... Handlers>
struct Routes {};

template <typename T>
struct is_handler_list : std::false_type {};

template <typename... Handlers>
struct is_handler_list<Routes<Handlers...>> : std::true_type {};

template <typename T>
concept HttpHandlers = is_handler_list<T>::value;

class Router {
 public:
  // Constructs a Router with the given handlers, using ctx to inject
  // dependencies into each handler's constructor.
  template <HttpHandlers Hs, HttpServerContext Ctx>
  static Result<Router> Make(const Ctx& ctx);

  struct Match {
    const Handler* handler;
    Pattern::Captures path_params;
  };

  // Matches on a given path. Returns a Match on a matched route, std::nullopt
  // otherwise.
  std::optional<Match> match(Method method, std::string_view path) const;

 private:
  template <HttpServerContext Ctx, HttpHandler... Hs>
  static Result<Router> make(const Ctx& ctx, Routes<Hs...>);

  template <HttpServerContext Ctx, HttpHandler H, typename... Deps>
  static std::unique_ptr<H> make_handler(const Ctx& ctx, Dependencies<Deps...>);

  template <HttpServerContext Ctx, HttpHandler... Hs>
  Result<void> add(const Ctx& ctx);

  Result<void> add(Method method, std::string_view raw_pattern,
                   std::unique_ptr<Handler> handler);

  struct Route {
    Pattern pattern;
    std::string raw_pattern;
    std::unique_ptr<Handler> handler;
  };

  std::unordered_map<Method, std::vector<Route>> routes_;
};

// Implementation details below;

template <HttpHandlers H, HttpServerContext Ctx>
Result<Router> Router::Make(const Ctx& ctx) {
  return Router::make(ctx, H{});
}

template <HttpServerContext Ctx, HttpHandler... Hs>
Result<Router> Router::make(const Ctx& ctx, Routes<Hs...>) {
  Router router;
  Result<void> result = router.add<Ctx, Hs...>(ctx);
  if (!result.ok()) {
    return result.error();
  }
  return router;
}

template <HttpServerContext Ctx, HttpHandler H, typename... Deps>
std::unique_ptr<H> Router::make_handler(const Ctx& ctx, Dependencies<Deps...>) {
  return std::make_unique<H>(ctx.template get<Deps>()...);
}

template <HttpServerContext Ctx, HttpHandler... Hs>
Result<void> Router::add(const Ctx& ctx) {
  if constexpr (sizeof...(Hs) == 0) {
    return {};
  }

  Result<void> result;
  ((result =
        add(Hs::kMethod, Hs::kPath,
            Router::make_handler<Ctx, Hs>(ctx, typename Hs::Dependencies{})))
       .ok() &&
   ...);

  return result;
}

}  // namespace pulse::http
