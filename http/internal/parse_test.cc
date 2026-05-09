#include "http/internal/parse.h"

#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "http/request.h"

namespace pulse::http {

void PrintTo(const Request& r, std::ostream* os) {
  *os << "Request{method=" << static_cast<int>(r.method) << ", path=" << r.path
      << ", body=" << r.body << "}";
}

namespace {

using ::testing::Eq;
using ::testing::HasSubstr;
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

TEST_P(SplitTest, split) {
  EXPECT_THAT(split(GetParam().input, GetParam().delimiter),
              Eq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    ParseTest, SplitTest,
    Values(SplitTestCase{.name = "DoubleSpace",
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

struct InvalidRequestTestCase {
  std::string name;
  std::string_view raw;
  std::string_view expected_message;
};

class InvalidRequestTest : public TestWithParam<InvalidRequestTestCase> {};

TEST_P(InvalidRequestTest, InvalidRequest) {
  Result<Request> request = parse(GetParam().raw);
  EXPECT_FALSE(request.ok());
  EXPECT_THAT(request.error().message, HasSubstr(GetParam().expected_message));
}

INSTANTIATE_TEST_SUITE_P(
    ParseTest, InvalidRequestTest,
    Values(
        InvalidRequestTestCase{
            .name = "EmptyInput",
            .raw = "",
            .expected_message = "missing header/body separator"},
        InvalidRequestTestCase{.name = "UnknownMethod",
                               .raw = "PATCH /entries HTTP/1.1\r\n"
                                      "Host: 100.x.x.x:8080\r\n"
                                      "\r\n",
                               .expected_message = "invalid method: PATCH"},
        InvalidRequestTestCase{.name = "TrailingAmpersand",
                               .raw = "GET /entries?month=01& HTTP/1.1\r\n"
                                      "Host: 100.x.x.x:8080\r\n"
                                      "\r\n",
                               .expected_message = "malformed query parameter"},
        InvalidRequestTestCase{.name = "InvalidPath",
                               .raw = "GET entries HTTP/1.1\r\n"
                                      "Host: 100.x.x.x:8080\r\n"
                                      "\r\n",
                               .expected_message = "invalid path"},
        InvalidRequestTestCase{
            .name = "MissingSeparator",
            .raw = "GET / HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080\r\n",
            .expected_message = "missing header/body separator"},
        InvalidRequestTestCase{.name = "EmptyHeaderValue",
                               .raw = "GET / HTTP/1.1\r\n"
                                      "Host: \r\n"
                                      "\r\n",
                               .expected_message = "malformed header field"},
        InvalidRequestTestCase{.name = "MalformedHeader",
                               .raw = "GET / HTTP/1.1\r\n"
                                      "BadHeader\r\n"
                                      "\r\n",
                               .expected_message = "malformed header field"}),
    [](const TestParamInfo<InvalidRequestTestCase>& info) {
      return info.param.name;
    });

struct ValidRequestTestCase {
  std::string name;
  std::string_view raw;
  Request expected;
};

class ValidRequestTest : public TestWithParam<ValidRequestTestCase> {};

TEST_P(ValidRequestTest, ValidRequest) {
  Result<Request> request = parse(GetParam().raw);
  EXPECT_TRUE(request.ok()) << request.error().message;
  EXPECT_THAT(*request, Eq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    ParseTest, ValidRequestTest,
    Values(
        ValidRequestTestCase{
            .name = "Request",
            .raw = "GET /entries?month=01 HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080\r\n"
                   "\r\n"
                   "{\"cc_bill\": 1}",
            .expected = Request{.method = Method::kGet,
                                .path = "/entries",
                                .params = {{"month", "01"}},
                                .headers = {{"Host", "100.x.x.x:8080"}},
                                .body = "{\"cc_bill\": 1}"}},
        ValidRequestTestCase{
            .name = "GetWithParams",
            .raw = "GET /entries?month=2026-04&limit=10 HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080\r\n"
                   "Accept: text/html\r\n"
                   "\r\n",
            .expected = Request{.method = Method::kGet,
                                .path = "/entries",
                                .params = {{"month", "2026-04"},
                                           {"limit", "10"}},
                                .headers = {{"Host", "100.x.x.x:8080"},
                                            {"Accept", "text/html"}},
                                .body = ""}},
        ValidRequestTestCase{
            .name = "PostWithBody",
            .raw = "POST /entries HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080\r\n"
                   "Content-Type: application/json\r\n"
                   "\r\n"
                   "{\"cc_bill\": 6600, \"savings\": 80000}",
            .expected =
                Request{.method = Method::kPost,
                        .path = "/entries",
                        .params = {},
                        .headers = {{"Host", "100.x.x.x:8080"},
                                    {"Content-Type", "application/json"}},
                        .body = "{\"cc_bill\": 6600, \"savings\": 80000}"}},
        ValidRequestTestCase{
            .name = "GetNoParams",
            .raw = "GET / HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080\r\n"
                   "\r\n",
            .expected = Request{.method = Method::kGet,
                                .path = "/",
                                .params = {},
                                .headers = {{"Host", "100.x.x.x:8080"}},
                                .body = ""}},
        ValidRequestTestCase{
            .name = "BodyWithCRLF",
            .raw = "POST /entries HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080\r\n"
                   "\r\n"
                   "{\"note\": \"line1\\r\\nline2\"}",
            .expected = Request{.method = Method::kPost,
                                .path = "/entries",
                                .params = {},
                                .headers = {{"Host", "100.x.x.x:8080"}},
                                .body = "{\"note\": \"line1\\r\\nline2\"}"}}),
    [](const TestParamInfo<ValidRequestTestCase>& info) {
      return info.param.name;
    });

}  // namespace

}  // namespace pulse::http
