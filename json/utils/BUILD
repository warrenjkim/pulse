test_suite(
    name = "tests",
    tests = [
        ":parse_test",
        ":to_string_test",
    ],
)

cc_library(
    name = "exception",
    hdrs = [
        "exception.h",
    ],
    include_prefix = "pulse/json/utils",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "parse",
    hdrs = [
        "parse.h",
    ],
    include_prefix = "pulse/json/utils",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        "//json/parse:lexer",
        "//json/parse:parser",
        "//json/value",
    ],
)

cc_test(
    name = "parse_test",
    srcs = ["parse_test.cc"],
    deps = [
        "//json/utils:parse",
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
    include_prefix = "pulse/json/utils",
    strip_include_prefix = ".",
    visibility = ["//visibility:public"],
    deps = [
        "//json/parse:lexer",
        "//json/value",
    ],
)

cc_test(
    name = "to_string_test",
    srcs = ["to_string_test.cc"],
    deps = [
        "//json/utils:parse",
        "//json/utils:to_string",
        "@googletest//:gtest_main",
    ],
)
