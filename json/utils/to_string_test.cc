#include "pulse/json/utils/to_string.h"

#include <sstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/json/utils/parse.h"

namespace pulse {
namespace json {

namespace {

using ::testing::Eq;

TEST(UtilsTest, PrettyPrint) {
  EXPECT_THAT(to_string(
                  R"json({
    "meta": null,
    "active": true,
    "count": 42,
    "ratio": 0.125,
    "message": "All systems nominal",
    "data": {
      "values": [1, 2.5, null, false, "ok"],
      "details": {
        "thresholds": { "min": -1, "max": 10 },
        "empty_array": [],
        "empty_object": {}
      }
    },
    "logs": [
      { "level": "info", "text": "startup complete" },
      { "level": "warn", "text": "low memory" }
    ]
  })json"_json),
              Eq(std::string(R"({
  "active": true,
  "count": 42,
  "data": {
    "details": {
      "empty_array": [],
      "empty_object": {},
      "thresholds": {
        "max": 10,
        "min": -1
      }
    },
    "values": [
      1,
      2.5,
      null,
      false,
      "ok"
    ]
  },
  "logs": [
    {
      "level": "info",
      "text": "startup complete"
    },
    {
      "level": "warn",
      "text": "low memory"
    }
  ],
  "message": "All systems nominal",
  "meta": null,
  "ratio": 0.125
})")));
}

TEST(UtilsTest, PrettyPrintFourSpaceTabs) {
  EXPECT_THAT(to_string(
                  R"json({
    "meta": null,
    "active": true,
    "count": 42,
    "ratio": 0.125,
    "message": "All systems nominal",
    "data": {
      "values": [1, 2.5, null, false, "ok"],
      "details": {
        "thresholds": { "min": -1, "max": 10 },
        "empty_array": [],
        "empty_object": {}
      }
    },
    "logs": [
      { "level": "info", "text": "startup complete" },
      { "level": "warn", "text": "low memory" }
    ]
  })json"_json,
                  PrintOptions{.tab_width = 4}),
              Eq(std::string(R"({
    "active": true,
    "count": 42,
    "data": {
        "details": {
            "empty_array": [],
            "empty_object": {},
            "thresholds": {
                "max": 10,
                "min": -1
            }
        },
        "values": [
            1,
            2.5,
            null,
            false,
            "ok"
        ]
    },
    "logs": [
        {
            "level": "info",
            "text": "startup complete"
        },
        {
            "level": "warn",
            "text": "low memory"
        }
    ],
    "message": "All systems nominal",
    "meta": null,
    "ratio": 0.125
})")));
}

TEST(UtilsTest, PrettyPrintTrailingCommas) {
  EXPECT_THAT(to_string("{ \"meta\": null, \"active\": true }"_json,
                        PrintOptions{.trailing_commas = true}),
              Eq(std::string(R"({
  "active": true,
  "meta": null,
})")));
}

TEST(UtilsTest, PrettyPrintCompact) {
  EXPECT_THAT(
      to_string(
          R"json({
    "meta": null,
    "active": true,
    "count": 42,
    "ratio": 0.125,
    "message": "All systems nominal",
    "data": {
      "values": [1, 2.5, null, false, "ok"],
      "details": {
        "thresholds": { "min": -1, "max": 10 },
        "empty_array": [],
        "empty_object": {}
      }
    },
    "logs": [
      { "level": "info", "text": "startup complete" },
      { "level": "warn", "text": "low memory" }
    ]
  })json"_json,
          PrintOptions{.compact = true}),
      Eq(std::string("{\"active\":true,\"count\":42,\"data\":{\"details\":{"
                     "\"empty_array\":[],\"empty_object\":{},\"thresholds\":{"
                     "\"max\":10,\"min\":-1}},\"values\":[1,2.5,null,false,"
                     "\"ok\"]},\"logs\":[{\"level\":\"info\",\"text\":"
                     "\"startup complete\"},{\"level\":\"warn\",\"text\":\"low "
                     "memory\"}],\"message\":\"All systems "
                     "nominal\",\"meta\":null,\"ratio\":0.125}")));
}

TEST(UtilsTest, PrettyPrintOstream) {
  std::ostringstream oss;
  oss <<
      R"json({
    "meta": null,
    "active": true,
    "count": 42,
    "ratio": 0.125,
    "message": "All systems nominal",
    "data": {
      "values": [1, 2.5, null, false, "ok"],
      "details": {
        "thresholds": { "min": -1, "max": 10 },
        "empty_array": [],
        "empty_object": {}
      }
    },
    "logs": [
      { "level": "info", "text": "startup complete" },
      { "level": "warn", "text": "low memory" }
    ]
  })json"_json;
  EXPECT_EQ(oss.str(), std::string(R"({
  "active": true,
  "count": 42,
  "data": {
    "details": {
      "empty_array": [],
      "empty_object": {},
      "thresholds": {
        "max": 10,
        "min": -1
      }
    },
    "values": [
      1,
      2.5,
      null,
      false,
      "ok"
    ]
  },
  "logs": [
    {
      "level": "info",
      "text": "startup complete"
    },
    {
      "level": "warn",
      "text": "low memory"
    }
  ],
  "message": "All systems nominal",
  "meta": null,
  "ratio": 0.125
})"));
}

}  // namespace

}  // namespace json
}  // namespace pulse
