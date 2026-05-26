#include "http/router.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dsa/result.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "http/handler.h"
#include "http/method.h"
#include "http/request.h"
#include "http/response.h"

namespace pulse::http {

namespace {

using ::testing::Eq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

class NamedHandler final : public Handler {
 public:
  explicit NamedHandler(std::string name) : name_(std::move(name)) {}

  Response operator()(const Request&) const override {
    return Response{.content_type = "text/plain", .status = 200, .body = name_};
  }

 private:
  std::string name_;
};

std::unique_ptr<Handler> MakeNamedHandler(std::string name) {
  return std::make_unique<NamedHandler>(std::move(name));
}

TEST(RouterTest, AddAcceptsValidRoute) {
  Router router;
  EXPECT_TRUE(
      router.add(Method::kGet, "/health", MakeNamedHandler("health")).ok());
}

TEST(RouterTest, AddRejectsMalformedPattern) {
  Router router;
  Result<void> result =
      router.add(Method::kGet, "/accounts/{id", MakeNamedHandler("bad"));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kInvalidArgument));
}

TEST(RouterTest, AddRejectsDuplicateRoute) {
  Router router;
  ASSERT_TRUE(
      router.add(Method::kGet, "/accounts", MakeNamedHandler("first")).ok());
  Result<void> result =
      router.add(Method::kGet, "/accounts", MakeNamedHandler("second"));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kAlreadyExists));
}

TEST(RouterTest, AddSamePatternDifferentMethodsAllowed) {
  Router router;
  EXPECT_TRUE(
      router.add(Method::kGet, "/accounts", MakeNamedHandler("list")).ok());
  EXPECT_TRUE(
      router.add(Method::kPost, "/accounts", MakeNamedHandler("create")).ok());
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
