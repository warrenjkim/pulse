#include "pulse/http/internal/cors.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/http/request.h"
#include "pulse/http/response.h"

namespace pulse::http {

namespace {

using ::testing::Eq;

TEST(CorsPreflightTest, MissingRequestHeaders) {
  Response response = CorsPreflight(Request{});
  EXPECT_THAT(response.status, Eq(400));
}

TEST(CorsPreflightTest, EchosRequestHeaders) {
  Request request{
      .headers = {{"Access-Control-Request-Headers", "Content-Type"}}};
  Response response = CorsPreflight(request);
  EXPECT_THAT(response.status, Eq(200));
  EXPECT_THAT(response.headers.at("Access-Control-Allow-Headers"),
              Eq("Content-Type"));
}

}  // namespace

}  // namespace pulse::http
