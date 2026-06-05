cc_library(
    name = "json",
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/json/parse:lexer",
        "//pulse/json/parse:parser",
        "//pulse/json/parse:reader",
        "//pulse/json/parse:token",
        "//pulse/json/utils:exception",
        "//pulse/json/utils:parse",
        "//pulse/json/utils:to_string",
        "//pulse/json/value",
    ],
)

cc_library(
    name = "value",
    hdrs = ["value.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/core:error",
        "//pulse/core:result",
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
        "//pulse/core:error",
        "//pulse/core:result",
        "//pulse/core:stringify",
        "//pulse/strings:cat",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
