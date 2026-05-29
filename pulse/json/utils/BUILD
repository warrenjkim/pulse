cc_library(
    name = "exception",
    hdrs = [
        "exception.h",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "parse",
    hdrs = [
        "parse.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/json/parse:lexer",
        "//pulse/json/parse:parser",
        "//pulse/json/value",
    ],
)

cc_test(
    name = "parse_test",
    srcs = ["parse_test.cc"],
    deps = [
        ":parse",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "to_string",
    srcs = [
        "to_string.cc",
    ],
    hdrs = [
        "to_string.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/json/parse:lexer",
        "//pulse/json/parse:token",
        "//pulse/json/value",
    ],
)

cc_test(
    name = "to_string_test",
    srcs = ["to_string_test.cc"],
    deps = [
        ":parse",
        ":to_string",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
