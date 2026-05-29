cc_library(
    name = "value",
    hdrs = [
        "value.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//pulse/json/utils:exception",
    ],
)

cc_test(
    name = "value_test",
    srcs = ["value_test.cc"],
    deps = [
        ":value",
        "//pulse/json/utils:exception",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)
