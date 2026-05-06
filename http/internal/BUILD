test_suite(
    name = "tests",
    tests = [
        ":parse_test",
    ],
)

cc_library(
    name = "parse",
    srcs = ["parse.cc"],
    hdrs = ["parse.h"],
    include_prefix = "pulse/http/internal",
    strip_include_prefix = ".",
    visibility = ["//visibility:__pkg__"],
    deps = [
        "//dsa:error",
        "//dsa:result",
        "//http:types",
    ],
)

cc_test(
    name = "parse_test",
    srcs = ["parse_test.cc"],
    deps = [
        ":parse",
        "@googletest//:gtest_main",
    ],
)
