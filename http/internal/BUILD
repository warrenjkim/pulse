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
        "//dsa:result",
        "//http:types",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "serialize",
    srcs = ["serialize.cc"],
    hdrs = ["serialize.h"],
    include_prefix = "pulse/http/internal",
    strip_include_prefix = ".",
    visibility = ["//visibility:__pkg__"],
    deps = [
        "//http:types",
    ],
)

cc_test(
    name = "serialize_test",
    srcs = ["serialize_test.cc"],
    deps = [
        ":serialize",
        "//http:types",
        "@googletest//:gtest_main",
    ],
)
