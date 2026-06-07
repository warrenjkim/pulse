#include "pulse/json/bind.h"

#include <optional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/core/result_or_die.h"
#include "pulse/json/parse.h"

namespace pulse::json {

namespace {

using ::testing::Eq;
using ::testing::Optional;
using ::testing::StrEq;

struct Simple {
  std::string name;
  double amount;

  static auto schema() {
    return Schema<Simple>{}
        .Field("name", &Simple::name)
        .Field("amount", &Simple::amount);
  }
};

struct WithOptional {
  std::string name;
  std::optional<std::string> description;

  static auto schema() {
    return Schema<WithOptional>{}
        .Field("name", &WithOptional::name)
        .Field("description", &WithOptional::description);
  }
};

TEST(BindTest, BindsRequiredFields) {
  Result<Simple> result = Bind<Simple>(
      pulse::unwrap_or_die(parse(R"({"name": "checking", "amount": 100.0})")));
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(result->name, StrEq("checking"));
  EXPECT_THAT(result->amount, Eq(100.0));
}

TEST(BindTest, MissingRequiredField) {
  Result<Simple> result =
      Bind<Simple>(pulse::unwrap_or_die(parse(R"({"name": "checking"})")));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(BindTest, WrongFieldType) {
  Result<Simple> result = Bind<Simple>(pulse::unwrap_or_die(
      parse(R"({"name": "checking", "amount": "not_a_number"})")));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(BindTest, OptionalFieldPresent) {
  Result<WithOptional> result = Bind<WithOptional>(pulse::unwrap_or_die(
      parse(R"({"name": "checking", "description": "paycheck"})")));
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(result->description, Optional(StrEq("paycheck")));
}

TEST(BindTest, OptionalFieldAbsent) {
  Result<WithOptional> result = Bind<WithOptional>(
      pulse::unwrap_or_die(parse(R"({"name": "checking"})")));
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(result->description, Eq(std::nullopt));
}

TEST(BindTest, OptionalFieldWrongType) {
  Result<WithOptional> result = Bind<WithOptional>(pulse::unwrap_or_die(
      parse(R"({"name": "checking", "description": 123})")));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(BindTest, NotAnObject) {
  Result<Simple> result =
      Bind<Simple>(pulse::unwrap_or_die(parse(R"([1, 2, 3])")));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(BindTest, ExtraFieldsIgnored) {
  Result<Simple> result = Bind<Simple>(pulse::unwrap_or_die(
      parse(R"({"name": "checking", "amount": 100.0, "extra": "ignored"})")));
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(result->name, StrEq("checking"));
  EXPECT_THAT(result->amount, Eq(100.0));
}

TEST(BindTest, IntegerAmountInsteadOfDouble) {
  Result<Simple> result = Bind<Simple>(
      pulse::unwrap_or_die(parse(R"({"name": "checking", "amount": 100})")));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(BindTest, EmptyObject) {
  Result<Simple> result = Bind<Simple>(pulse::unwrap_or_die(parse(R"({})")));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(BindTest, NullRequiredField) {
  Result<Simple> result = Bind<Simple>(
      pulse::unwrap_or_die(parse(R"({"name": null, "amount": 100.0})")));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(BindTest, NullOptionalField) {
  Result<WithOptional> result = Bind<WithOptional>(pulse::unwrap_or_die(
      parse(R"({"name": "checking", "description": null})")));
  ASSERT_FALSE(result.ok());
  EXPECT_THAT(result.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(BindTest, EmptyStringField) {
  Result<Simple> result = Bind<Simple>(
      pulse::unwrap_or_die(parse(R"({"name": "", "amount": 100.0})")));
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(result->name, StrEq(""));
}

}  // namespace

}  // namespace pulse::json
