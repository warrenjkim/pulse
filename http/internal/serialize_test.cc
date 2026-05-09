#include "http/internal/serialize.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "http/response.h"

namespace pulse::http {

namespace {

using ::testing::Eq;
using ::testing::TestParamInfo;
using ::testing::TestWithParam;
using ::testing::Values;

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
