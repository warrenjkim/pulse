#pragma once

#include <concepts>
#include <string_view>
#include <type_traits>

#include "pulse/core/type_map.h"
#include "pulse/http/method.h"
#include "pulse/http/request.h"
#include "pulse/http/response.h"
#include "pulse/strings/string_literal.h"

namespace pulse::http {

// A map of dependencies for one or more handlers.
//
// Example Usage:
//
//   ServerContext<UserStore*, Config> ctx;
//   ctx.set(&user_store);
//   ctx.set(config);
//
//   Router::Make<GetUserHandler, DeleteUserHandler>(ctx);
template <typename... Types>
using ServerContext = TypeMap<Types...>;

template <typename T>
struct is_server_context : std::false_type {};

template <typename... Types>
struct is_server_context<ServerContext<Types...>> : std::true_type {};

template <typename T>
concept HttpServerContext = is_server_context<T>::value;

// Declares the types this handler requires for construction.
template <typename... Deps>
struct Dependencies {};

namespace internal {

struct Handler {
  virtual Response operator()(const Request& request) const = 0;

  virtual ~Handler() = default;
};

}  // namespace internal

template <typename H>
concept HttpHandler = std::derived_from<H, internal::Handler>;

// Abstract base for HTTP request handlers.
//
// Subclass and override `operator()` to implement an endpoint.
//
// Example Usage:
//
//   struct GetUserHandler
//       : Handler<Method::kGet, "/users/{id}", Dependencies<UserStore*>> {
//     explicit GetUserHandler(UserStore* store) : store_(*store) {}
//
//     Response operator()(const Request& request) const override {
//       // ...
//     }
//
//   private:
//     UserStore& store_;
//   };
template <Method kHttpMethod, strings::StringLiteral kHttpPath,
          typename Deps = Dependencies<>>
struct Handler : public internal::Handler {
  static constexpr Method kMethod = kHttpMethod;
  static constexpr std::string_view kPath = kHttpPath;
  using Dependencies = Deps;
};

}  // namespace pulse::http
