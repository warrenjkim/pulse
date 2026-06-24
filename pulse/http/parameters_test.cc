#include "pulse/http/parameters.h"

#include <cstdint>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/error.h"
#include "pulse/core/result.h"

namespace pulse::http {

namespace {

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;

TEST(ParametersTest, GetBoolTrue) {
  Parameters params{{"key", "true"}};
  pulse::Result<bool> result = params.Get<bool>("key");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, IsTrue());
}

TEST(ParametersTest, GetBoolFalse) {
  Parameters params{{"key", "false"}};
  pulse::Result<bool> result = params.Get<bool>("key");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, IsFalse());
}

TEST(ParametersTest, GetInt) {
  Parameters params{{"key", "42"}};
  pulse::Result<int> result = params.Get<int>("key");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, Eq(42));
}

TEST(ParametersTest, GetInt64) {
  Parameters params{{"key", "1234567890123"}};
  pulse::Result<int64_t> result = params.Get<int64_t>("key");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, Eq(1234567890123));
}

TEST(ParametersTest, GetDouble) {
  Parameters params{{"key", "3.14"}};
  pulse::Result<double> result = params.Get<double>("key");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, Eq(3.14));
}

TEST(ParametersTest, GetString) {
  Parameters params{{"key", "value"}};
  pulse::Result<std::string> result = params.Get<std::string>("key");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, Eq("value"));
}

TEST(ParametersTest, MissingKey) {
  Parameters params{{"key", "value"}};
  pulse::Result<std::string> result = params.Get<std::string>("nonexistent");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kNotFound));
}

TEST(ParametersTest, InvalidInt) {
  Parameters params{{"key", "invalid"}};
  pulse::Result<int> result = params.Get<int>("key");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kInvalidArgument));
}

TEST(ParametersTest, InvalidDouble) {
  Parameters params{{"key", "invalid"}};
  pulse::Result<double> result = params.Get<double>("key");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kInvalidArgument));
}

TEST(ParametersTest, InvalidBool) {
  Parameters params{{"key", "invalid"}};
  pulse::Result<bool> result = params.Get<bool>("key");
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(pulse::Error::Code::kInvalidArgument));
}

TEST(ParametersTest, SetAndGet) {
  Parameters params;
  params["key"] = "value";
  pulse::Result<std::string> result = params.Get<std::string>("key");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, Eq("value"));
}

TEST(ParametersTest, OverwriteExistingKey) {
  Parameters params{{"key", "original"}};
  params["key"] = "updated";
  pulse::Result<std::string> result = params.Get<std::string>("key");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, Eq("updated"));
}

}  // namespace

}  // namespace pulse::http
