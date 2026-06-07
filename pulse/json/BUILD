cc_library(
    name = "json",
    visibility = ["//visibility:public"],
    deps = [
        ":parse",
        ":value",
    ],
)

cc_library(
    name = "parse",
    srcs = ["parse.cc"],
    hdrs = ["parse.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":value",
        "//pulse/core:error",
        "//pulse/core:result",
        "//pulse/strings:cat",
    ],
)

cc_test(
    name = "parse_test",
    srcs = ["parse_test.cc"],
    deps = [
        ":parse",
        ":value",
        "//pulse/core:error",
        "//pulse/core:result",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "value",
    hdrs = ["value.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/core:stringify",
        "//pulse/strings:cat",
    ],
)

cc_test(
    name = "value_test",
    srcs = ["value_test.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":value",
        "//pulse/core:stringify",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
