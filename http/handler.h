#pragma once

#include <concepts>
#include <string_view>
#include <type_traits>

#include "core/type_map.h"
#include "http/method.h"
#include "http/request.h"
#include "http/response.h"

// Declares kPath and kMethod for an HTTP handler.
#define PULSE_HTTP_ROUTE(path, method)                             \
  [[maybe_unused]] static constexpr std::string_view kPath = path; \
  [[maybe_unused]] static constexpr Method kMethod = method

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

// Abstract base for HTTP request handlers.
//
// Subclass and override `operator()` to implement an endpoint. Each handler
// must declare a `kPath`, `kMethod`, and `Dependencies` alias. Use
// `PULSE_HTTP_ROUTE` to declare both the path and method in one line.
//
// Example Usage:
//
//   struct GetUserHandler : Handler {
//     PULSE_HTTP_ROUTE("/users/{id}", Method::kGet);
//     using Dependencies = pulse::http::Dependencies<UserStore*, Config>;
//
//     explicit GetUserHandler(UserStore* store, Config config)
//         : store_(store), config_(std::move(config)) {}
//
//     Response operator()(const Request& request) const override {
//       std::string id = request.path_params.at("id");
//       return store_->get(id);
//     }
//
//   private:
//     UserStore* store_;
//     const Config config_;
//   };
struct Handler {
  virtual Response operator()(const Request& request) const = 0;

  virtual ~Handler() = default;
};

template <typename H>
concept HttpHandler = requires {
  { H::kPath } -> std::convertible_to<std::string_view>;
  { H::kMethod } -> std::same_as<const Method&>;
  typename H::Dependencies;
  requires std::derived_from<H, Handler>;
};

}  // namespace pulse::http
