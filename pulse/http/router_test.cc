#include "pulse/http/router.h"

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/http/handler.h"
#include "pulse/http/method.h"
#include "pulse/http/pattern.h"
#include "pulse/http/request.h"
#include "pulse/http/response.h"

namespace pulse::http {

namespace {

using ::testing::Eq;

class NoDepHandler final : public Handler {
 public:
  PULSE_HTTP_ROUTE("/health", Method::kGet);
  using Dependencies = Dependencies<>;

  Response operator()(const Request&) const override {
    return Response{
        .content_type = "text/plain", .status = 200, .body = "health"};
  }
};

class DepHandler final : public Handler {
 public:
  PULSE_HTTP_ROUTE("/name", Method::kGet);
  using Dependencies = Dependencies<std::string*>;

  explicit DepHandler(std::string* name) : name_(name) {}

  Response operator()(const Request&) const override {
    return Response{
        .content_type = "text/plain", .status = 200, .body = *name_};
  }

 private:
  std::string* name_;
};

class DuplicateHandler final : public Handler {
 public:
  PULSE_HTTP_ROUTE("/health", Method::kGet);
  using Dependencies = Dependencies<>;

  Response operator()(const Request&) const override {
    return Response{
        .content_type = "text/plain", .status = 200, .body = "duplicate"};
  }
};

class MixedDepHandler final : public Handler {
 public:
  PULSE_HTTP_ROUTE("/mixed", Method::kGet);
  using Dependencies = Dependencies<std::string*, int>;

  explicit MixedDepHandler(std::string* name, int code)
      : name_(name), code_(code) {}

  Response operator()(const Request&) const override {
    return Response{
        .content_type = "text/plain", .status = code_, .body = *name_};
  }

 private:
  std::string* name_;
  const int code_;
};

class GetNamedHandler final : public Handler {
 public:
  PULSE_HTTP_ROUTE("/named", Method::kGet);
  using Dependencies = Dependencies<std::string>;

  explicit GetNamedHandler(std::string name) : name_(std::move(name)) {}

  Response operator()(const Request&) const override {
    return Response{.content_type = "text/plain", .status = 200, .body = name_};
  }

 private:
  std::string name_;
};

class PostNamedHandler final : public Handler {
 public:
  PULSE_HTTP_ROUTE("/named", Method::kPost);
  using Dependencies = Dependencies<std::string>;

  explicit PostNamedHandler(std::string name) : name_(std::move(name)) {}

  Response operator()(const Request&) const override {
    return Response{.content_type = "text/plain", .status = 200, .body = name_};
  }

 private:
  std::string name_;
};

class MalformedHandler final : public Handler {
 public:
  PULSE_HTTP_ROUTE("/name/{bad", Method::kGet);
  using Dependencies = Dependencies<std::string>;

  explicit MalformedHandler(std::string name) : name_(std::move(name)) {}

  Response operator()(const Request&) const override {
    return Response{.content_type = "text/plain", .status = 200, .body = name_};
  }

 private:
  std::string name_;
};

struct GetItemsHandler final : public Handler {
  PULSE_HTTP_ROUTE("/items", Method::kGet);
  using Dependencies = Dependencies<>;

  Response operator()(const Request&) const override {
    return Response{.body = "get_items"};
  }
};

struct PostItemsHandler final : public Handler {
  PULSE_HTTP_ROUTE("/items", Method::kPost);
  using Dependencies = Dependencies<>;

  Response operator()(const Request&) const override {
    return Response{.body = "post_items"};
  }
};

struct GetItemByIdHandler final : public Handler {
  PULSE_HTTP_ROUTE("/items/{id}", Method::kGet);
  using Dependencies = Dependencies<>;

  Response operator()(const Request&) const override {
    return Response{.body = "item_by_id"};
  }
};

struct GetItemNewHandler final : public Handler {
  PULSE_HTTP_ROUTE("/items/new", Method::kGet);
  using Dependencies = Dependencies<>;

  Response operator()(const Request&) const override {
    return Response{.body = "item_new"};
  }
};

struct GetNestedItemHandler final : public Handler {
  PULSE_HTTP_ROUTE("/items/{item_id}/subitems/{subitem_id}", Method::kGet);
  using Dependencies = Dependencies<>;

  Response operator()(const Request&) const override {
    return Response{.body = "nested"};
  }
};

TEST(RouterTest, MakeWithNoDepHandler) {
  Result<Router> router = Router::Make<Routes<NoDepHandler>>(ServerContext{});
  ASSERT_TRUE(router.ok());

  std::optional<Router::RouteMatch> match =
      router->Match(Method::kGet, "/health");
  ASSERT_TRUE(match.has_value());
  EXPECT_THAT((*match->handler)(Request{}).body, Eq("health"));
}

TEST(RouterTest, MakeWithDepHandler) {
  std::string name = "injected";
  ServerContext<std::string*> ctx;
  ctx.Set(&name);

  Result<Router> router = Router::Make<Routes<DepHandler>>(ctx);
  ASSERT_TRUE(router.ok());

  std::optional<Router::RouteMatch> match =
      router->Match(Method::kGet, "/name");
  ASSERT_TRUE(match.has_value());
  EXPECT_THAT((*match->handler)(Request{}).body, Eq("injected"));
}

TEST(RouterTest, MakeWithMultipleHandlers) {
  std::string name = "injected";
  ServerContext<std::string*> ctx;
  ctx.Set(&name);

  Result<Router> router = Router::Make<Routes<NoDepHandler, DepHandler>>(ctx);
  ASSERT_TRUE(router.ok());

  EXPECT_TRUE(router->Match(Method::kGet, "/health").has_value());
  EXPECT_TRUE(router->Match(Method::kGet, "/name").has_value());
}

TEST(RouterTest, MakeWithNestedRoutes) {
  Result<Router> router = Router::Make<
      Routes<NoDepHandler, Routes<GetItemsHandler, PostItemsHandler>>>(
      ServerContext{});
  ASSERT_TRUE(router.ok());

  EXPECT_TRUE(router->Match(Method::kGet, "/health").has_value());
  EXPECT_TRUE(router->Match(Method::kGet, "/items").has_value());
  EXPECT_TRUE(router->Match(Method::kPost, "/items").has_value());
}

TEST(RouterTest, MakeWithNestedRoutesAndDeps) {
  std::string name = "injected";
  ServerContext<std::string, std::string*> ctx;
  ctx.Set(name);
  ctx.Set(&name);

  Result<Router> router =
      Router::Make<Routes<NoDepHandler, Routes<DepHandler, GetNamedHandler>>>(
          ctx);
  ASSERT_TRUE(router.ok());

  EXPECT_TRUE(router->Match(Method::kGet, "/health").has_value());
  EXPECT_TRUE(router->Match(Method::kGet, "/name").has_value());
  EXPECT_TRUE(router->Match(Method::kGet, "/named").has_value());
}

TEST(RouterTest, MakeReturnsFirstError) {
  Result<Router> router =
      Router::Make<Routes<NoDepHandler, DuplicateHandler>>(ServerContext{});
  ASSERT_FALSE(router.ok());
  EXPECT_THAT(router.error().code, Eq(pulse::Error::Code::kAlreadyExists));
}

TEST(RouterMakeTest, MakeWithMixedDeps) {
  std::string name = "mixed";
  int code = 202;
  ServerContext<std::string*, int> ctx;
  ctx.Set(&name);
  ctx.Set(code);

  Result<Router> router = Router::Make<Routes<MixedDepHandler>>(ctx);
  ASSERT_TRUE(router.ok());

  std::optional<Router::RouteMatch> match =
      router->Match(Method::kGet, "/mixed");
  ASSERT_TRUE(match.has_value());
  Response response = (*match->handler)(Request{});
  EXPECT_THAT(response.body, Eq("mixed"));
  EXPECT_THAT(response.status, Eq(202));

  // Mutating the pointer dep is visible; mutating the value dep is not.
  name = "mutated";
  code = 999;
  Response response2 = (*match->handler)(Request{});
  EXPECT_THAT(response2.body, Eq("mutated"));
  EXPECT_THAT(response2.status, Eq(202));
}

TEST(RouterTest, MakeRejectsMalformedPattern) {
  std::string name = "injected";
  ServerContext<std::string> ctx;
  ctx.Set(name);

  Result<Router> result = Router::Make<Routes<MalformedHandler>>(ctx);
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kInvalidArgument));
}

TEST(RouterTest, MakeSamePatternDifferentMethodsAllowed) {
  std::string name = "injected";
  ServerContext<std::string> ctx;
  ctx.Set(name);

  Result<Router> router =
      Router::Make<Routes<GetNamedHandler, PostNamedHandler>>(ctx);
  ASSERT_TRUE(router.ok());

  std::optional<Router::RouteMatch> get_match =
      router->Match(Method::kGet, "/named");
  ASSERT_TRUE(get_match.has_value());
  EXPECT_THAT((*get_match->handler)(Request{}).body, Eq("injected"));

  std::optional<Router::RouteMatch> post_match =
      router->Match(Method::kPost, "/named");
  ASSERT_TRUE(post_match.has_value());
  EXPECT_THAT((*post_match->handler)(Request{}).body, Eq("injected"));
}

TEST(RouterTest, MatchesMethod) {
  Result<Router> router =
      Router::Make<Routes<GetItemsHandler, PostItemsHandler>>(ServerContext{});
  ASSERT_TRUE(router.ok());

  std::optional<Router::RouteMatch> get_match =
      router->Match(Method::kGet, "/items");
  ASSERT_TRUE(get_match.has_value());
  EXPECT_THAT((*get_match->handler)(Request{}).body, Eq("get_items"));

  std::optional<Router::RouteMatch> post_match =
      router->Match(Method::kPost, "/items");
  ASSERT_TRUE(post_match.has_value());
  EXPECT_THAT((*post_match->handler)(Request{}).body, Eq("post_items"));
}

TEST(RouterTest, MatchLiteralBeatsCapture) {
  Result<Router> router =
      Router::Make<Routes<GetItemByIdHandler, GetItemNewHandler>>(
          ServerContext{});
  ASSERT_TRUE(router.ok());

  std::optional<Router::RouteMatch> literal_match =
      router->Match(Method::kGet, "/items/new");
  ASSERT_TRUE(literal_match.has_value());
  EXPECT_THAT((*literal_match->handler)(Request{}).body, Eq("item_new"));

  std::optional<Router::RouteMatch> capture_match =
      router->Match(Method::kGet, "/items/42");
  ASSERT_TRUE(capture_match.has_value());
  EXPECT_THAT((*capture_match->handler)(Request{}).body, Eq("item_by_id"));
  EXPECT_THAT(capture_match->path_params, Eq(Pattern::Captures{{"id", "42"}}));
}

TEST(RouterTest, MatchBindsNestedCaptures) {
  Result<Router> router =
      Router::Make<Routes<GetNestedItemHandler>>(ServerContext{});
  ASSERT_TRUE(router.ok());

  std::optional<Router::RouteMatch> match =
      router->Match(Method::kGet, "/items/1/subitems/2");
  ASSERT_TRUE(match.has_value());
  EXPECT_THAT(match->path_params,
              Eq(Pattern::Captures{{"item_id", "1"}, {"subitem_id", "2"}}));
}

TEST(RouterTest, NoMatchUnknownPath) {
  Result<Router> router =
      Router::Make<Routes<GetItemByIdHandler>>(ServerContext{});
  ASSERT_TRUE(router.ok());
  EXPECT_FALSE(router->Match(Method::kGet, "/unknown").has_value());
}

TEST(RouterTest, NoMatchTooFewSegments) {
  Result<Router> router =
      Router::Make<Routes<GetItemByIdHandler>>(ServerContext{});
  ASSERT_TRUE(router.ok());
  EXPECT_FALSE(router->Match(Method::kGet, "/items").has_value());
}

TEST(RouterTest, NoMatchTooManySegments) {
  Result<Router> router =
      Router::Make<Routes<GetItemByIdHandler>>(ServerContext{});
  ASSERT_TRUE(router.ok());
  EXPECT_FALSE(router->Match(Method::kGet, "/items/42/extra").has_value());
}

TEST(RouterTest, NoMatchWrongMethod) {
  Result<Router> router =
      Router::Make<Routes<GetItemsHandler>>(ServerContext{});
  ASSERT_TRUE(router.ok());
  EXPECT_FALSE(router->Match(Method::kPost, "/items").has_value());
}

TEST(RouterTest, NoMatchEmptyRouter) {
  Result<Router> router = Router::Make<Routes<>>(ServerContext{});
  ASSERT_TRUE(router.ok());
  EXPECT_FALSE(router->Match(Method::kGet, "/items").has_value());
}

}  // namespace

}  // namespace pulse::http
