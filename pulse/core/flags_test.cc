#include "pulse/core/flags.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/error.h"
#include "pulse/core/result.h"

namespace pulse {

namespace {

using ::testing::Eq;
using ::testing::Optional;

struct Custom {
  std::string host;
  int port;

  [[maybe_unused]] friend bool operator==(const Custom&,
                                          const Custom&) = default;
};

}  // namespace

template <>
struct FlagParser<Custom> {
  static Result<Custom> Parse(std::string_view raw) {
    size_t colon = raw.find(':');
    if (colon == std::string_view::npos) {
      return Error{
          .code = Error::Code::kInvalidArgument,
          .message = "expected host:port",
      };
    }

    Result<int> port = FlagParser<int>::Parse(raw.substr(colon + 1));
    if (!port.ok()) {
      return port.error();
    }

    return Custom{
        .host = std::string(raw.substr(0, colon)),
        .port = *port,
    };
  }
};

namespace {

TEST(FlagsTest, FindEqualsSyntax) {
  const char* argv[] = {"prog", "--db=test.db", "--port=9090"};
  Flags flags(3, argv);
  EXPECT_THAT(flags.Find("--db"), Optional(Eq("test.db")));
  EXPECT_THAT(flags.Find("--port"), Optional(Eq("9090")));
  EXPECT_THAT(flags.Find("--missing"), Eq(std::nullopt));
}

TEST(FlagsTest, FindSpaceSyntax) {
  const char* argv[] = {"prog", "--db", "test.db", "--port", "9090"};
  Flags flags(5, argv);
  EXPECT_THAT(flags.Find("--db"), Optional(Eq("test.db")));
  EXPECT_THAT(flags.Find("--port"), Optional(Eq("9090")));
}

TEST(FlagsTest, TrailingBareFlagHasNoValue) {
  const char* argv[] = {"prog", "--db"};
  Flags flags(2, argv);
  EXPECT_THAT(flags.Find("--db"), Eq(std::nullopt));
}

TEST(FlagsTest, EmptyValueIsFound) {
  const char* argv[] = {"prog", "--db="};
  Flags flags(2, argv);
  EXPECT_THAT(flags.Find("--db"), Optional(Eq("")));
  EXPECT_THAT(flags.Get("--db", "default.db"), Eq(""));
}

TEST(FlagsTest, PrefixCollisionDoesNotMatch) {
  const char* argv[] = {"prog", "--db_path=/tmp/a.db", "--db=real.db"};
  EXPECT_THAT(Flags(3, argv).Find("--db"), Optional(Eq("real.db")));
  EXPECT_THAT(Flags(2, argv).Find("--db"), Eq(std::nullopt));
}

TEST(FlagsTest, LastOccurrenceWins) {
  const char* argv[] = {"prog", "--port=8080", "--port=9090"};
  Flags flags(3, argv);
  EXPECT_THAT(flags.Find("--port"), Optional(Eq("9090")));
}

TEST(FlagsTest, StopsAtDoubleDash) {
  const char* argv[] = {"prog", "--db=before.db", "--", "--db=after.db",
                        "--extra=1"};
  Flags flags(5, argv);
  EXPECT_THAT(flags.Find("--db"), Optional(Eq("before.db")));
  EXPECT_THAT(flags.Find("--extra"), Eq(std::nullopt));
  EXPECT_FALSE(flags.Has("--extra"));
}

TEST(FlagsTest, SkipsArgv0) {
  const char* argv[] = {"--db=prog.db"};
  Flags flags(1, argv);
  EXPECT_THAT(flags.Find("--db"), Eq(std::nullopt));
  EXPECT_FALSE(flags.Has("--db"));
}

TEST(FlagsTest, EmptyNameAndNullArgv) {
  const char* argv[] = {"prog", "--db=test.db"};
  EXPECT_THAT(Flags(2, argv).Find(""), Eq(std::nullopt));
  EXPECT_THAT(Flags(2, nullptr).Find("--db"), Eq(std::nullopt));
  EXPECT_FALSE(Flags(2, argv).Has(""));
  EXPECT_FALSE(Flags(2, nullptr).Has("--db"));
}

TEST(FlagsTest, GetUsesDefaultWhenAbsent) {
  const char* argv[] = {"prog", "--db=custom.db"};
  Flags flags(2, argv);
  EXPECT_THAT(flags.Get("--db", "vigil.db"), Eq("custom.db"));
  EXPECT_THAT(flags.Get("--port", "8080"), Eq("8080"));
  EXPECT_THAT(flags.Get("--conduit_path"), Eq(""));
}

TEST(FlagsTest, HasDetectsBareAndValuedFlags) {
  const char* argv[] = {"prog", "--verbose", "--port=8080", "--verbose_extra"};
  Flags flags(4, argv);
  EXPECT_TRUE(flags.Has("--verbose"));
  EXPECT_TRUE(flags.Has("--port"));
  EXPECT_FALSE(flags.Has("--debug"));
  EXPECT_FALSE(flags.Has("--verb"));
}

TEST(FlagsTest, TypedGetInt) {
  const char* argv[] = {"prog", "--port=9090", "--bad=9090abc", "--neg=-42"};
  Flags flags(4, argv);

  Result<int> port = flags.Get<int>("--port", 8080);
  ASSERT_TRUE(port.ok());
  EXPECT_THAT(*port, Eq(9090));

  Result<int> missing = flags.Get<int>("--threads", 40);
  ASSERT_TRUE(missing.ok());
  EXPECT_THAT(*missing, Eq(40));

  Result<int64_t> neg = flags.Get<int64_t>("--neg", 0);
  ASSERT_TRUE(neg.ok());
  EXPECT_THAT(*neg, Eq(-42));

  Result<int> bad = flags.Get<int>("--bad", 8080);
  ASSERT_FALSE(bad.ok());
  EXPECT_THAT(bad.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(FlagsTest, TypedGetDouble) {
  const char* argv[] = {"prog", "--rate=0.75", "--bad=abc"};
  Flags flags(3, argv);

  Result<double> rate = flags.Get<double>("--rate", 1.0);
  ASSERT_TRUE(rate.ok());
  EXPECT_DOUBLE_EQ(*rate, 0.75);

  Result<double> bad = flags.Get<double>("--bad", 1.0);
  ASSERT_FALSE(bad.ok());
  EXPECT_THAT(bad.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(FlagsTest, TypedGetBool) {
  const char* argv[] = {"prog",  "--a=true", "--b=false",
                        "--c=1", "--d=0",    "--e=yes"};
  Flags flags(6, argv);

  Result<bool> a = flags.Get<bool>("--a", false);
  ASSERT_TRUE(a.ok());
  EXPECT_TRUE(*a);

  Result<bool> b = flags.Get<bool>("--b", true);
  ASSERT_TRUE(b.ok());
  EXPECT_FALSE(*b);

  Result<bool> c = flags.Get<bool>("--c", false);
  ASSERT_TRUE(c.ok());
  EXPECT_TRUE(*c);

  Result<bool> d = flags.Get<bool>("--d", true);
  ASSERT_TRUE(d.ok());
  EXPECT_FALSE(*d);

  Result<bool> missing = flags.Get<bool>("--missing", true);
  ASSERT_TRUE(missing.ok());
  EXPECT_TRUE(*missing);

  Result<bool> bad = flags.Get<bool>("--e", false);
  ASSERT_FALSE(bad.ok());
  EXPECT_THAT(bad.error().code, Eq(Error::Code::kInvalidArgument));
}

TEST(FlagsTest, TypedGetString) {
  const char* argv[] = {"prog", "--db=test.db"};
  Flags flags(2, argv);

  Result<std::string> db = flags.Get<std::string>("--db", "default.db");
  ASSERT_TRUE(db.ok());
  EXPECT_THAT(*db, Eq("test.db"));
}

TEST(FlagsTest, CustomFlagParserSpecialization) {
  const char* argv[] = {"prog", "--addr=localhost:8080", "--bad=nocolon"};
  Flags flags(3, argv);

  Result<Custom> addr = flags.Get<Custom>("--addr", {"127.0.0.1", 80});
  ASSERT_TRUE(addr.ok());
  EXPECT_THAT(*addr, Eq(Custom{.host = "localhost", .port = 8080}));

  Result<Custom> bad = flags.Get<Custom>("--bad", {"127.0.0.1", 80});
  ASSERT_FALSE(bad.ok());
  EXPECT_THAT(bad.error().code, Eq(Error::Code::kInvalidArgument));
}

}  // namespace

}  // namespace pulse
