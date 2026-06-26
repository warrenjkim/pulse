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

template <typename... T>
struct Routes {};

template <typename T>
struct is_handler_list : std::false_type {};

template <typename... Handlers>
struct is_handler_list<Routes<Handlers...>> : std::true_type {};

template <typename T>
concept HttpHandlers = is_handler_list<T>::value;

namespace internal {

template <typename Accumulator, typename... Rest>
struct Flatten;

template <typename... Accumulator>
struct Flatten<Routes<Accumulator...>> {
  using type = Routes<Accumulator...>;
};

template <typename... Accumulator, HttpHandler Handler, typename... Rest>
struct Flatten<Routes<Accumulator...>, Handler, Rest...> {
  using type = typename Flatten<Routes<Accumulator..., Handler>, Rest...>::type;
};

template <typename... Accumulator, typename... Nested, typename... Rest>
struct Flatten<Routes<Accumulator...>, Routes<Nested...>, Rest...> {
  using type =
      typename Flatten<Routes<Accumulator...>, Nested..., Rest...>::type;
};

}  // namespace internal

class Router {
 public:
  // Constructs a `Router` from `Hs`. `Hs` can be either `HttpHandler` or nested
  // `Routes<>`. Each handler is constructed using `ctx` to inject dependencies.
  //
  // Nesting is particularly useful for grouping related handlers:
  //
  //   using GroupA = Routes<HandlerB, HandlerC>;
  //   using GroupB = Routes<HandlerD, HandlerE>;
  //
  // Example Usage:
  //
  //   // flat list of handlers
  //   Router::Make<Routes<HandlerA, HandlerB, HandlerC, HandlerD>>(ctx);
  //
  //   // nested list of handlers
  //   Router::Make<Routes<HandlerA, GroupA, GroupB>>(ctx);
  template <HttpHandlers Hs, HttpServerContext Ctx>
  static Result<Router> Make(const Ctx& ctx);

  struct RouteMatch {
    const Handler* handler;
    Pattern::Captures path_params;
  };

  // Matches on a given path. Returns a RouteMatch on a matched route,
  // `pulse::Error` otherwise.
  Result<RouteMatch> Match(Method method, std::string_view path) const;

 private:
  template <HttpServerContext Ctx, HttpHandler... Hs>
  static Result<Router> Make(const Ctx& ctx, Routes<Hs...>);

  template <HttpServerContext Ctx, HttpHandler H, typename... Deps>
  static std::unique_ptr<H> MakeHandler(const Ctx& ctx, Dependencies<Deps...>);

  struct Route {
    Pattern pattern;
    std::string raw_pattern;
    std::unique_ptr<Handler> handler;
  };

  template <HttpServerContext Ctx, HttpHandler... Hs>
  Result<void> Add(const Ctx& ctx);

  Result<void> Add(Method method, std::string_view raw_pattern,
                   std::unique_ptr<Handler> handler);

  std::unordered_map<Method, std::vector<Route>> routes_;
};

// Implementation details below;

template <HttpHandlers Hs, HttpServerContext Ctx>
Result<Router> Router::Make(const Ctx& ctx) {
  return Router::Make(ctx, typename internal::Flatten<Routes<>, Hs>::type{});
}

template <HttpServerContext Ctx, HttpHandler... Hs>
Result<Router> Router::Make(const Ctx& ctx, Routes<Hs...>) {
  Router router;
  Result<void> result = router.Add<Ctx, Hs...>(ctx);
  if (!result.ok()) {
    return result.error();
  }
  return router;
}

template <HttpServerContext Ctx, HttpHandler H, typename... Deps>
std::unique_ptr<H> Router::MakeHandler(const Ctx& ctx, Dependencies<Deps...>) {
  return std::make_unique<H>(ctx.template get<Deps>()...);
}

template <HttpServerContext Ctx, HttpHandler... Hs>
Result<void> Router::Add(const Ctx& ctx) {
  if constexpr (sizeof...(Hs) == 0) {
    return {};
  }

  Result<void> result;
  ((result =
        Add(Hs::kMethod, Hs::kPath,
            Router::MakeHandler<Ctx, Hs>(ctx, typename Hs::Dependencies{})))
       .ok() &&
   ...);

  return result;
}

}  // namespace pulse::http
