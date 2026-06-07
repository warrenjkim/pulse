cc_library(
    name = "json",
    visibility = ["//visibility:public"],
    deps = [
        ":bind",
        ":parse",
        ":value",
    ],
)

cc_library(
    name = "bind",
    hdrs = ["bind.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":value",
        "@pulse//pulse/core:error",
        "@pulse//pulse/core:result",
        "@pulse//pulse/strings:cat",
    ],
)

cc_test(
    name = "bind_test",
    srcs = ["bind_test.cc"],
    deps = [
        ":bind",
        ":parse",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
        "@pulse//pulse/core:error",
        "@pulse//pulse/core:result",
        "@pulse//pulse/core:result_or_die",
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
