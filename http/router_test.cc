#include "http/router.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "core/error.h"
#include "core/result.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "http/handler.h"
#include "http/method.h"
#include "http/pattern.h"
#include "http/request.h"
#include "http/response.h"

namespace pulse::http {

namespace {

using ::testing::Eq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

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

TEST(RouterTest, MakeWithNoDepHandler) {
  Result<Router> router = Router::Make<Routes<NoDepHandler>>(ServerContext{});
  ASSERT_TRUE(router.ok());

  std::optional<Router::Match> match = router->match(Method::kGet, "/health");
  ASSERT_TRUE(match.has_value());
  EXPECT_THAT((*match->handler)(Request{}).body, Eq("health"));
}

TEST(RouterTest, MakeWithDepHandler) {
  std::string name = "injected";
  ServerContext<std::string*> ctx;
  ctx.set(&name);

  Result<Router> router = Router::Make<Routes<DepHandler>>(ctx);
  ASSERT_TRUE(router.ok());

  std::optional<Router::Match> match = router->match(Method::kGet, "/name");
  ASSERT_TRUE(match.has_value());
  EXPECT_THAT((*match->handler)(Request{}).body, Eq("injected"));
}

TEST(RouterTest, MakeWithMultipleHandlers) {
  std::string name = "injected";
  ServerContext<std::string*> ctx;
  ctx.set(&name);

  Result<Router> router = Router::Make<Routes<NoDepHandler, DepHandler>>(ctx);
  ASSERT_TRUE(router.ok());

  EXPECT_TRUE(router->match(Method::kGet, "/health").has_value());
  EXPECT_TRUE(router->match(Method::kGet, "/name").has_value());
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
  ctx.set(&name);
  ctx.set(code);

  Result<Router> router = Router::Make<Routes<MixedDepHandler>>(ctx);
  ASSERT_TRUE(router.ok());

  std::optional<Router::Match> match = router->match(Method::kGet, "/mixed");
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

std::unique_ptr<Handler> MakeNamedHandler(std::string name) {
  return std::make_unique<GetNamedHandler>(std::move(name));
}

TEST(RouterTest, MakeRejectsMalformedPattern) {
  std::string name = "injected";
  ServerContext<std::string> ctx;
  ctx.set(name);

  Result<Router> result = Router::Make<Routes<MalformedHandler>>(ctx);
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kInvalidArgument));
}

TEST(RouterTest, MakeSamePatternDifferentMethodsAllowed) {
  std::string name = "injected";
  ServerContext<std::string> ctx;
  ctx.set(name);

  Result<Router> router =
      Router::Make<Routes<GetNamedHandler, PostNamedHandler>>(ctx);
  ASSERT_TRUE(router.ok());

  std::optional<Router::Match> get_match =
      router->match(Method::kGet, "/named");
  ASSERT_TRUE(get_match.has_value());
  EXPECT_THAT((*get_match->handler)(Request{}).body, Eq("injected"));

  std::optional<Router::Match> post_match =
      router->match(Method::kPost, "/named");
  ASSERT_TRUE(post_match.has_value());
  EXPECT_THAT((*post_match->handler)(Request{}).body, Eq("injected"));
}

struct RouteSpec {
  Method method;
  std::string_view pattern;
  std::string_view handler_name;
};

struct MatchTestCase {
  std::string_view name;
  std::vector<RouteSpec> routes;
  Method query_method;
  std::string_view query_path;
  std::string_view expected_handler;
  Pattern::Captures expected_params;
};

class MatchTest : public TestWithParam<MatchTestCase> {};

TEST_P(MatchTest, Match) {
  const MatchTestCase& params = GetParam();

  Router router;
  for (const RouteSpec& spec : params.routes) {
    ASSERT_TRUE(router
                    .add(spec.method, spec.pattern,
                         MakeNamedHandler(std::string(spec.handler_name)))
                    .ok());
  }

  std::optional<Router::Match> match =
      router.match(params.query_method, params.query_path);

  ASSERT_TRUE(match.has_value());
  EXPECT_THAT((*match->handler)(Request{}).body, Eq(params.expected_handler));
  EXPECT_THAT(match->path_params, Eq(params.expected_params));
}

INSTANTIATE_TEST_SUITE_P(
    RouterTest, MatchTest,
    ValuesIn<MatchTestCase>({
        {.name = "ExactLiteral",
         .routes = {{.method = Method::kGet,
                     .pattern = "/health",
                     .handler_name = "health"}},
         .query_method = Method::kGet,
         .query_path = "/health",
         .expected_handler = "health",
         .expected_params = {}},
        {.name = "CaptureBinds",
         .routes = {{.method = Method::kGet,
                     .pattern = "/accounts/{id}",
                     .handler_name = "get_account"}},
         .query_method = Method::kGet,
         .query_path = "/accounts/42",
         .expected_handler = "get_account",
         .expected_params = {{"id", "42"}}},
        {.name = "LiteralBeatsCapture",
         .routes = {{.method = Method::kGet,
                     .pattern = "/accounts/{id}",
                     .handler_name = "by_id"},
                    {.method = Method::kGet,
                     .pattern = "/accounts/new",
                     .handler_name = "new_form"}},
         .query_method = Method::kGet,
         .query_path = "/accounts/new",
         .expected_handler = "new_form",
         .expected_params = {}},
        {.name = "CaptureStillMatchesNonLiteralPath",
         .routes = {{.method = Method::kGet,
                     .pattern = "/accounts/{id}",
                     .handler_name = "by_id"},
                    {.method = Method::kGet,
                     .pattern = "/accounts/new",
                     .handler_name = "new_form"}},
         .query_method = Method::kGet,
         .query_path = "/accounts/42",
         .expected_handler = "by_id",
         .expected_params = {{"id", "42"}}},
        {.name = "MethodSelectsGet",
         .routes = {{.method = Method::kGet,
                     .pattern = "/accounts",
                     .handler_name = "list"},
                    {.method = Method::kPost,
                     .pattern = "/accounts",
                     .handler_name = "create"}},
         .query_method = Method::kGet,
         .query_path = "/accounts",
         .expected_handler = "list",
         .expected_params = {}},
        {.name = "MethodSelectsPost",
         .routes = {{.method = Method::kGet,
                     .pattern = "/accounts",
                     .handler_name = "list"},
                    {.method = Method::kPost,
                     .pattern = "/accounts",
                     .handler_name = "create"}},
         .query_method = Method::kPost,
         .query_path = "/accounts",
         .expected_handler = "create",
         .expected_params = {}},
        {.name = "NestedCapturesBind",
         .routes = {{.method = Method::kGet,
                     .pattern = "/users/{user_id}/posts/{post_id}",
                     .handler_name = "user_post"}},
         .query_method = Method::kGet,
         .query_path = "/users/42/posts/7",
         .expected_handler = "user_post",
         .expected_params = {{"user_id", "42"}, {"post_id", "7"}}},
    }),
    [](const TestParamInfo<MatchTestCase>& info) {
      return std::string(info.param.name);
    });

struct NoMatchTestCase {
  std::string_view name;
  std::vector<RouteSpec> routes;
  Method query_method;
  std::string_view query_path;
};

class NoMatchTest : public TestWithParam<NoMatchTestCase> {};

TEST_P(NoMatchTest, NoMatch) {
  const NoMatchTestCase& params = GetParam();

  Router router;
  for (const RouteSpec& spec : params.routes) {
    ASSERT_TRUE(router
                    .add(spec.method, spec.pattern,
                         MakeNamedHandler(std::string(spec.handler_name)))
                    .ok());
  }

  EXPECT_FALSE(
      router.match(params.query_method, params.query_path).has_value());
}

INSTANTIATE_TEST_SUITE_P(RouterTest, NoMatchTest,
                         ValuesIn<NoMatchTestCase>({
                             {.name = "UnknownPath",
                              .routes = {{.method = Method::kGet,
                                          .pattern = "/accounts/{id}",
                                          .handler_name = "get"}},
                              .query_method = Method::kGet,
                              .query_path = "/unknown"},
                             {.name = "TooFewSegments",
                              .routes = {{.method = Method::kGet,
                                          .pattern = "/accounts/{id}",
                                          .handler_name = "get"}},
                              .query_method = Method::kGet,
                              .query_path = "/accounts"},
                             {.name = "TooManySegments",
                              .routes = {{.method = Method::kGet,
                                          .pattern = "/accounts/{id}",
                                          .handler_name = "get"}},
                              .query_method = Method::kGet,
                              .query_path = "/accounts/42/extra"},
                             {.name = "UnregisteredMethod",
                              .routes = {{.method = Method::kGet,
                                          .pattern = "/accounts",
                                          .handler_name = "list"}},
                              .query_method = Method::kPost,
                              .query_path = "/accounts"},
                             {.name = "EmptyRouter",
                              .routes = {},
                              .query_method = Method::kGet,
                              .query_path = "/anything"},
                         }),
                         [](const TestParamInfo<NoMatchTestCase>& info) {
                           return std::string(info.param.name);
                         });

}  // namespace

}  // namespace pulse::http
