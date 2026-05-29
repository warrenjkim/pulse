#include "core/result_or_die.h"

#include <string>
#include <utility>

#include "core/error.h"
#include "core/result.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse {

namespace {

using ::testing::Eq;

TEST(UnwrapOrDieTest, ReturnsValue) {
  Result<int> ok = 42;
  EXPECT_THAT(unwrap_or_die(ok), Eq(42));
}

TEST(UnwrapOrDieTest, ReturnsMovableValue) {
  Result<std::string> ok = std::string("hello");
  std::string out = unwrap_or_die(std::move(ok));
  EXPECT_THAT(out, Eq("hello"));
}

TEST(UnwrapOrDieDeathTest, AbortsOnError) {
  Result<int> err =
      pulse::Error{.code = pulse::Error::Code::kInternal, .message = "boom"};
  EXPECT_DEATH((void)unwrap_or_die(std::move(err)), "unwrap_or_die: boom");
}

TEST(DieIfErrorTest, NoOpOnSuccess) {
  die_if_error(Result<void>{});
  SUCCEED() << "die_if_error returned normally on an ok Result";
}

TEST(DieIfErrorDeathTest, AbortsOnError) {
  Result<void> err =
      pulse::Error{.code = pulse::Error::Code::kInternal, .message = "boom"};
  EXPECT_DEATH(die_if_error(std::move(err)), "die_if_error: boom");
}

}  // namespace

}  // namespace pulse
