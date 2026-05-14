#include "http/internal/transform.h"

#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "http/request.h"
#include "http/response.h"

namespace pulse::http {

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
  Result<Request> request = parse_header(GetParam().raw);
  EXPECT_FALSE(request.ok());
  EXPECT_THAT(request.error().message, HasSubstr(GetParam().expected_message));
}

INSTANTIATE_TEST_SUITE_P(
    ParseTest, InvalidRequestTest,
    Values(
        InvalidRequestTestCase{.name = "UnknownMethod",
                               .raw = "PATCH /entries HTTP/1.1\r\n"
                                      "Host: 100.x.x.x:8080",
                               .expected_message = "invalid method: PATCH"},
        InvalidRequestTestCase{.name = "TrailingAmpersand",
                               .raw = "GET /entries?month=01& HTTP/1.1\r\n"
                                      "Host: 100.x.x.x:8080",
                               .expected_message = "malformed query parameter"},
        InvalidRequestTestCase{.name = "InvalidPath",
                               .raw = "GET entries HTTP/1.1\r\n"
                                      "Host: 100.x.x.x:8080",
                               .expected_message = "invalid path"},
        InvalidRequestTestCase{.name = "EmptyHeaderValue",
                               .raw = "GET / HTTP/1.1\r\n"
                                      "Host: ",
                               .expected_message = "malformed header field"},
        InvalidRequestTestCase{.name = "MalformedHeader",
                               .raw = "GET / HTTP/1.1\r\n"
                                      "BadHeader",
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
  Result<Request> request = parse_header(GetParam().raw);
  EXPECT_TRUE(request.ok()) << request.error().message;
  EXPECT_THAT(*request, Eq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    ParseTest, ValidRequestTest,
    Values(
        ValidRequestTestCase{
            .name = "Request",
            .raw = "GET /entries?month=01 HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080",
            .expected = Request{.method = Method::kGet,
                                .path = "/entries",
                                .params = {{"month", "01"}},
                                .headers = {{"Host", "100.x.x.x:8080"}}}},
        ValidRequestTestCase{
            .name = "GetWithParams",
            .raw = "GET /entries?month=2026-04&limit=10 HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080\r\n"
                   "Accept: text/html",
            .expected = Request{.method = Method::kGet,
                                .path = "/entries",
                                .params = {{"month", "2026-04"},
                                           {"limit", "10"}},
                                .headers = {{"Host", "100.x.x.x:8080"},
                                            {"Accept", "text/html"}}}},
        ValidRequestTestCase{
            .name = "Post",
            .raw = "POST /entries HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080\r\n"
                   "Content-Type: application/json",
            .expected = Request{.method = Method::kPost,
                                .path = "/entries",
                                .params = {},
                                .headers = {{"Host", "100.x.x.x:8080"},
                                            {"Content-Type",
                                             "application/json"}}}},
        ValidRequestTestCase{
            .name = "GetNoParams",
            .raw = "GET / HTTP/1.1\r\n"
                   "Host: 100.x.x.x:8080",
            .expected = Request{.method = Method::kGet,
                                .path = "/",
                                .params = {},
                                .headers = {{"Host", "100.x.x.x:8080"}}}}),
    [](const TestParamInfo<ValidRequestTestCase>& info) {
      return info.param.name;
    });

struct SerializeTestCase {
  std::string name;
  Response response;
  std::string expected;
};

class SerializeTest : public TestWithParam<SerializeTestCase> {};

TEST_P(SerializeTest, Serialize) {
  EXPECT_THAT(serialize(GetParam().response), Eq(GetParam().expected));
}

INSTANTIATE_TEST_SUITE_P(
    SerializeTest, SerializeTest,
    Values(SerializeTestCase{.name = "Ok",
                             .response = Response{.content_type = "text/html",
                                                  .status = 200,
                                                  .body = "<html></html>"},
                             .expected = "HTTP/1.1 200 OK\r\n"
                                         "Content-Type: text/html\r\n"
                                         "Content-Length: 13\r\n"
                                         "\r\n"
                                         "<html></html>"},
           SerializeTestCase{.name = "NotFound",
                             .response = Response{.content_type = "text/html",
                                                  .status = 404,
                                                  .body = ""},
                             .expected = "HTTP/1.1 404 Not Found\r\n"
                                         "Content-Type: text/html\r\n"
                                         "Content-Length: 0\r\n"
                                         "\r\n"},
           SerializeTestCase{
               .name = "JsonBody",
               .response = Response{.content_type = "application/json",
                                    .status = 200,
                                    .body = "{\"cc_bill\": 6600}"},
               .expected = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: 17\r\n"
                           "\r\n"
                           "{\"cc_bill\": 6600}"},
           SerializeTestCase{
               .name = "InternalServerError",
               .response = Response{.content_type = "text/html",
                                    .status = 500,
                                    .body = "something went wrong"},
               .expected = "HTTP/1.1 500 Internal Server Error\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: 20\r\n"
                           "\r\n"
                           "something went wrong"}),
    [](const TestParamInfo<SerializeTestCase>& info) {
      return info.param.name;
    });

}  // namespace

}  // namespace pulse::http
