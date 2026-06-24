
#include "pulse/strings/split.h"

#include <string>
#include <string_view>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse::strings {

namespace {

using ::testing::Eq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::ValuesIn;

struct SplitTestCase {
  std::string name;
  std::string_view input;
  std::string_view delimiter;
  std::vector<std::string_view> expected;
};

class SplitTest : public TestWithParam<SplitTestCase> {};

TEST_P(SplitTest, Split) {
  EXPECT_THAT(strings::Split(GetParam().input, GetParam().delimiter),
              Eq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    Strings, SplitTest,
    ValuesIn<SplitTestCase>({{.name = "EmptyDelimiter",
                              .input = "a b c",
                              .delimiter = "",
                              .expected = {"a b c"}},
                             {.name = "DoubleSpace",
                              .input = "a  b c",
                              .delimiter = " ",
                              .expected = {"a", "", "b", "c"}},
                             {.name = "LeadingSpace",
                              .input = " a  b c",
                              .delimiter = " ",
                              .expected = {"", "a", "", "b", "c"}},
                             {.name = "TrailingSpace",
                              .input = "a  b c ",
                              .delimiter = " ",
                              .expected = {"a", "", "b", "c", ""}},
                             {.name = "SequenceDelimiter",
                              .input = "a  b c ",
                              .delimiter = "  ",
                              .expected = {"a", "b c "}},
                             {.name = "TrailingSequenceDelimiter",
                              .input = "a  b c  ",
                              .delimiter = "  ",
                              .expected = {"a", "b c", ""}},
                             {.name = "SpecialChars",
                              .input = "a\r\nb\r\n\"c\"",
                              .delimiter = "\r\n",
                              .expected = {"a", "b", "\"c\""}}}),
    [](const TestParamInfo<SplitTestCase>& info) { return info.param.name; });

}  // namespace

}  // namespace pulse::strings
