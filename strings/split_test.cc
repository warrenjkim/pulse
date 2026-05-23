
#include "strings/split.h"

#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse::strings {

namespace {

using ::testing::Eq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::Values;

struct SplitTestCase {
  std::string name;
  std::string_view input;
  std::string_view delimiter;
  std::vector<std::string_view> expected;
};

class SplitTest : public TestWithParam<SplitTestCase> {};

TEST_P(SplitTest, Split) {
  EXPECT_THAT(strings::split(GetParam().input, GetParam().delimiter),
              Eq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    Strings, SplitTest,
    Values(SplitTestCase{.name = "EmptyDelimiter",
                         .input = "a b c",
                         .delimiter = "",
                         .expected = {"a b c"}},

           SplitTestCase{.name = "DoubleSpace",
                         .input = "a  b c",
                         .delimiter = " ",
                         .expected = {"a", "", "b", "c"}},
           SplitTestCase{.name = "LeadingSpace",
                         .input = " a  b c",
                         .delimiter = " ",
                         .expected = {"", "a", "", "b", "c"}},
           SplitTestCase{.name = "TrailingSpace",
                         .input = "a  b c ",
                         .delimiter = " ",
                         .expected = {"a", "", "b", "c", ""}},
           SplitTestCase{.name = "SequenceDelimiter",
                         .input = "a  b c ",
                         .delimiter = "  ",
                         .expected = {"a", "b c "}},
           SplitTestCase{.name = "TrailingSequenceDelimiter",
                         .input = "a  b c  ",
                         .delimiter = "  ",
                         .expected = {"a", "b c", ""}},
           SplitTestCase{.name = "SpecialChars",
                         .input = "a\r\nb\r\n\"c\"",
                         .delimiter = "\r\n",
                         .expected = {"a", "b", "\"c\""}}),
    [](const TestParamInfo<SplitTestCase>& info) { return info.param.name; });

}  // namespace

}  // namespace pulse::strings
