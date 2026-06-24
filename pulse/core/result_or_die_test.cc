#include "pulse/core/result_or_die.h"

#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/error.h"
#include "pulse/core/result.h"

namespace pulse {

namespace {

using ::testing::Eq;

TEST(UnwrapOrDieTest, ReturnsValue) {
  Result<int> ok = 42;
  EXPECT_THAT(UnwrapOrDie(ok), Eq(42));
}

TEST(UnwrapOrDieTest, ReturnsMovableValue) {
  Result<std::string> ok = std::string("hello");
  std::string out = UnwrapOrDie(std::move(ok));
  EXPECT_THAT(out, Eq("hello"));
}

TEST(UnwrapOrDieDeathTest, AbortsOnError) {
  Result<int> err =
      pulse::Error{.code = pulse::Error::Code::kInternal, .message = "boom"};
  EXPECT_DEATH((void)UnwrapOrDie(std::move(err)), "UnwrapOrDie: boom");
}

TEST(DieIfErrorTest, NoOpOnSuccess) {
  DieIfError(Result<void>{});
  SUCCEED() << "DieIfError returned normally on an ok Result";
}

TEST(DieIfErrorDeathTest, AbortsOnError) {
  Result<void> err =
      pulse::Error{.code = pulse::Error::Code::kInternal, .message = "boom"};
  EXPECT_DEATH(DieIfError(std::move(err)), "DieIfError: boom");
}

}  // namespace

}  // namespace pulse
